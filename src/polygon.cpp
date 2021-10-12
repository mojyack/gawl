#include <numbers>

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

auto do_draw(const GLenum mode, Screen* const screen, const std::vector<GLfloat>& buffer, const Color& color) -> void {
    const auto copy_size = buffer.size() * sizeof(GLfloat);
    const auto vbbinder  = internal::VertexBufferBinder(vbo);
    if(current_vbo_capacity < copy_size) {
        glBufferData(GL_ARRAY_BUFFER, copy_size, buffer.data(), GL_DYNAMIC_DRAW);
        current_vbo_capacity = copy_size;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, copy_size, buffer.data());
    }
    auto&      shader   = *global->polygon_shader;
    const auto vabinder = shader.bind_vao();
    const auto fbbinder = screen->prepare();
    const auto shbinder = shader.use_shader();
    glUniform4f(glGetUniformLocation(shbinder.get(), "polygon_color"), color[0], color[1], color[2], color[3]);
    glDrawArrays(mode, 0, buffer.size() / 2);
}
} // namespace internal
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
