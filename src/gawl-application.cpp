#include <algorithm>

#include "gawl-application.hpp"
#include "gawl-window.hpp"

namespace gawl {
auto GawlApplication::register_window(GawlWindow* const window) -> void {
    if(std::find(windows.begin(), windows.end(), window) == windows.end()) {
        windows.emplace_back(window);
    }
}
auto GawlApplication::get_windows() const -> const std::vector<GawlWindow*>& {
    return windows;
}
auto GawlApplication::unregister_window(const GawlWindow* window) -> void {
    if(auto w = std::find(windows.begin(), windows.end(), window); w != windows.end()) {
        windows.erase(w);
    }
}
auto GawlApplication::close_all_windows() -> void {
    for(auto w : windows) {
        close_window(w);
    }
}
auto GawlApplication::close_window(GawlWindow* window) -> void {
    if(auto w = std::find(windows.begin(), windows.end(), window); w != windows.end()) {
        (*w)->status = GawlWindow::Status::CLOSE;
        tell_event(nullptr);
    }
}
} // namespace gawl
