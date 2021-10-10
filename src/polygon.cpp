#include <numbers>

#include "earcut.hpp"
#include "error.hpp"
#include "global.hpp"
#include "polygon.hpp"

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
namespace {
GLuint ebo;
size_t current_ebo_capacity;

GLuint vbo;
size_t current_vbo_capacity;
} // namespace

namespace internal {
auto init_polygon() -> std::pair<GLuint, GLuint> {
    glGenBuffers(1, &vbo);
    current_vbo_capacity = 0;

    glGenBuffers(1, &ebo);
    current_ebo_capacity = 0;

    return {vbo, ebo};
}
auto finish_polygon() -> void {
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}
extern GlobalVar* global;
} // namespace internal

template <auto MODE>
auto generic_draw(Screen* const screen, const std::vector<Point>& vertices, const Color& color) -> void {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    auto       buffer = std::vector<GLfloat>(vertices.size() * 2);
    const auto s      = screen->get_size();
    const auto scale  = screen->get_scale();
    for(auto i = size_t(0); i < vertices.size(); i += 1) {
        auto x            = vertices[i].x * scale;
        auto y            = vertices[i].y * scale;
        buffer[i * 2 + 0] = (x * 2 - s[0]) / static_cast<int>(s[0]);
        buffer[i * 2 + 1] = (y * 2 - s[1]) / -static_cast<int>(s[1]);
    }
    const auto copy_size = vertices.size() * 2 * sizeof(GLfloat);
    if(current_vbo_capacity < copy_size) {
        glBufferData(GL_ARRAY_BUFFER, copy_size, buffer.data(), GL_DYNAMIC_DRAW);
        current_vbo_capacity = copy_size;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, copy_size, buffer.data());
    }
    auto& shader = *internal::global->polygon_shader;
    shader.bind_vao();
    screen->prepare();
    glUseProgram(shader.get_shader());
    glUniform4f(glGetUniformLocation(shader.get_shader(), "polygon_color"), color[0], color[1], color[2], color[3]);
    glDrawArrays(MODE, 0, vertices.size());
}
auto draw_polygon(Screen* const screen, const std::vector<Point>& vertices, const Color& color) -> void {
    generic_draw<GL_TRIANGLES>(screen, vertices, color);
}
auto draw_polygon_fan(Screen* const screen, const std::vector<Point>& vertices, const Color& color) -> void {
    generic_draw<GL_TRIANGLE_FAN>(screen, vertices, color);
}
auto change_line_width(const GLfloat width) -> void {
    glLineWidth(width);
}
auto draw_lines(Screen* const screen, const std::vector<Point>& vertices, const Color& color, const GLfloat width) -> void {
    change_line_width(width * screen->get_scale());
    generic_draw<GL_LINE_STRIP>(screen, vertices, color);
}
auto draw_outlines(Screen* const screen, const std::vector<Point>& vertices, const Color& color, const GLfloat width) -> void {
    change_line_width(width * screen->get_scale());
    generic_draw<GL_LINE_LOOP>(screen, vertices, color);
}
auto triangulate(const std::vector<std::vector<Point>>& vertices) -> std::vector<Point> {
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
auto trianglate_circle(const Point& point, const double radius) -> std::vector<Point> {
    return trianglate_circle_angle(point, radius, {0, 1});
}
auto trianglate_circle_angle(const Point& point, const double radius, const std::pair<double, double>& angle) -> std::vector<Point> {
    const auto n = size_t((1 + radius * 2) * angle.second);
    auto       r = std::vector<Point>(n + 1);
    r[0]         = point;
    for(auto i = size_t(0); i < n; i += 1) {
        const auto a = (angle.first + angle.second * i / (n - 1)) * 2 * std::numbers::pi;
        r[i + 1]     = {point.x + radius * std::cos(a), point.y + radius * std::sin(a)};
    }
    return r;
}
} // namespace gawl
