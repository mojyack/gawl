#pragma once
#include <coop/generator.hpp>

#include "../application.hpp"
#include "../window-creat-hint.hpp"
#include "window.hpp"

namespace gawl {
class WaylandApplication : public Application {
  private:
    std::unique_ptr<impl::WaylandClientObjects> wl;
    impl::EGLObject                             egl;
    coop::Event                                 application_event;
    bool                                        running = false;

    auto create_window(const WindowCreateHint& hint, std::shared_ptr<WindowCallbacks> callbacks) -> coop::Async<Window*> override;
    auto wayland_main() -> coop::Async<void>;

  public:
    auto run() -> coop::Async<void>;
    auto quit() -> void override;
    auto is_running() const -> bool;

    WaylandApplication();
    ~WaylandApplication();
};
} // namespace gawl
