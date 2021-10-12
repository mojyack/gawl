#pragma once
#include <poll.h>
#include <wayland-client.hpp>

#include "fd.hpp"
#include "gawl-application.hpp"
#include "type.hpp"

namespace gawl {
class WaylandWindow;

class WaylandApplication : public GawlApplication {
  private:
    wayland::display_t  display;
    EventFileDescriptor window_event;
    EventFileDescriptor quit_event;
    FileDescriptor      wl_display_event;
    bool                quitted = false;
    bool                running = false;

    Critical<std::vector<WaylandWindow*>> to_handle;

  public:
    auto get_display() -> wayland::display_t&;
    auto tell_event(GawlWindow* window) -> void final;
    auto run() -> void final;
    auto quit() -> void final;
    auto is_running() const -> bool final;

    WaylandApplication();
    virtual ~WaylandApplication();
};
} // namespace gawl
