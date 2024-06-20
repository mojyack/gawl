#pragma once
#include "window-callbacks.hpp"

namespace gawl {
// touch to mouse emulation
class WindowNoTouchCallbacks : public WindowCallbacks {
  public:
    auto on_touch_down(uint32_t id, const Point& pos) -> void override;
    auto on_touch_motion(uint32_t id, const Point& pos) -> void override;
    auto on_touch_up(uint32_t id) -> void override;

    virtual ~WindowNoTouchCallbacks(){};
};
} // namespace gawl
