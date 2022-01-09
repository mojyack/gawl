#pragma once
#include "screen.hpp"
#include "type.hpp"

namespace gawl {
template <PointArray Vertices, concepts::MetaScreen Screen>
auto convert_screen_to_viewport(const Screen& screen, Vertices& vertices) -> void {
    const auto& s = screen.get_screen_size();
    for(auto& v : vertices) {
        v.x = (v.x * 2 - s[0]) / static_cast<int>(s[0]);
        v.y = (v.y * 2 - s[1]) / -static_cast<int>(s[1]);
    }
}
template <concepts::MetaScreen Screen>
auto convert_screen_to_viewport(const Screen& screen, Rectangle& rect) -> void {
    const auto& s = screen.get_screen_size();

    rect.a.x = (rect.a.x * 2 - s[0]) / static_cast<double>(s[0]);
    rect.a.y = (rect.a.y * 2 - s[1]) / -static_cast<double>(s[1]);
    rect.b.x = (rect.b.x * 2 - s[0]) / static_cast<double>(s[0]);
    rect.b.y = (rect.b.y * 2 - s[1]) / -static_cast<double>(s[1]);
}
inline auto calc_fit_rect(const Rectangle& rect, const double width, const double height, const Align horizontal = Align::Center, const Align vertical = Align::Center) -> Rectangle {
    auto       r = double();
    const auto w = rect.width(), h = rect.height();
    {
        const auto wr = w / width;
        const auto hr = h / height;
        r             = wr < hr ? wr : hr;
    }
    const auto dw = width * r, dh = height * r;
    const auto pad = std::array{
        horizontal == Align::Left ? 0.0 : horizontal == Align::Center ? (w - dw) / 2
                                                                      : dw,
        vertical == Align::Left ? 0.0 : vertical == Align::Center ? (h - dh) / 2
                                                                  : dh,
    };
    const auto xo = rect.a.x + pad[0], yo = rect.a.y + pad[1];
    return {{xo, yo}, {xo + dw, yo + dh}};
}
inline auto clear_screen(const Color& color = {0, 0, 0, 0}) -> void {
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}
template <concepts::Screen Screen>
auto draw_rect(Screen& screen, const Rectangle& rect, const Color& color) -> void {
    auto r = rect;
    r.magnify(screen.get_scale());
    gawl::convert_screen_to_viewport(screen, r);
    const auto fbbinder = screen.prepare();
    glColor4f(color[0], color[1], color[2], color[3]);
    glRectf(r.a.x, r.a.y, r.b.x, r.b.y);
}
inline auto mask_alpha() -> void {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}
inline auto unmask_alpha() -> void {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
} // namespace gawl
