#pragma once
#include <cstdint>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "graphic-base.hpp"
#include "type.hpp"

namespace gawl {
struct GlobalVar {
    Shader*    graphic_shader;
    Shader*    textrender_shader;
    FT_Library freetype = nullptr;
    GlobalVar(const std::pair<GLuint, GLuint>& buffer);
    ~GlobalVar();
};
} // namespace gawl
