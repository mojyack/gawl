#pragma once
#include <cstdint>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "graphic-base.hpp"
#include "type.hpp"

namespace gawl::internal {
struct GlobalVar {
    Shader*    graphic_shader;
    Shader*    textrender_shader;
    Shader*    polygon_shader;
    FT_Library freetype = nullptr;
    GlobalVar(const std::pair<GLuint, GLuint>& buffer, const std::pair<GLuint, GLuint>& polygon_buffer);
    ~GlobalVar();
};
} // namespace gawl::internal
