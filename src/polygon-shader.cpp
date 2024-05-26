#include "polygon-shader.hpp"
#include "shader-source.hpp"

namespace gawl::impl {
auto PolygonShader::write_buffer(const std::vector<GLfloat>& buffer) -> void {
    const auto copy_size = buffer.size() * sizeof(GLfloat);
    if(vbo_capacity < copy_size) {
        glBufferData(GL_ARRAY_BUFFER, copy_size, buffer.data(), GL_DYNAMIC_DRAW);
        vbo_capacity = copy_size;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, copy_size, buffer.data());
    }
}

PolygonShader::PolygonShader() : Shader(polygon_vertex_shader_source, polygon_fragment_shader_source) {
    const auto vabinder = bind_vao();
    const auto vbbinder = bind_vbo();
    const auto ebbinder = bind_ebo();

    const auto pos_attrib = glGetAttribLocation(shader_program, "position");
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
    glEnableVertexAttribArray(pos_attrib);
}
} // namespace gawl::impl
