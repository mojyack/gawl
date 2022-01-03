#pragma once
#include <thread>

#include <wayland-client-core.h>
#include <wayland-client-protocol-extra.hpp>
#include <wayland-client-protocol.hpp>
#include <wayland-cursor.hpp>
#include <wayland-egl.hpp>

#include "../binder.hpp"
#include "../type.hpp"
#include "../variant-buffer.hpp"
#include "../window-creat-hint.hpp"
#include "../window-impl-concept.hpp"
#include "../window.hpp"
#include "eglobject.hpp"
#include "shared-data.hpp"

namespace gawl::internal::wl {
template <class Impl>
class WindowBackend : public gawl::internal::Window<Impl> {
  private:
    struct RefreshCallbackArgs {};
    struct WindowResizeCallbackArgs {};
    struct KeyBoardCallbackArgs {
        uint32_t          key;
        gawl::ButtonState state;
    };
    struct PointermoveCallbackArgs {
        gawl::Point point;
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
    using CallbackQueue = VariantBuffer<RefreshCallbackArgs, WindowResizeCallbackArgs, KeyBoardCallbackArgs, PointermoveCallbackArgs, ClickCallbackArgs, ScrollCallbackArgs, CloseRequestCallbackArgs, UserCallbackArgs>;
    CallbackQueue callback_queue;

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

    using BackendType = WindowBackend;
    using SharedData  = SharedData<WindowBackend<Impl>>;
    using BufferType  = typename SharedData::BufferType;

    BufferType& application_events;
    EGLObject&  egl;

    std::atomic_bool               frame_done   = true;
    std::atomic_bool               latest_frame = true;
    int                            buffer_scale = 0;
    TimerEvent                     key_delay_timer;
    std::thread                    key_repeater;
    Critical<uint32_t>             last_pressed_key = -1;
    std::optional<KeyRepeatConfig> repeat_config;

    auto init_egl() -> void {
        eglsurface = eglCreateWindowSurface(egl.display, egl.config, egl_window, nullptr);
        assert(eglsurface != EGL_NO_SURFACE);
        choose_surface();
    }
    auto resize_buffer(const int width, const int height, const int scale) -> void {
        auto window_size = this->get_window_size();
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
        const auto bw = window_size[0] * buffer_scale;
        const auto bh = window_size[1] * buffer_scale;
        if(this->get_state() == WindowState::Running) {
            choose_surface();
            egl_window.resize(bw, bh);
        }
        this->on_buffer_resize(bw, bh, buffer_scale);
        if constexpr(gawl::concepts::WindowImplWithWindowResizeCallback<Impl>) {
            queue_callback(WindowResizeCallbackArgs{});
        }
        refresh();
    }
    auto swap_buffer() -> void {
        assert(eglSwapBuffers(egl.display, eglsurface) != EGL_FALSE);
    }
    auto choose_surface() -> void {
        static auto current_surface = EGLSurface(nullptr);
        if(current_surface == eglsurface) {
            return;
        }
        assert(eglMakeCurrent(egl.display, eglsurface, eglsurface, egl.context) != EGL_FALSE);
        current_surface = eglsurface;
    }
    auto wait_for_key_repeater_exit() -> void {
        last_pressed_key.store(-1);
        key_delay_timer.wakeup();
        if(key_repeater.joinable()) {
            key_repeater.join();
        }
    }
    auto queue_callback(auto&& args) -> void {
        if(this->get_state() != gawl::internal::WindowState::Running) {
            return;
        }
        callback_queue.push(std::move(args));
        application_events.push(typename SharedData::HandleEventArgs{*this});
    }

  public:
    using WindowCreateHintType = WindowCreateHint<internal::wl::SharedData<WindowBackend<Impl>>>;

    auto handle_event() -> void {
        auto queue = callback_queue.exchange();
        do {
            for(const auto& a : queue) {
                switch(a.index()) {
                case decltype(callback_queue)::template index_of<RefreshCallbackArgs>(): {
                    frame_done = false;
                    choose_surface();
                    if constexpr(gawl::concepts::WindowImplWithRefreshCallback<Impl>) {
                        this->impl()->refresh_callback();
                    }
                    frame_cb           = surface.frame();
                    frame_cb.on_done() = [&](uint32_t /* elapsed */) {
                        frame_done = true;
                        if(!latest_frame.exchange(true) || !this->get_event_driven()) {
                            queue_callback(RefreshCallbackArgs{});
                        }
                    };
                    swap_buffer();
                } break;
                case decltype(callback_queue)::template index_of<WindowResizeCallbackArgs>():
                    if constexpr(gawl::concepts::WindowImplWithWindowResizeCallback<Impl>) {
                        this->impl()->window_resize_callback();
                    }
                    break;
                case decltype(callback_queue)::template index_of<KeyBoardCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
                        const auto& args = a.template get<KeyBoardCallbackArgs>();
                        this->impl()->keyboard_callback(args.key, args.state);
                    }
                } break;
                case decltype(callback_queue)::template index_of<PointermoveCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithPointermoveCallback<Impl>) {
                        const auto& args = a.template get<PointermoveCallbackArgs>();
                        this->impl()->pointermove_callback(args.point);
                    }
                } break;
                case decltype(callback_queue)::template index_of<ClickCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithClickCallback<Impl>) {
                        const auto& args = a.template get<ClickCallbackArgs>();
                        this->impl()->click_callback(args.key, args.state);
                    }
                } break;
                case decltype(callback_queue)::template index_of<ScrollCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithScrollCallback<Impl>) {
                        const auto& args = a.template get<ScrollCallbackArgs>();
                        this->impl()->scroll_callback(args.axis, args.value);
                    }
                } break;
                case decltype(callback_queue)::template index_of<CloseRequestCallbackArgs>():
                    if constexpr(gawl::concepts::WindowImplWithCloseRequestCallback<Impl>) {
                        this->impl()->close_request_callback();
                    }
                    break;
                case decltype(callback_queue)::template index_of<UserCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithUserCallback<Impl>) {
                        const auto& args = a.template get<UserCallbackArgs>();
                        this->impl()->user_callback(args.data);
                    }
                } break;
                }
            }
        } while(!(queue = callback_queue.exchange()).empty());
    }
    auto prepare() -> gawl::internal::FramebufferBinder {
        auto       binder = gawl::internal::FramebufferBinder(0);
        const auto size   = this->get_size();
        glViewport(0, 0, size[0], size[1]);
        return binder;
    }
    auto refresh() -> void {
        latest_frame.store(false);
        if(!frame_done) {
            return;
        }
        queue_callback(RefreshCallbackArgs{});
    }
    auto invoke_user_callback(void* data = nullptr) -> void {
        if constexpr(gawl::concepts::WindowImplWithUserCallback<Impl>) {
            queue_callback(UserCallbackArgs{data});
        }
    }
    auto close_window() -> void {
        application_events.push(typename SharedData::CloseWindowArgs{*this});
    }
    auto quit_application() -> void {
        application_events.push(typename SharedData::QuitApplicationArgs{});
    }
    WindowBackend(const WindowCreateHintType& hint) : display(*hint.backend_hint.display), application_events(*hint.backend_hint.application_events), egl(*hint.backend_hint.egl) {
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
        assert(xdg_wm_base);
        output.on_scale() = [this](const int32_t s) {
            resize_buffer(-1, -1, s);
        };

        auto has_pointer       = false;
        auto has_keyboard      = false;
        seat.on_capabilities() = [&has_keyboard, &has_pointer](const wayland::seat_capability& capability) {
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
        xdg_toplevel.on_close() = [this]() {
            if constexpr(gawl::concepts::WindowImplWithCloseRequestCallback<Impl>) {
                queue_callback(CloseRequestCallbackArgs{});
            } else {
                this->quit_application();
            }
        };

        // create cursor surface
        cursor_surface = compositor.create_surface();

        surface.commit();
        display.roundtrip();

        // get input devices
        assert(has_keyboard);
        assert(has_pointer);
        pointer  = seat.get_pointer();
        keyboard = seat.get_keyboard();

        // draw cursor
        pointer.on_enter() = [this](const uint32_t serial, const wayland::surface_t& /*unused*/, const int32_t /*unused*/, const int32_t /*unused*/) {
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
        if constexpr(gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
            keyboard.on_key() = [this](const uint32_t /*unused*/, const uint32_t /*unused*/, const uint32_t key, const wayland::keyboard_key_state state) {
                const auto s = state == wayland::keyboard_key_state::pressed ? gawl::ButtonState::Press : gawl::ButtonState::Release;
                queue_callback(KeyBoardCallbackArgs{key, s});
                if(!repeat_config.has_value()) {
                    return;
                }
                if(s == gawl::ButtonState::Press) {
                    wait_for_key_repeater_exit();
                    last_pressed_key.store(key);
                    key_repeater = std::thread([this, key]() {
                        key_delay_timer.wait_for(std::chrono::milliseconds(repeat_config->delay_in_milisec));
                        while(last_pressed_key.load() == key) {
                            queue_callback(KeyBoardCallbackArgs{key, gawl::ButtonState::Repeat});
                            key_delay_timer.wait_for(std::chrono::milliseconds(repeat_config->interval));
                        }
                    });
                } else if(last_pressed_key.load() == key) {
                    wait_for_key_repeater_exit();
                }
            };
            keyboard.on_repeat_info() = [this](const uint32_t repeat_per_sec, const uint32_t delay_in_milisec) {
                repeat_config.emplace(KeyRepeatConfig{1000 / repeat_per_sec, delay_in_milisec});
            };
            keyboard.on_leave() = [this](const uint32_t /* serial */, const wayland::surface_t /* surface */) {
                wait_for_key_repeater_exit();
                queue_callback(KeyBoardCallbackArgs{static_cast<uint32_t>(-1), gawl::ButtonState::Leave});
            };
        }
        if constexpr(gawl::concepts::WindowImplWithClickCallback<Impl>) {
            pointer.on_button() = [this](const uint32_t /*serial*/, const uint32_t /*time*/, const uint32_t button, const wayland::pointer_button_state state) {
                const auto s = state == wayland::pointer_button_state::pressed ? gawl::ButtonState::Press : gawl::ButtonState::Release;
                queue_callback(ClickCallbackArgs{button, s});
            };
        }
        if constexpr(gawl::concepts::WindowImplWithPointermoveCallback<Impl>) {
            pointer.on_motion() = [this](const uint32_t, const double x, const double y) {
                queue_callback(PointermoveCallbackArgs{gawl::Point{x, y}});
            };
        }
        if constexpr(gawl::concepts::WindowImplWithScrollCallback<Impl>) {
            pointer.on_axis() = [this](const uint32_t, const wayland::pointer_axis axis, const double value) {
                const auto w = axis == wayland::pointer_axis::horizontal_scroll ? gawl::WheelAxis::Horizontal : gawl::WheelAxis::Vertical;
                queue_callback(ScrollCallbackArgs{w, value});
            };
        }

        this->set_event_driven(hint.manual_refresh);
        this->set_state(gawl::internal::WindowState::Running);
        refresh();
    }
    ~WindowBackend() {
        wait_for_key_repeater_exit();
        // finialize EGL.
        assert(eglDestroySurface(egl.display, eglsurface) != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
