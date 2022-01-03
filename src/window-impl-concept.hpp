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
} // namespace gawl::concepts
