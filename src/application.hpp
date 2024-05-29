#pragma once
#include "window-creat-hint.hpp"
#include "window.hpp"

namespace gawl {
class Application {
  protected:
    Critical<std::vector<std::unique_ptr<Window>>> critical_windows;

    auto erase_window(const Window* const window) -> void;

    virtual auto create_window(const WindowCreateHint& hint, WindowCallbacks* callbacks) -> Window* = 0;
    virtual auto close_window_impl(Window* window) -> void                                          = 0;

  public:
    auto open_window(const WindowCreateHint& hint, WindowCallbacks* const callbacks) -> Window*;
    auto close_window(Window* const window) -> void;
    auto close_all_windows() -> void;

    virtual auto quit() -> void = 0;

    virtual ~Application() {}
};
} // namespace gawl
