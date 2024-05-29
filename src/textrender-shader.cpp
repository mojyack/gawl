#include <ft2build.h>
#include FT_FREETYPE_H

#include "textrender-shader.hpp"

namespace gawl::impl {
auto TextRenderShader::set_text_color(const Color& text_color) -> void {
    color = text_color;
}

auto TextRenderShader::set_parameters(const GLuint shader) -> void {
    const auto l = glGetUniformLocation(shader, "text_color");
    glUniform4f(l, color[0], color[1], color[2], color[3]);
};

TextRenderShader::TextRenderShader() : GraphicShader(textrender_vertex_shader_source, textrender_fragment_shader_source) {
    FT_Init_FreeType(std::bit_cast<FT_Library*>(&freetype));
}

TextRenderShader::~TextRenderShader() {
    FT_Done_FreeType(std::bit_cast<FT_Library>(freetype));
}
} // namespace gawl::impl
