#include <coop/io.hpp>
#include <coop/parallel.hpp>
#include <coop/promise.hpp>

#include "../global.hpp"
#include "../util/assert.hpp"
#include "application.hpp"

namespace gawl {
auto WaylandApplication::create_window(const WindowCreateHint& hint, std::shared_ptr<WindowCallbacks> callbacks) -> coop::Async<Window*> {
    auto window = new WaylandWindow();
    line_assert(co_await window->init(hint, std::move(callbacks), wl.get(), &egl, application_event));
    co_return window;
}

auto WaylandApplication::run() -> coop::Async<void> {
    auto wayland_thread = coop::TaskHandle();
    co_await coop::run_args([](const int fd, coop::Event& event, impl::WaylandClientObjects* wl) -> coop::Async<void> {
        while(true) {
            wl->display.flush();
            const auto result = co_await coop::wait_for_file(fd, true, false);
            line_assert(result.read && !result.error);
            wl->display.dispatch();
            event.notify();
        }
    }(wl->display.get_fd(), application_event, wl.get()))
        .detach({&wayland_thread});

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
        for(const auto window : closing_windows) {
            erase_window(window);
        }
    }

    wayland_thread.cancel();
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
    line_assert(!wl->compositor_binder.interfaces.empty());
    line_assert(!wl->xdg_wm_base_binder.interfaces.empty());

    // initialize egl
    line_assert(eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl.context) != EGL_FALSE);
    impl::global = new impl::Shaders();
    line_assert(impl::global->init());
}

WaylandApplication::~WaylandApplication() {
    delete impl::global;
}
} // namespace gawl
