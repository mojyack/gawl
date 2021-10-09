#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace gawl::internal {
class Shader {
  private:
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    GLuint vao;

  public:
    auto bind_vao() -> void;
    auto get_shader() -> GLuint;
    Shader(const char* vertex_shader_source, const char* fragment_shader_source, GLuint vbo, GLuint ebo, bool has_texture);
    ~Shader();
};
}
