#include "window.hpp"
#include "../macros/assert.hpp"
#include "../util/assert.hpp"
#include "eglobject.hpp"

namespace gawl {
class WaylandWindowCallbacks : public towl::SurfaceCallbacks,
                               public towl::XDGToplevelCallbacks {
  private:
    WaylandWindow* window;

    // SurfaceCallbacks
    auto on_wl_surface_enter(wl_output* const output) -> void override {
        window->output = output;
        // window->resize_buffer(-1, -1, towl::get_scale(output));
    }

    auto on_wl_surface_leave(wl_output* /*output*/) -> void override {
        window->output = nullptr;
    }

    auto on_wl_surface_preferred_buffer_scale(int32_t /*factor*/) -> void override {}

    auto on_wl_surface_frame() -> void override {
        window->frame_done = true;
        if(!window->latest_frame.exchange(true) || !window->get_event_driven()) {
            window->push_window_event<impl::WindowEvents::Refresh>();
        }
    }

    // XDGToplevelCallbacks
    auto on_xdg_toplevel_configure(const int width, const int height) -> void override {
        {
            const auto& buffer_size = window->get_buffer_size();
            const auto [lock, data] = buffer_size.access();
            if(data.size[0] == static_cast<size_t>(width) * data.scale && data.size[1] == static_cast<size_t>(height) * data.scale) {
                return;
            }
        }
        window->resize_buffer(width, height, -1);
    }

    auto on_xdg_toplevel_close() -> void override {
        window->push_window_event<impl::WindowEvents::CloseRequest>();
    }

  public:
    WaylandWindowCallbacks(WaylandWindow* const window)
        : window(window) {}
};

namespace {
auto choose_surface(const EGLSurface eglsurface, const impl::EGLObject& egl) -> void {
    static auto current_surface = EGLSurface(nullptr);
    if(current_surface == eglsurface) {
        return;
    }
    dynamic_assert(eglMakeCurrent(egl.display, eglsurface, eglsurface, egl.context) != EGL_FALSE);
    current_surface = eglsurface;
}

template <class T>
auto get_primary_interface(auto& binder) -> T* {
    return std::bit_cast<T*>(binder.interfaces[0].get());
}
} // namespace

auto WaylandWindow::init_egl() -> void {
    egl_surface = eglCreateWindowSurface(egl->display, egl->config, std::bit_cast<EGLNativeWindowType>(egl_window.native()), nullptr);
    DYN_ASSERT(egl_surface != EGL_NO_SURFACE);
    choose_surface(egl_surface, *egl);
    DYN_ASSERT(eglSwapInterval(egl->display, 0) == EGL_TRUE); // make eglSwapBuffers non-blocking
}

auto WaylandWindow::swap_buffer() -> void {
    DYN_ASSERT(eglSwapBuffers(egl->display, egl_surface) != EGL_FALSE);
}

auto WaylandWindow::wait_for_key_repeater_exit(std::thread& repeater) -> void {
    last_pressed_key.store(-1);
    key_delay_timer.wakeup();
    if(repeater.joinable()) {
        repeater.join();
    }
}

auto WaylandWindow::resize_buffer(const int width, const int height, const int scale) -> void {
    auto new_scale  = size_t();
    auto new_width  = size_t();
    auto new_height = size_t();
    {
        const auto& buffer      = get_buffer_size();
        const auto [lock, data] = buffer.access();
        if(width != -1 && height != -1) {
            // update buffer size
            new_scale  = data.scale;
            new_width  = width;
            new_height = height;
        }
        if(scale != -1) {
            // update scale
            new_scale  = scale;
            new_width  = data.size[0];
            new_height = data.size[1];
        }
    }
    new_width *= new_scale;
    new_height *= new_scale;
    on_buffer_resize(std::array{new_width, new_height}, new_scale);

    obsolete_egl_window_size = true;
    push_window_event<impl::WindowEvents::WindowResize>();
    refresh();
}

auto WaylandWindow::handle_event() -> void {
    using Events = impl::WindowEvents;
    for(const auto& a : window_event_queue.swap()) {
        switch(a.get_index()) {
        case Events::Queue::index_of<Events::Refresh>: {
            choose_surface(egl_surface, *egl);
            if(obsolete_egl_window_size) {
                obsolete_egl_window_size         = false;
                const auto& critical_buffer_size = this->get_buffer_size();
                const auto [lock, buffer_size]   = critical_buffer_size.access();
                egl_window.resize(buffer_size.size[0], buffer_size.size[1], 0, 0);
                swap_buffer(); // ensure buffer sizes are changed to prevent "Buffer size is not divisible by scale" error
                wayland_surface.set_buffer_scale(buffer_size.scale);
            }
            callbacks->refresh();
            wayland_surface.set_frame();
            swap_buffer();
        } break;
        case Events::Queue::index_of<Events::WindowResize>:
            callbacks->on_resize();
            break;
        case Events::Queue::index_of<Events::Keycode>: {
            const auto& args = a.template as<Events::Keycode>();
            callbacks->on_keycode(args.key, args.state);
        } break;
        case Events::Queue::index_of<Events::PointerMove>: {
            const auto& args = a.template as<Events::PointerMove>();
            callbacks->on_pointer(args.pos);
        } break;
        case Events::Queue::index_of<Events::Click>: {
            const auto& args = a.template as<Events::Click>();
            callbacks->on_click(args.key, args.state);
        } break;
        case Events::Queue::index_of<Events::Scroll>: {
            const auto& args = a.template as<Events::Scroll>();
            callbacks->on_scroll(args.axis, args.value);
        } break;
        case Events::Queue::index_of<Events::TouchDown>: {
            const auto& args = a.template as<Events::TouchDown>();
            callbacks->on_touch_down(args.id, args.pos);
        } break;
        case Events::Queue::index_of<Events::TouchUp>: {
            const auto& args = a.template as<Events::TouchUp>();
            callbacks->on_touch_up(args.id);
        } break;
        case Events::Queue::index_of<Events::TouchMotion>: {
            const auto& args = a.template as<Events::TouchMotion>();
            callbacks->on_touch_motion(args.id, args.pos);
        } break;
        case Events::Queue::index_of<Events::CloseRequest>:
            callbacks->close();
            break;
        }
    }
}

auto WaylandWindow::wl_get_surface() -> wl_surface* {
    return wayland_surface.native();
}

auto WaylandWindow::wl_get_output() const -> wl_output* {
    return output;
}

auto WaylandWindow::wl_on_keycode_enter(const towl::Array<uint32_t>& keys) -> void {
    for(auto i = size_t(0); i < keys.size; i += 1) {
        push_window_event<impl::WindowEvents::Keycode>(keys.data[i], gawl::ButtonState::Enter);
    }
}

auto WaylandWindow::wl_on_key_leave() -> void {
    auto [lock, repeater] = key_repeater.access();
    wait_for_key_repeater_exit(repeater);
    push_window_event<impl::WindowEvents::Keycode>(static_cast<uint32_t>(-1), gawl::ButtonState::Leave);
}

auto WaylandWindow::wl_on_keycode_input(const uint32_t keycode, const uint32_t state) -> void {
    const auto key_state = state == WL_KEYBOARD_KEY_STATE_PRESSED ? ButtonState::Press : ButtonState::Release;
    push_window_event<impl::WindowEvents::Keycode>(keycode, key_state);
    if(!wl->repeat_config) {
        return;
    }
    auto [lock, repeater] = key_repeater.access();
    if(key_state == gawl::ButtonState::Press) {
        if(get_state() != impl::WindowState::Running) {
            return;
        }
        wait_for_key_repeater_exit(repeater);
        last_pressed_key.store(keycode);

        repeater = std::thread([this, keycode]() {
            key_delay_timer.wait_for(std::chrono::milliseconds(wl->repeat_config->delay_in_milisec));
            while(last_pressed_key.load() == keycode) {
                push_window_event<impl::WindowEvents::Keycode>(keycode, ButtonState::Repeat);
                key_delay_timer.wait_for(std::chrono::milliseconds(wl->repeat_config->interval));
            }
        });
    } else if(last_pressed_key.load() == keycode) {
        wait_for_key_repeater_exit(repeater);
    }
}

auto WaylandWindow::wl_on_pointer_motion(const double x, const double y) -> void {
    push_window_event<impl::WindowEvents::PointerMove>(Point{x, y});
}

auto WaylandWindow::wl_on_pointer_button(const uint32_t button, const uint32_t state) -> void {
    const auto key_state = state == WL_POINTER_BUTTON_STATE_PRESSED ? ButtonState::Press : ButtonState::Release;
    push_window_event<impl::WindowEvents::Click>(button, key_state);
}

auto WaylandWindow::wl_on_pointer_axis(const uint32_t axis, const double value) -> void {
    const auto axis_enum = axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? WheelAxis::Horizontal : WheelAxis::Vertical;
    push_window_event<impl::WindowEvents::Scroll>(axis_enum, value);
}

auto WaylandWindow::wl_on_touch_down(const uint32_t id, const double x, const double y) -> void {
    push_window_event<impl::WindowEvents::TouchDown>(id, Point{x, y});
}

auto WaylandWindow::wl_on_touch_up(const uint32_t id) -> void {
    push_window_event<impl::WindowEvents::TouchUp>(id);
}

auto WaylandWindow::wl_on_touch_motion(const uint32_t id, const double x, const double y) -> void {
    push_window_event<impl::WindowEvents::TouchMotion>(id, Point{x, y});
}

auto WaylandWindow::wl_set_output_scale(uint32_t scale) -> void {
    resize_buffer(-1, -1, scale);
}

auto WaylandWindow::refresh() -> void {
    latest_frame.store(false);
    if(!frame_done) {
        return;
    }
    frame_done = false;
    push_window_event<impl::WindowEvents::Refresh>();
}

auto WaylandWindow::close_window() -> void {
    set_state(impl::WindowState::Destructing);
    app_event_queue->push<impl::CloseWindowArgs>(this);
}

auto WaylandWindow::quit_application() -> void {
    app_event_queue->push<impl::QuitApplicationArgs>();
}

auto WaylandWindow::fork_context() -> EGLSubObject {
    return egl->fork();
}

WaylandWindow::WaylandWindow(
    const WindowCreateHint&           hint,
    WindowCallbacks* const            callbacks,
    impl::WaylandClientObjects* const wl,
    impl::EGLObject* const            egl,
    impl::AppEventQueue* const        app_event_queue)
    : Window(callbacks),
      wl(wl), egl(egl), app_event_queue(app_event_queue),
      wl_callbacks(new WaylandWindowCallbacks(this)),
      wayland_surface(get_primary_interface<towl::Compositor>(wl->compositor_binder)->create_surface(wl_callbacks)),
      xdg_surface(get_primary_interface<towl::XDGWMBase>(wl->xdg_wm_base_binder)->create_xdg_surface(wayland_surface.native())),
      xdg_toplevel(xdg_surface.create_xdg_toplevel(wl_callbacks)),
      egl_window(wayland_surface.native(), hint.width, hint.height) {
    wayland_surface.commit();
    wl->display.wait_sync();
    xdg_toplevel.set_title(hint.title);
    init_egl();
    resize_buffer(hint.width, hint.height, -1);

    // swap_buffer();
    wl->display.wait_sync();

    set_event_driven(hint.manual_refresh);
    set_state(impl::WindowState::Running);
}
} // namespace gawl
