#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "screen.hpp"
#include "shader.hpp"
#include "type.hpp"

namespace gawl {
namespace internal {
auto init_graphics() -> std::pair<GLuint, GLuint>; // vbo, ebo
auto finish_graphics() -> void;
} // namespace internal
class GraphicBase {
  private:
    GLuint texture;
    auto   do_draw(Screen* screen) const -> void;

  protected:
    internal::Shader& type_specific;
    int               width, height;
    bool              invert_top_bottom = false;
    auto              get_texture() const -> GLuint;

  public:
    virtual auto get_width(const Screen* screen) const -> int;
    virtual auto get_height(const Screen* screen) const -> int;
    auto         draw(Screen* screen, const Point& point) const -> void;
    auto         draw_rect(Screen* screen, const Rectangle& rect) const -> void;
    auto         draw_fit_rect(Screen* screen, const Rectangle& rect) const -> void;
    auto         draw_transformed(Screen* screen, const std::array<Point, 4>& vertices) const -> void;
    GraphicBase(internal::Shader& type_specific);
    virtual ~GraphicBase();
};
} // namespace gawl
