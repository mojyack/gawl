#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H

#include "graphic-globject.hpp"

namespace gawl::internal {
class TextRenderGLObject : public GraphicGLObject {
  private:
    Color color;

  public:
    FT_Library freetype = nullptr;

    auto set_text_color(const Color& text_color) {
        color = text_color;
    }

    auto set_shader_parameters(const GLuint shader) -> void {
        const auto l = glGetUniformLocation(shader, "text_color");
        glUniform4f(l, color[0], color[1], color[2], color[3]);
    };

    TextRenderGLObject() : GraphicGLObject(textrender_vertex_shader_source, textrender_fragment_shader_source) {
        FT_Init_FreeType(&freetype);
    }

    ~TextRenderGLObject() {
        FT_Done_FreeType(freetype);
    }
};
} // namespace gawl::internal
