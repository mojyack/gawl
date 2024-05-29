#pragma once
#include <array>

#include "binder.hpp"
#include "screen.hpp"
#include "viewport.hpp"
#include "window-callbacks.hpp"

#define CUTIL_NS gawl
#include "util/critical.hpp"
#undef CUTIL_NS

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
    impl::WindowState          state               = impl::WindowState::Constructing;
    bool                       follow_buffer_scale = true;
    double                     specified_scale     = 0;
    double                     draw_scale          = 1.0;
    Critical<impl::BufferSize> buffer_size;
    std::array<int, 2>         window_size;
    Viewport                   viewport = {{0, 0}, {800, 600}};

  protected:
    WindowCallbacks* callbacks;
    bool             event_driven = false;

    auto on_buffer_resize(const std::optional<std::array<size_t, 2>> size, const std::optional<size_t> scale) -> void {
        constexpr auto MIN_SCALE = 0.01;
        const auto [lock, data]  = buffer_size.access();
        if(size) {
            data.size = *size;
        }
        if(scale) {
            data.scale = *scale;
            draw_scale = specified_scale >= MIN_SCALE ? specified_scale : follow_buffer_scale ? data.scale
                                                                                              : 1;
        }
        viewport.unset(data.size);
        window_size[0] = viewport.size[0] / draw_scale;
        window_size[1] = viewport.size[1] / draw_scale;
    }

    auto get_buffer_size() const -> const Critical<impl::BufferSize>& {
        return buffer_size;
    }

  public:
    // MetaScreen
    auto get_scale() const -> double override {
        return draw_scale;
    }

    auto set_scale(const double scale) -> void {
        specified_scale = scale;
        on_buffer_resize(std::nullopt, std::nullopt);
    }

    // Screen
    auto prepare() -> impl::FramebufferBinder override {
        auto binder = impl::FramebufferBinder(0);
        glViewport(viewport.base[0], viewport.gl_y, viewport.size[0], viewport.size[1]);
        return binder;
    }

    auto get_viewport() const -> const Viewport& override {
        return viewport;
    }

    auto set_viewport(const gawl::Rectangle& region) -> void override {
        const auto [lock, data] = buffer_size.access();
        viewport.set(Rectangle(region).magnify(draw_scale), data.size);
    }

    auto unset_viewport() -> void override {
        const auto [lock, data] = buffer_size.access();
        viewport.unset(data.size);
    }

    // for application
    auto get_state() const -> impl::WindowState {
        return state;
    }

    auto set_state(const impl::WindowState new_state) -> void {
        state = new_state;
    }

    // user apis
    virtual auto refresh() -> void = 0;

    auto get_window_size() const -> const std::array<int, 2>& {
        return window_size;
    }

    auto set_follow_buffer_scale(const bool flag) -> void {
        if(flag == follow_buffer_scale) {
            return;
        }
        follow_buffer_scale = flag;
        on_buffer_resize(std::nullopt, std::nullopt);
    }

    auto get_follow_buffer_scale() const -> bool {
        return follow_buffer_scale;
    }

    auto set_event_driven(const bool flag) -> void {
        event_driven = flag;
    }

    auto get_event_driven() const -> bool {
        return event_driven;
    }

    Window(WindowCallbacks* callbacks) : callbacks(callbacks) {
        callbacks->window = this;
    }

    virtual ~Window() {}
};
} // namespace gawl
