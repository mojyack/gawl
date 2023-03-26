#pragma once
#include <concepts>

#include <xkbcommon/xkbcommon.h>

#include "type.hpp"

namespace gawl::concepts {
namespace internal {
template <class t>
concept IsComplete = requires() {
                         { sizeof(t) };
                     };

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
concept WindowImplWithPointerMoveCallback = requires(Impl& m) {
                                                { m.pointer_move_callback(gawl::Point()) } -> std::same_as<void>;
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

} // namespace internal

template <class Impl>
consteval auto has_refresh_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithRefreshCallback<Impl>;
}

template <class Impl>
consteval auto has_window_resize_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithWindowResizeCallback<Impl>;
}

template <class Impl>
consteval auto has_keycode_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithKeycodeCallback<Impl>;
}

template <class Impl>
consteval auto has_keysym_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithKeysymCallback<Impl>;
}

template <class Impl>
consteval auto has_pointer_move_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithPointerMoveCallback<Impl>;
}

template <class Impl>
consteval auto has_click_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithClickCallback<Impl>;
}

template <class Impl>
consteval auto has_scroll_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithScrollCallback<Impl>;
}

template <class Impl>
consteval auto has_close_request_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithCloseRequestCallback<Impl>;
}

template <class Impl>
consteval auto has_user_callback() -> bool {
    static_assert(internal::IsComplete<Impl>, "incomplete type passed");
    return internal::WindowImplWithUserCallback<Impl>;
}
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
    using HasRefreshCallback = typename std::conditional_t<gawl::concepts::has_refresh_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasWindowResizeCallback = typename std::conditional_t<gawl::concepts::has_window_resize_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasKeycodeCallback = typename std::conditional_t<gawl::concepts::has_keycode_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasKeysymCallback = typename std::conditional_t<gawl::concepts::has_keysym_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasPointerMoveCallback = typename std::conditional_t<gawl::concepts::has_pointer_move_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasClickCallback = typename std::conditional_t<gawl::concepts::has_click_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasScrollCallback = typename std::conditional_t<gawl::concepts::has_scroll_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasCloseCallback = typename std::conditional_t<gawl::concepts::has_click_callback<Impl>(), std::true_type, std::false_type>;

    template <class Impl>
    using HasUserCallback = typename std::conditional_t<gawl::concepts::has_user_callback<Impl>(), std::true_type, std::false_type>;

    constexpr static auto keycode  = !not_implemented<HasKeycodeCallback, Impls...>();
    constexpr static auto keysym   = !not_implemented<HasKeysymCallback, Impls...>();
    constexpr static auto keyboard = keycode || keysym;
    constexpr static auto click    = !not_implemented<HasClickCallback, Impls...>();
    constexpr static auto motion   = !not_implemented<HasPointerMoveCallback, Impls...>();
    constexpr static auto pointer  = click || motion;
};
} // namespace gawl::internal
