#include "global.hpp"
#include "macros/assert.hpp"

namespace gawl::impl {
auto Shaders::init() -> bool {
    ensure(graphic_shader.init());
    ensure(textrender_shader.init());
    ensure(polygon_shader.init());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return true;
}
} // namespace gawl::impl
