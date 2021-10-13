#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "binder.hpp"

namespace gawl::internal {
class GLObject {
  private:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;

  public:
    [[nodiscard]] auto bind_vao() const -> VArrayBinder;
    [[nodiscard]] auto bind_vbo() const -> VertexBufferBinder;
    [[nodiscard]] auto bind_ebo() const -> ElementBufferBinder;
    [[nodiscard]] auto use_shader() const -> ShaderBinder;
    auto               get_shader() const -> GLuint;
    virtual auto       set_shader_parameters([[maybe_unused]] GLuint shader) -> void{};
    GLObject(const char* vertex_shader_source, const char* fragment_shader_source, bool has_texture);
    virtual ~GLObject();
};
} // namespace gawl::internal
