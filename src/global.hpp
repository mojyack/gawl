#pragma once
#include "graphic-globject.hpp"
#include "polygon-globject.hpp"
#include "shader-source.hpp"
#include "textrender-globject.hpp"

namespace gawl::internal {
struct GLObjects {
    GraphicGLObject    graphic_shader;
    TextRenderGLObject textrender_shader;
    PolygonGLObject    polygon_shader;

    GLObjects() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

inline GLObjects* global = nullptr;
} // namespace gawl::internal
