#pragma once
#include "graphic-shader.hpp"
#include "polygon-shader.hpp"
#include "textrender-shader.hpp"

namespace gawl::impl {
struct Shaders {
    GraphicShader    graphic_shader;
    TextRenderShader textrender_shader;
    PolygonShader    polygon_shader;

    auto init() -> bool;
};

inline auto global = (Shaders*)(nullptr);
} // namespace gawl::impl
