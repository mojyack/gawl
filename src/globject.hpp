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
    auto         bind_vao() const -> VArrayBinder;
    auto         bind_vbo() const -> VertexBufferBinder;
    auto         bind_ebo() const -> ElementBufferBinder;
    auto         use_shader() const -> ShaderBinder;
    auto         get_shader() const -> GLuint;
    virtual auto set_shader_parameters([[maybe_unused]] GLuint shader) -> void{};
    GLObject(const char* vertex_shader_source, const char* fragment_shader_source);
    virtual ~GLObject();
};
} // namespace gawl::internal
