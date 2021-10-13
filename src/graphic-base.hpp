#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "globject.hpp"
#include "screen.hpp"
#include "type.hpp"

namespace gawl::internal {
class GraphicGLObject : public GLObject {
  private:
    GLfloat vertices[4][4];

  public:
    auto move_vertices(const Screen* screen, const Rectangle& rect, bool invert) -> void;
    auto move_vertices(const Screen* screen, const std::array<Point, 4>& points, bool invert) -> void;
    GraphicGLObject(const char* vertex_shader_source, const char* fragment_shader_source);
};
class GraphicBase {
  private:
    GLuint           texture;
    GraphicGLObject* gl;

    auto do_draw(Screen* screen) const -> void;

  protected:
    int  width, height;
    bool invert_top_bottom = false;
    auto get_texture() const -> GLuint;
    auto bind_texture() const -> TextureBinder;

  public:
    virtual auto get_width(const Screen* screen) const -> int;
    virtual auto get_height(const Screen* screen) const -> int;
    auto         draw(Screen* screen, const Point& point) -> void;
    auto         draw_rect(Screen* screen, const Rectangle& rect) -> void;
    auto         draw_fit_rect(Screen* screen, const Rectangle& rect) -> void;
    auto         draw_transformed(Screen* screen, const std::array<Point, 4>& vertices) -> void;
    GraphicBase(GraphicGLObject* gl);
    virtual ~GraphicBase();
};
} // namespace gawl::internal
