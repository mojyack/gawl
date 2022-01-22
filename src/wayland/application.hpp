#pragma once
#include <thread>
#include <unordered_map>

#include <poll.h>

#include "../application.hpp"
#include "../global.hpp"
#include "../window-impl-concept.hpp"
#include "../window.hpp"
#include "eglobject.hpp"
#include "shared-data.hpp"
#include "wlobject.hpp"

namespace gawl::internal::wl {
template <class... Impls>
class ApplicationBackend : public Application<ApplicationBackend<Impls...>, Impls...> {
  private:
    using SharedData = SharedData<Impls...>;
    using BufferType = typename SharedData::BufferType;

    BufferType          application_events;
    WaylandClientObject wl;
    EGLObject           egl;
    bool                quitted          = false;
    Critical<bool>      running          = false;
    wayland::surface_t  keyboard_focused = nullptr;
    wayland::surface_t  pointer_focused  = nullptr;
    EventFileDescriptor wayland_main_suspend_request;
    EventFileDescriptor wayland_main_suspend_reply;

  public:
    auto run() -> void {
        running.store(true);
        auto wayland_main_stop = EventFileDescriptor();
        auto wayland_main      = std::thread([this, &wayland_main_stop]() {
            auto  fds                       = std::array<pollfd, 3>{pollfd{wl.display.get_fd(), POLLIN, 0}, pollfd{wayland_main_stop, POLLIN, 0}, pollfd{wayland_main_suspend_request, POLLIN, 0}};
            auto& wl_display_event_poll     = fds[0];
            auto& wayland_main_stop_poll    = fds[1];
            auto& wayland_main_suspend_poll = fds[2];
            while(true) {
                auto read_intent = wl.display.obtain_read_intent();
                wl.display.flush();
                poll(fds.data(), fds.size(), -1);
                if(wl_display_event_poll.revents & POLLIN) {
                    read_intent.read();
                    wl.display.dispatch_pending();
                }
                if(wayland_main_stop_poll.revents & POLLIN) {
                    wayland_main_stop.consume();
                    break;
                }
                if(wayland_main_suspend_poll.revents & POLLIN) {
                    wayland_main_suspend_request.consume();
                    if(!read_intent.is_finalized()) {
                        read_intent.cancel();
                    }
                    wayland_main_suspend_reply.notify();
                    wayland_main_suspend_request.wait();
                }
            }
             });

        while(true) {
            auto events = application_events.exchange();
            do {
                for(auto& e : events) {
                    switch(e.index()) {
                    case decltype(application_events)::template index_of<typename SharedData::HandleEventArgs>(): {
                        auto& args = e.template get<typename SharedData::HandleEventArgs>();
                        std::visit([](auto& w) { w->handle_event(); }, args.window.as_variant());
                    } break;
                    case decltype(application_events)::template index_of<typename SharedData::CloseWindowArgs>(): {
                        auto&      args        = e.template get<typename SharedData::CloseWindowArgs>();
                        const auto last_window = std::visit([this](auto& w) -> bool { return this->destroy_window(*w); }, args.window.as_variant());
                        if(quitted && last_window) {
                            quitted = false;
                            goto exit;
                        }
                    } break;
                    case decltype(application_events)::template index_of<typename SharedData::QuitApplicationArgs>():
                        quitted = true;
                        this->close_all_windows();
                        break;
                    }
                }
            } while(!(events = application_events.exchange()).empty());
            application_events.wait();
        }

    exit:
        // running shoud be locked in order to avoid dead locking in open_window()
        {
            const auto lock = running.get_lock();
            *running        = false;

            wayland_main_stop.notify();
            wayland_main.join();
        }

        wl.display.roundtrip();
    }
    // wayland_main must be suspended while constructing new window.
    // so we define our original open_window() function.
    template <class Impl, class... Args>
    auto open_window(typename Impl::WindowCreateHintType hint, Args&&... args) -> void {
        const auto lock = running.get_lock();

        // suspend wayland_main
        if(*running) {
            wayland_main_suspend_request.notify();
            wayland_main_suspend_reply.wait();
        }
        static_assert(std::disjunction_v<std::is_same<Impl, Impls>...>);
        hint.backend_hint = get_shared_data();
        this->windows.emplace_back(std::in_place_type<Impl>, hint, args...);

        // resume wayland_main
        if(*running) {
            wayland_main_suspend_request.notify();
        }
    }
    auto close_window(auto& window) -> void {
        application_events.push(typename SharedData::CloseWindowArgs{&window});
    }
    auto quit() -> void {
        application_events.push(typename SharedData::QuitApplicationArgs{});
    }
    auto is_running() const -> bool {
        return running.load();
    }
    auto get_shared_data() -> SharedData {
        return SharedData{&wl, &egl, &application_events};
    }
    ApplicationBackend() : Application<ApplicationBackend<Impls...>, Impls...>(), egl(wl.display) {
        wl.registry             = wl.display.get_registry();
        wl.registry.on_global() = [this](uint32_t name, const std::string& interface, uint32_t version) {
            if(auto i = wl.registry_map.find(interface); i != wl.registry_map.end()) {
                wl.registry.bind(name, i->second, version);
            }
        };
        wl.display.roundtrip();

        assert(wl.xdg_wm_base);
        wl.xdg_wm_base.on_ping() = [this](const uint32_t serial) { wl.xdg_wm_base.pong(serial); };

        wl.output.on_scale() = [this](const int32_t s) {
            wl.buffer_scale = s;
            for(auto& w : this->windows) {
                std::visit([](auto& w) { w.resize_buffer(-1, -1); }, w.as_variant());
            }
        };

        auto has_pointer          = false;
        auto has_keyboard         = false;
        wl.seat.on_capabilities() = [&has_keyboard, &has_pointer](const wayland::seat_capability& capability) {
            has_keyboard = capability & wayland::seat_capability::keyboard;
            has_pointer  = capability & wayland::seat_capability::pointer;
        };
        wl.display.roundtrip();

        // get input devices
        constexpr auto enable_keyboard = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::KeyboardCallback, Impls...>();
        if constexpr(enable_keyboard) {
            assert(has_keyboard);
            wl.keyboard = wl.seat.get_keyboard();

            wl.keyboard.on_repeat_info() = [this](const uint32_t repeat_per_sec, const uint32_t delay_in_milisec) {
                wl.repeat_config.emplace(WaylandClientObject::KeyRepeatConfig{1000 / repeat_per_sec, delay_in_milisec});
            };
            wl.keyboard.on_enter() = [this](const uint32_t /*serial*/, const wayland::surface_t surface, const wayland::array_t keys) {
                keyboard_focused = surface;
                for(auto& w : this->windows) {
                    const auto matched = std::visit(
                        [surface, keys](auto& w) -> bool {
                            const auto& wlw = w.wl_get_object();
                            if(wlw.surface == surface) {
                                w.wl_on_key_enter(keys);
                                return true;
                            }
                            return false;
                        },
                        w.as_variant());
                    if(matched) {
                        break;
                    }
                }
            };
            wl.keyboard.on_leave() = [this](const uint32_t /*serial*/, const wayland::surface_t surface) {
                keyboard_focused = nullptr;
                for(auto& w : this->windows) {
                    std::visit(
                        [surface](auto& w) {
                            const auto& wlw = w.wl_get_object();
                            if(wlw.surface != surface) {
                                return;
                            }
                            w.wl_on_key_leave();
                        },
                        w.as_variant());
                }
            };
            wl.keyboard.on_key() = [this](const uint32_t /*serial*/, const uint32_t /*time*/, const uint32_t key, const wayland::keyboard_key_state state) {
                if(!keyboard_focused) {
                    return;
                }
                for(auto& w : this->windows) {
                    const auto matched = std::visit(
                        [this, key, state](auto& w) -> bool {
                            const auto& wlw = w.wl_get_object();
                            if(wlw.surface == keyboard_focused) {
                                w.wl_on_key_input(key, state);
                                return true;
                            }
                            return false;
                        },
                        w.as_variant());
                    if(matched) {
                        break;
                    }
                }
            };
        }

        constexpr auto enable_click   = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::ClickCallback, Impls...>();
        constexpr auto enable_motion  = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::PointermoveCallback, Impls...>();
        constexpr auto enable_pointer = enable_click || enable_motion;
        if constexpr(enable_pointer) {
            assert(has_pointer);
            wl.pointer            = wl.seat.get_pointer();
            wl.pointer.on_enter() = [this](const uint32_t serial, const wayland::surface_t surface, const double /*x*/, const double /*y*/) {
                pointer_focused = surface;
                for(auto& w : this->windows) {
                    const auto matched = std::visit(
                        [this, surface, serial](auto& w) -> bool {
                            auto& wlw = w.wl_get_object();
                            if(wlw.surface != surface) {
                                return false;
                            }
                            wlw.cursor_surface.attach(wlw.cursor_buffer, 0, 0);
                            wlw.cursor_surface.damage(0, 0, wlw.cursor_image.width() * wl.buffer_scale, wlw.cursor_image.height() * wl.buffer_scale);
                            wlw.cursor_surface.commit();
                            wl.pointer.set_cursor(serial, wlw.cursor_surface, 0, 0);
                            return true;
                        },
                        w.as_variant());
                    if(matched) {
                        break;
                    }
                }
            };
            wl.pointer.on_leave() = [this](const uint32_t /*serial*/, const wayland::surface_t /*surface*/) {
                pointer_focused = nullptr;
            };
        }
        if constexpr(enable_click) {
            wl.pointer.on_button() = [this](const uint32_t /*serial*/, const uint32_t /*time*/, const uint32_t button, const wayland::pointer_button_state state) {
                if(!pointer_focused) {
                    return;
                }
                for(auto& w : this->windows) {
                    const auto matched = std::visit(
                        [this, button, state](auto& w) -> bool {
                            const auto& wlw = w.wl_get_object();
                            if(wlw.surface == pointer_focused) {
                                w.wl_on_click(button, state);
                                return true;
                            }
                            return false;
                        },
                        w.as_variant());
                    if(matched) {
                        break;
                    }
                }
            };
        }
        if constexpr(enable_motion) {
            wl.pointer.on_motion() = [this](const uint32_t /*serial*/, const double x, const double y) {
                if(!pointer_focused) {
                    return;
                }
                for(auto& w : this->windows) {
                    const auto matched = std::visit(
                        [this, x, y](auto& w) -> bool {
                            const auto& wlw = w.wl_get_object();
                            if(wlw.surface == pointer_focused) {
                                w.wl_on_pointer_motion(x, y);
                                return true;
                            }
                            return false;
                        },
                        w.as_variant());
                    if(matched) {
                        break;
                    }
                }
            };
        }
        constexpr auto enable_scroll = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::ScrollCallback, Impls...>();
        if constexpr(enable_scroll) {
            wl.pointer.on_axis() = [this](const uint32_t /*serial*/, const wayland::pointer_axis axis, const double value) {
                if(!pointer_focused) {
                    return;
                }
                for(auto& w : this->windows) {
                    const auto matched = std::visit(
                        [this, axis, value](auto& w) -> bool {
                            const auto& wlw = w.wl_get_object();
                            if(wlw.surface == pointer_focused) {
                                w.wl_on_pointer_axis(axis, value);
                                return true;
                            }
                            return false;
                        },
                        w.as_variant());
                    if(matched) {
                        break;
                    }
                }
            };
        }

        assert(eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl.context) != EGL_FALSE);
        gawl::internal::global = new gawl::internal::GLObjects();
    }
    ~ApplicationBackend() {
        delete gawl::internal::global;
    }
};
}; // namespace gawl::internal::wl
