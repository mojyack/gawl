#pragma once
#include <EGL/egl.h>

#include "../window-creat-hint.hpp"
#include "../window.hpp"
#include "eglobject.hpp"
#include "wl-object.hpp"

namespace gawl {
class WaylandWindow;
class WaylandWindowCallbacks : public towl::SurfaceCallbacks,
                               public towl::XDGSurfaceCallbacks,
                               public towl::XDGToplevelCallbacks {
  private:
    WaylandWindow* window;
    int            pending_width  = 0;
    int            pending_height = 0;

    // SurfaceCallbacks
    auto on_wl_surface_preferred_buffer_scale(int32_t factor) -> void override;
    auto on_wl_surface_frame() -> void override;
    // XDGSurfaceCallbacks
    auto on_xdg_surface_configure() -> void override;
    // XDGToplevelCallbacks
    auto on_xdg_toplevel_configure(const int width, const int height) -> void override;
    auto on_xdg_toplevel_close() -> void override;

  public:
    WaylandWindowCallbacks(WaylandWindow* window);
};

class WaylandWindow : public Window {
  public:
    // private

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
    auto dispatch_pending_callbacks() -> coop::Async<bool>;

    // for users
    auto refresh() -> bool override;
    auto fork_context() -> EGLSubObject;

    auto init(WindowCreateHint hint, std::shared_ptr<WindowCallbacks> callbacks, impl::WaylandClientObjects* wl, impl::EGLObject* egl) -> coop::Async<bool>;

    ~WaylandWindow();
};
}; // namespace gawl
