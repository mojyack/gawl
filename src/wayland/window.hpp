#pragma once
#include <thread>

#include <EGL/egl.h>

#include "../window-creat-hint.hpp"
#include "../window.hpp"
#include "app-events.hpp"
#include "eglobject.hpp"
#include "window-events.hpp"
#include "wl-object.hpp"

#define CUTIL_NS gawl
#include "../util/timer-event.hpp"
#undef CUTIL_NS

namespace gawl {
class WaylandWindowCallbacks;
class WaylandWindow : public Window {
    friend class WaylandApplication;
    friend class WaylandWindowCallbacks;

  private:
    impl::WaylandClientObjects* wl;
    impl::EGLObject*            egl;
    impl::AppEventQueue*        app_event_queue;
    impl::WindowEvents::Queue   window_event_queue;
    WaylandWindowCallbacks*     wl_callbacks;

    EGLSurface        egl_surface = nullptr;
    towl::Surface     wayland_surface;
    towl::XDGSurface  xdg_surface;
    towl::XDGToplevel xdg_toplevel;
    towl::EGLWindow   egl_window;

    TimerEvent            key_delay_timer;
    Critical<std::thread> key_repeater;
    std::atomic_uint32_t  last_pressed_key = -1;

    std::atomic_bool obsolete_egl_window_size = true;
    std::atomic_bool frame_done               = true;
    std::atomic_bool latest_frame             = true;

    auto init_egl() -> void;
    auto swap_buffer() -> void;
    auto wait_for_key_repeater_exit(std::thread& repeater) -> void;
    auto resize_buffer(const int width, const int height, const int scale) -> void;
    auto handle_event() -> void;

    template <class T, class... Args>
    auto push_window_event(Args&&... args) -> void {
        if(get_state() == impl::WindowState::Destructing) {
            return;
        }

        window_event_queue.push<T>(std::forward<Args>(args)...);
        app_event_queue->push<impl::HandleEventArgs>(this);
    }

  public:
    // wayland callbacks
    auto wl_get_surface() -> wl_surface*;
    auto wl_on_keycode_enter(const towl::Array<uint32_t>& keys) -> void;
    auto wl_on_key_leave() -> void;
    auto wl_on_keycode_input(const uint32_t keycode, const uint32_t state) -> void;
    auto wl_on_pointer_motion(const double x, const double y) -> void;
    auto wl_on_pointer_button(const uint32_t button, const uint32_t state) -> void;
    auto wl_on_pointer_axis(const uint32_t axis, const double value) -> void;
    auto wl_on_touch_down(const uint32_t id, const double x, const double y) -> void;
    auto wl_on_touch_up(const uint32_t id) -> void;
    auto wl_on_touch_motion(const uint32_t id, const double x, const double y) -> void;

    // for users
    auto refresh() -> void override;
    auto fork_context() -> EGLSubObject;

    WaylandWindow(const WindowCreateHint& hint, std::shared_ptr<WindowCallbacks> callbacks, impl::WaylandClientObjects* wl, impl::EGLObject* egl, impl::AppEventQueue* app_event_queue);

    ~WaylandWindow();
};
}; // namespace gawl
