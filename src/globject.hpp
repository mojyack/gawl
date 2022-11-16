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

        auto status = GLint();

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
        dynamic_assert(status == GL_TRUE);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
        dynamic_assert(status == GL_TRUE);

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
        dynamic_assert(status == GL_TRUE);
        glBindFragDataLocation(shader_program, 0, "color");
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
