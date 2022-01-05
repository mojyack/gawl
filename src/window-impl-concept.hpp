#pragma once
#include <concepts>

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
concept WindowImplWithKeyboardCallback = requires(Impl& m) {
    { m.keyboard_callback(uint32_t(), gawl::ButtonState()) } -> std::same_as<void>;
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

namespace impl {
    template <class Impl>
    using RefreshCallback = typename std::conditional<WindowImplWithRefreshCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using WindowResizeCallback = typename std::conditional<WindowImplWithWindowResizeCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using KeyboardCallback = typename std::conditional<WindowImplWithKeyboardCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using PointermoveCallback = typename std::conditional<WindowImplWithPointermoveCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using ClickCallback = typename std::conditional<WindowImplWithClickCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using ScrollCallback = typename std::conditional<WindowImplWithScrollCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using CloseCallback = typename std::conditional<WindowImplWithCloseRequestCallback<Impl>, std::true_type, std::false_type>::type;
    template <class Impl>
    using UserCallback = typename std::conditional<WindowImplWithUserCallback<Impl>, std::true_type, std::false_type>::type;

    template <template <class> class Concept, class T, class... Ts>
    constexpr auto not_implemented()->bool {
        if constexpr(!Concept<T>::value) {
            if constexpr(sizeof...(Ts) == 0) {
                return true;
            } else {
                return not_implemented<Concept, Ts...>();
            }
        } else {
            return false;
        }
    }
} // namespace impl
} // namespace gawl::concepts
