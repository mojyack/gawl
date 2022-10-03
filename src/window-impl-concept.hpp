#pragma once
#include <concepts>

#include <xkbcommon/xkbcommon.h>

#include "type.hpp"

namespace gawl::concepts {
template <class Impl, class Backend, class... Args>
concept WindowImpl = std::constructible_from<Impl, typename Backend::WindowCreateHintType&, Args&&...> && std::derived_from<Impl, Backend>;

template <class Impl>
concept WindowImplWithRefreshCallback = requires(Impl& m) {
    { m.refresh_callback() } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithWindowResizeCallback = requires(Impl& m) {
    { m.window_resize_callback() } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithKeycodeCallback = requires(Impl& m, uint32_t keycode, gawl::ButtonState state) {
    { m.keycode_callback(keycode, state) } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithKeysymCallback = requires(Impl& m, xkb_keycode_t keycode, gawl::ButtonState state, xkb_state* xkb_state) {
    { m.keysym_callback(keycode, state, xkb_state) } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithPointermoveCallback = requires(Impl& m) {
    { m.pointermove_callback(gawl::Point()) } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithClickCallback = requires(Impl& m) {
    { m.click_callback(uint32_t(), gawl::ButtonState()) } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithScrollCallback = requires(Impl& m) {
    { m.scroll_callback(gawl::WheelAxis(), double()) } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithCloseRequestCallback = requires(Impl& m) {
    { m.close_request_callback() } -> std::same_as<void>;
};

template <class Impl>
concept WindowImplWithUserCallback = requires(Impl& m) {
    { m.user_callback(nullptr) } -> std::same_as<void>;
};

} // namespace gawl::concepts

namespace gawl::internal {
template <template <class> class Concept, class T, class... Ts>
constexpr auto not_implemented() -> bool {
    if constexpr(!Concept<T>()) {
        if constexpr(sizeof...(Ts) == 0) {
            return true;
        } else {
            return not_implemented<Concept, Ts...>();
        }
    } else {
        return false;
    }
}

template <class... Impls>
struct Implement {
    template <class Impl>
    using HasRefreshCallback = typename std::conditional_t<gawl::concepts::WindowImplWithRefreshCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasWindowResizeCallback = typename std::conditional_t<gawl::concepts::WindowImplWithWindowResizeCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasKeycodeCallback = typename std::conditional_t<gawl::concepts::WindowImplWithKeycodeCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasKeysymCallback = typename std::conditional_t<gawl::concepts::WindowImplWithKeysymCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasPointermoveCallback = typename std::conditional_t<gawl::concepts::WindowImplWithPointermoveCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasClickCallback = typename std::conditional_t<gawl::concepts::WindowImplWithClickCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasScrollCallback = typename std::conditional_t<gawl::concepts::WindowImplWithScrollCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasCloseCallback = typename std::conditional_t<gawl::concepts::WindowImplWithCloseRequestCallback<Impl>, std::true_type, std::false_type>;

    template <class Impl>
    using HasUserCallback = typename std::conditional_t<gawl::concepts::WindowImplWithUserCallback<Impl>, std::true_type, std::false_type>;

    constexpr static auto keycode  = !not_implemented<HasKeycodeCallback, Impls...>();
    constexpr static auto keysym   = !not_implemented<HasKeysymCallback, Impls...>();
    constexpr static auto keyboard = keycode || keysym;
    constexpr static auto click    = !not_implemented<HasClickCallback, Impls...>();
    constexpr static auto motion   = !not_implemented<HasPointermoveCallback, Impls...>();
    constexpr static auto pointer  = click || motion;
};
} // namespace gawl::internal
