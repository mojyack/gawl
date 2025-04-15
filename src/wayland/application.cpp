#include <coop/io.hpp>
#include <coop/parallel.hpp>
#include <coop/promise.hpp>

#include "../global.hpp"
#include "../macros/assert.hpp"
#include "application.hpp"

namespace gawl {
auto WaylandApplication::create_window(const WindowCreateHint hint, std::shared_ptr<WindowCallbacks> callbacks) -> coop::Async<Window*> {
    auto window = new WaylandWindow();
    windows.emplace_back(window);
    ASSERT(co_await window->init(hint, std::move(callbacks), wl.get(), &egl, application_event));
    co_return window;
}

auto WaylandApplication::run() -> coop::Async<void> {
    auto& runner       = *(co_await coop::reveal_runner());
    auto  wayland_task = coop::TaskHandle();
    runner.push_task([](const int fd, coop::SingleEvent& event, impl::WaylandClientObjects* wl) -> coop::Async<void> {
        loop:
            wl->display.flush();
            const auto result = co_await coop::wait_for_file(fd, true, false);
            ASSERT(result.read && !result.error);
            wl->display.dispatch();
            event.notify();
            goto loop;
    }(wl->display.get_fd(), application_event, wl.get()),
                     &wayland_task);

    running = true;

    while(running) {
        co_await application_event;

        for(auto& window : windows) {
            auto& wl_window = *std::bit_cast<WaylandWindow*>(window.get());
            if(!co_await wl_window.dispatch_pending_callbacks()) {
                close_window(&wl_window);
            }
        }

        if(closing_windows.empty()) {
            continue;
        }
        for(const auto window : std::exchange(closing_windows, {})) {
            erase_window(window);
        }
    }

    wayland_task.cancel();
    windows.clear();
}

auto WaylandApplication::quit() -> void {
    running = false;
}

auto WaylandApplication::is_running() const -> bool {
    return running;
}

WaylandApplication::WaylandApplication()
    : wl(impl::WaylandClientObjects::create(windows)),
      egl(wl->display) {
    // bind wayland interfaces
    wl->display.roundtrip();
    ASSERT(!wl->compositor_binder.interfaces.empty());
    ASSERT(!wl->xdg_wm_base_binder.interfaces.empty());

    // initialize egl
    ASSERT(eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl.context) != EGL_FALSE);
    impl::global = new impl::Shaders();
    ASSERT(impl::global->init());
}

WaylandApplication::~WaylandApplication() {
    delete impl::global;
}
} // namespace gawl
