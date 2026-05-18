#pragma once
#include <array>
#include <memory>

#include "binder.hpp"
#include "screen.hpp"
#include "viewport.hpp"
#include "window-callbacks.hpp"

namespace gawl::impl {
struct BufferSize {
    std::array<size_t, 2> size  = {800, 600};
    size_t                scale = 1;
};
} // namespace gawl::impl

namespace gawl {
class Window : public Screen {
  public:
    bool               event_driven        = false;
    bool               follow_buffer_scale = true;      // use set_follow_buffer_scale to set
    double             specified_scale     = 0;         // use set_scale to set
    double             draw_scale          = 1.0;       // read-only
    impl::BufferSize   buffer_size;                     // read-only
    std::array<int, 2> window_size;                     // read-only
    Viewport           viewport = {{0, 0}, {800, 600}}; // read-only

    std::shared_ptr<WindowCallbacks> callbacks; // use set_callbacks to set

    auto on_buffer_resize(std::optional<std::array<size_t, 2>> size, std::optional<size_t> scale) -> void;
    auto set_callbacks(std::shared_ptr<WindowCallbacks> callbacks) -> void;

    // MetaScreen
    auto get_scale() const -> double override;
    auto set_scale(double scale) -> void;

    // Screen
    auto prepare() -> impl::FramebufferBinder override;
    auto get_viewport() const -> const Viewport& override;
    auto set_viewport(const gawl::Rectangle& region) -> void override;
    auto unset_viewport() -> void override;

    // user apis
    virtual auto refresh() -> bool = 0; // trigger screen refresh
    auto         set_follow_buffer_scale(bool flag) -> void;

    virtual ~Window() {}
};
} // namespace gawl
