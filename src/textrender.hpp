#pragma once
#include "textrender-internal.hpp"

namespace gawl {
namespace internal {
inline auto convert_utf8_to_unicode32(const char* const str) -> std::u32string {
    auto utf8  = std::string_view(str);
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
} // namespace internal

class TextRender {
  private:
    std::unordered_map<int, std::unique_ptr<internal::CharacterCache>> caches;
    const std::vector<std::string>                                     font_names;
    int                                                                default_size;

    auto clear() -> void {
        caches.clear();
    }

    auto get_chara(const int size) -> internal::CharacterCache& {
        if(const auto p = caches.find(size); p != caches.end()) {
            return *p->second.get();
        }
        return *caches.insert(std::make_pair(size, new internal::CharacterCache(font_names, size))).first->second.get();
    }

    auto get_chara_graphic(const int size, const char32_t chara) -> internal::Character& {
        return get_chara(size).get_character(chara);
    }

  public:
    using Callback = std::function<bool(size_t, const gawl::Rectangle&, gawl::TextRenderCharacterGraphic&)>;

    auto set_char_color(const Color& color) -> void {
        internal::global->textrender_shader.set_text_color(color);
    }

    auto get_rect(const gawl::concepts::MetaScreen auto& screen, const char* const text, const int size = 0) -> Rectangle {
        const auto uni = internal::convert_utf8_to_unicode32(text);
        return get_rect(screen, uni.data(), size);
    }

    auto get_rect(const gawl::concepts::MetaScreen auto& screen, const char32_t* const text, int size = 0) -> Rectangle {
        size = size != 0 ? size : default_size;
        if(size <= 0) {
            return {{0, 0}, {0, 0}};
        }

        const auto scale = screen.get_scale();
        auto       pen_x = 0.0;
        auto       pen_y = 0.0;
        auto       rx    = Rectangle{{0, 0}, {0, 0}};
        auto       c     = text;

        while(*c != '\0') {
            const auto first = c == text;

            const auto& chara = get_chara_graphic(size * scale, *c);

            const auto x_a = pen_x + (!first ? chara.offset[0] : 0);
            const auto x_b = x_a + chara.get_width(screen) * scale;
            rx.a.x         = rx.a.x > x_a ? x_a : rx.a.x;
            rx.b.x         = c == text ? x_b : (rx.b.x < x_b ? x_b : rx.b.x);

            const auto y_a = pen_y - chara.offset[1];
            const auto y_b = y_a + chara.get_height(screen) * scale;
            rx.a.y         = rx.a.y > y_a ? y_a : rx.a.y;
            rx.b.y         = first ? y_b : (rx.b.y < y_b ? y_b : rx.b.y);

            // wprintf(L"%c, {{%f,%f},{%f,%f}}\n", *c, x_a, y_a, x_b, y_b);

            c += 1;

            pen_x += chara.advance;
        }

        return rx.magnify(1 / scale);
    }

    auto draw(gawl::concepts::Screen auto& screen, const Point& point, const Color& color, const char* const text, const int size = 0, const Callback callback = nullptr) -> Rectangle {
        const auto uni = internal::convert_utf8_to_unicode32(text);
        return draw(screen, point, color, uni.data(), size, callback);
    }

    auto draw(gawl::concepts::Screen auto& screen, const Point& point, const Color& color, const char32_t* const text, int size = 0, const Callback callback = nullptr) -> Rectangle {
        size = size != 0 ? size : default_size;
        if(size <= 0) {
            return Rectangle{point, point};
        }
        const auto scale       = screen.get_scale();
        auto       pen         = point;
        auto       drawed_area = Rectangle{point, point};

        set_char_color(color);

        auto c = text;
        while(*c != '\0') {
            const auto first = c == text;

            auto&      chara = get_chara_graphic(size * scale, *c);
            const auto x_a   = pen.x + (!first ? chara.offset[0] / scale : 0);
            const auto x_b   = x_a + chara.get_width(screen);
            drawed_area.a.x  = drawed_area.a.x < x_a ? drawed_area.a.x : x_a;
            drawed_area.b.x  = drawed_area.b.x > x_b ? drawed_area.b.x : x_b;

            const auto y_a  = pen.y - chara.offset[1] / scale;
            const auto y_b  = y_a + chara.get_height(screen);
            drawed_area.a.y = drawed_area.a.y < y_a ? drawed_area.a.y : y_a;
            drawed_area.b.y = drawed_area.b.y > y_b ? drawed_area.b.y : y_b;

            if(!callback || !callback(c - text, {{x_a, y_a}, {x_b, y_b}}, chara)) {
                chara.draw_rect(screen, {{x_a, y_a}, {x_b, y_b}});
            }

            c += 1;

            pen.x += chara.advance / scale;
        }
        return drawed_area;
    }

    auto draw_fit_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect, const Color& color, const char* const text, const int size = 0, const gawl::Align alignx = gawl::Align::Center, const gawl::Align aligny = gawl::Align::Center, const Callback callback = nullptr) -> Rectangle {
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

    auto draw_wrapped(gawl::concepts::Screen auto& screen, const Rectangle& rect, const double line_spacing, const Color& color, const char* const text, const int size = 0, const gawl::Align alignx = gawl::Align::Center, const gawl::Align aligny = gawl::Align::Center) -> void {
        const auto str   = internal::convert_utf8_to_unicode32(text);
        auto       lines = std::vector<std::u32string>(1);

        const auto max_width  = rect.width();
        const auto max_height = rect.height();
        const auto len        = str.size();
        for(auto i = size_t(0); i < len; i += 1) {
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
            auto area = get_rect(screen, lines.back().data(), size);
            if(area.width() > max_width) {
                lines.back().pop_back();

                if((lines.size() + 1) * line_spacing > max_height) {
                    goto draw;
                }
                lines.emplace_back(1, chara);

                area = get_rect(screen, lines.back().data(), size);
                if(area.width() > max_width) {
                    lines.pop_back();
                    goto draw;
                }
            }
        }

    draw:
        const auto total_height = size_t(lines.size() * line_spacing);
        const auto y_offset     = aligny == Align::Left ? 0.0 : aligny == Align::Right ? max_height - total_height
                                                                                       : (max_height - total_height) / 2.0;
        for(auto i = size_t(0); i < lines.size(); i += 1) {
            const auto& line        = lines[i];
            const auto  area        = get_rect(screen, line.data(), size);
            const auto  total_width = area.width();
            const auto  x_offset    = alignx == Align::Left ? 0.0 : alignx == Align::Right ? max_width - total_width
                                                                                           : (max_width - total_width) / 2.0;
            draw(screen, {rect.a.x + x_offset, rect.a.y + y_offset + i * line_spacing - area.a.y}, color, line.data(), size);
        }
    }

    TextRender(std::vector<std::string> font_names, const int default_size) : font_names(std::move(font_names)), default_size(default_size) {}
};
} // namespace gawl
