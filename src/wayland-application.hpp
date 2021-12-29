#pragma once
#include <wayland-client.hpp>

#include "fd.hpp"
#include "gawl-application.hpp"
#include "type.hpp"
#include "variant-buffer.hpp"

namespace gawl {
class WaylandWindow;

class WaylandApplication : public GawlApplication {
  private:
    struct HandleEventArgs {
        WaylandWindow& window;
    };
    struct CloseWindowArgs {
        GawlWindow& window;
    };
    struct QuitApplicationArgs {};

    VariantEventBuffer<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs> application_events;

    wayland::display_t display;
    bool               quitted = false;
    bool               running = false;

  public:
    auto get_display() -> wayland::display_t&;
    auto close_window(GawlWindow& window) -> void final;
    auto tell_event(WaylandWindow& window) -> void;
    auto run() -> void final;
    auto quit() -> void final;
    auto is_running() const -> bool final;

    WaylandApplication(){};
    virtual ~WaylandApplication(){};
};
} // namespace gawl
