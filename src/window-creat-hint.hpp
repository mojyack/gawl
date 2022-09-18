#pragma once
#include <cstddef>

namespace gawl::internal {
template <class BackendHint>
struct WindowCreateHint {
    size_t      width          = 800;
    size_t      height         = 600;
    const char* title          = "Window";
    bool        manual_refresh = false;

    // internal use
    BackendHint backend_hint = BackendHint{};
};
} // namespace gawl::internal
