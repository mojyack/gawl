#include <gawl/wayland/gawl.hpp>

#include "fc.hpp"

class Impl;

using Gawl = gawl::Gawl<Impl>;

class Impl {
  private:
    Gawl::Window<Impl>&   window;
    gawl::TextRender      font;
    gawl::Point           pointer;
    std::array<bool, 3>   click  = {false, false};
    std::array<double, 2> scroll = {0.0, 0.0};

    template <class... Args>
    auto build_string(Args... args) -> std::string {
        auto ss = std::stringstream();
        (ss << ... << args);
        return ss.str();
    }

  public:
    auto refresh_callback() -> void {
        gawl::clear_screen({0, 0, 0, 1});
        gawl::draw_rect(window, {{pointer.x - 10 - scroll[1], pointer.y - 10 - scroll[0]}, {pointer.x + 10 + scroll[1], pointer.y + 10 + scroll[0]}}, {click[0] ? 0.0 : 1.0, click[1] ? 0.0 : 1.0, click[2] ? 0.0 : 1.0, 1});
        font.draw_fit_rect(window, {pointer, {pointer.x + 100, pointer.y + 30}}, {1, 1, 1, 1}, build_string(static_cast<int>(pointer.x), ",", static_cast<int>(pointer.y)).data());
    }

    auto pointermove_callback(const gawl::Point& point) -> void {
        pointer = point;
        window.refresh();
    }

    auto click_callback(const uint32_t button, const gawl::ButtonState state) -> void {
        if(button < BTN_LEFT && button > BTN_MIDDLE) {
            return;
        }
        click[button - BTN_LEFT] = state == gawl::ButtonState::Press;
        window.refresh();
    }

    auto scroll_callback(const gawl::WheelAxis axis, const double value) -> void {
        auto& s = scroll[axis == gawl::WheelAxis::Horizontal];
        s += value;
        if(s < 0.0) {
            s = 0;
        }
    }

    Impl(Gawl::Window<Impl>& window) : window(window), font({fc::find_fontpath_from_name("Noto Sans CJK JP").data()}, 16) {}
};

auto main() -> int {
    auto app = Gawl::Application();
    app.open_window<Impl>({.manual_refresh = false});
    app.run();
    return 0;
}
