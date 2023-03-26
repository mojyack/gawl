#pragma once
#include <concepts>
#include <list>

#include "internal-type.hpp"
#include "util.hpp"
#include "window.hpp"

namespace gawl::internal {
class Application {
  protected:
    Critical<std::vector<std::unique_ptr<Window>>> critical_windows;

    virtual auto backend_get_window_create_hint() -> void* = 0;

    virtual auto backend_close_window(Window* window) -> void = 0;

    auto erase_window(const Window* const window) -> void {
        auto [lock, windows] = critical_windows.access();
        for(auto i = windows.begin(); i < windows.end(); i += 1) {
            if(i->get() == window) {
                windows.erase(i);
                return;
            }
        }
    }

  public:
    // open_window is provided by backend class

    auto close_window(Window* const window) -> void {
        window->set_state(WindowState::Destructing);
        backend_close_window(window);
    }

    auto close_all_windows() -> void {
        auto [lock, windows] = critical_windows.access();
        for(const auto& w : windows) {
            close_window(w.get());
        }
    }

    virtual ~Application() {}
};
} // namespace gawl::internal
