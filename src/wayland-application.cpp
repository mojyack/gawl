#include <sys/eventfd.h>
#include <unistd.h>

#include "wayland-application.hpp"
#include "wayland-window.hpp"
#include "error.hpp"

namespace gawl {
auto WaylandApplication::get_display() -> wayland::display_t& {
    return display;
}
auto WaylandApplication::tell_event(GawlWindow* window) -> void {
    {
        const auto lock = to_handle.get_lock();
        to_handle.data.emplace_back(dynamic_cast<WaylandWindow*>(window));
    }
    const static size_t count = 1;
    ASSERT(write(fds[0].fd, &count, sizeof(count)) == sizeof(size_t), "failed to notify event")
}
auto WaylandApplication::run() -> void {
    running = true;
    if(!quitted) {
        for(auto& w : get_windows()) {
            w->refresh();
        }
    }
    while(!quitted) {
        auto read_intent = display.obtain_read_intent();
        display.flush();
        poll(fds, 3, -1);
        if(fds[0].revents & POLLIN) {
            static size_t count;
            ASSERT(read(fds[0].fd, &count, sizeof(count)) == sizeof(count), "failed to read()")
            std::vector<WaylandWindow*> handle_copy;
            {
                const auto lock = to_handle.get_lock();
                handle_copy     = std::move(to_handle.data);
            }
            for(auto w : handle_copy) {
                if(w != nullptr) {
                    w->handle_event();
                } else {
                    auto windows = get_windows();
                    for(auto w = windows.begin(); w != windows.end(); w += 1) {
                        (*w)->is_close_pending();
                        if((*w)->is_close_pending()) {
                            unregister_window(*w);
                            delete *w;
                            break;
                        }
                    }
                }
            }
        }
        if(fds[1].revents & POLLIN) {
            read_intent.read();
        }
        if(fds[2].revents & POLLIN) {
            static size_t count;
            ASSERT(read(fds[2].fd, &count, sizeof(count)) == sizeof(count), "failed to read()")
            break;
        }
        display.dispatch_pending();
    }
    running = false;
    close_all_windows();
}
auto WaylandApplication::quit() -> void {
    quitted                   = true;
    const static size_t count = 1;
    ASSERT(write(fds[2].fd, &count, sizeof(count)) == sizeof(size_t), "failed to close window")
}
auto WaylandApplication::is_running() const -> bool {
    return running;
}
WaylandApplication::WaylandApplication() {
    fds[0] = pollfd{eventfd(0, 0), POLLIN, 0};
    fds[1] = pollfd{display.get_fd(), POLLIN, 0};
    fds[2] = pollfd{eventfd(0, 0), POLLIN, 0};
}
} // namespace gawl
