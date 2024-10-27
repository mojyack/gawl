#pragma once
#include <coop/generator.hpp>
#include <coop/promise.hpp>
#include <xkbcommon/xkbcommon.h>

#include "buttons.hpp"
#include "point.hpp"

namespace gawl {
class Window;
class Application;

class WindowCallbacks {
  protected:
    friend class Application;
    friend class Window;

    Application* application;
    Window*      window;

  public:
    // ask user to redraw its contents
    virtual auto refresh() -> void = 0;
    // ask user to close the window
    virtual auto close() -> void;
    // event handlers
    virtual auto on_created(Window* /*window*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_keycode(uint32_t /*keycode*/, gawl::ButtonState /*state*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_pointer(Point /*pos*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_click(uint32_t /*button*/, gawl::ButtonState /*state*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_scroll(gawl::WheelAxis /*axis*/, double /*value*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_touch_down(uint32_t /*id*/, const Point /*pos*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_touch_motion(uint32_t /*id*/, const Point /*pos*/) -> coop::Async<bool> { co_return true; }
    virtual auto on_touch_up(uint32_t /*id*/) -> coop::Async<bool> { co_return true; }

    virtual ~WindowCallbacks() {};
};
} // namespace gawl
