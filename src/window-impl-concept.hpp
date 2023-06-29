#pragma once
#include <concepts>

#include <xkbcommon/xkbcommon.h>

#include "type.hpp"

namespace gawl::concepts {
namespace internal {
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
concept WindowImplWithPointerMoveCallback = requires(Impl& m, gawl::Point pos) {
                                                { m.pointer_move_callback(pos) } -> std::same_as<void>;
                                            };

template <class Impl>
concept WindowImplWithClickCallback = requires(Impl& m, uint32_t button, gawl::ButtonState state) {
                                          { m.click_callback(button, state) } -> std::same_as<void>;
                                      };

template <class Impl>
concept WindowImplWithScrollCallback = requires(Impl& m, gawl::WheelAxis axis, double value) {
                                           { m.scroll_callback(axis, value) } -> std::same_as<void>;
                                       };

template <class Impl>
concept WindowImplWithTouchDown = requires(Impl& m, uint32_t id, gawl::Point pos) {
                                      { m.touch_down_callback(id, pos) } -> std::same_as<void>;
                                  };

template <class Impl>
concept WindowImplWithTouchUp = requires(Impl& m, uint32_t id) {
                                    { m.touch_down_callback(id) } -> std::same_as<void>;
                                };

template <class Impl>
concept WindowImplWithTouchMotionCallback = requires(Impl& m, uint32_t id, gawl::Point pos) {
                                                { m.touch_motion_callback(id, pos) } -> std::same_as<void>;
                                            };

template <class Impl>
concept WindowImplWithCloseRequestCallback = requires(Impl& m) {
                                                 { m.close_request_callback() } -> std::same_as<void>;
                                             };

template <class Impl>
concept WindowImplWithUserCallback = requires(Impl& m, void* data) {
                                         { m.user_callback(data) } -> std::same_as<void>;
                                     };

template <class T>
consteval auto is_complete() -> bool {
    static_assert(sizeof(T), "incomplete type");
    return true;
}

} // namespace internal

template <class Impl>
constexpr auto has_refresh_callback = internal::WindowImplWithRefreshCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_window_resize_callback = internal::WindowImplWithWindowResizeCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_keycode_callback = internal::WindowImplWithKeycodeCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_keysym_callback = internal::WindowImplWithKeysymCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_pointer_move_callback = internal::WindowImplWithPointerMoveCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_click_callback = internal::WindowImplWithClickCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_scroll_callback = internal::WindowImplWithScrollCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_touch_down_callback = internal::WindowImplWithTouchDown<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_touch_up_callback = internal::WindowImplWithTouchUp<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_touch_motion_callback = internal::WindowImplWithTouchMotionCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_close_request_callback = internal::WindowImplWithCloseRequestCallback<Impl> && internal::is_complete<Impl>();

template <class Impl>
constexpr auto has_user_callback = internal::WindowImplWithUserCallback<Impl> && internal::is_complete<Impl>();
} // namespace gawl::concepts
