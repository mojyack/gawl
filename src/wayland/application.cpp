#include "application.hpp"
#include "../global.hpp"
#include "../macros/assert.hpp"
#include "../util/assert.hpp"

namespace gawl {
auto WaylandApplication::close_window_impl(Window* const window) -> void {
    events.push<impl::CloseWindowArgs>(window);
}

auto WaylandApplication::wayland_main() -> void {
    auto  fds                    = std::array{pollfd{wl->display.get_fd(), POLLIN, 0}, pollfd{wayland_thread_stop, POLLIN, 0}};
    auto& wl_display_event_poll  = fds[0];
    auto& wayland_main_stop_poll = fds[1];

loop:
    auto read_intent = wl->display.obtain_read_intent();
    wl->display.flush();
    dynamic_assert(poll(fds.data(), fds.size(), -1) != -1);
    if(wl_display_event_poll.revents & POLLIN) {
        read_intent.read();
        wl->display.dispatch_pending();
    }
    if(wayland_main_stop_poll.revents & POLLIN) {
        wayland_thread_stop.consume();
        return;
    }
    goto loop;
}

auto WaylandApplication::open_window(const WindowCreateHint& hint, WindowCallbacks* const callbacks) -> WaylandWindow* {
    const auto ptr       = new WaylandWindow(hint, callbacks, wl.get(), &egl, &events);
    auto [lock, windows] = critical_windows.access();
    windows.emplace_back(ptr);
    return ptr;
}

auto WaylandApplication::run() -> void {
    running = true;
    wl->display.wait_sync();
loop:
    auto& es = events.swap();
    if(es.empty()) {
        events.wait();
        goto loop;
    }
    for(auto& e : es) {
        switch(e.get_index()) {
        case impl::AppEventQueue::index_of<impl::HandleEventArgs>: {
            auto& args = e.as<impl::HandleEventArgs>();
            std::bit_cast<WaylandWindow*>(args.window)->handle_event();
        } break;
        case impl::AppEventQueue::index_of<impl::CloseWindowArgs>: {
            auto& args = e.as<impl::CloseWindowArgs>();
            erase_window(args.window);
            wl->display.flush();

            auto [lock, windows] = critical_windows.access();
            if(quitted && windows.empty()) {
                quitted = false;
                goto exit;
            }
        } break;
        case impl::AppEventQueue::index_of<impl::QuitApplicationArgs>:
            quitted = true;
            close_all_windows();
            break;
        }
    }
    goto loop;

exit:
    running = false;
}

auto WaylandApplication::quit() -> void {
    events.push<impl::QuitApplicationArgs>();
}

auto WaylandApplication::is_running() const -> bool {
    return running;
}

WaylandApplication::WaylandApplication()
    : wl(impl::WaylandClientObjects::create(&critical_windows)),
      egl(wl->display) {
    // bind wayland interfaces
    wl->display.roundtrip();
    DYN_ASSERT(!wl->compositor_binder.interfaces.empty());
    DYN_ASSERT(!wl->xdg_wm_base_binder.interfaces.empty());

    // initialize egl
    DYN_ASSERT(eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl.context) != EGL_FALSE);
    impl::global = new impl::Shaders();

    // start wayland event loop
    wayland_thread = std::thread(std::bind(&WaylandApplication::wayland_main, this));
}

WaylandApplication::~WaylandApplication() {
    wayland_thread_stop.notify();
    wayland_thread.join();
    delete impl::global;
}
} // namespace gawl
