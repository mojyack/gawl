#pragma once
#include <span>
#include <vector>

#include "color.hpp"
#include "screen.hpp"

namespace gawl {
auto draw_polygon(Screen& screen, std::span<const Point> vertices, const Color& color) -> void;
auto draw_polygon_fan(Screen& screen, std::span<const Point> vertices, const Color& color) -> void;
auto draw_lines(Screen& screen, std::span<const Point> vertices, const Color& color, GLfloat width) -> void;
auto draw_outlines(Screen& screen, std::span<const Point> vertices, const Color& color, GLfloat width) -> void;
auto triangulate_circle_angle(const Point& point, double radius, const std::pair<double, double>& angle) -> std::vector<Point>;
auto triangulate_circle(const Point& point, double radius) -> std::vector<Point>;
} // namespace gawl
