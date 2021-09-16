#pragma once
#include "screen.hpp"
#include "type.hpp"

namespace gawl {
auto convert_screen_to_viewport(const Screen* screen, Area& area) -> void;
auto calc_fit_rect(const Area& area, const double width, const double height, const Align horizontal = Align::center, const Align vertical = Align::center) -> Area;
auto clear_screen(const Color& color) -> void;
auto draw_rect(Screen* screen, Area area, const Color& color) -> void;
auto mask_alpha() -> void;
auto unmask_alpha() -> void;
} // namespace gawl
