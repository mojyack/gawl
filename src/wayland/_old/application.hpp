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
#include "window.hpp"
#include "wlobject.hpp"

namespace gawl::wl {
class Application : public ::gawl::impl::Application {
  private:
    internal::app_event::Queue    app_queue;
    internal::WaylandClientObject wl;
    internal::EGLObject           egl;
    std::thread                   wayland_thread;
    EventFileDescriptor           wayland_thread_stop;
    std::atomic_bool              running;
    bool                          quitted = false;

    auto backend_get_window_create_hint() -> void* override {
        static auto hint = internal::SharedData();
        hint             = internal::SharedData{&wl, &egl, &app_queue};
        return &hint;
    }

    auto backend_close_window(::gawl::internal::Window* const window) -> void override {
        app_queue.push<internal::app_event::CloseWindowArgs>(internal::downcast<internal::WindowBase>(window));
    }

    auto wayland_main() -> void {
        auto  fds                    = std::array{pollfd{wl.display.get_fd(), POLLIN, 0}, pollfd{wayland_thread_stop, POLLIN, 0}};
        auto& wl_display_event_poll  = fds[0];
        auto& wayland_main_stop_poll = fds[1];

    loop:
        auto read_intent = wl.display.obtain_read_intent();
        wl.display.flush();
        dynamic_assert(poll(fds.data(), fds.size(), -1) != -1);
        if(wl_display_event_poll.revents & POLLIN) {
            read_intent.read();
            wl.display.dispatch_pending();
        }
        if(wayland_main_stop_poll.revents & POLLIN) {
            wayland_thread_stop.consume();
            return;
        }
        goto loop;
    }

  public:
    template <class T, class... Args>
    auto open_window(const WindowCreateHint hint, Args&&... args) -> T& {
        const auto ptr       = new Window<T>(hint, backend_get_window_create_hint(), std::forward<Args>(args)...);
        auto [lock, windows] = critical_windows.access();
        windows.emplace_back(ptr);
        return ptr->get_impl();
    }

    auto run() -> void {
        running = true;
        wl.display.wait_sync();
    loop:
        auto& events = app_queue.swap();
        if(events.empty()) {
            app_queue.wait();
            goto loop;
        }
        for(auto& e : events) {
            using Queue = internal::app_event::Queue;
            switch(e.get_index()) {
            case Queue::index_of<internal::app_event::HandleEventArgs>: {
                auto& args = e.template as<internal::app_event::HandleEventArgs>();
                args.window->handle_event();
            } break;
            case Queue::index_of<internal::app_event::CloseWindowArgs>: {
                auto& args = e.template as<internal::app_event::CloseWindowArgs>();
                erase_window(args.window);
                wl.display.flush();

                auto [lock, windows] = critical_windows.access();
                if(quitted && windows.empty()) {
                    quitted = false;
                    goto exit;
                }
            } break;
            case Queue::index_of<internal::app_event::QuitApplicationArgs>:
                quitted = true;
                close_all_windows();
                break;
            }
        }
        goto loop;

    exit:
        running = false;
    }

    auto quit() -> void {
        app_queue.push<internal::app_event::QuitApplicationArgs>();
    }

    auto is_running() const -> bool {
        return running;
    }

    Application()
        : wl(critical_windows),
          egl(wl.display) {
        // bind wayland interfaces
        wl.display.roundtrip();
        if(wl.registry.template interface<internal::Compositor>().empty() || wl.registry.template interface<internal::WMBase>().empty()) {
            panic("wayland server doesn't provide necessary interfaces");
        }

        // initialize egl
        dynamic_assert(eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl.context) != EGL_FALSE);
        ::gawl::internal::global = new ::gawl::internal::GLObjects();

        // start wayland event loop
        wayland_thread = std::thread(std::bind(&Application::wayland_main, this));
    }

    ~Application() {
        wayland_thread_stop.notify();
        wayland_thread.join();
        delete ::gawl::internal::global;
    }
};
}; // namespace gawl::wl
