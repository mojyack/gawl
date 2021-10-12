#include <iostream>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "error.hpp"
#include "wayland-application.hpp"
#include "wayland-window.hpp"

namespace gawl {
namespace {
struct EGLGlobal {
    EGLDisplay display = nullptr;
    EGLConfig  config  = nullptr;
    EGLContext context = nullptr;
    EGLGlobal(const wayland::display_t& wl_display) {
        display = eglGetDisplay(wl_display);
        ASSERT(display != EGL_NO_DISPLAY, "eglGetDisplay() failed")

        auto major = EGLint(0);
        auto minor = EGLint(0);
        ASSERT(eglInitialize(display, &major, &minor) != EGL_FALSE, "eglInitialize() failed")
        ASSERT((major == 1 && minor >= 4) || major >= 2, "EGL version too old")
        ASSERT(eglBindAPI(EGL_OPENGL_API) != EGL_FALSE, "eglBindAPI() failed")

        constexpr auto config_attribs = std::array<EGLint, 15>{EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                                               EGL_RED_SIZE, 8,
                                                               EGL_GREEN_SIZE, 8,
                                                               EGL_BLUE_SIZE, 8,
                                                               EGL_ALPHA_SIZE, 8,
                                                               EGL_SAMPLES, 4,
                                                               EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                                               EGL_NONE};

        auto num = EGLint(0);
        ASSERT(eglChooseConfig(display, config_attribs.data(), &config, 1, &num) != EGL_FALSE && num != 0, "eglChooseConfig() failed")

        constexpr auto context_attribs = std::array<EGLint, 3>{EGL_CONTEXT_CLIENT_VERSION, 2,
                                                               EGL_NONE};

        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs.data());
        ASSERT(context != EGL_NO_CONTEXT, "eglCreateContext() failed")
    }
    ~EGLGlobal() {
        ASSERT(eglDestroyContext(display, context) != EGL_FALSE, "eglDestroyContext() failed")
        ASSERT(eglTerminate(display) != EGL_FALSE, "eglTerminate() failed")
    }
};
auto egl_global_count = size_t(0);
auto egl_global       = std::optional<EGLGlobal>();
} // namespace
auto WaylandWindow::init_egl() -> void {
    if(egl_global_count == 0) {
        egl_global.emplace(display);
    }
    eglsurface = eglCreateWindowSurface(egl_global->display, egl_global->config, egl_window, nullptr);
    ASSERT(eglsurface != EGL_NO_SURFACE, "eglCreateWindowSurface() failed")
    choose_surface();
    egl_global_count += 1;
}
auto WaylandWindow::resize_buffer(int width, int height, int scale) -> void {
    if((width != -1 && height != -1 && width == window_size[0] && height == window_size[1]) || (scale != -1 && scale == buffer_scale)) {
        return;
    }
    if(scale != -1 && scale != buffer_scale) {
        buffer_scale = scale;
        surface.set_buffer_scale(buffer_scale);

        // load cursor theme
        wayland::cursor_theme_t cursor_theme = wayland::cursor_theme_t("default", 24 * buffer_scale, shm);
        wayland::cursor_t       cursor       = cursor_theme.get_cursor("left_ptr");
        cursor_image                         = cursor.image(0);
        cursor_buffer                        = cursor_image.get_buffer();
        cursor_surface.set_buffer_scale(buffer_scale);
    }
    if(width != -1 && height != -1) {
        window_size[0] = width;
        window_size[1] = height;
    }
    // apply new scale
    int bw = window_size[0] * buffer_scale;
    int bh = window_size[1] * buffer_scale;
    if(is_running()) {
        choose_surface();
        egl_window.resize(bw, bh);
        glViewport(0, 0, bw, bh);
    }
    on_buffer_resize(bw, bh, buffer_scale);
    window_resize_callback();
    app.tell_event(this);
}
auto WaylandWindow::handle_event() -> void {
    uint32_t repeated = key_repeated.load();
    key_repeated.store(0);
    if(repeated != 0) {
        auto key = last_pressed_key.load();
        for(uint32_t i = 0; i < repeated; ++i) {
            keyboard_callback(key, ButtonState::repeat);
        }
    }
    if(do_refresh.load()) {
        refresh();
    }
}
auto WaylandWindow::prepare() -> internal::FramebufferBinder {
    auto       binder = internal::FramebufferBinder(0);
    const auto size   = get_size();
    glViewport(0, 0, size[0], size[1]);
    return binder;
}
auto WaylandWindow::refresh() -> void {
    if(!is_running()) {
        return;
    }
    if(std::this_thread::get_id() != main_thread_id) {
        do_refresh.store(true);
        app.tell_event(this);
        return;
    }
    if(!frame_ready) {
        current_frame.store(false);
        return;
    }
    do_refresh.store(false);
    frame_ready = false;
    choose_surface();
    refresh_callback();

    frame_cb           = surface.frame();
    frame_cb.on_done() = [&](uint32_t /* elapsed */) {
        frame_ready  = true;
        auto current = bool();
        {
            const auto lock    = current_frame.get_lock();
            current            = current_frame.data;
            current_frame.data = true;
        }
        if(!get_event_driven() || !current) {
            refresh();
        }
    };
    swap_buffer();
    app.tell_event(this);
}
auto WaylandWindow::swap_buffer() -> void {
    ASSERT(eglSwapBuffers(egl_global->display, eglsurface) != EGL_FALSE, "eglSwapBuffers() failed")
}
auto WaylandWindow::choose_surface() -> void {
    static auto current_surface = EGLSurface(nullptr);
    if(current_surface == eglsurface) {
        return;
    }
    ASSERT(eglMakeCurrent(egl_global->display, eglsurface, eglsurface, egl_global->context) != EGL_FALSE, "eglMakeCurrent() failed")
    current_surface = eglsurface;
}
auto WaylandWindow::wait_for_key_repeater_exit() -> void {
    last_pressed_key.store(-1);
    key_delay_timer.wakeup();
    if(key_repeater.joinable()) {
        key_repeater.join();
    }
}
WaylandWindow::WaylandWindow(GawlApplication& app, WindowCreateHint hint)
    : GawlWindow(app), display(dynamic_cast<WaylandApplication*>(&app)->get_display()) {
    // retrieve global objects
    registry             = display.get_registry();
    registry.on_global() = [&](uint32_t name, const std::string& interface, uint32_t version) {
        if(interface == wayland::compositor_t::interface_name)
            registry.bind(name, compositor, version);
        else if(interface == wayland::xdg_wm_base_t::interface_name)
            registry.bind(name, xdg_wm_base, version);
        else if(interface == wayland::seat_t::interface_name)
            registry.bind(name, seat, version);
        else if(interface == wayland::shm_t::interface_name)
            registry.bind(name, shm, version);
        else if(interface == wayland::output_t::interface_name)
            registry.bind(name, output, version);
    };
    display.roundtrip();
    ASSERT(xdg_wm_base, "xdg_wm_base not supported")
    output.on_scale() = [&](const int32_t s) {
        resize_buffer(-1, -1, s);
    };
    seat.on_capabilities() = [&](const wayland::seat_capability& capability) {
        has_keyboard = capability & wayland::seat_capability::keyboard;
        has_pointer  = capability & wayland::seat_capability::pointer;
    };

    // create a surface
    surface                    = compositor.create_surface();
    xdg_wm_base.on_ping()      = [&](const uint32_t serial) { xdg_wm_base.pong(serial); };
    xdg_surface                = xdg_wm_base.get_xdg_surface(surface);
    xdg_surface.on_configure() = [&](const uint32_t serial) { xdg_surface.ack_configure(serial); };
    xdg_toplevel               = xdg_surface.get_toplevel();
    xdg_toplevel.set_title(hint.title);
    xdg_toplevel.on_close() = [&]() { close_request_callback(); };

    // create cursor surface
    cursor_surface = compositor.create_surface();

    surface.commit();
    display.roundtrip();

    // Get input devices
    ASSERT(has_keyboard, "no keyboard found")
    ASSERT(has_pointer, "no pointer found")
    pointer  = seat.get_pointer();
    keyboard = seat.get_keyboard();

    // draw cursor
    pointer.on_enter() = [&](const uint32_t serial, const wayland::surface_t& /*unused*/, const int32_t /*unused*/, const int32_t /*unused*/) {
        cursor_surface.attach(cursor_buffer, 0, 0);
        cursor_surface.damage(0, 0, cursor_image.width() * buffer_scale, cursor_image.height() * buffer_scale);
        cursor_surface.commit();
        pointer.set_cursor(serial, cursor_surface, 0, 0);
    };

    // intitialize egl
    egl_window = wayland::egl_window_t(surface, hint.width, hint.height);
    init_egl();

    // other configuration
    xdg_toplevel.on_configure() = [&](const int32_t w, const int32_t h, const wayland::array_t /*s*/) {
        resize_buffer(w, h, -1);
    };
    keyboard.on_key()         = [this](const uint32_t /*unused*/, const uint32_t /*unused*/, const uint32_t key, const wayland::keyboard_key_state state) {
        const auto s = state == wayland::keyboard_key_state::pressed ? gawl::ButtonState::press : gawl::ButtonState::release;
        keyboard_callback(key, s);
        if(!is_repeat_info_valid) {
            return;
        }
        if(s == gawl::ButtonState::press) {
            wait_for_key_repeater_exit();
            last_pressed_key.store(key);
            key_repeater                   = std::thread([this, key]() {
                key_delay_timer.wait_for(std::chrono::milliseconds(delay_in_milisec));
                while(last_pressed_key.load() == key) {
                    key_repeated.store(key_repeated.load() + 1);
                    this->app.tell_event(this);
                    key_delay_timer.wait_for(std::chrono::milliseconds(repeat_interval));
                }
            });
        } else if(last_pressed_key.load() == key){
            wait_for_key_repeater_exit();
        } };
    keyboard.on_repeat_info() = [&](const uint32_t repeat_per_sec, const uint32_t delay_in_milisec) {
        this->repeat_interval  = 1000 / repeat_per_sec;
        this->delay_in_milisec = delay_in_milisec;
        is_repeat_info_valid   = true;
    };
    keyboard.on_leave() = [this](const uint32_t /* serial */, const wayland::surface_t /* surface */) {
        wait_for_key_repeater_exit();
        keyboard_callback(-1, ButtonState::leave);
    };
    pointer.on_button() = [&](const uint32_t /*serial*/, const uint32_t /*time*/, const uint32_t button, const wayland::pointer_button_state state) {
        const auto s = state == wayland::pointer_button_state::pressed ? gawl::ButtonState::press : gawl::ButtonState::release;
        click_callback(button, s);
    };
    pointer.on_motion() = [&](const uint32_t, const double x, const double y) {
        pointermove_callback(x, y);
    };
    pointer.on_axis() = [&](const uint32_t, const wayland::pointer_axis axis, const double value) {
        const auto w = axis == wayland::pointer_axis::horizontal_scroll ? gawl::WheelAxis::horizontal : gawl::WheelAxis::vertical;
        scroll_callback(w, value);
    };
    main_thread_id = std::this_thread::get_id();

    init_global();
    init_complete();
}
WaylandWindow::~WaylandWindow() {
    wait_for_key_repeater_exit();
    // finialize EGL.
    ASSERT(eglDestroySurface(egl_global->display, eglsurface) != EGL_FALSE, "eglDestroyContext() failed")
    if(egl_global_count == 1) {
        egl_global.reset();
    }
    egl_global_count -= 1;
}
} // namespace gawl
