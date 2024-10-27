#include <linux/input.h>

#include "window-no-touch-callbacks.hpp"

namespace gawl {
auto WindowNoTouchCallbacks::on_touch_down(const uint32_t id, const Point pos) -> coop::Async<bool> {
    if(id != 0) {
        co_return true;
    }
    co_await on_pointer(pos);
    co_await on_click(BTN_LEFT, gawl::ButtonState::Press);
    co_return true;
}

auto WindowNoTouchCallbacks::on_touch_motion(const uint32_t id, const Point pos) -> coop::Async<bool> {
    if(id != 0) {
        co_return true;
    }
    co_await on_pointer(pos);
    co_return true;
}

auto WindowNoTouchCallbacks::on_touch_up(const uint32_t id) -> coop::Async<bool> {
    if(id != 0) {
        co_return true;
    }
    co_await on_click(BTN_LEFT, gawl::ButtonState::Release);
    co_return true;
}
} // namespace gawl
