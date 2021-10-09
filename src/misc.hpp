#pragma once
#include "screen.hpp"
#include "type.hpp"

namespace gawl {
auto convert_screen_to_viewport(const Screen* screen, Rectangle& rect) -> void;
auto calc_fit_rect(const Rectangle& rect, const double width, const double height, const Align horizontal = Align::center, const Align vertical = Align::center) -> Rectangle;
auto clear_screen(const Color& color = {0, 0, 0, 0}) -> void;
auto draw_rect(Screen* screen, const Rectangle& rect, const Color& color) -> void;
auto mask_alpha() -> void;
auto unmask_alpha() -> void;
} // namespace gawl
