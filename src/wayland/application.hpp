#pragma once
#include <poll.h>
#include <thread>

#include "../application.hpp"
#include "../fd.hpp"
#include "../window.hpp"
#include "eglobject.hpp"
#include "shared-data.hpp"
#include "window.hpp"

namespace gawl::internal::wl {
template <class Impl>
class ApplicationBackend : public gawl::Application<Impl, ApplicationBackend<Impl>> {
  private:
    using WindowBackend = WindowBackend<Impl>;
    using SharedData    = SharedData<WindowBackend>;
    using BufferType    = typename SharedData::BufferType;

    wayland::display_t display;
    EGLObject          egl;
    BufferType         application_events;
    bool               quitted = false;
    bool               running = false;

  public:
    auto run() -> void {
        running                = true;
        auto wayland_main_stop = EventFileDescriptor();
        auto wayland_main      = std::thread([this, &wayland_main_stop]() {
            auto  fds                    = std::array<pollfd, 2>{pollfd{display.get_fd(), POLLIN, 0}, pollfd{wayland_main_stop, POLLIN, 0}};
            auto& wl_display_event_poll  = fds[0];
            auto& wayland_main_stop_poll = fds[1];
            while(true) {
                auto read_intent = display.obtain_read_intent();
                display.flush();
                poll(fds.data(), fds.size(), -1);
                if(wl_display_event_poll.revents & POLLIN) {
                    read_intent.read();
                    display.dispatch_pending();
                }
                if(wayland_main_stop_poll.revents & POLLIN) {
                    wayland_main_stop.consume();
                    break;
                }
            }
             });

        while(true) {
            auto events = application_events.exchange();
            do {
                for(const auto& e : events) {
                    switch(e.index()) {
                    case decltype(application_events)::template index_of<typename SharedData::HandleEventArgs>():
                        e.template get<typename SharedData::HandleEventArgs>().window.handle_event();
                        break;
                    case decltype(application_events)::template index_of<typename SharedData::CloseWindowArgs>(): {
                        auto&      w           = e.template get<typename SharedData::CloseWindowArgs>().window;
                        const auto last_window = this->destroy_window(&w);
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
        wayland_main_stop.notify();
        wayland_main.join();

        running = false;
        display.roundtrip();
    }
    auto close_window(Impl& window) -> void {
        application_events.push(typename SharedData::CloseWindowArgs{window});
    }
    auto quit() -> void {
        application_events.push(typename SharedData::QuitApplicationArgs{});
    }
    auto is_running() const -> bool {
        return running;
    }
    auto get_shared_data() -> SharedData {
        return SharedData{&display, &application_events, &egl};
    }
    ApplicationBackend() : gawl::Application<Impl, ApplicationBackend<Impl>>(*this), egl(display) {}
};
}; // namespace gawl::internal::wl
