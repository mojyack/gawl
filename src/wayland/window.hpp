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
struct RefreshCallbackArgs {};
struct WindowResizeCallbackArgs {};
struct KeycodeCallbackArgs {
    uint32_t          key;
    gawl::ButtonState state;
};
struct KeysymCallbackArgs {
    xkb_keycode_t     key;
    gawl::ButtonState state;
    xkb_state*        xkbstate;
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
using CallbackQueue = VariantBuffer<RefreshCallbackArgs, WindowResizeCallbackArgs, KeycodeCallbackArgs, KeysymCallbackArgs, PointermoveCallbackArgs, ClickCallbackArgs, ScrollCallbackArgs, CloseRequestCallbackArgs, UserCallbackArgs>;

template <class Impl, class... Impls>
class WindowBackend;
} // namespace gawl::internal::wl

namespace gawl::wl {
template <class Impl, class... Impls>
class Window : public internal::Window {
  private:
    auto backend() -> gawl::internal::wl::WindowBackend<Impl, Impls...>* {
        return reinterpret_cast<gawl::internal::wl::WindowBackend<Impl, Impls...>*>(this);
    }

  protected:
    internal::wl::EGLObject& egl;

    std::atomic_bool                  frame_done   = true;
    std::atomic_bool                  latest_frame = true;
    gawl::internal::wl::CallbackQueue queue;

    using SharedData = internal::wl::SharedData<gawl::internal::wl::WindowBackend, Impls...>;
    typename SharedData::BufferType& application_events;

    auto queue_callback(auto&& args) -> void {
        if(get_state() == internal::WindowState::Destructing) {
            return;
        }

        queue.push(std::move(args));
        application_events.push(typename SharedData::HandleEventArgs{backend()});
    }

  public:
    auto refresh() -> void {
        latest_frame.store(false);
        if(!frame_done) {
            return;
        }
        frame_done = false;
        queue_callback(internal::wl::RefreshCallbackArgs{});
    }

    auto invoke_user_callback(void* const data = nullptr) -> void {
        if constexpr(gawl::concepts::WindowImplWithUserCallback<Impl>) {
            queue_callback(internal::wl::UserCallbackArgs{data});
        }
    }

    auto close_window() -> void {
        set_state(internal::WindowState::Destructing);
        application_events.push(typename SharedData::CloseWindowArgs{backend()});
    }

    auto quit_application() -> void {
        application_events.push(typename SharedData::QuitApplicationArgs{});
    }

    auto fork_context() -> EGLSubObject {
        return egl.fork();
    }

    Window(internal::wl::EGLObject& egl, typename SharedData::BufferType& application_events) : egl(egl), application_events(application_events) {}
};
} // namespace gawl::wl

namespace gawl::internal::wl {
inline auto choose_surface(const EGLSurface eglsurface, const EGLObject& egl) -> void {
    static auto current_surface = EGLSurface(nullptr);
    if(current_surface == eglsurface) {
        return;
    }
    dynamic_assert(eglMakeCurrent(egl.display, eglsurface, eglsurface, egl.context) != EGL_FALSE);
    current_surface = eglsurface;
}

template <class Impl, class... Impls>
class WindowBackend : public gawl::wl::Window<Impl, Impls...> {
  private:
    class SurfaceGlue {
      private:
        WindowBackend*    backend;
        towl::OutputTag*  current_output;
        std::atomic_bool* frame_done;
        std::atomic_bool* latest_frame;

      public:
        auto on_enter(const towl::OutputTag output) -> void {
            *current_output = output;
            backend->resize_buffer(-1, -1, WlType::Output::from_tag(output).get_scale());
        }

        auto on_leave(const towl::OutputTag /*output*/) -> void {
            *current_output = towl::nulltag;
        }

        auto on_frame() -> void {
            *frame_done = true;
            if(!latest_frame->exchange(true) || !backend->get_event_driven()) {
                backend->queue_callback(RefreshCallbackArgs{});
            }
        }

        SurfaceGlue(WindowBackend& backend, towl::OutputTag& output, std::atomic_bool& frame_done, std::atomic_bool& latest_frame) : backend(&backend),
                                                                                                                                     current_output(&output),
                                                                                                                                     frame_done(&frame_done),
                                                                                                                                     latest_frame(&latest_frame) {}
    };

    class XDGToplevelGlue {
      private:
        WindowBackend* backend;

      public:
        auto on_configure(const int32_t width, const int32_t height) -> void {
            {
                const auto& buffer_size = backend->get_buffer_size();
                const auto [lock, data] = buffer_size.access();
                if(data.size[0] == static_cast<size_t>(width) * data.scale && data.size[1] == static_cast<size_t>(height) * data.scale) {
                    return;
                }
            }
            backend->resize_buffer(width, height, -1);
        }

        auto on_close() -> void {
            if constexpr(gawl::concepts::WindowImplWithCloseRequestCallback<Impl>) {
                backend->queue_callback(CloseRequestCallbackArgs{});
            } else {
                backend->quit_application();
            }
        }

        XDGToplevelGlue(WindowBackend& backend) : backend(&backend) {}
    };

    using BackendType = WindowBackend;
    using Shared      = internal::wl::SharedData<WindowBackend, Impls...>;
    using BufferType  = typename Shared::BufferType;
    using WlType      = typename Shared::WlType;

    constexpr static auto enable_keycode  = gawl::concepts::WindowImplWithKeycodeCallback<Impl>;
    constexpr static auto enable_keysym   = gawl::concepts::WindowImplWithKeysymCallback<Impl>;
    constexpr static auto enable_keyboard = enable_keycode || enable_keysym;

    struct Keyboard {
        TimerEvent            key_delay_timer;
        Critical<std::thread> key_repeater;
        std::atomic_uint32_t  last_pressed_key = -1;
    };

    // shared with application
    typename WlType::WaylandClientObject& wl;
    BufferType&                           application_events;

    // per window objects
    Impl                                                           impl;
    typename WlType::Compositor::template Surface<SurfaceGlue>     surface;
    towl::OutputTag                                                output;
    typename WlType::WMBase::XDGSurface                            xdg_surface;
    typename WlType::WMBase::template XDGToplevel<XDGToplevelGlue> xdg_toplevel;
    towl::EGLWindow                                                egl_window;
    EGLSurface                                                     eglsurface               = nullptr;
    std::atomic_bool                                               obsolete_egl_window_size = true;

    [[no_unique_address]] std::conditional_t<enable_keyboard, Keyboard, towl::Empty> keyboard;

    auto init_egl() -> void {
        eglsurface = eglCreateWindowSurface(this->egl.display, this->egl.config, reinterpret_cast<EGLNativeWindowType>(this->egl_window.native()), nullptr);
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

  public:
    using WindowCreateHintType = WindowCreateHint<Shared>;

    auto get_impl() -> Impl& {
        return impl;
    }

    auto wl_get_surface() const -> towl::SurfaceTag {
        return surface.as_tag();
    }

    auto wl_get_output() const -> towl::OutputTag {
        return output;
    }

    auto wl_on_keycode_enter(const towl::Array<uint32_t>& keys) -> void {
        if constexpr(enable_keycode) {
            for(auto i = size_t(0); i < keys.size; i += 1) {
                this->queue_callback(KeycodeCallbackArgs{keys.data[i], gawl::ButtonState::Enter});
            }
        }
    }

    auto wl_on_keysym_enter(const std::vector<xkb_keycode_t>& keys, xkb_state* const xkb_state) -> void {
        if constexpr(enable_keysym) {
            for(const auto k : keys) {
                this->queue_callback(KeysymCallbackArgs{k, gawl::ButtonState::Enter, xkb_state});
            }
        }
    }

    auto wl_on_key_leave() -> void {
        if constexpr(enable_keyboard) {
            auto [lock, repeater] = keyboard.key_repeater.access();
            wait_for_key_repeater_exit(repeater);
        }
        if constexpr(enable_keycode) {
            this->queue_callback(KeycodeCallbackArgs{static_cast<uint32_t>(-1), gawl::ButtonState::Leave});
        }
        if constexpr(enable_keysym) {
            this->queue_callback(KeysymCallbackArgs{XKB_KEYCODE_INVALID, gawl::ButtonState::Leave, nullptr});
        }
    }

    // the third and subsequent arguments are only valid if enable_keysym == true
    auto wl_on_key_input(const uint32_t keycode, const uint32_t state, const xkb_keycode_t keysym, const bool enable_repeat, xkb_state* const xkb_state) -> void {
        if constexpr(enable_keyboard) {
            const auto s = state == WL_KEYBOARD_KEY_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;

            if constexpr(enable_keycode) {
                this->queue_callback(KeycodeCallbackArgs{keycode, s});
            }
            if constexpr(enable_keysym) {
                this->queue_callback(KeysymCallbackArgs{keysym, s, xkb_state});
            }
            if(!wl.repeat_config.has_value()) {
                return;
            }
            auto [lock, repeater] = keyboard.key_repeater.access();
            if(s == gawl::ButtonState::Press && (!enable_keysym || enable_repeat)) {
                if(this->get_state() != WindowState::Running) {
                    return;
                }
                wait_for_key_repeater_exit(repeater);
                keyboard.last_pressed_key.store(keycode);

                if constexpr(enable_keysym) {
                    repeater = std::thread([this, keycode, keysym, xkb_state]() {
                        keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->delay_in_milisec));
                        while(keyboard.last_pressed_key.load() == keycode) {
                            if constexpr(enable_keycode) {
                                this->queue_callback(KeycodeCallbackArgs{keycode, gawl::ButtonState::Repeat});
                            }
                            this->queue_callback(KeysymCallbackArgs{keysym, gawl::ButtonState::Repeat, xkb_state});
                            keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->interval));
                        }
                    });
                } else if constexpr(enable_keycode) {
                    repeater = std::thread([this, keycode]() {
                        keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->delay_in_milisec));
                        while(keyboard.last_pressed_key.load() == keycode) {
                            this->queue_callback(KeycodeCallbackArgs{keycode, gawl::ButtonState::Repeat});
                            keyboard.key_delay_timer.wait_for(std::chrono::milliseconds(wl.repeat_config->interval));
                        }
                    });
                }

            } else if(keyboard.last_pressed_key.load() == keycode) {
                wait_for_key_repeater_exit(repeater);
            }
        }
    }

    auto wl_on_click(const uint32_t button, const uint32_t state) -> void {
        if constexpr(gawl::concepts::WindowImplWithClickCallback<Impl>) {
            const auto s = state == WL_POINTER_BUTTON_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;
            this->queue_callback(ClickCallbackArgs{button, s});
        }
    }

    auto wl_on_pointer_motion(const double x, const double y) -> void {
        if constexpr(gawl::concepts::WindowImplWithPointermoveCallback<Impl>) {
            this->queue_callback(PointermoveCallbackArgs{gawl::Point{x, y}});
        }
    }

    auto wl_on_pointer_axis(const uint32_t axis, const double value) -> void {
        if constexpr(gawl::concepts::WindowImplWithScrollCallback<Impl>) {
            const auto w = axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? gawl::WheelAxis::Horizontal : gawl::WheelAxis::Vertical;
            this->queue_callback(ScrollCallbackArgs{w, value});
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

                surface.set_buffer_scale(scale);
            }
        }
        new_width *= new_scale;
        new_height *= new_scale;
        this->on_buffer_resize(std::array{new_width, new_height}, new_scale);

        obsolete_egl_window_size = true;
        if constexpr(gawl::concepts::WindowImplWithWindowResizeCallback<Impl>) {
            this->queue_callback(WindowResizeCallbackArgs{});
        }
        this->refresh();
    }

    auto handle_event() -> void {
        auto queue = this->queue.exchange();
        do {
            for(const auto& a : queue) {
                switch(a.index()) {
                case CallbackQueue::template index_of<RefreshCallbackArgs>(): {
                    choose_surface(eglsurface, this->egl);
                    if(obsolete_egl_window_size) {
                        obsolete_egl_window_size = false;
                        const auto& buffer_size  = this->get_buffer_size();
                        const auto [lock, data]  = buffer_size.access();
                        egl_window.resize(data.size[0], data.size[1], 0, 0);
                    }
                    if constexpr(gawl::concepts::WindowImplWithRefreshCallback<Impl>) {
                        impl.refresh_callback();
                    }
                    surface.set_frame();
                    swap_buffer();
                } break;
                case CallbackQueue::template index_of<WindowResizeCallbackArgs>():
                    if constexpr(gawl::concepts::WindowImplWithWindowResizeCallback<Impl>) {
                        impl.window_resize_callback();
                    }
                    break;
                case CallbackQueue::template index_of<KeycodeCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithKeycodeCallback<Impl>) {
                        const auto& args = a.template get<KeycodeCallbackArgs>();
                        impl.keycode_callback(args.key, args.state);
                    }
                } break;
                case CallbackQueue::template index_of<KeysymCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithKeysymCallback<Impl>) {
                        const auto& args = a.template get<KeysymCallbackArgs>();
                        impl.keysym_callback(args.key, args.state, args.xkbstate);
                    }
                } break;
                case CallbackQueue::template index_of<PointermoveCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithPointermoveCallback<Impl>) {
                        const auto& args = a.template get<PointermoveCallbackArgs>();
                        impl.pointermove_callback(args.point);
                    }
                } break;
                case CallbackQueue::template index_of<ClickCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithClickCallback<Impl>) {
                        const auto& args = a.template get<ClickCallbackArgs>();
                        impl.click_callback(args.key, args.state);
                    }
                } break;
                case CallbackQueue::template index_of<ScrollCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithScrollCallback<Impl>) {
                        const auto& args = a.template get<ScrollCallbackArgs>();
                        impl.scroll_callback(args.axis, args.value);
                    }
                } break;
                case CallbackQueue::template index_of<CloseRequestCallbackArgs>():
                    if constexpr(gawl::concepts::WindowImplWithCloseRequestCallback<Impl>) {
                        impl.close_request_callback();
                    }
                    break;
                case CallbackQueue::template index_of<UserCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithUserCallback<Impl>) {
                        const auto& args = a.template get<UserCallbackArgs>();
                        impl.user_callback(args.data);
                    }
                } break;
                }
            }
        } while(!(queue = this->queue.exchange()).empty());
    }

    template <class... Args>
    WindowBackend(const WindowCreateHintType& hint, Args&&... args) : gawl::wl::Window<Impl, Impls...>(*hint.backend_hint.egl, *hint.backend_hint.application_events),
                                                                      wl(*hint.backend_hint.wl),
                                                                      application_events(*hint.backend_hint.application_events),
                                                                      impl(*this, args...),
                                                                      surface(wl.registry.template interface<typename WlType::Compositor>()[0].create_surface(SurfaceGlue(*this, output, this->frame_done, this->latest_frame))),
                                                                      xdg_surface(wl.registry.template interface<typename WlType::WMBase>()[0].create_xdg_surface(surface)),
                                                                      xdg_toplevel(xdg_surface.create_xdg_toplevel(XDGToplevelGlue(*this))),
                                                                      egl_window(surface, hint.width, hint.height) {
        surface.commit();
        wl.display.wait_sync();
        xdg_toplevel.set_title(hint.title);
        init_egl();
        resize_buffer(hint.width, hint.height, -1);

        swap_buffer();
        wl.display.wait_sync();

        this->set_event_driven(hint.manual_refresh);
        this->set_state(WindowState::Running);
    }

    ~WindowBackend() {
        if constexpr(enable_keyboard) {
            const auto [lock, data] = keyboard.key_repeater.access();
            wait_for_key_repeater_exit(data);
        }
        dynamic_assert(eglDestroySurface(this->egl.display, this->eglsurface) != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
