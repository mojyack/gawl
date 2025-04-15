#include <linux/input.h>

#include "gawl/fc.hpp"
#include "gawl/misc.hpp"
#include "gawl/textrender.hpp"
#include "gawl/wayland/application.hpp"
#include "gawl/window-no-touch-callbacks.hpp"
#include "macros/unwrap.hpp"

class Callbacks : public gawl::WindowNoTouchCallbacks {
  private:
    constexpr static auto error_value = false;

    gawl::TextRender      font;
    gawl::Point           pointer;
    std::array<bool, 3>   click  = {false, false};
    std::array<double, 2> scroll = {0.0, 0.0};

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
        gawl::draw_rect(*window, {{pointer.x - 10 - scroll[1], pointer.y - 10 - scroll[0]}, {pointer.x + 10 + scroll[1], pointer.y + 10 + scroll[0]}}, {click[0] ? 0.0 : 1.0, click[1] ? 0.0 : 1.0, click[2] ? 0.0 : 1.0, 1});
        font.draw_fit_rect(*window, {pointer, {pointer.x + 100, pointer.y + 30}}, {1, 1, 1, 1}, std::format("{:.4},{:.4}", pointer.x, pointer.y));
    }

    auto close() -> void override {
        application->quit();
    }

    auto on_created(gawl::Window* /*window*/) -> coop::Async<bool> override {
        co_unwrap_v_mut(fontpath, gawl::find_fontpath_from_name("Noto Sans CJK JP"));
        font.init({std::move(fontpath)}, 16);
        co_return true;
    }

    auto on_pointer(const gawl::Point point) -> coop::Async<bool> override {
        std::println("point {:.4} {:.4}", point.x, point.y);
        pointer = point;
        window->refresh();
        co_return true;
    }

    auto on_click(const uint32_t button, const gawl::ButtonState state) -> coop::Async<bool> override {
        if(button < BTN_LEFT && button > BTN_MIDDLE) {
            co_return true;
        }
        click[button - BTN_LEFT] = state == gawl::ButtonState::Press;
        co_ensure_v(window->refresh());
        co_return true;
    }

    auto on_scroll(const gawl::WheelAxis axis, const double value) -> coop::Async<bool> override {
        auto& s = scroll[axis == gawl::WheelAxis::Horizontal];
        s += value;
        if(s < 0.0) {
            s = 0;
        }
        co_ensure_v(window->refresh());
        co_return true;
    }
};

auto main() -> int {
    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<Callbacks>(new Callbacks());
    runner.push_task(app.run());
    runner.push_task(app.open_window({.manual_refresh = true}, std::move(cbs)));
    runner.run();
}
