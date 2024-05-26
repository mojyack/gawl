#include "polygon.hpp"
#include "global.hpp"
#include "polygon-shader.hpp"

namespace gawl::impl {
namespace {
template <GLenum mode>
auto generic_draw(Screen& screen, const std::span<const Point> vertices, const Color& color) -> void {
    auto        buffer = std::vector<GLfloat>(vertices.size() * 2);
    auto&       gl     = global->polygon_shader;
    const auto& s      = screen.get_viewport();
    const auto  scale  = screen.get_scale();
    for(auto i = size_t(0); i < vertices.size(); i += 1) {
        auto x            = vertices[i].x * scale;
        auto y            = vertices[i].y * scale;
        buffer[i * 2 + 0] = ((x - s.base[0]) * 2 - s.size[0]) / static_cast<int>(s.size[0]);
        buffer[i * 2 + 1] = ((y - s.base[1]) * 2 - s.size[1]) / -static_cast<int>(s.size[1]);
    }
    const auto vabinder = gl.bind_vao();
    const auto shbinder = gl.use_shader();
    const auto vbbinder = gl.bind_vbo();
    const auto fbbinder = screen.prepare();

    gl.write_buffer(buffer);

    glUniform4f(glGetUniformLocation(shbinder.get(), "polygon_color"), color[0], color[1], color[2], color[3]);
    glDrawArrays(mode, 0, buffer.size() / 2);
}
} // namespace
} // namespace gawl::impl

namespace gawl {
auto draw_polygon(Screen& screen, const std::span<const Point> vertices, const Color& color) -> void {
    impl::generic_draw<GL_TRIANGLES>(screen, vertices, color);
}

auto draw_polygon_fan(Screen& screen, const std::span<const Point> vertices, const Color& color) -> void {
    impl::generic_draw<GL_TRIANGLE_FAN>(screen, vertices, color);
}

auto draw_lines(Screen& screen, const std::span<const Point> vertices, const Color& color, const GLfloat width) -> void {
    glLineWidth(width * screen.get_scale());
    impl::generic_draw<GL_LINE_STRIP>(screen, vertices, color);
}

auto draw_outlines(Screen& screen, const std::span<const Point> vertices, const Color& color, const GLfloat width) -> void {
    glLineWidth(width * screen.get_scale());
    impl::generic_draw<GL_LINE_LOOP>(screen, vertices, color);
}

auto triangulate_circle_angle(const Point& point, const double radius, const std::pair<double, double>& angle) -> std::vector<Point> {
    const auto n = size_t((1 + radius * 2) * angle.second);
    auto       r = std::vector<Point>(n + 1);
    r[0]         = point;
    for(auto i = size_t(0); i < n; i += 1) {
        const auto a = (angle.first + angle.second * i / (n - 1)) * 2 * std::numbers::pi;
        r[i + 1]     = {point.x + radius * std::cos(a), point.y + radius * std::sin(a)};
    }
    return r;
}

auto triangulate_circle(const Point& point, const double radius) -> std::vector<Point> {
    return triangulate_circle_angle(point, radius, {0, 1});
}
} // namespace gawl
