#include <iterator>
#include <stdexcept>
#include <vector>

#include "gawl-window.hpp"
#include "global.hpp"
#include "misc.hpp"
#include "textrender.hpp"
#include "type.hpp"

namespace gawl {
namespace {
std::u32string convert_utf8_to_unicode32(const char* str) {
    std::string    utf8 = str;
    std::u32string uni32;

    for(size_t i = 0; i < utf8.size(); i++) {
        uint32_t c = (uint32_t)utf8[i];

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
    FT_Face face        = nullptr;
    int     glyph_index = -1;
    for(auto f : faces) {
        auto i = FT_Get_Char_Index(f, code);
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
    Faces      faces;
    Map        cache;
    Character* get_character(char32_t c) {
        if(auto f = cache.find(c); f != cache.end()) {
            return f->second;
        } else {
            auto chara = new Character(c, faces);
            cache.insert(std::make_pair(c, chara));
            return chara;
        }
    }
    CharacterCache(const std::vector<std::string>& font_names, const int size) {
        for(const auto& path : font_names) {
            FT_Face face;
            if(auto err = FT_New_Face(global->freetype, path.data(), 0, &(face)); err != 0) {
                throw std::runtime_error("failed to open font.");
            }
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
    std::map<int, std::shared_ptr<CharacterCache>> caches;

    const std::vector<std::string> font_names;
    const int                      size;

  public:
    void clear() {
        for(auto& s : caches) {
            for(auto& c : s.second->cache) {
                delete c.second;
            }
        }
        caches.clear();
    }
    int get_size() const noexcept {
        return size;
    }
    CharacterCache& operator[](int size) {
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

Character* TextRender::get_chara_graphic(int size, char32_t c) {
    return (*data)[size].get_character(c);
}
void TextRender::set_char_color(Color const& color) {
    glUniform4f(glGetUniformLocation(global->textrender_shader->get_shader(), "textColor"), color[0], color[1], color[2], color[3]);
}
Area TextRender::draw(FrameBufferInfo info, double x, double y, Color const& color, const char* text, DrawFunc func) {
    auto uni = convert_utf8_to_unicode32(text);
    return draw(info, x, y, color, uni.data(), func);
}
Area TextRender::draw(FrameBufferInfo info, double x, double y, Color const& color, const char32_t* text, DrawFunc func) {
    if(!data) {
        throw ::std::runtime_error("unititialized font.");
    }
    auto prep = [&]() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(global->textrender_shader->get_shader());
        set_char_color(color);
    };
    double xpos        = x;
    Area   drawed_area = {x, y, x, y};
    prep();
    const auto scale = info.get_scale();

    const char32_t* c = text;
    while(*c != '\0') {
        auto   chara   = get_chara_graphic(data->get_size() * scale, *c);
        double p[2]    = {xpos + 1. * chara->offset[0] / scale, y - 1. * chara->offset[1] / scale};
        Area   area    = {p[0], p[1], p[0] + chara->get_width(info), p[1] + chara->get_height(info)};
        drawed_area[0] = drawed_area[0] < area[0] ? drawed_area[0] : area[0];
        drawed_area[1] = drawed_area[1] < area[1] ? drawed_area[1] : area[1];
        drawed_area[2] = drawed_area[2] > area[2] ? drawed_area[2] : area[2];
        drawed_area[3] = drawed_area[3] > area[3] ? drawed_area[3] : area[3];
        if(func) {
            auto result = func(c - text, area, *chara);
            if(result) {
                prep();
            } else {
                chara->draw_rect(info, area);
            }
        } else {
            chara->draw_rect(info, area);
        }
        xpos += 1. * chara->advance / scale;
        c += 1;
    }
    return drawed_area;

}
Area TextRender::draw_fit_rect(FrameBufferInfo info, Area rect, Color const& color, const char* text, Align alignx, Align aligny, DrawFunc func) {
    const auto scale = info.get_scale();
    rect.magnify(scale);
    Area font_area = {0, 0};
    get_rect(info, font_area, text);
    font_area.magnify(scale);
    double sw = font_area[2] - font_area[0], sh = font_area[3] - font_area[1];
    double dw = rect[2] - rect[0], dh = rect[3] - rect[1];
    double pad[2] = {dw - sw, dh - sh};
    double x      = alignx == Align::left ? rect[0] - font_area[0] : alignx == Align::center ? rect[0] - font_area[0] + pad[0] / 2
                                                                                             : rect[2] - sw;
    double y      = aligny == Align::left ? rect[1] - font_area[1] : aligny == Align::center ? rect[1] - font_area[1] + pad[1] / 2
                                                                                             : rect[3] - sh;
    x /= scale;
    y /= scale;
    return draw(info, x, y, color, text, func);
}
void TextRender::get_rect(FrameBufferInfo info, Area& rect, const char* text) {
    auto       uni        = convert_utf8_to_unicode32(text);
    get_rect(info, rect, uni.data());
}
void TextRender::get_rect(FrameBufferInfo info, Area& rect, const char32_t* text) {
    if(!data) {
        throw ::std::runtime_error("unititialized font.");
    }
    rect[2] = rect[0];
    rect[3] = rect[1];
    double rx1, ry1, rx2, ry2;
    rx1 = ry1 = rx2 = ry2 = 0;
    const auto scale      = info.get_scale();
    auto c = text;
    while(*c != '\0') {
        auto   chara   = get_chara_graphic(data->get_size() * scale, *c);
        double xpos[2] = {static_cast<double>(rx1 + chara->offset[0]), static_cast<double>(rx1 + chara->offset[0] + chara->get_width(info) * scale)};

        rx1 = rx1 > xpos[0] ? xpos[0] : rx1;
        rx2 = rx2 < xpos[1] ? xpos[1] : rx2;

        double ypos[2] = {static_cast<double>(-chara->offset[1]), static_cast<double>(-chara->offset[1] + chara->get_height(info) * scale)};

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
void TextRender::draw_wrapped(FrameBufferInfo info, Area& rect, int line_spacing, Color const& color, const char* text, Align alignx, Align aligny) {
    if(!data) {
        throw ::std::runtime_error("unititialized font.");
    }

    const auto                  str = convert_utf8_to_unicode32(text);
    std::vector<std::u32string> lines(1);

    const auto max_width  = rect.width();
    const auto max_height = rect.height();
    const auto len        = str.size();
    for(size_t i = 0; i < len; ++i) {
        const bool last  = i + 1 == len;
        char32_t   chara = str[i];
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
        Area area = {0, 0};
        get_rect(info, area, lines.back().data());
        if(area.width() > max_width) {
            lines.back().pop_back();

            if((lines.size() + 1) * line_spacing > max_height) {
                goto draw;
            }
            lines.emplace_back(1, chara);

            get_rect(info, area, lines.back().data());
            if(area.width() > max_width) {
                lines.pop_back();
                goto draw;
            }
        }
    }

draw:
    const size_t total_height = lines.size() * line_spacing;
    const double y_offset     = aligny == Align::left ? 0 : aligny == Align::right ? max_height - total_height
                                                                                   : (max_height - total_height) / 2.0;
    for(size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        Area area = {0, 0};
        get_rect(info, area, line.data());
        const auto total_width = area.width();
        const double x_offset = alignx == Align::left ? 0 : alignx == Align::right ? max_width - total_width 
                                                                                         : (max_width - total_width) / 2.0;
        draw(info, rect[0] + x_offset, rect[1] + y_offset + i * line_spacing - area[1], color, line.data());
    }
}
TextRender::operator bool() const {
    return data.get() != nullptr;
}
TextRender::TextRender(const std::vector<const char*>&& font_names, int size) {
    std::vector<std::string> fonts;
    for(auto path : font_names) {
        fonts.emplace_back(path);
    }
    data.reset(new TextRenderPrivate(std::move(fonts), size));
}
TextRender::TextRender() {}
TextRender::~TextRender() {}
} // namespace gawl
