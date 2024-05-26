#pragma once
#include "screen.hpp"
#include "shader-source.hpp"
#include "shader.hpp"

namespace gawl::impl {
class GraphicShader : public Shader {
  private:
    GLfloat vertices[4][4];

  public:
    auto move_vertices(const Screen& screen, const Rectangle& rect, bool invert) -> void;
    auto move_vertices(const Screen& screen, const std::array<Point, 4>& points, bool invert) -> void;

    virtual auto set_parameters(GLuint /*param*/) -> void {}

    GraphicShader(const char* vertex_shader_source = graphic_vertex_shader_source, const char* fragment_shader_source = graphic_fragment_shader_source);
    virtual ~GraphicShader(){};
};
}; // namespace gawl::impl
