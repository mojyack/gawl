#pragma once
#include <concepts>
#include <list>

#include "internal-type.hpp"
#include "util.hpp"

namespace gawl::internal {
template <class Backend, class... Impls>
class Application {
  protected:
    std::list<Variant<Impls...>> windows;

    auto backend() -> Backend* {
        return reinterpret_cast<Backend*>(this);
    }

  public:
    template <class Impl, class... Args>
    auto open_window(typename Impl::WindowCreateHintType hint, Args&&... args) -> void {
        static_assert(std::disjunction_v<std::is_same<Impl, Impls>...>);
        hint.backend_hint = backend()->get_shared_data();
        windows.emplace_back(std::in_place_type<Impl>, hint, args...);
    }
    template <class Impl>
    auto destroy_window(const Impl& window) -> bool {
        for(auto i = windows.begin(); i != windows.end(); i = std::next(i)) {
            const auto match = std::visit([&window](auto& w) -> bool { if constexpr(std::is_same_v<decltype(w), Impl&>) return &w == &window; return false; }, i->as_variant());
            if(match) {
                windows.erase(i);
                return windows.empty();
            }
        }
        panic("unknown window");
    }
    template <class Impl>
    auto close_window(Impl& window) -> void {
        static_assert(std::disjunction_v<std::is_same<Impl, Impls>...>);
        window.set_state(internal::WindowState::Destructing);
        backend()->close_window(window);
    }
    auto close_all_windows() -> void {
        for(auto& w : windows) {
            std::visit([this](auto& w) { this->close_window(w); }, w.as_variant());
        }
    }
};
} // namespace gawl::internal
