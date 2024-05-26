#include "graphic-shader.hpp"
#include "misc.hpp"

namespace gawl::impl {
auto GraphicShader::move_vertices(const Screen& screen, const Rectangle& rect, const bool invert) -> void {
    auto r = rect;
    convert_screen_to_viewport(screen, r);
    vertices[0][0]      = r.a.x;
    vertices[0][1]      = invert ? r.b.y : r.a.y;
    vertices[1][0]      = r.b.x;
    vertices[1][1]      = invert ? r.b.y : r.a.y;
    vertices[2][0]      = r.b.x;
    vertices[2][1]      = invert ? r.a.y : r.b.y;
    vertices[3][0]      = r.a.x;
    vertices[3][1]      = invert ? r.a.y : r.b.y;
    const auto vbbinder = bind_vbo();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

auto GraphicShader::move_vertices(const Screen& screen, const std::array<Point, 4>& points, const bool invert) -> void {
    auto v = points;
    gawl::convert_screen_to_viewport(screen, v);
    vertices[0][0]      = v[0].x;
    vertices[0][1]      = invert ? v[3].y : v[0].y;
    vertices[1][0]      = v[1].x;
    vertices[1][1]      = invert ? v[2].y : v[1].y;
    vertices[2][0]      = v[2].x;
    vertices[2][1]      = invert ? v[1].y : v[2].y;
    vertices[3][0]      = v[3].x;
    vertices[3][1]      = invert ? v[0].y : v[3].y;
    const auto vbbinder = bind_vbo();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

GraphicShader::GraphicShader(const char* const vertex_shader_source, const char* const fragment_shader_source)
    : Shader(vertex_shader_source, fragment_shader_source) {
    const auto vabinder = bind_vao();
    const auto vbbinder = bind_vbo();
    const auto ebbinder = bind_ebo();

    const auto pos_attrib = glGetAttribLocation(shader_program, "position");
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
    glEnableVertexAttribArray(pos_attrib);

    const auto tex_attrib = glGetAttribLocation(shader_program, "texcoord");
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(tex_attrib);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);
    const static GLuint elements[] = {
        0, 1, 2,
        2, 3, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    vertices[0][2] = 0.0;
    vertices[0][3] = 0.0;
    vertices[1][2] = 1.0;
    vertices[1][3] = 0.0;
    vertices[2][2] = 1.0;
    vertices[2][3] = 1.0;
    vertices[3][2] = 0.0;
    vertices[3][3] = 1.0;
}
} // namespace gawl::impl
