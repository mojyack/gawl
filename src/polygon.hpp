#pragma once
#include <vector>

#include "earcut.hpp"
#include "global.hpp"
#include "polygon-globject.hpp"
#include "screen.hpp"
#include "type.hpp"

// for triangulate
namespace mapbox::util {
template <>
struct nth<0, gawl::Point> {
    inline static auto get(const gawl::Point& t) {
        return t.x;
    };
};
template <>
struct nth<1, gawl::Point> {
    inline static auto get(const gawl::Point& t) {
        return t.y;
    };
};
} // namespace mapbox::util

namespace gawl {
namespace internal {
template <GLenum mode>
auto generic_draw(gawl::concepts::Screen auto& screen, const PointArray auto& vertices, const Color& color) -> void {
    auto       buffer = std::vector<GLfloat>(vertices.size() * 2);
    auto&      gl     = get_global()->polygon_shader;
    const auto s      = screen->get_size();
    const auto scale  = screen->get_scale();
    for(auto i = size_t(0); i < vertices.size(); i += 1) {
        auto x            = vertices[i].x * scale;
        auto y            = vertices[i].y * scale;
        buffer[i * 2 + 0] = (x * 2 - s[0]) / static_cast<int>(s[0]);
        buffer[i * 2 + 1] = (y * 2 - s[1]) / -static_cast<int>(s[1]);
    }
    const auto vabinder = gl.bind_vao();
    const auto shbinder = gl.use_shader();
    const auto vbbinder = gl.bind_vbo();
    const auto fbbinder = screen->prepare();

    gl.write_buffer(buffer);

    glUniform4f(glGetUniformLocation(shbinder.get(), "polygon_color"), color[0], color[1], color[2], color[3]);
    glDrawArrays(mode, 0, buffer.size() / 2);
}
} // namespace internal
auto draw_polygon(gawl::concepts::Screen auto& screen, const PointArray auto& vertices, const Color& color) -> void {
    internal::generic_draw<GL_TRIANGLES>(screen, vertices, color);
}
auto draw_polygon_fan(gawl::concepts::Screen auto& screen, const PointArray auto& vertices, const Color& color) -> void {
    internal::generic_draw<GL_TRIANGLE_FAN>(screen, vertices, color);
}
auto draw_lines(gawl::concepts::Screen auto& screen, const PointArray auto& vertices, const Color& color, const GLfloat width) -> void {
    glLineWidth(width * screen.get_scale());
    internal::generic_draw<GL_LINE_STRIP>(screen, vertices, color);
}
auto draw_outlines(gawl::concepts::Screen auto& screen, const PointArray auto& vertices, const Color& color, const GLfloat width) -> void {
    glLineWidth(width * screen.get_scale());
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
inline auto trianglate_circle_angle(const Point& point, const double radius, const std::pair<double, double>& angle) -> std::vector<Point> {
    const auto n = size_t((1 + radius * 2) * angle.second);
    auto       r = std::vector<Point>(n + 1);
    r[0]         = point;
    for(auto i = size_t(0); i < n; i += 1) {
        const auto a = (angle.first + angle.second * i / (n - 1)) * 2 * std::numbers::pi;
        r[i + 1]     = {point.x + radius * std::cos(a), point.y + radius * std::sin(a)};
    }
    return r;
}
inline auto trianglate_circle(const Point& point, const double radius) -> std::vector<Point> {
    return trianglate_circle_angle(point, radius, {0, 1});
}
} // namespace gawl
