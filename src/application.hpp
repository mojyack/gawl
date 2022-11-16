#pragma once
#include <concepts>
#include <list>

#include "internal-type.hpp"
#include "util.hpp"

namespace gawl::internal {
template <class Backend, template <class, class...> class WindowBackend, class... Impls>
class Application {
  protected:
    std::list<Variant<WindowBackend<Impls, Impls...>...>> windows;

    auto backend() -> Backend* {
        return reinterpret_cast<Backend*>(this);
    }

    template <class Window>
    auto close_window_backend(Window& window) -> void {
        window.set_state(internal::WindowState::Destructing);
        backend()->close_window(window);
    }

    template <class Window>
    auto destroy_window(const Window& window) -> bool {
        static_assert(std::disjunction_v<std::is_same<Window, WindowBackend<Impls, Impls...>>...>);
        for(auto i = windows.begin(); i != windows.end(); i = std::next(i)) {
            const auto match = i->visit([&window](auto& w) -> bool {
                if constexpr(std::is_same_v<decltype(w), Window&>) {
                    return &w == &window;
                } else {
                    return false;
                }
            });
            if(match) {
                windows.erase(i);
                return windows.empty();
            }
        }
        panic("unknown window");
    }

  public:
    template <class Impl, class... Args>
    auto open_window(typename WindowBackend<Impl, Impls...>::WindowCreateHintType hint, Args&&... args) -> Impl& {
        static_assert(std::disjunction_v<std::is_same<Impl, Impls>...>);
        hint.backend_hint = backend()->get_shared_data();
        auto& impl        = windows.emplace_back(std::in_place_type<WindowBackend<Impl, Impls...>>, hint, std::forward<Args>(args)...);
        return impl.template get<WindowBackend<Impl, Impls...>>().get_impl();
    }

    auto close_all_windows() -> void {
        for(auto& w : windows) {
            w.visit([this](auto& w) { this->close_window_backend(w); });
        }
    }
};
} // namespace gawl::internal
