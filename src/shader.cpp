#include "shader.hpp"
#include "error.hpp"

namespace gawl::internal {
auto Shader::bind_vao() -> VArrayBinder {
    return VArrayBinder(vao);
}
auto Shader::use_shader() -> ShaderBinder {
    return ShaderBinder(shader_program);
}
auto Shader::get_shader() const -> GLuint {
    return shader_program;
}
Shader::Shader(const char* const vertex_shader_source, const char* const fragment_shader_source, const GLuint vbo, const GLuint ebo, const bool has_texture) {
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

    glGenVertexArrays(1, &vao);
    auto vabinder = VArrayBinder(vao);
    auto vbbinder = VertexBufferBinder(vbo);
    auto ebbinder = ElementBufferBinder(ebo);

    const auto pos_attrib = glGetAttribLocation(shader_program, "position");
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * (has_texture ? 4 : 2), 0);
    glEnableVertexAttribArray(pos_attrib);

    if(has_texture) {
        const auto tex_attrib = glGetAttribLocation(shader_program, "texcoord");
        glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLfloat) * 2));
        glEnableVertexAttribArray(tex_attrib);
    }
}
Shader::~Shader() {
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
}
} // namespace gawl::internal
