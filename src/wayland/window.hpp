#pragma once
#include <thread>

#include "../binder.hpp"
#include "../type.hpp"
#include "../variant-buffer.hpp"
#include "../window-creat-hint.hpp"
#include "../window-impl-concept.hpp"
#include "../window.hpp"
#include "eglobject.hpp"
#include "shared-data.hpp"
#include "wlobject.hpp"

namespace gawl::internal::wl {
inline auto choose_surface(const EGLSurface eglsurface, const EGLObject& egl) -> void {
    static auto current_surface = EGLSurface(nullptr);
    if(current_surface == eglsurface) {
        return;
    }
    assert(eglMakeCurrent(egl.display, eglsurface, eglsurface, egl.context) != EGL_FALSE);
    current_surface = eglsurface;
}

template <class Impl, class... Impls>
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

    using BackendType = WindowBackend;
    using SharedData  = internal::wl::SharedData<Impls...>;
    using BufferType  = typename SharedData::BufferType;

    WaylandClientObject&  wl;
    WaylandWindowObject   wlw;
    EGLObject&            egl;
    BufferType&           application_events;
    CallbackQueue         callback_queue;
    wayland::egl_window_t egl_window;
    EGLSurface            eglsurface = nullptr;

    std::atomic_bool      frame_done               = true;
    std::atomic_bool      latest_frame             = true;
    std::atomic_bool      obsolete_egl_window_size = true;
    TimerEvent            key_delay_timer;
    Critical<std::thread> key_repeater;
    std::atomic_uint32_t  last_pressed_key = -1;
    int                   buffer_scale     = -1;

    auto init_egl() -> void {
        eglsurface = eglCreateWindowSurface(egl.display, egl.config, egl_window, nullptr);
        assert(eglsurface != EGL_NO_SURFACE);
        choose_surface(eglsurface, egl);
        assert(eglSwapInterval(egl.display, 0) == EGL_TRUE); // make eglSwapBuffers non-blocking
    }
    auto swap_buffer() -> void {
        assert(eglSwapBuffers(egl.display, eglsurface) != EGL_FALSE);
    }
    auto wait_for_key_repeater_exit() -> void {
        last_pressed_key.store(-1);
        key_delay_timer.wakeup();
        if(key_repeater->joinable()) {
            key_repeater->join();
        }
    }
    auto queue_callback(auto&& args) -> void {
        if(this->get_state() != gawl::internal::WindowState::Running) {
            return;
        }
        callback_queue.push(std::move(args));
        application_events.push(typename SharedData::HandleEventArgs{this->impl()});
    }

  public:
    using WindowCreateHintType = WindowCreateHint<internal::wl::SharedData<Impls...>>;

    auto wl_get_object() -> WaylandWindowObject& {
        return wlw;
    }
    auto wl_on_key_enter(const std::vector<uint32_t>& keys) -> void {
        for(const auto key : keys) {
            queue_callback(KeyBoardCallbackArgs{key, gawl::ButtonState::Enter});
        }
    }
    auto wl_on_key_leave() -> void {
        wait_for_key_repeater_exit();
        queue_callback(KeyBoardCallbackArgs{static_cast<uint32_t>(-1), gawl::ButtonState::Leave});
    }
    auto wl_on_key_input(const uint32_t key, const wayland::keyboard_key_state state) -> void {
        const auto s = state == wayland::keyboard_key_state::pressed ? gawl::ButtonState::Press : gawl::ButtonState::Release;
        queue_callback(KeyBoardCallbackArgs{key, s});
        if(!wl.repeat_config.has_value()) {
            return;
        }
        if(s == gawl::ButtonState::Press) {
            const auto lock = key_repeater.get_lock();
            if(this->get_state() != gawl::internal::WindowState::Running) {
                return;
            }
            wait_for_key_repeater_exit();
            last_pressed_key.store(key);
            *key_repeater = std::thread([this, key]() {
                key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->delay_in_milisec));
                while(last_pressed_key.load() == key) {
                    queue_callback(KeyBoardCallbackArgs{key, gawl::ButtonState::Repeat});
                    key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->interval));
                }
            });
        } else if(last_pressed_key.load() == key) {
            wait_for_key_repeater_exit();
        }
    }
    auto wl_on_click(const uint32_t button, const wayland::pointer_button_state state) -> void {
        const auto s = state == wayland::pointer_button_state::pressed ? gawl::ButtonState::Press : gawl::ButtonState::Release;
        queue_callback(ClickCallbackArgs{button, s});
    }
    auto wl_on_pointer_motion(const double x, const double y) -> void {
        queue_callback(PointermoveCallbackArgs{gawl::Point{x, y}});
    }
    auto wl_on_pointer_axis(const wayland::pointer_axis axis, const double value) -> void {
        const auto w = axis == wayland::pointer_axis::horizontal_scroll ? gawl::WheelAxis::Horizontal : gawl::WheelAxis::Vertical;
        queue_callback(ScrollCallbackArgs{w, value});
    }

    auto resize_buffer(const int width, const int height) -> void {
        if(width == -1 || height == -1) {
            // update scale
            if(buffer_scale == wl.buffer_scale) {
                return;
            }
            buffer_scale = wl.buffer_scale;
            wlw.surface.set_buffer_scale(buffer_scale);

            // load cursor theme
            wayland::cursor_theme_t cursor_theme = wayland::cursor_theme_t("default", 24 * buffer_scale, wl.shm);
            wayland::cursor_t       cursor       = cursor_theme.get_cursor("left_ptr");
            wlw.cursor_image                     = cursor.image(0);
            wlw.cursor_buffer                    = wlw.cursor_image.get_buffer();
            wlw.cursor_surface.set_buffer_scale(buffer_scale);
            this->on_buffer_resize(std::nullopt, buffer_scale);
        } else {
            // update buffer size
            auto new_width  = size_t(width * buffer_scale);
            auto new_height = size_t(height * buffer_scale);
            this->on_buffer_resize(std::array{new_width, new_height}, std::nullopt);
        }

        // apply new buffer size
        obsolete_egl_window_size = true;
        if constexpr(gawl::concepts::WindowImplWithWindowResizeCallback<Impl>) {
            queue_callback(WindowResizeCallbackArgs{});
        }
        refresh();
    }
    auto handle_event() -> void {
        auto queue = callback_queue.exchange();
        do {
            for(const auto& a : queue) {
                switch(a.index()) {
                case decltype(callback_queue)::template index_of<RefreshCallbackArgs>(): {
                    frame_done = false;
                    choose_surface(eglsurface, egl);
                    if(obsolete_egl_window_size) {
                        obsolete_egl_window_size = false;
                        const auto& buffer_size  = this->get_buffer_size();
                        const auto  lock         = buffer_size.get_lock();
                        egl_window.resize(buffer_size->size[0], buffer_size->size[1]);
                    }
                    if constexpr(gawl::concepts::WindowImplWithRefreshCallback<Impl>) {
                        this->impl()->refresh_callback();
                    }
                    wlw.frame_cb           = wlw.surface.frame();
                    wlw.frame_cb.on_done() = [&](uint32_t /* elapsed */) {
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
        this->set_state(gawl::internal::WindowState::Destructing);
        application_events.push(typename SharedData::CloseWindowArgs{this->impl()});
    }
    auto quit_application() -> void {
        application_events.push(typename SharedData::QuitApplicationArgs{});
    }
    WindowBackend(const WindowCreateHintType& hint) : wl(*hint.backend_hint.wl), egl(*hint.backend_hint.egl), application_events(*hint.backend_hint.application_events) {
        // create a surface
        wlw.surface                    = wl.compositor.create_surface();
        wlw.xdg_surface                = wl.xdg_wm_base.get_xdg_surface(wlw.surface);
        wlw.xdg_surface.on_configure() = [&](const uint32_t serial) { wlw.xdg_surface.ack_configure(serial); };
        wlw.xdg_toplevel               = wlw.xdg_surface.get_toplevel();
        wlw.xdg_toplevel.set_title(hint.title);
        wlw.xdg_toplevel.on_close() = [this]() {
            if constexpr(gawl::concepts::WindowImplWithCloseRequestCallback<Impl>) {
                queue_callback(CloseRequestCallbackArgs{});
            } else {
                this->quit_application();
            }
        };

        // create cursor surface
        wlw.cursor_surface = wl.compositor.create_surface();

        wlw.surface.commit();
        wl.display.roundtrip();

        // intitialize egl
        egl_window = wayland::egl_window_t(wlw.surface, hint.width, hint.height);
        init_egl();

        // other configuration
        wlw.xdg_toplevel.on_configure() = [this](const int32_t w, const int32_t h, const wayland::array_t /*s*/) {
            {
                const auto& buffer_size = this->get_buffer_size();
                const auto  lock        = buffer_size.get_lock();
                if(buffer_size->size[0] == static_cast<size_t>(w) * buffer_size->scale && buffer_size->size[1] == static_cast<size_t>(h) * buffer_size->scale) {
                    return;
                }
            }
            resize_buffer(w, h);
        };

        resize_buffer(-1, -1); // load buffer scale from application

        this->set_event_driven(hint.manual_refresh);
        this->set_state(gawl::internal::WindowState::Running);
        refresh();
    }
    ~WindowBackend() {
        const auto lock = key_repeater.get_lock();
        wait_for_key_repeater_exit();
        // finialize EGL.
        assert(eglDestroySurface(egl.display, eglsurface) != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
