#pragma once
#include "color.hpp"
#include "graphic-shader.hpp"

namespace gawl::impl {
class TextRenderShader : public GraphicShader {
  private:
    Color color;

  public:
    void* freetype = nullptr; // FT_Library

    auto set_parameters(const GLuint shader) -> void override;

    auto set_text_color(const Color& text_color) -> void;
    auto init() -> bool;

    ~TextRenderShader();
};
} // namespace gawl::impl
