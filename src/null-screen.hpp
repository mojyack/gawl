#pragma once
#include "screen.hpp"

namespace gawl {
class NullScreen : MetaScreen {
  public:
    auto get_scale() const -> double override {
        return 1.0;
    }

    auto get_viewport() const -> const Viewport& override {
        static const auto vp = Viewport{{0, 0}, {0, 0}};
        return vp;
    }
};
} // namespace gawl
