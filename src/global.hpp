#pragma once
#include "graphic-shader.hpp"
#include "polygon-shader.hpp"
#include "textrender-shader.hpp"

namespace gawl::impl {
struct Shaders {
    GraphicShader    graphic_shader;
    TextRenderShader textrender_shader;
    PolygonShader    polygon_shader;

    Shaders() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

inline auto global = (Shaders*)(nullptr);
} // namespace gawl::impl
