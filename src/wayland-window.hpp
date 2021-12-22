#pragma once
#include <thread>
#include <variant>

#include <wayland-client-core.h>
#include <wayland-client-protocol-extra.hpp>
#include <wayland-client-protocol.hpp>
#include <wayland-cursor.hpp>
#include <wayland-egl.hpp>

#include "gawl-window.hpp"

namespace gawl {
namespace internal {
struct RefreshCallbackArgs {};
struct WindowResizeCallbackArgs {};
struct KeyBoardCallbackArgs {
    uint32_t          key;
    gawl::ButtonState state;
};
struct PointermoveCallbackArgs {
    double x;
    double y;
};
struct ClickCallbackArgs {
    uint32_t          key;
    gawl::ButtonState state;
};
struct ScrollCallbackArgs {
    gawl::WheelAxis axis;
    double          value;
};
struct CloseRequestCallbackArgs {};
struct UserCallbackArgs {
    void* data;
};
using CallbackArgs = std::variant<RefreshCallbackArgs, WindowResizeCallbackArgs, KeyBoardCallbackArgs, PointermoveCallbackArgs, ClickCallbackArgs, ScrollCallbackArgs, CloseRequestCallbackArgs, UserCallbackArgs>;
} // namespace internal

class WaylandWindow : public GawlWindow {
    friend class WaylandApplication;

  private:
    // global objects
    wayland::display_t&    display;
    wayland::registry_t    registry;
    wayland::compositor_t  compositor;
    wayland::xdg_wm_base_t xdg_wm_base;
    wayland::seat_t        seat;
    wayland::shm_t         shm;
    wayland::output_t      output;

    // local objects
    wayland::surface_t       surface;
    wayland::shell_surface_t shell_surface;
    wayland::xdg_surface_t   xdg_surface;
    wayland::xdg_toplevel_t  xdg_toplevel;
    wayland::pointer_t       pointer;
    wayland::keyboard_t      keyboard;
    wayland::callback_t      frame_cb;
    wayland::cursor_image_t  cursor_image;
    wayland::buffer_t        cursor_buffer;
    wayland::surface_t       cursor_surface;

    // EGL
    wayland::egl_window_t egl_window;
    EGLSurface            eglsurface = nullptr;

    // gawl
    struct KeyRepeatConfig {
        uint32_t interval;
        uint32_t delay_in_milisec;
    };

    bool                           frame_done     = true;
    Critical<bool>                 latest_frame   = true;
    int                            window_size[2] = {0, 0};
    int                            buffer_scale   = 0;
    Event                          key_delay_timer;
    std::thread                    key_repeater;
    Critical<uint32_t>             last_pressed_key = -1;
    std::optional<KeyRepeatConfig> repeat_config;

    Critical<std::vector<internal::CallbackArgs>> callback_queue;

    auto init_egl() -> void;
    auto resize_buffer(int width, int height, int scale) -> void;
    auto handle_event() -> void;
    auto swap_buffer() -> void;
    auto choose_surface() -> void;
    auto wait_for_key_repeater_exit() -> void;
    auto queue_callback(internal::CallbackArgs args) -> void;

  public:
    auto prepare() -> internal::FramebufferBinder override;

    auto refresh() -> void final;
    auto invoke_user_callback(void* data = nullptr) -> void final;

    WaylandWindow(const WindowCreateHint& hint);
    virtual ~WaylandWindow();
};
} // namespace gawl
