#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "frame-buffer-info.hpp"
#include "type.hpp"

namespace gawl {
auto init_graphics() -> void;
auto finish_graphics() -> void;

class Shader {
  private:
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    GLuint vao;

  public:
    auto bind_vao() -> void;
    auto get_shader() -> GLuint;
    Shader(const char* vertex_shader_source, const char* fragment_shader_source);
    ~Shader();
};

class GawlWindow;
class GraphicBase {
  private:
    GLuint texture;

  protected:
    Shader& type_specific;
    int     width, height;
    bool    invert_top_bottom = false;
    auto    get_texture() const -> GLuint;

  public:
    virtual auto get_width(FrameBufferInfo info) const -> int;
    virtual auto get_height(FrameBufferInfo info) const -> int;
    auto         draw(FrameBufferInfo infoconst, double x, double y) const -> void;
    auto         draw_rect(FrameBufferInfo infoconst, Area area) const -> void;
    auto         draw_fit_rect(FrameBufferInfo info, Area area) const -> void;
    GraphicBase(Shader& type_specific);
    virtual ~GraphicBase();
};
} // namespace gawl
