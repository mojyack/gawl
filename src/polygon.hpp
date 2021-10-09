#pragma once
#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "screen.hpp"
#include "type.hpp"

namespace gawl {
namespace internal {
auto init_polygon() -> std::pair<GLuint, GLuint>; // vbo, ebo
auto finish_polygon() -> void;
} // namespace internal
auto draw_polygon(Screen* const screen, const std::vector<Point>& vertices, const Color& color) -> void;
auto draw_polygon_fan(Screen* screen, const std::vector<Point>& vertices, const Color& color) -> void;
auto change_line_width(GLfloat width) -> void;
auto draw_lines(Screen* screen, const std::vector<Point>& vertices, const Color& color) -> void;
auto draw_lines(Screen* screen, const std::vector<Point>& vertices, const Color& color, GLfloat width) -> void;
auto draw_outlines(Screen* screen, const std::vector<Point>& vertices, const Color& color) -> void;
auto draw_outlines(Screen* screen, const std::vector<Point>& vertices, const Color& color, GLfloat width) -> void;
auto triangulate(const std::vector<std::vector<Point>>& vertices) -> std::vector<Point>;
auto trianglate_circle(const Point& point, double radius) -> std::vector<Point>;
auto trianglate_circle_angle(const Point& point, double radius, const std::pair<double, double>& angle) -> std::vector<Point>;
} // namespace gawl
