#pragma once
#include <vector>

#include <coop/generator.hpp>

#include "window-creat-hint.hpp"
#include "window.hpp"

namespace gawl {
class Application {
  protected:
    std::vector<std::unique_ptr<Window>> windows;
    std::vector<Window*>                 closing_windows;

    auto erase_window(const Window* const window) -> bool;

    virtual auto create_window(WindowCreateHint hint, std::shared_ptr<WindowCallbacks> callbacks) -> coop::Async<Window*> = 0;

  public:
    auto open_window(WindowCreateHint hint, std::shared_ptr<WindowCallbacks> callbacks) -> coop::Async<Window*>;
    auto close_window(Window* const window) -> void;
    auto close_all_windows() -> void;

    virtual auto quit() -> void = 0;

    virtual ~Application() {}
};
} // namespace gawl
