#include "textrender.hpp"
#include "global.hpp"
#include "util/assert.hpp"

namespace gawl {
namespace impl {
Character::Character(char32_t code, const std::vector<FT_Face>& faces) : GraphicBase(global->textrender_shader) {
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

    dynamic_assert(FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) == 0);
    dynamic_assert(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) == 0);

    this->width  = face->glyph->bitmap.width;
    this->height = face->glyph->bitmap.rows;
    offset[0]    = face->glyph->bitmap_left;
    offset[1]    = face->glyph->bitmap_top;
    advance      = static_cast<int>(face->glyph->advance.x) >> 6;

    const auto txbinder = this->bind_texture();
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, this->width);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, this->width, this->height, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
}

auto CharacterCache::get_character(const char32_t c) -> Character& {
    if(const auto f = cache.find(c); f != cache.end()) {
        return f->second;
    } else {
        return cache.insert(std::make_pair(c, Character(c, faces))).first->second;
    }
}

CharacterCache::CharacterCache(const std::vector<std::string>& font_names, const int size) {
    for(const auto& path : font_names) {
        auto face = FT_Face();
        dynamic_assert(FT_New_Face(FT_Library(global->textrender_shader.freetype), path.data(), 0, &(face)) == 0);
        FT_Set_Pixel_Sizes(face, 0, size);
        faces.emplace_back(face);
    }
}

CharacterCache::~CharacterCache() {
    for(auto f : faces) {
        FT_Done_Face(f);
    }
}

auto convert_utf8_to_unicode32(const std::string_view utf8) -> std::u32string {
    auto uni32 = std::u32string();

    for(auto i = size_t(0); i < utf8.size(); i += 1) {
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
} // namespace impl

auto WrappedText::is_changed(const double width, const double screen_scale) const -> bool {
    return this->width != width || this->screen_scale != screen_scale;
}

auto WrappedText::reset() -> void {
    width        = 0;
    screen_scale = 0;
}

auto WrappedText::get_lines() const -> const std::vector<std::u32string>& {
    return lines;
}

WrappedText::WrappedText(const double width, const double screen_scale, const std::vector<std::u32string> lines)
    : width(width),
      screen_scale(screen_scale),
      lines(std::move(lines)) {}

auto TextRender::clear() -> void {
    caches.clear();
}

auto TextRender::get_chara(const int size) -> impl::CharacterCache& {
    if(const auto p = caches.find(size); p != caches.end()) {
        return p->second;
    }
    return caches.insert(std::make_pair(size, impl::CharacterCache(font_names, size))).first->second;
}

auto TextRender::get_chara_graphic(const int size, const char32_t chara) -> impl::Character& {
    return get_chara(size).get_character(chara);
}

auto TextRender::create_wrapped_text(const MetaScreen& screen, const double width, const std::string_view text, const int size) -> WrappedText {
    const auto str   = impl::convert_utf8_to_unicode32(text);
    auto       lines = std::vector<std::u32string>(1);
    const auto len   = str.size();

    auto line = &lines.back();
    for(auto i = size_t(0); i < len; i += 1) {
        auto chara = str[i];
        switch(chara) {
        case U'\n':
            line = &lines.emplace_back();
            break;
        default:
            *line += chara;
            break;
        }
        auto area = get_rect(screen, *line, size);
        if(area.width() > width) {
            line->pop_back();
            line = &lines.emplace_back(1, chara);
            area = get_rect(screen, *line, size);
            if(area.width() > width) {
                lines.pop_back();
                goto done;
            }
        }
    }
done:
    return WrappedText(width, screen.get_scale(), std::move(lines));
}

auto TextRender::init(std::vector<std::string> font_names, const int default_size) -> void {
    this->font_names   = std::move(font_names);
    this->default_size = default_size;
}

auto TextRender::get_default_size() const -> int {
    return default_size;
}

auto TextRender::set_char_color(const Color& color) -> void {
    impl::global->textrender_shader.set_text_color(color);
}

auto TextRender::get_rect(const MetaScreen& screen, const std::string_view text, const int size) -> Rectangle {
    const auto uni = impl::convert_utf8_to_unicode32(text);
    return get_rect(screen, uni.data(), size);
}

auto TextRender::get_rect(const MetaScreen& screen, const std::u32string_view text, int size) -> Rectangle {
    size = size != 0 ? size : default_size;
    if(size <= 0) {
        return {{0, 0}, {0, 0}};
    }

    const auto scale = screen.get_scale();
    auto       pen_x = 0.0;
    auto       pen_y = 0.0;
    auto       rx    = Rectangle{{0, 0}, {0, 0}};

    for(auto i = size_t(0); i < text.size(); i += 1) {
        const auto c     = text[i];
        const auto first = i == 0;

        const auto& chara = get_chara_graphic(size * scale, c);

        const auto x_a = pen_x + (!first ? chara.offset[0] : 0);
        const auto x_b = x_a + chara.get_width(screen) * scale;
        rx.a.x         = rx.a.x > x_a ? x_a : rx.a.x;
        rx.b.x         = first ? x_b : (rx.b.x < x_b ? x_b : rx.b.x);

        const auto y_a = pen_y - chara.offset[1];
        const auto y_b = y_a + chara.get_height(screen) * scale;
        rx.a.y         = rx.a.y > y_a ? y_a : rx.a.y;
        rx.b.y         = first ? y_b : (rx.b.y < y_b ? y_b : rx.b.y);

        // wprintf(L"%c, {{%f,%f},{%f,%f}}\n", *c, x_a, y_a, x_b, y_b);

        pen_x += chara.advance;
    }

    return rx.magnify(1 / scale);
}

auto TextRender::draw(Screen& screen, const Point& point, const Color& color, const std::string_view text, const int size, const Callback callback) -> Rectangle {
    const auto uni = impl::convert_utf8_to_unicode32(text);
    return draw(screen, point, color, uni.data(), size, callback);
}

auto TextRender::draw(Screen& screen, const Point& point, const Color& color, const std::u32string_view text, int size, const Callback callback) -> Rectangle {
    size = size != 0 ? size : default_size;
    if(size <= 0) {
        return Rectangle{point, point};
    }
    const auto scale       = screen.get_scale();
    auto       pen         = point;
    auto       drawed_area = Rectangle{point, point};

    set_char_color(color);

    for(auto i = size_t(0); i < text.size(); i += 1) {
        const auto c = text[i];

        auto&      chara = get_chara_graphic(size * scale, c);
        const auto x_a   = pen.x + chara.offset[0] / scale;
        const auto x_b   = x_a + chara.get_width(screen);
        drawed_area.a.x  = drawed_area.a.x < x_a ? drawed_area.a.x : x_a;
        drawed_area.b.x  = drawed_area.b.x > x_b ? drawed_area.b.x : x_b;

        const auto y_a  = pen.y - chara.offset[1] / scale;
        const auto y_b  = y_a + chara.get_height(screen);
        drawed_area.a.y = drawed_area.a.y < y_a ? drawed_area.a.y : y_a;
        drawed_area.b.y = drawed_area.b.y > y_b ? drawed_area.b.y : y_b;

        if(!callback || !callback(i, {{x_a, y_a}, {x_b, y_b}}, chara)) {
            chara.draw_rect(screen, {{x_a, y_a}, {x_b, y_b}});
        }

        pen.x += chara.advance / scale;
    }

    return drawed_area;
}

auto TextRender::draw_fit_rect(Screen& screen, const Rectangle& rect, const Color& color, const std::string_view text, const int size, const gawl::Align alignx, const gawl::Align aligny, const Callback callback) -> Rectangle {
    const auto scale     = screen.get_scale();
    const auto r         = Rectangle(rect).magnify(scale);
    const auto font_area = get_rect(screen, text, size).magnify(scale);
    const auto pad       = std::array{r.width() - font_area.width(), r.height() - font_area.height()};

    auto x = alignx == Align::Left ? r.a.x - font_area.a.x : alignx == Align::Center ? r.a.x + pad[0] / 2
                                                                                     : r.b.x - font_area.width();
    auto y = aligny == Align::Left ? r.a.y - font_area.a.y : aligny == Align::Center ? r.a.y - font_area.a.y + pad[1] / 2
                                                                                     : r.b.y - font_area.b.y;
    x /= scale;
    y /= scale;
    return draw(screen, {x, y}, color, text, size, callback);
}

auto TextRender::calc_wrapped_text_height(Screen& screen, const double width, const double line_height, const std::string_view text, WrappedText& wrapped_text, const int size) -> double {
    if(wrapped_text.is_changed(width, screen.get_scale())) {
        wrapped_text = create_wrapped_text(screen, width, text, size);
    }

    const auto& lines        = wrapped_text.get_lines();
    const auto  total_height = lines.size() * line_height;

    return total_height;
}

auto TextRender::draw_wrapped(Screen& screen, const Rectangle& rect, const double line_height, const Color& color, const std::string_view text, WrappedText& wrapped_text, const int size, const gawl::Align alignx, const gawl::Align aligny) -> void {
    const auto rect_width  = rect.width();
    const auto rect_height = rect.height();

    if(wrapped_text.is_changed(rect_width, screen.get_scale())) {
        wrapped_text = create_wrapped_text(screen, rect_width, text, size);
    }

    const auto& lines = wrapped_text.get_lines();

    const auto total_height = lines.size() * line_height;
    const auto y_offset     = aligny == Align::Left ? 0.0 : aligny == Align::Right ? rect_height - total_height
                                                                                   : (rect_height - total_height) / 2.0;

    const auto visible_rect = Viewport(screen.get_viewport()).to_rectangle().magnify(1.0 / screen.get_scale()) &= rect;
    const auto y_pos_begin  = rect.a.y + y_offset;
    const auto index_begin  = int(std::max(0.0, -(y_pos_begin - visible_rect.a.y) / line_height));
    const auto index_end    = int(std::min(double(lines.size()), index_begin + (visible_rect.height() + line_height - 1) / line_height));
    for(auto i = index_begin; i < index_end; i += 1) {
        const auto& line        = lines[i];
        const auto  area        = get_rect(screen, line, size);
        const auto  total_width = area.width();

        const auto x_offset = alignx == Align::Left ? -area.a.x : alignx == Align::Right ? rect_width - total_width
                                                                                         : (rect_width - total_width) / 2.0;
        draw(screen, {rect.a.x + x_offset, y_pos_begin + i * line_height - area.a.y}, color, line.data(), size);
    }
}

TextRender::TextRender(std::vector<std::string> font_names, const int default_size) {
    init(std::move(font_names), default_size);
}
} // namespace gawl
