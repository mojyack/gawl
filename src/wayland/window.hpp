#pragma once
#include <thread>

#include "../window-impl-concept.hpp"
#include "shared-data.hpp"
#include "wlobject.hpp"

namespace gawl::wl {
namespace internal {
inline auto choose_surface(const EGLSurface eglsurface, const EGLObject& egl) -> void {
    static auto current_surface = EGLSurface(nullptr);
    if(current_surface == eglsurface) {
        return;
    }
    dynamic_assert(eglMakeCurrent(egl.display, eglsurface, eglsurface, egl.context) != EGL_FALSE);
    current_surface = eglsurface;
}
} // namespace internal

template <class Impl>
class Window : public internal::WindowBase {
  private:
    class SurfaceGlue {
      private:
        Window*                    window;
        internal::towl::OutputTag* current_output;

      public:
        auto on_enter(const internal::towl::OutputTag output) -> void {
            *current_output = output;
            window->resize_buffer(-1, -1, internal::Output::from_tag(output).get_scale());
        }

        auto on_leave(const internal::towl::OutputTag /*output*/) -> void {
            *current_output = internal::towl::nulltag;
        }

        auto on_frame() -> void {
            window->frame_done = true;
            if(!window->latest_frame.exchange(true) || !window->get_event_driven()) {
                window->queue_callback<internal::callback::Refresh>();
            }
        }

        SurfaceGlue(Window& window, internal::towl::OutputTag& output)
            : window(&window),
              current_output(&output) {}
    };

    class XDGToplevelGlue {
      private:
        Window* window;

      public:
        auto on_configure(const int32_t width, const int32_t height) -> void {
            {
                const auto& buffer_size = window->get_buffer_size();
                const auto [lock, data] = buffer_size.access();
                if(data.size[0] == static_cast<size_t>(width) * data.scale && data.size[1] == static_cast<size_t>(height) * data.scale) {
                    return;
                }
            }
            window->resize_buffer(width, height, -1);
        }

        auto on_close() -> void {
            if constexpr(gawl::concepts::has_close_request_callback<Impl>) {
                window->queue_callback<internal::callback::CloseRequest>();
            } else {
                window->quit_application();
            }
        }

        XDGToplevelGlue(Window& window)
            : window(&window) {}
    };

    struct Keyboard {
        TimerEvent            key_delay_timer;
        Critical<std::thread> key_repeater;
        std::atomic_uint32_t  last_pressed_key = -1;
    };

    constexpr static auto enable_keycode  = gawl::concepts::has_keycode_callback<Impl>;
    constexpr static auto enable_keysym   = gawl::concepts::has_keysym_callback<Impl>;
    constexpr static auto enable_keyboard = enable_keycode || enable_keysym;
    constexpr static auto enable_mouse =
        gawl::concepts::has_click_callback<Impl> ||
        gawl::concepts::has_pointer_move_callback<Impl> ||
        gawl::concepts::has_scroll_callback<Impl>;

#if !defined(GAWL_KEYCODE)
    static_assert(!enable_keycode, "add #define GAWL_KEYCODE to use keycode feature");
#endif
#if !defined(GAWL_KEYSYM)
    static_assert(!enable_keysym, "add #define GAWL_KEYSYM to use keysym feature");
#endif
#if !defined(GAWL_MOUSE)
    static_assert(!enable_mouse, "add #define GAWL_MOUSE to use mouse feature");
#endif

    using KeyboardOpt = std::conditional_t<enable_keyboard, Keyboard, internal::towl::Empty>;

    internal::towl::OutputTag                      output;
    EGLSurface                                     eglsurface = nullptr;
    internal::Compositor::Surface<SurfaceGlue>     surface;
    internal::WMBase::XDGSurface                   xdg_surface;
    internal::WMBase::XDGToplevel<XDGToplevelGlue> xdg_toplevel;
    internal::towl::EGLWindow                      egl_window;
    [[no_unique_address]] KeyboardOpt              keyboard;
    Impl                                           impl;
    std::atomic_bool                               obsolete_egl_window_size = true;

    auto init_egl() -> void {
        eglsurface = eglCreateWindowSurface(this->egl.display, this->egl.config, std::bit_cast<EGLNativeWindowType>(this->egl_window.native()), nullptr);
        dynamic_assert(eglsurface != EGL_NO_SURFACE);
        choose_surface(eglsurface, this->egl);
        dynamic_assert(eglSwapInterval(this->egl.display, 0) == EGL_TRUE); // make eglSwapBuffers non-blocking
    }

    auto swap_buffer() -> void {
        dynamic_assert(eglSwapBuffers(this->egl.display, this->eglsurface) != EGL_FALSE);
    }

    auto wait_for_key_repeater_exit(std::thread& repeater) -> void {
        if constexpr(enable_keyboard) {
            keyboard.last_pressed_key.store(-1);
            keyboard.key_delay_timer.wakeup();
            if(repeater.joinable()) {
                repeater.join();
            }
        }
    }

    auto resize_buffer(const int width, const int height, const int scale) -> void {
        auto new_scale  = size_t();
        auto new_width  = size_t();
        auto new_height = size_t();
        {
            const auto& buffer      = this->get_buffer_size();
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
        this->on_buffer_resize(std::array{new_width, new_height}, new_scale);

        obsolete_egl_window_size = true;
        if constexpr(gawl::concepts::has_window_resize_callback<Impl>) {
            queue_callback<internal::callback::WindowResize>();
        }
        this->refresh();
    }

    auto handle_event() -> void override {
        using Queue = internal::callback::Queue;
        for(const auto& a : this->queue.swap()) {
            switch(a.get_index()) {
            case Queue::index_of<internal::callback::Refresh>: {
                choose_surface(eglsurface, this->egl);
                if(obsolete_egl_window_size) {
                    obsolete_egl_window_size         = false;
                    const auto& critical_buffer_size = this->get_buffer_size();
                    const auto [lock, buffer_size]   = critical_buffer_size.access();
                    egl_window.resize(buffer_size.size[0], buffer_size.size[1], 0, 0);
                    swap_buffer(); // ensure buffer sizes are changed to prevent "Buffer size is not divisible by scale" error
                    surface.set_buffer_scale(buffer_size.scale);
                }
                if constexpr(gawl::concepts::has_refresh_callback<Impl>) {
                    impl.refresh_callback();
                }
                surface.set_frame();
                swap_buffer();
            } break;
            case Queue::index_of<internal::callback::WindowResize>:
                if constexpr(gawl::concepts::has_window_resize_callback<Impl>) {
                    impl.window_resize_callback();
                }
                break;
            case Queue::index_of<internal::callback::Keycode>: {
                if constexpr(gawl::concepts::has_keycode_callback<Impl>) {
                    const auto& args = a.template as<internal::callback::Keycode>();
                    impl.keycode_callback(args.key, args.state);
                }
            } break;
            case Queue::index_of<internal::callback::Keysym>: {
                if constexpr(gawl::concepts::has_keysym_callback<Impl>) {
                    const auto& args = a.template as<internal::callback::Keysym>();
                    impl.keysym_callback(args.key, args.state, args.xkbstate);
                }
            } break;
            case Queue::index_of<internal::callback::PointerMove>: {
                if constexpr(gawl::concepts::has_pointer_move_callback<Impl>) {
                    const auto& args = a.template as<internal::callback::PointerMove>();
                    impl.pointer_move_callback(args.pos);
                }
            } break;
            case Queue::index_of<internal::callback::Click>: {
                if constexpr(gawl::concepts::has_click_callback<Impl>) {
                    const auto& args = a.template as<internal::callback::Click>();
                    impl.click_callback(args.key, args.state);
                }
            } break;
            case Queue::index_of<internal::callback::Scroll>: {
                if constexpr(gawl::concepts::has_scroll_callback<Impl>) {
                    const auto& args = a.template as<internal::callback::Scroll>();
                    impl.scroll_callback(args.axis, args.value);
                }
            } break;
            case Queue::index_of<internal::callback::CloseRequest>:
                if constexpr(gawl::concepts::has_close_request_callback<Impl>) {
                    impl.close_request_callback();
                }
                break;
            case Queue::index_of<internal::callback::User>: {
                if constexpr(gawl::concepts::has_user_callback<Impl>) {
                    const auto& args = a.template as<internal::callback::User>();
                    impl.user_callback(args.data);
                }
            } break;
            }
        }
    }

    auto wl_get_surface() -> internal::towl::SurfaceTag override {
        return surface.as_tag();
    }

    auto wl_get_output() const -> internal::towl::OutputTag override {
        return output;
    }

    auto wl_on_keycode_enter(const internal::towl::Array<uint32_t>& keys) -> void override {
        if constexpr(enable_keycode) {
            for(auto i = size_t(0); i < keys.size; i += 1) {
                queue_callback<internal::callback::Keycode>(keys.data[i], gawl::ButtonState::Enter);
            }
        }
    }

    auto wl_on_keysym_enter(const std::vector<xkb_keycode_t>& keys, xkb_state* const xkb_state) -> void override {
        if constexpr(enable_keysym) {
            for(const auto k : keys) {
                queue_callback<internal::callback::Keysym>(k, gawl::ButtonState::Enter, xkb_state);
            }
        }
    }

    auto wl_on_key_leave() -> void override {
        if constexpr(enable_keyboard) {
            auto [lock, repeater] = keyboard.key_repeater.access();
            wait_for_key_repeater_exit(repeater);
        }
        if constexpr(enable_keycode) {
            queue_callback<internal::callback::Keycode>(static_cast<uint32_t>(-1), gawl::ButtonState::Leave);
        }
        if constexpr(enable_keysym) {
            queue_callback<internal::callback::Keysym>(XKB_KEYCODE_INVALID, gawl::ButtonState::Leave, nullptr);
        }
    }

    auto wl_on_keycode_input(const uint32_t keycode, const uint32_t state) -> void override {
        if constexpr(enable_keycode) {
            const auto s = state == WL_KEYBOARD_KEY_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;
            queue_callback<internal::callback::Keycode>(keycode, s);
            if(!wl.repeat_config.has_value()) {
                return;
            }
            auto [lock, repeater] = keyboard.key_repeater.access();
            if(s == gawl::ButtonState::Press) {
                if(this->get_state() != ::gawl::internal::WindowState::Running) {
                    return;
                }
                wait_for_key_repeater_exit(repeater);
                keyboard.last_pressed_key.store(keycode);

                repeater = std::thread([this, keycode]() {
                    keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->delay_in_milisec));
                    while(keyboard.last_pressed_key.load() == keycode) {
                        queue_callback<internal::callback::Keycode>(keycode, gawl::ButtonState::Repeat);
                        keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->interval));
                    }
                });

            } else if(keyboard.last_pressed_key.load() == keycode) {
                wait_for_key_repeater_exit(repeater);
            }
        }
    }

    auto wl_on_keysym_input(const xkb_keycode_t keysym, uint32_t state, const bool enable_repeat, xkb_state* const xkb_state) -> void override {
        if constexpr(enable_keysym) {
            const auto s = state == WL_KEYBOARD_KEY_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;
            queue_callback<internal::callback::Keysym>(keysym, s, xkb_state);
            if(!wl.repeat_config.has_value()) {
                return;
            }
            auto [lock, repeater] = keyboard.key_repeater.access();
            if(s == gawl::ButtonState::Press && enable_repeat) {
                if(this->get_state() != ::gawl::internal::WindowState::Running) {
                    return;
                }
                wait_for_key_repeater_exit(repeater);
                keyboard.last_pressed_key.store(keysym);

                repeater = std::thread([this, keysym, xkb_state]() {
                    keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->delay_in_milisec));
                    while(keyboard.last_pressed_key.load() == keysym) {
                        queue_callback<internal::callback::Keysym>(keysym, gawl::ButtonState::Repeat, xkb_state);
                        keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->interval));
                    }
                });

            } else if(keyboard.last_pressed_key.load() == keysym) {
                wait_for_key_repeater_exit(repeater);
            }
        }
    }

    auto wl_on_click(const uint32_t button, const uint32_t state) -> void override {
        if constexpr(gawl::concepts::has_click_callback<Impl>) {
            const auto s = state == WL_POINTER_BUTTON_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;
            queue_callback<internal::callback::Click>(button, s);
        }
    }

    auto wl_on_pointer_motion(const double x, const double y) -> void override {
        if constexpr(gawl::concepts::has_pointer_move_callback<Impl>) {
            queue_callback<internal::callback::PointerMove>(gawl::Point{x, y});
        }
    }

    auto wl_on_pointer_axis(const uint32_t axis, const double value) -> void override {
        if constexpr(gawl::concepts::has_scroll_callback<Impl>) {
            const auto w = axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? gawl::WheelAxis::Horizontal : gawl::WheelAxis::Vertical;
            queue_callback<internal::callback::Scroll>(w, value);
        }
    }

    auto wl_set_output_scale(uint32_t scale) -> void override {
        resize_buffer(-1, -1, scale);
    }

  public:
    auto get_impl() -> Impl& {
        return impl;
    }

    template <class... Args>
    Window(const WindowCreateHint hint, const void* const backend_hint, Args&&... args)
        : WindowBase(*std::bit_cast<internal::SharedData*>(backend_hint)->wl,
                     *std::bit_cast<internal::SharedData*>(backend_hint)->egl,
                     *std::bit_cast<internal::SharedData*>(backend_hint)->application_events),
          surface(wl.registry.template interface<internal::Compositor>()[0].create_surface(SurfaceGlue(*this, output))),
          xdg_surface(wl.registry.template interface<internal::WMBase>()[0].create_xdg_surface(surface)),
          xdg_toplevel(xdg_surface.create_xdg_toplevel(XDGToplevelGlue(*this))),
          egl_window(surface, hint.width, hint.height), impl(*this, args...) {
        surface.commit();
        wl.display.wait_sync();
        xdg_toplevel.set_title(hint.title);
        init_egl();
        resize_buffer(hint.width, hint.height, -1);

        // swap_buffer();
        wl.display.wait_sync();

        this->set_event_driven(hint.manual_refresh);
        this->set_state(::gawl::internal::WindowState::Running);
    }

    ~Window() {
        if constexpr(enable_keyboard) {
            const auto [lock, data] = keyboard.key_repeater.access();
            wait_for_key_repeater_exit(data);
        }
        dynamic_assert(eglDestroySurface(this->egl.display, this->eglsurface) != EGL_FALSE);
    }
};
} // namespace gawl::wl
