#include "globject.hpp"

namespace gawl::internal {
auto GLObject::bind_vao() const -> VArrayBinder {
    return vao;
}
auto GLObject::bind_vbo() const -> VertexBufferBinder {
    return vbo;
}
auto GLObject::bind_ebo() const -> ElementBufferBinder {
    return ebo;
}
auto GLObject::use_shader() const -> ShaderBinder {
    return shader_program;
}
auto GLObject::get_shader() const -> GLuint {
    return shader_program;
}
GLObject::GLObject(const char* const vertex_shader_source, const char* const fragment_shader_source) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    auto status = GLint();

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    ASSERT(status == GL_TRUE, "failed to load vertex shader")

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    ASSERT(status == GL_TRUE, "failed to load fragment shader")

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    ASSERT(status == GL_TRUE, "failed to link shaders")
    glBindFragDataLocation(shader_program, 0, "color");
}
GLObject::~GLObject() {
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
}
} // namespace gawl::internal
