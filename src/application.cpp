#include <coop/promise.hpp>
#include <coop/runner.hpp>

#include "application.hpp"

namespace gawl {
auto Application::erase_window(const Window* const window) -> bool {
    for(auto i = windows.begin(); i < windows.end(); i += 1) {
        if(i->get() == window) {
            windows.erase(i);
            return true;
        }
    }
    return false;
}

auto Application::open_window(const WindowCreateHint hint, std::shared_ptr<WindowCallbacks> callbacks) -> coop::Async<Window*> {
    callbacks->application = this;
    co_return co_await create_window(hint, std::move(callbacks));
}

auto Application::close_window(Window* const window) -> void {
    closing_windows.push_back(window);
}

auto Application::close_all_windows() -> void {
    for(const auto& w : windows) {
        close_window(w.get());
    }
}
} // namespace gawl
