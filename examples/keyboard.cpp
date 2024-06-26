#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"

#include <iostream>

class Callbacks : public gawl::WindowCallbacks {
  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
    }

    auto on_keycode(const uint32_t keycode, const gawl::ButtonState state) -> void override {
        std::cout << keycode << " " << static_cast<int>(state) << std::endl;
    }

    auto close() -> void override {
        application->quit();
    }
};

auto main() -> int {
    auto app = gawl::WaylandApplication();
    app.open_window({.manual_refresh = true}, std::shared_ptr<Callbacks>(new Callbacks()));
    app.run();
    return 0;
}
