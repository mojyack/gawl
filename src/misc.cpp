#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "gawl-window.hpp"
#include "global.hpp"
#include "misc.hpp"
#include "type.hpp"

namespace gawl {
extern GlobalVar* global;

auto convert_screen_to_viewport(const Screen* screen, Rectangle& rect) -> void {
    const auto s  = screen->get_size();
    rect.a.x = (rect.a.x - s[0] * (s[0] - rect.a.x) / s[0]) / static_cast<int>(s[0]);
    rect.a.y = (rect.a.y - s[1] * (s[1] - rect.a.y) / s[1]) / -static_cast<int>(s[1]);
    rect.b.x = (rect.b.x - s[0] * (s[0] - rect.b.x) / s[0]) / static_cast<int>(s[0]);
    rect.b.y = (rect.b.y - s[1] * (s[1] - rect.b.y) / s[1]) / -static_cast<int>(s[1]);
}
auto calc_fit_rect(const Rectangle& rect, const double width, const double height, const Align horizontal, const Align vertical) -> Rectangle {
    auto       r = double();
    const auto w = rect.width(), h = rect.height();
    {
        const auto wr = w / width;
        const auto hr = h / height;
        r             = wr < hr ? wr : hr;
    }
    const auto dw = width * r, dh = height * r;
    const auto pad = std::array{
        horizontal == Align::left ? 0.0 : horizontal == Align::center ? (w - dw) / 2
                                                                      : dw,
        vertical == Align::left ? 0.0 : vertical == Align::center ? (h - dh) / 2
                                                                  : dh,
    };
    const auto xo = rect.a.x + pad[0], yo = rect.a.y + pad[1];
    return {{xo, yo}, {xo + dw, yo + dh}};
}
auto clear_screen(const Color& color) -> void {
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}
auto draw_rect(Screen* const screen, const Rectangle& rect, const Color& color) -> void {
    auto r = rect;
    r.magnify(screen->get_scale());
    gawl::convert_screen_to_viewport(screen, r);
    glUseProgram(0);
    screen->prepare();
    glColor4f(color[0], color[1], color[2], color[3]);
    glRectf(rect.a.x, rect.a.y, rect.b.x, rect.b.y);
}
auto mask_alpha() -> void {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}
auto unmask_alpha() -> void {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
} // namespace gawl
