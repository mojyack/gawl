#pragma once
#include <EGL/egl.h>

#include "../window-creat-hint.hpp"
#include "../window.hpp"
#include "eglobject.hpp"
#include "wl-object.hpp"

namespace gawl {
class WaylandWindow;
class WaylandWindowCallbacks : public towl::SurfaceCallbacks,
                               public towl::XDGToplevelCallbacks {
  private:
    WaylandWindow* window;
    // SurfaceCallbacks
    auto on_wl_surface_preferred_buffer_scale(int32_t factor) -> void override;
    auto on_wl_surface_frame() -> void override;
    // XDGToplevelCallbacks
    auto on_xdg_toplevel_configure(const int width, const int height) -> void override;
    auto on_xdg_toplevel_close() -> void override;

  public:
    WaylandWindowCallbacks(WaylandWindow* window);
};

class WaylandWindow : public Window {
    friend class WaylandApplication;
    friend class WaylandWindowCallbacks;

  private:
    impl::WaylandClientObjects*             wl;
    impl::EGLObject*                        egl;
    std::unique_ptr<WaylandWindowCallbacks> wl_callbacks;

    EGLSurface        egl_surface = nullptr;
    towl::Surface     wayland_surface;
    towl::XDGSurface  xdg_surface;
    towl::XDGToplevel xdg_toplevel;
    towl::EGLWindow   egl_window;

    std::vector<coop::Async<bool>> pending_callbacks;

    coop::Runner*    runner;
    coop::TaskHandle key_repeater;

    bool obsolete_egl_window_size = true;
    bool frame_done               = true;
    bool latest_frame             = true;

    auto init_egl() -> bool;
    auto swap_buffer() -> bool;
    auto resize_buffer(const int width, const int height, const int scale) -> bool;

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
    auto dispatch_pending_callbacks() -> coop::Async<bool>;

    // for users
    auto refresh() -> bool override;
    auto fork_context() -> EGLSubObject;

    auto init(WindowCreateHint hint, std::shared_ptr<WindowCallbacks> callbacks, impl::WaylandClientObjects* wl, impl::EGLObject* egl) -> coop::Async<bool>;

    ~WaylandWindow();
};
}; // namespace gawl
