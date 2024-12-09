#include <coop/single-event.hpp>
#include <coop/timer.hpp>

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

auto WaylandWindowCallbacks::on_xdg_toplevel_configure(const int width, const int height) -> void {
    {
        const auto& buffer_size = window->get_buffer_size();
        if(buffer_size.size[0] == static_cast<size_t>(width) * buffer_size.scale && buffer_size.size[1] == static_cast<size_t>(height) * buffer_size.scale) {
            return;
        }
    }
    window->resize_buffer(width, height, -1);
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
        const auto& buffer_size = get_buffer_size();
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

auto WaylandWindow::wl_get_surface() -> wl_surface* {
    return wayland_surface.native();
}

auto WaylandWindow::wl_on_keycode_enter(const towl::Array<uint32_t>& keys) -> void {
    for(auto i = size_t(0); i < keys.size; i += 1) {
        pending_callbacks.emplace_back(callbacks->on_keycode(keys.data[i], gawl::ButtonState::Enter));
    }
}

auto WaylandWindow::wl_on_key_leave() -> void {
    pending_callbacks.emplace_back(callbacks->on_keycode(-1, gawl::ButtonState::Leave));
    key_repeater.cancel();
}

auto WaylandWindow::wl_on_keycode_input(const uint32_t keycode, const uint32_t state) -> void {
    const auto key_state = state == WL_KEYBOARD_KEY_STATE_PRESSED ? ButtonState::Press : ButtonState::Release;
    pending_callbacks.emplace_back(callbacks->on_keycode(keycode, key_state));

    key_repeater.cancel();
    if(!wl->repeat_config || key_state != ButtonState::Press) {
        return;
    }
    runner->push_task(std::array{&key_repeater},
                      [](WaylandWindow& self, uint32_t keycode) -> coop::Async<void> {
                          co_await coop::sleep(std::chrono::milliseconds(self.wl->repeat_config->delay_in_milisec));
                          while(true) {
                              self.pending_callbacks.emplace_back(self.callbacks->on_keycode(keycode, ButtonState::Repeat));
                              self.application_event->notify();
                              co_await coop::sleep(std::chrono::milliseconds(self.wl->repeat_config->interval));
                          }
                      }(*this, keycode));
}

auto WaylandWindow::wl_on_pointer_motion(const double x, const double y) -> void {
    pending_callbacks.emplace_back(callbacks->on_pointer({x, y}));
}

auto WaylandWindow::wl_on_pointer_button(const uint32_t button, const uint32_t state) -> void {
    const auto key_state = state == WL_POINTER_BUTTON_STATE_PRESSED ? ButtonState::Press : ButtonState::Release;
    pending_callbacks.emplace_back(callbacks->on_click(button, key_state));
}

auto WaylandWindow::wl_on_pointer_axis(const uint32_t axis, const double value) -> void {
    const auto axis_enum = axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? WheelAxis::Horizontal : WheelAxis::Vertical;
    pending_callbacks.emplace_back(callbacks->on_scroll(axis_enum, value));
}

auto WaylandWindow::wl_on_touch_down(const uint32_t id, const double x, const double y) -> void {
    pending_callbacks.emplace_back(callbacks->on_touch_down(id, {x, y}));
}

auto WaylandWindow::wl_on_touch_up(const uint32_t id) -> void {
    pending_callbacks.emplace_back(callbacks->on_touch_up(id));
}

auto WaylandWindow::wl_on_touch_motion(const uint32_t id, const double x, const double y) -> void {
    pending_callbacks.emplace_back(callbacks->on_touch_motion(id, {x, y}));
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
        const auto& buffer_size  = this->get_buffer_size();
        egl_window.resize(buffer_size.size[0], buffer_size.size[1], 0, 0);
        swap_buffer(); // ensure buffer sizes are changed to prevent "Buffer size is not divisible by scale" error
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
    impl::EGLObject* const            egl,
    coop::SingleEvent&                application_event) -> coop::Async<bool> {
    constexpr auto error_value = false;

    runner = co_await coop::reveal_runner();

    set_callbacks(callbacks);
    this->wl                = wl;
    this->egl               = egl;
    this->application_event = &application_event;
    wl_callbacks.reset(new WaylandWindowCallbacks(this));
    wayland_surface = get_primary_interface<towl::Compositor>(wl->compositor_binder)->create_surface();
    co_ensure_v(wayland_surface.init(wl_callbacks.get()));
    xdg_surface = get_primary_interface<towl::XDGWMBase>(wl->xdg_wm_base_binder)->create_xdg_surface(wayland_surface.native());
    co_ensure_v(xdg_surface.init());
    xdg_toplevel = xdg_surface.create_xdg_toplevel();
    co_ensure_v(xdg_toplevel.init(wl_callbacks.get()));
    egl_window = towl::EGLWindow(wayland_surface.native(), hint.width, hint.height);
    wayland_surface.commit();
    co_await wl->display.wait_sync();
    xdg_toplevel.set_title(hint.title);
    wayland_surface.commit();
    co_ensure_v(co_await callbacks->on_created(this));

    co_ensure_v(init_egl());
    co_ensure_v(resize_buffer(hint.width, hint.height, -1));

    co_await wl->display.wait_sync();

    set_event_driven(hint.manual_refresh);
    co_return true;
}

WaylandWindow::~WaylandWindow() {
    key_repeater.cancel();
    ASSERT(eglDestroySurface(egl->display, egl_surface) == EGL_TRUE);
}
} // namespace gawl
