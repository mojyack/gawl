#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "gawl-window.hpp"
#include "global.hpp"
#include "misc.hpp"
#include "type.hpp"

namespace gawl {
extern GlobalVar* global;

auto convert_screen_to_viewport(const Screen* screen, Area& area) -> void {
    const auto s  = screen->get_size();
    area[0] = (area[0] - s[0] * (s[0] - area[0]) / s[0]) / static_cast<int>(s[0]);
    area[1] = (area[1] - s[1] * (s[1] - area[1]) / s[1]) / -static_cast<int>(s[1]);
    area[2] = (area[2] - s[0] * (s[0] - area[2]) / s[0]) / static_cast<int>(s[0]);
    area[3] = (area[3] - s[1] * (s[1] - area[3]) / s[1]) / -static_cast<int>(s[1]);
}
auto calc_fit_rect(Area const& area, const double width, const double height, const Align horizontal, const Align vertical) -> Area {
    auto       r = double();
    const auto w = area[2] - area[0], h = area[3] - area[1];
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
    const auto xo = area[0] + pad[0], yo = area[1] + pad[1];
    return {xo, yo, xo + dw, yo + dh};
}
auto clear_screen(const Color& color) -> void {
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}
auto draw_rect(Screen* screen, Area area, const Color& color) -> void {
    area.magnify(screen->get_scale());
    gawl::convert_screen_to_viewport(screen, area);
    glUseProgram(0);
    screen->prepare();
    glColor4f(color[0], color[1], color[2], color[3]);
    glRectf(area[0], area[1], area[2], area[3]);
}
auto mask_alpha() -> void {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}
auto unmask_alpha() -> void {
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
} // namespace gawl
