#pragma once
#include <variant>

#include <wayland-client.hpp>

#include "fd.hpp"
#include "gawl-application.hpp"
#include "type.hpp"

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
    using ApplicationEventArgs = std::variant<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs>;

    wayland::display_t display;
    Event              application_event;
    bool               quitted = false;
    bool               running = false;

    Critical<std::vector<ApplicationEventArgs>> application_events;

    auto queue_application_event(ApplicationEventArgs&& args) -> void;

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
