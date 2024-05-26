#pragma once
#include "binder.hpp"

namespace gawl::impl {
class Shader {
  protected:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;

    static auto compile_shader(uint32_t type, const char* source) -> GLuint;

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

    Shader(const char* vertex_shader_source, const char* fragment_shader_source);
    ~Shader();
};
} // namespace gawl::impl
