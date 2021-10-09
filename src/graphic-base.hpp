#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "screen.hpp"
#include "shader.hpp"
#include "type.hpp"

namespace gawl {
auto init_graphics() -> std::pair<GLuint, GLuint>; // vbo, ebo
auto finish_graphics() -> void;
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
    auto         draw(Screen* screen, const Point& point) const -> void;
    auto         draw_rect(Screen* screen, const Rectangle& rect) const -> void;
    auto         draw_fit_rect(Screen* screen, const Rectangle& rect) const -> void;
    GraphicBase(Shader& type_specific);
    virtual ~GraphicBase();
};
} // namespace gawl
