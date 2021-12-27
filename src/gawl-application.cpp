#include <algorithm>

#include "gawl-application.hpp"
#include "gawl-window.hpp"

namespace gawl {
auto GawlApplication::register_window(GawlWindow* const window) -> void {
    if(std::find(windows.begin(), windows.end(), window) == windows.end()) {
        windows.emplace_back(window);
    }
}
auto GawlApplication::unregister_window(const GawlWindow* window) -> bool {
    const auto w = std::find(windows.begin(), windows.end(), window);
    ASSERT(w != windows.end(), "invalid window handle");
    windows.erase(w);
    return !windows.empty();
}
auto GawlApplication::close_all_windows() -> void {
    for(auto w : windows) {
        close_window(*w);
    }
}
auto GawlApplication::run() -> void {
    for(auto& w : windows) {
        w->refresh();
    }
}
} // namespace gawl
