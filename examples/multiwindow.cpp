#include <linux/input.h>

#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"

class Callbacks : public gawl::WindowCallbacks {
  protected:
    static inline int windows = 0;

    int count = 0;
    int color = 0;

  public:
    auto refresh() -> void override {
        const auto v = count % 60 < 30 ? (count % 60) / 30.0 : (30 - count % 30) / 30.0;
        const auto r = color % 3 == 0 ? v : 0;
        const auto g = color % 3 == 1 ? v : 0;
        const auto b = color % 3 == 2 ? v : 0;
        gawl::clear_screen({r, g, b, 1});
        count += 1;
    }

    auto on_keycode(const uint32_t key, const gawl::ButtonState state) -> coop::Async<bool> override {
        if(state != gawl::ButtonState::Press) {
            co_return true;
        }
        switch(key) {
        case KEY_N:
            co_await application->open_window({}, std::shared_ptr<Callbacks>(new Callbacks(color + 1)));
            break;
        case KEY_M:
            application->close_window(window);
            break;
        case KEY_Q:
            application->quit();
            break;
        }
        co_return true;
    }

    Callbacks(const int color) : color(color) {
        windows += 1;
    }

    ~Callbacks() {
        windows -= 1;
        if(windows == 0) {
            application->quit();
        }
    }
};

auto main() -> int {
    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<Callbacks>(new Callbacks(0));
    runner.push_task(app.run());
    runner.push_task(app.open_window({}, std::move(cbs)));
    runner.run();
}
