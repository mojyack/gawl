#pragma once
#include "graphic-globject.hpp"
#include "polygon-globject.hpp"
#include "shader-source.hpp"
#include "textrender-globject.hpp"

namespace gawl::internal {
struct GLObjects {
    internal::GraphicGLObject    graphic_shader;
    internal::TextRenderGLObject textrender_shader;
    internal::PolygonGLObject    polygon_shader;
    GLObjects() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

struct ProcessUniqueObject {
    GLObjects* gl = nullptr;
};
extern internal::ProcessUniqueObject global;
} // namespace gawl::internal
