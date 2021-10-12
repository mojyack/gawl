#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "binder.hpp"

namespace gawl::internal {
class Shader {
  private:
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    GLuint vao;

  public:
    [[nodiscard]] auto bind_vao() -> VArrayBinder;
    [[nodiscard]] auto use_shader() -> ShaderBinder;
    auto               get_shader() const -> GLuint;
    Shader(const char* vertex_shader_source, const char* fragment_shader_source, GLuint vbo, GLuint ebo, bool has_texture);
    ~Shader();
};
} // namespace gawl::internal
