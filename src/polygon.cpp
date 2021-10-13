#include <numbers>

#include "error.hpp"
#include "polygon.hpp"
#include "shader-source.hpp"

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
class PolygonGLObject : public GLObject {
  private:
    size_t vbo_capacity = 0;

  public:
    auto write_buffer(const std::vector<GLfloat>& buffer) -> void {
        const auto copy_size = buffer.size() * sizeof(GLfloat);
        if(vbo_capacity < copy_size) {
            glBufferData(GL_ARRAY_BUFFER, copy_size, buffer.data(), GL_DYNAMIC_DRAW);
            vbo_capacity = copy_size;
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, copy_size, buffer.data());
        }
    }
    PolygonGLObject() : GLObject(polygon_vertex_shader_source, polygon_fragment_shader_source) {
        auto vabinder = bind_vao();
        auto vbbinder = bind_vbo();
        auto ebbinder = bind_ebo();

        const auto pos_attrib = glGetAttribLocation(shader_program, "position");
        glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
        glEnableVertexAttribArray(pos_attrib);
    }
};

static PolygonGLObject* gl;

auto create_polygon_globject() -> GLObject* {
    gl = new PolygonGLObject();
    return gl;
}

auto do_draw(const GLenum mode, Screen* const screen, const std::vector<GLfloat>& buffer, const Color& color) -> void {
    const auto vabinder = gl->bind_vao();
    const auto shbinder = gl->use_shader();
    const auto vbbinder = gl->bind_vbo();
    const auto fbbinder = screen->prepare();

    gl->write_buffer(buffer);

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
