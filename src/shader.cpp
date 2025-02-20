#include <vector>

#include "macros/unwrap.hpp"
#include "shader.hpp"

namespace {
auto compile_shader(const uint32_t type, const char* const source) -> std::optional<GLuint> {
    auto status = GLint();
    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        auto max_len = GLint(0);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

        auto error_log = std::vector<GLchar>(max_len);
        glGetShaderInfoLog(shader, max_len, &max_len, error_log.data());
        bail("shader compile error: {}", std::string_view{error_log.data(), size_t(max_len)});
    }
    return shader;
}
} // namespace

namespace gawl::impl {
auto Shader::init(const char* vertex_shader_source, const char* fragment_shader_source) -> bool {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    unwrap(vertex_shader, compile_shader(GL_VERTEX_SHADER, vertex_shader_source));
    unwrap(fragment_shader, compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source));

    auto status    = GLint();
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    ensure(status == GL_TRUE);
    return true;
}

Shader::~Shader() {
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
}
} // namespace gawl::impl
