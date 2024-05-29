#include "application.hpp"

namespace gawl {
auto Application::erase_window(const Window* const window) -> void {
    auto [lock, windows] = critical_windows.access();
    for(auto i = windows.begin(); i < windows.end(); i += 1) {
        if(i->get() == window) {
            windows.erase(i);
            return;
        }
    }
}

auto Application::open_window(const WindowCreateHint& hint, WindowCallbacks* const callbacks) -> Window* {
    const auto ptr       = create_window(hint, callbacks);
    auto [lock, windows] = critical_windows.access();
    windows.emplace_back(ptr);
    callbacks->application = this;
    return ptr;
}

auto Application::close_window(Window* const window) -> void {
    window->set_state(impl::WindowState::Destructing);
    close_window_impl(window);
}

auto Application::close_all_windows() -> void {
    auto [lock, windows] = critical_windows.access();
    for(const auto& w : windows) {
        close_window(w.get());
    }
}
} // namespace gawl
