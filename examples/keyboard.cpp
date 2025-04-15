#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"

#include <iostream>

class Callbacks : public gawl::WindowCallbacks {
  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
    }

    auto close() -> void override {
        application->quit();
    }

    auto on_keycode(const uint32_t keycode, const gawl::ButtonState state) -> coop::Async<bool> override {
        std::cout << keycode << " " << static_cast<int>(state) << std::endl;
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
    return 0;
}
