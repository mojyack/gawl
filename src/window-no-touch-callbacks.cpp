#include <linux/input.h>

#include "window-no-touch-callbacks.hpp"

namespace gawl {
auto WindowNoTouchCallbacks::on_touch_down(const uint32_t id, const Point& pos) -> void {
    if(id != 0) {
        return;
    }
    on_pointer(pos);
    on_click(BTN_LEFT, gawl::ButtonState::Press);
}

auto WindowNoTouchCallbacks::on_touch_motion(const uint32_t id, const Point& pos) -> void {
    if(id != 0) {
        return;
    }
    on_pointer(pos);
}

auto WindowNoTouchCallbacks::on_touch_up(const uint32_t id) -> void {
    if(id != 0) {
        return;
    }
    on_click(BTN_LEFT, gawl::ButtonState::Release);
}
} // namespace gawl
