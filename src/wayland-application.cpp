#include <poll.h>

#include "error.hpp"
#include "wayland-application.hpp"
#include "wayland-window.hpp"

namespace gawl {
auto WaylandApplication::get_display() -> wayland::display_t& {
    return display;
}
auto WaylandApplication::tell_event(GawlWindow* window) -> void {
    {
        const auto lock = to_handle.get_lock();
        to_handle.data.emplace_back(dynamic_cast<WaylandWindow*>(window));
    }
    window_event.notify();
}
auto WaylandApplication::run() -> void {
    running = true;
    if(!quitted) {
        for(auto& w : get_windows()) {
            w->refresh();
        }
    }

    auto wayland_main_stop = EventFileDescriptor();
    auto wayland_main = std::thread([&]() {
        auto fds = std::array<pollfd, 2>{pollfd{display.get_fd(), POLLIN, 0}, pollfd{wayland_main_stop, POLLIN, 0}};
        auto& wl_display_event_poll = fds[0];
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

    auto  fds                   = std::array<pollfd, 3>{pollfd{window_event, POLLIN, 0}, pollfd{quit_event, POLLIN, 0}};
    auto& window_event_poll     = fds[0];
    auto& quit_event_poll       = fds[1];
    while(true) {
        poll(fds.data(), fds.size(), -1);
        if(window_event_poll.revents & POLLIN) {
            window_event.consume();
            for(const auto w : to_handle.replace()) {
                if(w != nullptr) {
                    w->handle_event();
                    continue;
                }
                auto windows = get_windows();
                for(auto w = windows.begin(); w != windows.end(); w += 1) {
                    if((*w)->is_close_pending()) {
                        unregister_window(*w);
                        delete *w;
                        break;
                    }
                }
                if(quitted && get_windows().empty()) {
                    quitted = false;
                    goto exit;
                }
            }
        }
        if(quit_event_poll.revents & POLLIN) {
            quit_event.consume();
            close_all_windows();
        }
    }

exit:
    wayland_main_stop.notify();
    wayland_main.join();

    running = false;
    display.roundtrip();
}
auto WaylandApplication::quit() -> void {
    quitted = true;
    quit_event.notify();
}
auto WaylandApplication::is_running() const -> bool {
    return running;
}
} // namespace gawl
