#pragma once
#include "window.hpp"

namespace gawl {
class Application {
  protected:
    Critical<std::vector<std::unique_ptr<Window>>> critical_windows;

    auto erase_window(const Window* const window) -> void {
        auto [lock, windows] = critical_windows.access();
        for(auto i = windows.begin(); i < windows.end(); i += 1) {
            if(i->get() == window) {
                windows.erase(i);
                return;
            }
        }
    }

    virtual auto close_window_impl(Window* window) -> void = 0;

  public:
    // open_window is provided by backend class

    auto close_window(Window* const window) -> void {
        window->set_state(impl::WindowState::Destructing);
        close_window_impl(window);
    }

    auto close_all_windows() -> void {
        auto [lock, windows] = critical_windows.access();
        for(const auto& w : windows) {
            close_window(w.get());
        }
    }

    virtual ~Application() {}
};
} // namespace gawl
