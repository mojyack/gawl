#include <poll.h>

#include "error.hpp"
#include "wayland-application.hpp"
#include "wayland-window.hpp"

namespace gawl {
auto WaylandApplication::queue_application_event(ApplicationEventArgs&& args) -> void {
    const auto lock = application_events.get_lock();
    application_events->emplace_back(args);
    application_event.wakeup();
}
auto WaylandApplication::get_display() -> wayland::display_t& {
    return display;
}
auto WaylandApplication::tell_event(WaylandWindow& window) -> void {
    queue_application_event(HandleEventArgs{window});
}
auto WaylandApplication::close_window(GawlWindow& window) -> void {
    queue_application_event(CloseWindowArgs{window});
}
auto WaylandApplication::run() -> void {
    running = true;
    if(!quitted) {
        GawlApplication::run();
    }

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
        auto events = application_events.replace();
        do {
            for(const auto& e : events) {
                if(std::holds_alternative<HandleEventArgs>(e)) {
                    std::get<HandleEventArgs>(e).window.handle_event();
                } else if(std::holds_alternative<CloseWindowArgs>(e)) {
                    auto&      w           = std::get<CloseWindowArgs>(e).window;
                    const auto last_window = !unregister_window(&w);
                    delete &w;
                    if(quitted && last_window) {
                        quitted = false;
                        goto exit;
                    }
                } else if(std::holds_alternative<QuitApplicationArgs>(e)) {
                    close_all_windows();
                }
            }
        } while(!(events = application_events.replace()).empty());
        application_event.wait();
    }

exit:
    wayland_main_stop.notify();
    wayland_main.join();

    running = false;
    display.roundtrip();
}
auto WaylandApplication::quit() -> void {
    quitted = true;
    queue_application_event(QuitApplicationArgs{});
}
auto WaylandApplication::is_running() const -> bool {
    return running;
}
} // namespace gawl
