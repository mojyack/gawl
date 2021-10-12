#pragma once
#include "screen.hpp"
#include "type.hpp"

namespace gawl {
template <PointArray T>
auto convert_screen_to_viewport(const Screen* screen, T& vertices) -> void {
    const auto s = screen->get_size();
    for(auto& v : vertices) {
        v.x = (v.x * 2 - s[0]) / static_cast<int>(s[0]);
        v.y = (v.y * 2 - s[1]) / -static_cast<int>(s[1]);
    }
}
auto convert_screen_to_viewport(const Screen* screen, Rectangle& rect) -> void;
auto calc_fit_rect(const Rectangle& rect, const double width, const double height, const Align horizontal = Align::center, const Align vertical = Align::center) -> Rectangle;
auto clear_screen(const Color& color = {0, 0, 0, 0}) -> void;
auto draw_rect(Screen* screen, const Rectangle& rect, const Color& color) -> void;
auto mask_alpha() -> void;
auto unmask_alpha() -> void;
} // namespace gawl
