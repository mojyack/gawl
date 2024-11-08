#pragma once
#include <array>

#include "binder.hpp"
#include "screen.hpp"
#include "viewport.hpp"
#include "window-callbacks.hpp"

namespace gawl::impl {
enum class WindowState {
    Constructing,
    Running,
    Destructing,
};

struct BufferSize {
    std::array<size_t, 2> size  = {800, 600};
    size_t                scale = 1;
};
} // namespace gawl::impl

namespace gawl {
class Window : public Screen {
  private:
    impl::WindowState  state               = impl::WindowState::Constructing;
    bool               follow_buffer_scale = true;
    double             specified_scale     = 0;
    double             draw_scale          = 1.0;
    impl::BufferSize   buffer_size;
    std::array<int, 2> window_size;
    Viewport           viewport = {{0, 0}, {800, 600}};

  protected:
    std::shared_ptr<WindowCallbacks> callbacks;
    bool                             event_driven = false;

    auto on_buffer_resize(std::optional<std::array<size_t, 2>> size, std::optional<size_t> scale) -> void;
    auto get_buffer_size() const -> const impl::BufferSize&;
    auto set_callbacks(std::shared_ptr<WindowCallbacks> callbacks) -> void;

  public:
    // MetaScreen
    auto get_scale() const -> double override;
    auto set_scale(double scale) -> void;
    // Screen
    auto prepare() -> impl::FramebufferBinder override;
    auto get_viewport() const -> const Viewport& override;
    auto set_viewport(const gawl::Rectangle& region) -> void override;
    auto unset_viewport() -> void override;
    // for application
    auto get_state() const -> impl::WindowState;
    auto set_state(const impl::WindowState new_state) -> void;

    // user apis
    virtual auto refresh() -> bool = 0;
    auto         get_window_size() const -> const std::array<int, 2>&;
    auto         set_follow_buffer_scale(bool flag) -> void;
    auto         get_follow_buffer_scale() const -> bool;
    auto         set_event_driven(bool flag) -> void;
    auto         get_event_driven() const -> bool;

    virtual ~Window() {}
};
} // namespace gawl
