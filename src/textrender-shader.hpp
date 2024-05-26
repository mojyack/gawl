#pragma once
#include "color.hpp"
#include "graphic-shader.hpp"

namespace gawl::impl {
class TextRenderShader : public GraphicShader {
  private:
    Color color;

  public:
    void* freetype = nullptr; // FT_Library

    auto set_text_color(const Color& text_color) -> void;
    auto set_shader_parameters(const GLuint shader) -> void;

    TextRenderShader();
    ~TextRenderShader();
};
} // namespace gawl::impl
