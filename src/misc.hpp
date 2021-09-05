#pragma once
#include "type.hpp"

#include "frame-buffer-info.hpp"

namespace gawl {
void convert_screen_to_viewport(FrameBufferInfo info, Area& area);
Area calc_fit_rect(const Area& area, const double width, const double height, const Align horizontal = Align::center, const Align vertical = Align::center);
void clear_screen(const Color& color);
void draw_rect(FrameBufferInfo info, Area area, const Color& color);
void mask_alpha();
void unmask_alpha();
} // namespace gawl
