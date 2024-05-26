#pragma once
#include <span>

#include "align.hpp"
#include "color.hpp"
#include "point.hpp"
#include "screen.hpp"

namespace gawl {
auto convert_screen_to_viewport(const Screen& screen, std::span<Point> vertices) -> void;
auto convert_screen_to_viewport(const Screen& screen, Rectangle& rect) -> void;
auto calc_fit_rect(const Rectangle& rect, double width, double height, Align horizontal = Align::Center, Align vertical = Align::Center) -> Rectangle;
auto clear_screen(const Color& color = {0, 0, 0, 0}) -> void;
auto draw_rect(Screen& screen, const Rectangle& rect, const Color& color) -> void;
auto mask_alpha() -> void;
auto unmask_alpha() -> void;
} // namespace gawl
