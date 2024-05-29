#pragma once
#include "window-creat-hint.hpp"
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

    virtual auto create_window(const WindowCreateHint& hint, WindowCallbacks* callbacks) -> Window* = 0;
    virtual auto close_window_impl(Window* window) -> void                                          = 0;

  public:
    auto open_window(const WindowCreateHint& hint, WindowCallbacks* const callbacks) -> Window* {
        const auto ptr       = create_window(hint, callbacks);
        auto [lock, windows] = critical_windows.access();
        windows.emplace_back(ptr);
        callbacks->application = this;
        return ptr;
    }

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

    virtual auto quit() -> void = 0;

    virtual ~Application() {}
};
} // namespace gawl
