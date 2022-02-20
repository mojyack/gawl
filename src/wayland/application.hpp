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
    using Shared = SharedData<Impls...>;
    using Wl     = Wl<Impls...>;

    typename Shared::BufferType      application_events;
    typename Wl::WaylandClientObject wl;
    EGLObject                        egl;
    bool                             quitted = false;
    Critical<bool>                   running = false;
    std::thread                      wayland_main;
    EventFileDescriptor              wayland_main_stop;

  public:
    auto run() -> void {
        *running = true;
        wl.display.wait_sync();
        while(true) {
            auto events = application_events.exchange();
            do {
                for(auto& e : events) {
                    switch(e.index()) {
                    case decltype(application_events)::template index_of<typename Shared::HandleEventArgs>(): {
                        auto& args = e.template get<typename Shared::HandleEventArgs>();
                        std::visit([](auto& w) { w->handle_event(); }, args.window.as_variant());
                    } break;
                    case decltype(application_events)::template index_of<typename Shared::CloseWindowArgs>(): {
                        auto&      args        = e.template get<typename Shared::CloseWindowArgs>();
                        const auto last_window = std::visit([this](auto& w) -> bool { return this->destroy_window(*w); }, args.window.as_variant());
                        if(quitted && last_window) {
                            quitted = false;
                            goto exit;
                        }
                    } break;
                    case decltype(application_events)::template index_of<typename Shared::QuitApplicationArgs>():
                        quitted = true;
                        this->close_all_windows();
                        break;
                    }
                }
            } while(!(events = application_events.exchange()).empty());
            application_events.wait();
        }

    exit:
        *running = false;
    }
    auto close_window(auto& window) -> void {
        application_events.push(typename Shared::CloseWindowArgs{&window});
    }
    auto quit() -> void {
        application_events.push(typename Shared::QuitApplicationArgs{});
    }
    auto is_running() const -> bool {
        return running.load();
    }
    auto get_shared_data() -> Shared {
        return Shared{&wl, &egl, &application_events};
    }
    ApplicationBackend() : Application<ApplicationBackend<Impls...>, Impls...>(), wl(this->windows), egl(wl.display) {
        wl.display.roundtrip();
        if(wl.registry.template interface<typename Wl::Compositor>().empty() || wl.registry.template interface<typename Wl::WMBase>().empty()) {
            panic("wayland server doesn't provide necessary interfaces");
        }

        // get input devices
        [[maybe_unused]] constexpr auto enable_keyboard = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::KeyboardCallback, Impls...>();
        [[maybe_unused]] constexpr auto enable_click    = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::ClickCallback, Impls...>();
        [[maybe_unused]] constexpr auto enable_motion   = !gawl::concepts::impl::not_implemented<gawl::concepts::impl::PointermoveCallback, Impls...>();
        [[maybe_unused]] constexpr auto enable_pointer  = enable_click || enable_motion;

        dynamic_assert(eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl.context) != EGL_FALSE);
        global = new GLObjects();

        wayland_main = std::thread([this]() {
            auto  fds                    = std::array{pollfd{wl.display.get_fd(), POLLIN, 0}, pollfd{wayland_main_stop, POLLIN, 0}};
            auto& wl_display_event_poll  = fds[0];
            auto& wayland_main_stop_poll = fds[1];
            while(true) {
                auto read_intent = wl.display.obtain_read_intent();
                wl.display.flush();
                dynamic_assert(poll(fds.data(), fds.size(), -1) != -1);
                if(wl_display_event_poll.revents & POLLIN) {
                    read_intent.read();
                    wl.display.dispatch_pending();
                }
                if(wayland_main_stop_poll.revents & POLLIN) {
                    wayland_main_stop.consume();
                    break;
                }
            }
        });
    }
    ~ApplicationBackend() {
        wayland_main_stop.notify();
        wayland_main.join();
        delete global;
    }
};
}; // namespace gawl::internal::wl
