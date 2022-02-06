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

// #define GAWL_DEBUG

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
    class SurfaceGlue {
      private:
        WindowBackend*    backend;
        towl::OutputTag*  current_output;
        std::atomic_bool* frame_done;
        std::atomic_bool* latest_frame;

      public:
        auto on_enter(const towl::OutputTag output) -> void {
            *current_output = output;
            backend->resize_buffer(-1, -1, Output<Impls...>::from_tag(output).get_scale());
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
                const auto  lock        = buffer_size.get_lock();
                if(buffer_size->size[0] == static_cast<size_t>(width) * buffer_size->scale && buffer_size->size[1] == static_cast<size_t>(height) * buffer_size->scale) {
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

    WaylandClientObject<Impls...>&       wl;
    Compositor::Surface<SurfaceGlue>     surface;
    towl::OutputTag                      output;
    WMBase::XDGSurface                   xdg_surface;
    WMBase::XDGToplevel<XDGToplevelGlue> xdg_toplevel;
    EGLObject&                           egl;
    towl::EGLWindow                      egl_window;
    EGLSurface                           eglsurface = nullptr;
    BufferType&                          application_events;
    CallbackQueue                        callback_queue;

    std::atomic_bool      frame_done               = true;
    std::atomic_bool      latest_frame             = true;
    std::atomic_bool      obsolete_egl_window_size = true;
    TimerEvent            key_delay_timer;
    Critical<std::thread> key_repeater;
    std::atomic_uint32_t  last_pressed_key = -1;

    auto init_egl() -> void {
        eglsurface = eglCreateWindowSurface(egl.display, egl.config, reinterpret_cast<EGLNativeWindowType>(egl_window.native()), nullptr);
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

  public:
    using WindowCreateHintType = WindowCreateHint<internal::wl::SharedData<Impls...>>;

    auto queue_callback(auto&& args) -> void {
        if(this->get_state() == gawl::internal::WindowState::Destructing) {
            return;
        }

#ifdef GAWL_DEBUG
        print(this, ": queue ", typeid(args).name());
#endif

        callback_queue.push(std::move(args));
        application_events.push(typename SharedData::HandleEventArgs{this->impl()});
    }
    auto wl_get_surface() const -> towl::SurfaceTag {
        return surface.as_tag();
    }
    auto wl_get_output() const -> towl::OutputTag {
        return output;
    }

    auto wl_on_key_enter(const towl::Array<uint32_t>& keys) -> void {
        if(!gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
            return;
        }
        for(auto i = size_t(0); i < keys.size; i += 1) {
            queue_callback(KeyBoardCallbackArgs{keys.data[i], gawl::ButtonState::Enter});
        }
    }
    auto wl_on_key_leave() -> void {
        if(!gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
            return;
        }
        wait_for_key_repeater_exit();
        queue_callback(KeyBoardCallbackArgs{static_cast<uint32_t>(-1), gawl::ButtonState::Leave});
    }
    auto wl_on_key_input(const uint32_t key, const uint32_t state) -> void {
        if(!gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
            return;
        }
        const auto s = state == WL_KEYBOARD_KEY_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;
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
    auto wl_on_click(const uint32_t button, const uint32_t state) -> void {
        if(!gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
            return;
        }
        const auto s = state == WL_POINTER_BUTTON_STATE_PRESSED ? gawl::ButtonState::Press : gawl::ButtonState::Release;
        queue_callback(ClickCallbackArgs{button, s});
    }
    auto wl_on_pointer_motion(const double x, const double y) -> void {
        if(!gawl::concepts::WindowImplWithPointermoveCallback<Impl>) {
            return;
        }
        queue_callback(PointermoveCallbackArgs{gawl::Point{x, y}});
    }
    auto wl_on_pointer_axis(const uint32_t axis, const double value) -> void {
        if(!gawl::concepts::WindowImplWithScrollCallback<Impl>) {
            return;
        }
        const auto w = axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? gawl::WheelAxis::Horizontal : gawl::WheelAxis::Vertical;
        queue_callback(ScrollCallbackArgs{w, value});
    }

    auto resize_buffer(const int width, const int height, const int scale) -> void {
        auto new_scale  = size_t();
        auto new_width  = size_t();
        auto new_height = size_t();
        {
            const auto& buffer = this->get_buffer_size();
            const auto  lock   = buffer.get_lock();
            if(width != -1 && height != -1) {
                // update buffer size
                new_scale  = buffer->scale;
                new_width  = width;
                new_height = height;
            }
            if(scale != -1) {
                // update scale
                new_scale  = scale;
                new_width  = buffer->size[0];
                new_height = buffer->size[1];

                surface.set_buffer_scale(scale);
            }
        }
        new_width *= new_scale;
        new_height *= new_scale;
        this->on_buffer_resize(std::array{new_width, new_height}, new_scale);

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
                case CallbackQueue::template index_of<RefreshCallbackArgs>(): {
                    choose_surface(eglsurface, egl);
                    if(obsolete_egl_window_size) {
                        obsolete_egl_window_size = false;
                        const auto& buffer_size  = this->get_buffer_size();
                        const auto  lock         = buffer_size.get_lock();
                        egl_window.resize(buffer_size->size[0], buffer_size->size[1], 0, 0);
                    }
                    if constexpr(gawl::concepts::WindowImplWithRefreshCallback<Impl>) {
                        this->impl()->refresh_callback();
                    }
                    surface.set_frame();
                    swap_buffer();
                } break;
                case CallbackQueue::template index_of<WindowResizeCallbackArgs>():
                    if constexpr(gawl::concepts::WindowImplWithWindowResizeCallback<Impl>) {
                        this->impl()->window_resize_callback();
                    }
                    break;
                case CallbackQueue::template index_of<KeyBoardCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithKeyboardCallback<Impl>) {
                        const auto& args = a.template get<KeyBoardCallbackArgs>();
                        this->impl()->keyboard_callback(args.key, args.state);
                    }
                } break;
                case CallbackQueue::template index_of<PointermoveCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithPointermoveCallback<Impl>) {
                        const auto& args = a.template get<PointermoveCallbackArgs>();
                        this->impl()->pointermove_callback(args.point);
                    }
                } break;
                case CallbackQueue::template index_of<ClickCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithClickCallback<Impl>) {
                        const auto& args = a.template get<ClickCallbackArgs>();
                        this->impl()->click_callback(args.key, args.state);
                    }
                } break;
                case CallbackQueue::template index_of<ScrollCallbackArgs>(): {
                    if constexpr(gawl::concepts::WindowImplWithScrollCallback<Impl>) {
                        const auto& args = a.template get<ScrollCallbackArgs>();
                        this->impl()->scroll_callback(args.axis, args.value);
                    }
                } break;
                case CallbackQueue::template index_of<CloseRequestCallbackArgs>():
                    if constexpr(gawl::concepts::WindowImplWithCloseRequestCallback<Impl>) {
                        this->impl()->close_request_callback();
                    }
                    break;
                case CallbackQueue::template index_of<UserCallbackArgs>(): {
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
        frame_done = false;
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
    WindowBackend(const WindowCreateHintType& hint) : wl(*hint.backend_hint.wl),
                                                      surface(wl.registry.template interface<Compositor>()[0].create_surface(SurfaceGlue(*this, output, frame_done, latest_frame))),
                                                      xdg_surface(wl.registry.template interface<WMBase>()[0].create_xdg_surface(surface)),
                                                      xdg_toplevel(xdg_surface.create_xdg_toplevel(XDGToplevelGlue(*this))),
                                                      egl(*hint.backend_hint.egl),
                                                      egl_window(surface, hint.width, hint.height),
                                                      application_events(*hint.backend_hint.application_events) {
        surface.commit();
        wl.display.wait_sync();
        xdg_toplevel.set_title(hint.title);
        init_egl();
        resize_buffer(hint.width, hint.height, -1);

        swap_buffer();
        wl.display.wait_sync();

        this->set_event_driven(hint.manual_refresh);
        this->set_state(gawl::internal::WindowState::Running);
    }
    ~WindowBackend() {
#ifdef GAWL_DEBUG
        print(this, ": destroy ", eglsurface);
#endif
        const auto lock = key_repeater.get_lock();
        wait_for_key_repeater_exit();
        assert(eglDestroySurface(egl.display, eglsurface) != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
