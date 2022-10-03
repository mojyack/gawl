#pragma once
#include <array>

#include "binder.hpp"
#include "internal-type.hpp"

namespace gawl {
class NullScreen {
  public:
    auto get_scale() const -> double {
        return 1.0;
    }

    auto get_viewport() const -> internal::Viewport {
        return {{0, 0}, {0, 0}};
    }
};
} // namespace gawl
