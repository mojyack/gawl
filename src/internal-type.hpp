#pragma once
#include <array>

namespace gawl::internal {
enum class WindowState {
    Constructing,
    Running,
    Destructing,
};

struct Viewport {
    std::array<size_t, 2> base;
    std::array<size_t, 2> size;
};
} // namespace gawl::internal
