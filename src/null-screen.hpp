#pragma once
#include <array>

#include "binder.hpp"

namespace gawl {
class NullScreen {
  public:
    auto get_scale() const -> double {
        return 1.0;
    }
    auto get_screen_size() const -> std::array<size_t, 2> {
        return {0, 0};
    }
};
} // namespace gawl
