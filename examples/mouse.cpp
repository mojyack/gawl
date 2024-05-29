#include <linux/input.h>

#include "gawl/fc.hpp"
#include "gawl/misc.hpp"
#include "gawl/textrender.hpp"
#include "gawl/wayland/application.hpp"
#include "util/print.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    gawl::TextRender      font;
    gawl::Point           pointer;
    std::array<bool, 3>   click  = {false, false};
    std::array<double, 2> scroll = {0.0, 0.0};

  public:
    auto refresh() -> void override {
        const auto str = build_string(int(pointer.x), ",", int(pointer.y));

        gawl::clear_screen({0, 0, 0, 1});
        gawl::draw_rect(*window, {{pointer.x - 10 - scroll[1], pointer.y - 10 - scroll[0]}, {pointer.x + 10 + scroll[1], pointer.y + 10 + scroll[0]}}, {click[0] ? 0.0 : 1.0, click[1] ? 0.0 : 1.0, click[2] ? 0.0 : 1.0, 1});
        font.draw_fit_rect(*window, {pointer, {pointer.x + 100, pointer.y + 30}}, {1, 1, 1, 1}, str.data());
    }

    auto close() -> void override {
        application->quit();
    }

    auto on_pointer(const gawl::Point& point) -> void override {
        pointer = point;
        window->refresh();
    }

    auto on_click(const uint32_t button, const gawl::ButtonState state) -> void override {
        if(button < BTN_LEFT && button > BTN_MIDDLE) {
            return;
        }
        click[button - BTN_LEFT] = state == gawl::ButtonState::Press;
        window->refresh();
    }

    auto on_scroll(const gawl::WheelAxis axis, const double value) -> void override {
        auto& s = scroll[axis == gawl::WheelAxis::Horizontal];
        s += value;
        if(s < 0.0) {
            s = 0;
        }
    }

    Callbacks()
        : font({gawl::find_fontpath_from_name("Noto Sans CJK JP").value().data()}, 16) {}
};

auto main() -> int {
    auto app = gawl::WaylandApplication();
    app.open_window({.manual_refresh = true}, std::shared_ptr<Callbacks>(new Callbacks()));
    app.run();
    return 0;
}
