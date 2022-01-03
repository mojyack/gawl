#pragma once
#include "textrender-data.hpp"

namespace gawl {
namespace concepts {
template <class Hook>
concept TextRenderHook = requires(Hook& hook, gawl::Rectangle& rect, gawl::TextRenderCharacterGraphic& graphic) {
    { hook.callback(size_t(), rect, graphic) } -> std::same_as<bool>;
};
} // namespace concepts

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

template <int default_size = 24>
class TextRender {
  private:
    struct DefaultHook {};

    std::shared_ptr<internal::TextRenderData> data;

    internal::Character* get_chara_graphic(const int size, const char32_t c) {
        return (*data)[size].get_character(c);
    }

  public:
    auto set_char_color(const Color& color) -> void {
        internal::global.gl->textrender_shader.set_text_color(color);
    }
    auto get_rect(const gawl::concepts::MetaScreen auto& screen, const Point& base, const char* const text, const int size = 0) -> Rectangle {
        const auto uni = internal::convert_utf8_to_unicode32(text);
        return get_rect(screen, base, uni.data(), size);
    }
    auto get_rect(const gawl::concepts::MetaScreen auto& screen, const Point& base, const char32_t* const text, int size = 0) -> Rectangle {
        assert(data);
        auto r = Rectangle{base, base};
        size   = size != 0 ? size : default_size;
        if(size <= 0) {
            return r;
        }
        auto       rx1 = 0.0, ry1 = 0.0, rx2 = 0.0, ry2 = 0.0;
        const auto scale = screen.get_scale();
        auto       c     = text;

        while(*c != '\0') {
            auto       chara = get_chara_graphic(size * scale, *c);
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
        r.a.x += rx1 / scale;
        r.a.y += ry1 / scale;
        r.b.x += rx2 / scale;
        r.b.y += ry2 / scale;
        return r;
    }

    template <auto hook = DefaultHook()>
    auto draw(gawl::concepts::Screen auto& screen, const Point& point, const Color& color, const char* const text, const int size = 0) -> Rectangle {
        const auto uni = internal::convert_utf8_to_unicode32(text);
        return draw<hook>(screen, point, color, uni.data(), size);
    }
    template <auto hook = DefaultHook()>
    auto draw(gawl::concepts::Screen auto& screen, const Point& point, const Color& color, const char32_t* const text, int size = 0) -> Rectangle {
        assert(data);

        size = size != 0 ? size : default_size;
        if(size <= 0) {
            return Rectangle{point, point};
        }
        const auto scale       = screen.get_scale();
        auto       xpos        = point.x;
        auto       drawed_area = Rectangle{point, point};

        set_char_color(color);

        auto c = text;
        while(*c != '\0') {
            const auto chara = get_chara_graphic(size * scale, *c);
            const auto p     = std::array{xpos + 1. * chara->offset[0] / scale, point.y - 1. * chara->offset[1] / scale};
            const auto area  = Rectangle{{p[0], p[1]}, {p[0] + chara->get_width(screen), p[1] + chara->get_height(screen)}};
            drawed_area.a.x  = drawed_area.a.x < area.a.x ? drawed_area.a.x : area.a.x;
            drawed_area.a.y  = drawed_area.a.y < area.a.y ? drawed_area.a.y : area.a.y;
            drawed_area.b.x  = drawed_area.b.x > area.b.x ? drawed_area.b.x : area.b.x;
            drawed_area.b.y  = drawed_area.b.y > area.b.y ? drawed_area.b.y : area.b.y;
            if constexpr(concepts::TextRenderHook<decltype(hook)>) {
                if(!hook.callback(c - text, area, *chara)) {
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

    template <auto hook = DefaultHook()>
    auto draw_fit_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect, Color const& color, const char* const text, const int size = 0, const gawl::Align alignx = gawl::Align::Center, const gawl::Align aligny = gawl::Align::Center) -> Rectangle {
        const auto scale = screen.get_scale();
        auto       r     = rect;
        r.magnify(scale);
        auto font_area = get_rect(screen, {0, 0}, text, size);
        font_area.magnify(scale);
        const auto pad = std::array{r.width() - font_area.width(), r.height() - font_area.height()};

        auto x = alignx == Align::Left ? r.a.x - font_area.a.x : alignx == Align::Center ? r.a.x - font_area.a.x + pad[0] / 2
                                                                                         : r.b.x - font_area.width();
        auto y = aligny == Align::Left ? r.a.y - font_area.a.y : aligny == Align::Center ? r.a.y - font_area.a.y + pad[1] / 2
                                                                                         : r.b.y - font_area.height();
        x /= scale;
        y /= scale;
        return draw<hook>(screen, {x, y}, color, text, size);
    }

    auto draw_wrapped(gawl::concepts::Screen auto& screen, const Rectangle& rect, const double line_spacing, const Color& color, const char* const text, const int size = 0, const gawl::Align alignx = gawl::Align::Center, const gawl::Align aligny = gawl::Align::Center) -> void {
        assert(data);

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
            auto area = get_rect(screen, {0, 0}, lines.back().data(), size);
            if(area.width() > max_width) {
                lines.back().pop_back();

                if((lines.size() + 1) * line_spacing > max_height) {
                    goto draw;
                }
                lines.emplace_back(1, chara);

                area = get_rect(screen, area.a, lines.back().data(), size);
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
            const auto  area        = get_rect(screen, {0, 0}, line.data(), size);
            const auto  total_width = area.width();
            const auto  x_offset    = alignx == Align::Left ? 0.0 : alignx == Align::Right ? max_width - total_width
                                                                                           : (max_width - total_width) / 2.0;
            draw(screen, {rect.a.x + x_offset, rect.a.y + y_offset + i * line_spacing - area.a.y}, color, line.data(), size);
        }
    }

    operator bool() const {
        return static_cast<bool>(data);
    }
    TextRender() {}
    TextRender(const std::vector<const char*>& font_names) {
        auto fonts = std::vector<std::string>();
        for(auto path : font_names) {
            fonts.emplace_back(path);
        }
        data.reset(new internal::TextRenderData(std::move(fonts)));
    }
};

} // namespace gawl
