#pragma once
#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "earcut.hpp"
#include "globject.hpp"
#include "screen.hpp"
#include "type.hpp"

namespace gawl {
namespace internal {
auto create_polygon_globject() -> GLObject*;

auto do_draw(GLenum mode, Screen* screen, const std::vector<GLfloat>& buffer, const Color& color) -> void;
template <GLenum MODE, PointArray T>
auto generic_draw(Screen* const screen, const T& vertices, const Color& color) -> void {
    auto       buffer = std::vector<GLfloat>(vertices.size() * 2);
    const auto s      = screen->get_size();
    const auto scale  = screen->get_scale();
    for(auto i = size_t(0); i < vertices.size(); i += 1) {
        auto x            = vertices[i].x * scale;
        auto y            = vertices[i].y * scale;
        buffer[i * 2 + 0] = (x * 2 - s[0]) / static_cast<int>(s[0]);
        buffer[i * 2 + 1] = (y * 2 - s[1]) / -static_cast<int>(s[1]);
    }
    do_draw(MODE, screen, buffer, color);
}
} // namespace internal
template <PointArray T>
auto draw_polygon(Screen* const screen, const T& vertices, const Color& color) -> void {
    internal::generic_draw<GL_TRIANGLES>(screen, vertices, color);
}
template <PointArray T>
auto draw_polygon_fan(Screen* const screen, const T& vertices, const Color& color) -> void {
    internal::generic_draw<GL_TRIANGLE_FAN>(screen, vertices, color);
}
template <PointArray T>
auto draw_lines(Screen* const screen, const T& vertices, const Color& color, const GLfloat width) -> void {
    glLineWidth(width * screen->get_scale());
    internal::generic_draw<GL_LINE_STRIP>(screen, vertices, color);
}
template <PointArray T>
auto draw_outlines(Screen* const screen, const T& vertices, const Color& color, const GLfloat width) -> void {
    glLineWidth(width * screen->get_scale());
    internal::generic_draw<GL_LINE_LOOP>(screen, vertices, color);
}
template <PointArray T>
auto triangulate(const std::vector<T>& vertices) -> std::vector<Point> {
    const auto indices = mapbox::earcut<size_t>(vertices);
    auto       r       = std::vector<Point>(indices.size());
    for(auto i = size_t(0); i < indices.size(); i += 1) {
        auto index = indices[i];
        for(const auto& v : vertices) {
            if(index < v.size()) {
                r[i] = v[index];
                break;
            }
            index -= v.size();
        }
    }
    return r;
}
auto trianglate_circle(const Point& point, double radius) -> std::vector<Point>;
auto trianglate_circle_angle(const Point& point, double radius, const std::pair<double, double>& angle) -> std::vector<Point>;
} // namespace gawl
