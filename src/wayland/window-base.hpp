#pragma once
#include "../window.hpp"
#include "app-event.hpp"
#include "eglobject.hpp"
#include "wayland-window.hpp"

namespace gawl::wl {
namespace internal {
namespace callback {
struct Refresh {};

struct WindowResize {};

struct Keycode {
    uint32_t          key;
    gawl::ButtonState state;
};

struct Keysym {
    xkb_keycode_t     key;
    gawl::ButtonState state;
    xkb_state*        xkbstate;
};

struct PointerMove {
    gawl::Point pos;
};

struct Click {
    uint32_t          key;
    gawl::ButtonState state;
};

struct Scroll {
    gawl::WheelAxis axis;
    double          value;
};

struct TouchDown {
    uint32_t    id;
    gawl::Point pos;
};

struct TouchUp {
    uint32_t id;
};

struct TouchMotion {
    uint32_t    id;
    gawl::Point pos;
};

struct CloseRequest {};

struct User {
    void* data;
};

using Queue = ::gawl::internal::VariantBuffer<Refresh, WindowResize, Keycode, Keysym, PointerMove, Click, Scroll, TouchDown, TouchUp, TouchMotion, CloseRequest, User>;
} // namespace callback

struct WaylandClientObject;

class WindowBase : public ::gawl::internal::Window, public internal::WaylandWindow {
  protected:
    internal::WaylandClientObject& wl;
    internal::EGLObject&           egl;
    internal::app_event::Queue&    app_queue;
    internal::callback::Queue      queue;

    std::atomic_bool frame_done   = true;
    std::atomic_bool latest_frame = true;

  public:
    // internal use
    template <class T, class... Args>
    auto queue_callback(Args&&... args) -> void {
        if(get_state() == ::gawl::internal::WindowState::Destructing) {
            return;
        }

        queue.push<T>(std::forward<Args>(args)...);
        app_queue.push<app_event::HandleEventArgs>(this);
    }

    virtual auto handle_event() -> void = 0;
    // ~internal use

    auto refresh() -> void {
        latest_frame.store(false);
        if(!frame_done) {
            return;
        }
        frame_done = false;
        queue_callback<internal::callback::Refresh>();
    }

    auto invoke_user_callback(void* const data) -> void {
        queue_callback<internal::callback::User>(data);
    }

    auto close_window() -> void {
        set_state(::gawl::internal::WindowState::Destructing);
        app_queue.push<app_event::CloseWindowArgs>(this);
    }

    auto quit_application() -> void {
        app_queue.push<app_event::QuitApplicationArgs>();
    }

    auto fork_context() -> EGLSubObject {
        return egl.fork();
    }

    WindowBase(internal::WaylandClientObject& wl, internal::EGLObject& egl, internal::app_event::Queue& app_queue)
        : wl(wl),
          egl(egl),
          app_queue(app_queue) {}

    virtual ~WindowBase() {}
};
} // namespace internal
} // namespace gawl::wl
