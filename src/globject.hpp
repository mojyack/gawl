#pragma once
#include "binder.hpp"

namespace gawl::internal {
class GLObject {
  protected:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;

    static auto compile_shader(const uint32_t type, const char* const source) -> GLuint {
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
            printf("shader compile error: %s\n", error_log.data());
            exit(1);
        }
        return shader;
    }

  public:
    auto bind_vao() const -> VArrayBinder {
        return vao;
    }

    auto bind_vbo() const -> VertexBufferBinder {
        return vbo;
    }

    auto bind_ebo() const -> ElementBufferBinder {
        return ebo;
    }

    auto use_shader() const -> ShaderBinder {
        return shader_program;
    }

    auto get_shader() const -> GLuint {
        return shader_program;
    }

    GLObject(const char* vertex_shader_source, const char* fragment_shader_source) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        vertex_shader   = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
        fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

        auto status = GLint();
        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
        dynamic_assert(status == GL_TRUE);
    }

    ~GLObject() {
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(shader_program);
        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);
    }
};

namespace concepts {
template <class G>
concept GLObjectWithParameter = requires(G& m) {
                                    { m.set_shader_parameters(GLuint()) } -> std::same_as<void>;
                                } && std::derived_from<G, GLObject>;
} // namespace concepts
} // namespace gawl::internal
