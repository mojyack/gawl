#include <iterator>
#include <stdexcept>
#include <vector>

#include "error.hpp"
#include "gawl-window.hpp"
#include "global.hpp"
#include "misc.hpp"
#include "textrender.hpp"
#include "type.hpp"

namespace gawl {
namespace {
auto convert_utf8_to_unicode32(const char* const str) -> std::u32string {
    auto utf8  = std::string(str);
    auto uni32 = std::u32string();

    for(size_t i = 0; i < utf8.size(); i++) {
        auto c = (uint32_t)utf8[i];

        if((0x80 & c) == 0) {
            uni32.push_back(c);
        } else if((0xE0 & c) == 0xC0) {
            if((i + 1) < utf8.size()) {
                c = (uint32_t)(0x1F & utf8[i + 0]) << 6;
                c |= (uint32_t)(0x3F & utf8[i + 1]) << 0;
            } else {
                c = '?';
            }
            i += 1;
            uni32.push_back(c);
        } else if((0xF0 & c) == 0xE0) {
            if((i + 2) < utf8.size()) {
                c = (uint32_t)(0x0F & utf8[i + 0]) << 12;
                c |= (uint32_t)(0x3F & utf8[i + 1]) << 6;
                c |= (uint32_t)(0x3F & utf8[i + 2]) << 0;
            } else {
                c = '?';
            }
            i += 2;
            uni32.push_back(c);
        } else if((0xF8 & c) == 0xF0) {
            if((i + 3) < utf8.size()) {
                c = (uint32_t)(0x07 & utf8[i + 0]) << 18;
                c |= (uint32_t)(0x3F & utf8[i + 1]) << 12;
                c |= (uint32_t)(0x3F & utf8[i + 2]) << 6;
                c |= (uint32_t)(0x3F & utf8[i + 3]) << 0;
            } else {
                c = '?';
            }
            i += 3;
            uni32.push_back(c);
        }
    }
    return uni32;
}
using Faces = std::vector<FT_Face>;
} // namespace

extern GlobalVar* global;

class Character : public GraphicBase {
  public:
    int offset[2];
    int advance;
    Character(char32_t code, Faces const& faces);
};

Character::Character(char32_t code, Faces const& faces) : GraphicBase(*global->textrender_shader) {
    auto face        = FT_Face(nullptr);
    auto glyph_index = int(-1);
    for(auto f : faces) {
        const auto i = FT_Get_Char_Index(f, code);
        if(i != 0) {
            face        = f;
            glyph_index = i;
            break;
        }
    }
    if(glyph_index == -1) {
        // no font have the glygh. fallback to first font and remove character.
        code        = U' ';
        face        = faces[0];
        glyph_index = FT_Get_Char_Index(faces[0], code);
    }

    auto error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    if(error) {
        throw std::runtime_error("failed to load Glyph");
    };
    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

    width     = face->glyph->bitmap.width;
    height    = face->glyph->bitmap.rows;
    offset[0] = face->glyph->bitmap_left;
    offset[1] = face->glyph->bitmap_top;
    advance   = static_cast<int>(face->glyph->advance.x) >> 6;
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
}

struct CharacterCache {
    using Map = std::unordered_map<char32_t, Character*>;
    Faces faces;
    Map   cache;
    auto  get_character(const char32_t c) -> Character* {
        if(const auto f = cache.find(c); f != cache.end()) {
            return f->second;
        } else {
            const auto chara = new Character(c, faces);
            cache.insert(std::make_pair(c, chara));
            return chara;
        }
    }
    CharacterCache(const std::vector<std::string>& font_names, const int size) {
        for(const auto& path : font_names) {
            auto face = FT_Face();
            ASSERT(FT_New_Face(global->freetype, path.data(), 0, &(face)) == 0, "failed to open font")
            FT_Set_Pixel_Sizes(face, 0, size);
            faces.emplace_back(face);
        }
    }
    ~CharacterCache() {
        for(auto f : faces) {
            FT_Done_Face(f);
        }
    }
};

class TextRenderPrivate {
  private:
    std::unordered_map<int, std::shared_ptr<CharacterCache>> caches;

    const std::vector<std::string> font_names;
    const int                      size;

  public:
    auto clear() -> void {
        for(auto& s : caches) {
            for(auto& c : s.second->cache) {
                delete c.second;
            }
        }
        caches.clear();
    }
    auto get_size() const -> int {
        return size;
    }
    auto operator[](const int size) -> CharacterCache& {
        if(!caches.contains(size)) {
            caches.insert(std::make_pair(size, new CharacterCache(font_names, size)));
        }
        return *caches.find(size)->second;
    }
    TextRenderPrivate(const std::vector<std::string>&& font_names, const int size) : font_names(font_names), size(size) {}
    ~TextRenderPrivate() {
        clear();
    }
};

auto TextRender::get_chara_graphic(const int size, const char32_t c) -> Character* {
    return (*data)[size].get_character(c);
}
auto TextRender::draw(Screen* screen, const double x, const double y, Color const& color, const char* const text, const DrawFunc func) -> Area {
    const auto uni = convert_utf8_to_unicode32(text);
    return draw(screen, x, y, color, uni.data(), func);
}
auto TextRender::draw(Screen* screen, const double x, const double y, Color const& color, const char32_t* const text, const DrawFunc func) -> Area {
    ASSERT(data, "font not initialized")
    const auto prep = [&]() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(global->textrender_shader->get_shader());
        glUniform4f(glGetUniformLocation(global->textrender_shader->get_shader(), "textColor"), color[0], color[1], color[2], color[3]);
    };
    const auto scale       = screen->get_scale();
    auto       xpos        = double(x);
    auto       drawed_area = Area{x, y, x, y};
    prep();

    auto c = text;
    while(*c != '\0') {
        const auto chara = get_chara_graphic(data->get_size() * scale, *c);
        const auto p     = std::array{xpos + 1. * chara->offset[0] / scale, y - 1. * chara->offset[1] / scale};
        const auto area  = Area{p[0], p[1], p[0] + chara->get_width(screen), p[1] + chara->get_height(screen)};
        drawed_area[0]   = drawed_area[0] < area[0] ? drawed_area[0] : area[0];
        drawed_area[1]   = drawed_area[1] < area[1] ? drawed_area[1] : area[1];
        drawed_area[2]   = drawed_area[2] > area[2] ? drawed_area[2] : area[2];
        drawed_area[3]   = drawed_area[3] > area[3] ? drawed_area[3] : area[3];
        if(func) {
            const auto result = func(c - text, area, *chara);
            if(result) {
                prep();
            } else {
                chara->draw_rect(screen, area);
            }
        } else {
            chara->draw_rect(screen, area);
        }
        xpos += 1. * chara->advance / scale;
        c += 1;
    }
    return drawed_area;
}
auto TextRender::draw_fit_rect(Screen* screen, Area rect, Color const& color, const char* const text, const Align alignx, const Align aligny, const DrawFunc func) -> Area {
    const auto scale = screen->get_scale();
    rect.magnify(scale);
    auto font_area = Area{0, 0};
    get_rect(screen, font_area, text);
    font_area.magnify(scale);
    const auto sw = double(font_area[2] - font_area[0]), sh = double(font_area[3] - font_area[1]);
    const auto dw = double(rect[2] - rect[0]), dh = double(rect[3] - rect[1]);
    const auto pad = std::array{dw - sw, dh - sh};

    auto x = alignx == Align::left ? rect[0] - font_area[0] : alignx == Align::center ? rect[0] - font_area[0] + pad[0] / 2
                                                                                      : rect[2] - sw;
    auto y = aligny == Align::left ? rect[1] - font_area[1] : aligny == Align::center ? rect[1] - font_area[1] + pad[1] / 2
                                                                                      : rect[3] - sh;
    x /= scale;
    y /= scale;
    return draw(screen, x, y, color, text, func);
}
auto TextRender::get_rect(const Screen* screen, Area& rect, const char* const text) -> void {
    const auto uni = convert_utf8_to_unicode32(text);
    get_rect(screen, rect, uni.data());
}
auto TextRender::get_rect(const Screen* screen, Area& rect, const char32_t* const text) -> void {
    ASSERT(data, "font not initialized")
    rect[2]        = rect[0];
    rect[3]        = rect[1];
    auto       rx1 = 0.0, ry1 = 0.0, rx2 = 0.0, ry2 = 0.0;
    const auto scale = screen->get_scale();
    auto       c     = text;
    while(*c != '\0') {
        auto       chara = get_chara_graphic(data->get_size() * scale, *c);
        const auto xpos  = std::array{static_cast<double>(rx1 + chara->offset[0]), static_cast<double>(rx1 + chara->offset[0] + chara->get_width(screen) * scale)};

        rx1 = rx1 > xpos[0] ? xpos[0] : rx1;
        rx2 = rx2 < xpos[1] ? xpos[1] : rx2;

        const auto ypos = std::array{static_cast<double>(-chara->offset[1]), static_cast<double>(-chara->offset[1] + chara->get_height(screen) * scale)};

        ry1 = ry1 > ypos[0] ? ypos[0] : ry1;
        ry2 = ry2 < ypos[1] ? ypos[1] : ry2;

        c += 1;

        if(*c != '\0') {
            rx2 += chara->advance;
        }
    }
    rect[0] += rx1 / scale;
    rect[1] += ry1 / scale;
    rect[2] += rx2 / scale;
    rect[3] += ry2 / scale;
}
auto TextRender::draw_wrapped(Screen* screen, Area& rect, const int line_spacing, const Color& color, const char* const text, const Align alignx, const Align aligny) -> void {
    ASSERT(data, "font not initialized")

    const auto str   = convert_utf8_to_unicode32(text);
    auto       lines = std::vector<std::u32string>(1);

    const auto max_width  = rect.width();
    const auto max_height = rect.height();
    const auto len        = str.size();
    for(size_t i = 0; i < len; ++i) {
        const auto last  = i + 1 == len;
        auto       chara = str[i];
        if(str[i] == U'\\' && !last) {
            i += 1;
            const auto following = str[i];
            if(following == U'\\') {
                chara = following;
            } else {
                switch(following) {
                case U'n':
                    if((lines.size() + 1) * line_spacing > max_height) {
                        goto draw;
                    }
                    lines.emplace_back();
                    break;
                }
                continue;
            }
        }
        lines.back() += chara;
        auto area = Area{0, 0};
        get_rect(screen, area, lines.back().data());
        if(area.width() > max_width) {
            lines.back().pop_back();

            if((lines.size() + 1) * line_spacing > max_height) {
                goto draw;
            }
            lines.emplace_back(1, chara);

            get_rect(screen, area, lines.back().data());
            if(area.width() > max_width) {
                lines.pop_back();
                goto draw;
            }
        }
    }

draw:
    const auto total_height = size_t(lines.size() * line_spacing);
    const auto y_offset     = aligny == Align::left ? 0.0 : aligny == Align::right ? max_height - total_height
                                                                                   : (max_height - total_height) / 2.0;
    for(size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        auto        area = Area{0, 0};
        get_rect(screen, area, line.data());
        const auto total_width = area.width();
        const auto x_offset    = alignx == Align::left ? 0.0 : alignx == Align::right ? max_width - total_width
                                                                                      : (max_width - total_width) / 2.0;
        draw(screen, rect[0] + x_offset, rect[1] + y_offset + i * line_spacing - area[1], color, line.data());
    }
}
TextRender::operator bool() const {
    return data.get() != nullptr;
}
TextRender::TextRender(const std::vector<const char*>&& font_names, const int size) {
    std::vector<std::string> fonts;
    for(auto path : font_names) {
        fonts.emplace_back(path);
    }
    data.reset(new TextRenderPrivate(std::move(fonts), size));
}
TextRender::TextRender() {}
TextRender::~TextRender() {}
} // namespace gawl
