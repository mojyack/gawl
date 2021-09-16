#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "screen.hpp"
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
    virtual auto get_width(const Screen* screen) const -> int;
    virtual auto get_height(const Screen* screen) const -> int;
    auto         draw(Screen* screen, double x, double y) const -> void;
    auto         draw_rect(Screen* screen, Area area) const -> void;
    auto         draw_fit_rect(Screen* screen, Area area) const -> void;
    GraphicBase(Shader& type_specific);
    virtual ~GraphicBase();
};
} // namespace gawl
