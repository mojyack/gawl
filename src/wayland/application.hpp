#pragma once
#include "../application.hpp"
#include "../window-creat-hint.hpp"
#include "app-events.hpp"
#include "window.hpp"

#define CUTIL_NS gawl
#include "../util/fd.hpp"
#undef CUTIL_NS

namespace gawl {
class WaylandApplication : public Application {
  private:
    impl::AppEventQueue                         events;
    std::unique_ptr<impl::WaylandClientObjects> wl;
    impl::EGLObject                             egl;
    std::thread                                 wayland_thread;
    EventFileDescriptor                         wayland_thread_stop;
    std::atomic_bool                            running = false;
    bool                                        quitted = false;

    auto create_window(const WindowCreateHint& hint, std::shared_ptr<WindowCallbacks> callbacks) -> Window* override;
    auto close_window_impl(Window* window) -> void override;
    auto wayland_main() -> void;

  public:
    auto run() -> void;
    auto quit() -> void override;
    auto is_running() const -> bool;

    WaylandApplication();
    ~WaylandApplication();
};
} // namespace gawl
