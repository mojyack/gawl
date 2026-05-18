#include <coop/single-event.hpp>
#include <coop/task-handle.hpp>

#include "../macros/assert.hpp"
#include "eglobject.hpp"
#include "window.hpp"

namespace gawl {
auto WaylandWindowCallbacks::on_wl_surface_preferred_buffer_scale(const int32_t factor) -> void {
    window->resize_buffer(-1, -1, factor);
}

auto WaylandWindowCallbacks::on_wl_surface_frame() -> void {
    window->frame_done = true;
    if(!window->latest_frame || !window->event_driven) {
        ensure(window->refresh());
        window->latest_frame = true;
    }
}

auto WaylandWindowCallbacks::on_xdg_surface_configure() -> void {
    {
        if(window->buffer_size.size[0] == static_cast<size_t>(pending_width) * window->buffer_size.scale && window->buffer_size.size[1] == static_cast<size_t>(pending_height) * window->buffer_size.scale) {
            return;
        }
    }
    window->resize_buffer(pending_width, pending_height, -1);
}

auto WaylandWindowCallbacks::on_xdg_toplevel_configure(const int width, const int height) -> void {
    pending_width  = width;
    pending_height = height;
}

auto WaylandWindowCallbacks::on_xdg_toplevel_close() -> void {
    window->callbacks->close();
}

WaylandWindowCallbacks::WaylandWindowCallbacks(WaylandWindow* const window)
    : window(window) {}

namespace {
auto choose_surface(const EGLSurface eglsurface, const impl::EGLObject& egl) -> void {
    static auto current_surface = EGLSurface(nullptr);
    if(current_surface == eglsurface) {
        return;
    }
    ASSERT(eglMakeCurrent(egl.display, eglsurface, eglsurface, egl.context) != EGL_FALSE);
    current_surface = eglsurface;
}

template <class T>
auto get_primary_interface(auto& binder) -> T* {
    return std::bit_cast<T*>(binder.interfaces[0].get());
}
} // namespace

auto WaylandWindow::init_egl() -> bool {
    egl_surface = eglCreateWindowSurface(egl->display, egl->config, std::bit_cast<EGLNativeWindowType>(egl_window.native()), nullptr);
    ensure(egl_surface != EGL_NO_SURFACE);
    choose_surface(egl_surface, *egl);
    ensure(eglSwapInterval(egl->display, 0) == EGL_TRUE); // make eglSwapBuffers non-blocking
    return true;
}

auto WaylandWindow::swap_buffer() -> bool {
    ensure(eglSwapBuffers(egl->display, egl_surface) == EGL_TRUE);
    return true;
}

auto WaylandWindow::resize_buffer(const int width, const int height, const int scale) -> bool {
    auto new_scale  = size_t();
    auto new_width  = size_t();
    auto new_height = size_t();
    {
        if(width != -1 && height != -1) {
            // update buffer size
            new_scale  = buffer_size.scale;
            new_width  = width;
            new_height = height;
        }
        if(scale != -1) {
            // update scale
            new_scale  = scale;
            new_width  = buffer_size.size[0];
            new_height = buffer_size.size[1];
        }
    }
    new_width *= new_scale;
    new_height *= new_scale;
    on_buffer_resize(std::array{new_width, new_height}, new_scale);

    obsolete_egl_window_size = true;
    ensure(egl_surface == nullptr || refresh());
    return true;
}

auto WaylandWindow::dispatch_pending_callbacks() -> coop::Async<bool> {
    auto ret = true;
    for(auto& task : std::exchange(pending_callbacks, {})) {
        ret &= co_await task;
    }
    co_return ret;
}

auto WaylandWindow::refresh() -> bool {
    latest_frame = false;
    if(!frame_done) {
        return true;
    }
    frame_done = false;

    choose_surface(egl_surface, *egl);
    if(obsolete_egl_window_size) {
        obsolete_egl_window_size = false;
        egl_window.resize(buffer_size.size[0], buffer_size.size[1], 0, 0);
        wayland_surface.set_buffer_scale(buffer_size.scale);
    }
    callbacks->refresh();
    wayland_surface.set_frame();
    ensure(swap_buffer());
    return true;
}

auto WaylandWindow::fork_context() -> EGLSubObject {
    return egl->fork();
}

auto WaylandWindow::init(
    const WindowCreateHint            hint,
    std::shared_ptr<WindowCallbacks>  callbacks,
    impl::WaylandClientObjects* const wl,
    impl::EGLObject* const            egl) -> coop::Async<bool> {
    constexpr auto error_value = false;

    runner = co_await coop::reveal_runner();

    set_callbacks(callbacks);
    this->wl  = wl;
    this->egl = egl;
    wl_callbacks.reset(new WaylandWindowCallbacks(this));
    wayland_surface = get_primary_interface<towl::Compositor>(wl->compositor_binder)->create_surface();
    co_ensure_v(wayland_surface.init(wl_callbacks.get()));
    xdg_surface = get_primary_interface<towl::XDGWMBase>(wl->xdg_wm_base_binder)->create_xdg_surface(wayland_surface.native());
    co_ensure_v(xdg_surface.init(wl_callbacks.get()));
    xdg_toplevel = xdg_surface.create_xdg_toplevel();
    co_ensure_v(xdg_toplevel.init(wl_callbacks.get()));
    egl_window = towl::EGLWindow(wayland_surface.native(), hint.width, hint.height);
    wayland_surface.commit();
    co_await wl->display.wait_sync();
    xdg_toplevel.set_title(hint.title);
    wayland_surface.commit();

    co_ensure_v(init_egl());
    co_ensure_v(co_await callbacks->on_created(this));
    co_ensure_v(resize_buffer(hint.width, hint.height, -1));

    co_await wl->display.wait_sync();

    event_driven = hint.manual_refresh;
    co_return true;
}

WaylandWindow::~WaylandWindow() {
    key_repeater.cancel();
    ASSERT(eglDestroySurface(egl->display, egl_surface) == EGL_TRUE);
}
} // namespace gawl
