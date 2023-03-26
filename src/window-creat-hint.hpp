#pragma once
#include <cstddef>

namespace gawl {
struct WindowCreateHint {
    size_t      width          = 800;
    size_t      height         = 600;
    const char* title          = "Window";
    bool        manual_refresh = false;
};
} // namespace gawl::internal
