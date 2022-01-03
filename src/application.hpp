#pragma once
#include <concepts>
#include <list>

#include "error.hpp"
#include "internal-type.hpp"

namespace gawl {
template <class Impl, class Backend>
class Application {
  private:
    Backend&        backend;
    std::list<Impl> windows;

  public:
    template <class... Args>
    auto open_window(typename Impl::WindowCreateHintType hint, Args&&... args) -> void {
        hint.backend_hint = backend.get_shared_data();
        windows.emplace_back(hint, args...);
    }
    auto destroy_window(void* const window) -> bool {
        for(auto i = windows.begin(); i != windows.end(); i = std::next(i)) {
            if(&(*i) == window) {
                windows.erase(i);
                return windows.empty();
            }
        }
        panic("unknown window");
    }
    auto close_window(Impl& window) -> void {
        window.set_state(internal::WindowState::Destructing);
        backend.close_window(window);
    }
    auto close_all_windows() -> void {
        for(auto& w : windows) {
            backend.close_window(w);
        }
    }
    Application(Backend& backend) : backend(backend) {}
};
} // namespace gawl
