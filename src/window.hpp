#pragma once
#include <array>

#include "internal-type.hpp"
#include "type.hpp"
#include "window-creat-hint.hpp"

namespace gawl::internal {
template <class Impl>
class Window {
  private:
    struct BufferSize {
        std::array<size_t, 2> size  = {800, 600};
        size_t                scale = 1;
    };

    WindowState        state               = WindowState::Constructing;
    bool               follow_buffer_scale = true;
    double             specified_scale     = 0;
    double             draw_scale          = 1.0;
    bool               event_driven        = false;
    BufferSize         buffer_size;
    std::array<int, 2> window_size;

    std::array<std::array<size_t, 2>, 2> viewport = {{{0, 0}, {800, 600}}};

  protected:
    auto impl() -> Impl* {
        return reinterpret_cast<Impl*>(this);
    }
    auto on_buffer_resize(const std::optional<std::array<size_t, 2>> size, const std::optional<size_t> scale) -> void {
        constexpr auto MIN_SCALE = 0.01;
        if(size) {
            buffer_size.size = *size;
            viewport[0]      = {0, 0};
            viewport[1]      = buffer_size.size;
        }
        if(scale) {
            buffer_size.scale = *scale;
            draw_scale        = specified_scale >= MIN_SCALE ? specified_scale : follow_buffer_scale ? buffer_size.scale
                                                                                                     : 1;
        }
        window_size[0] = viewport[1][0] / draw_scale;
        window_size[1] = viewport[1][1] / draw_scale;
    }
    auto get_buffer_size() const -> const std::array<std::size_t, 2>& {
        return buffer_size.size;
    }

  public:
    auto get_screen_size() const -> const std::array<std::size_t, 2>& {
        return viewport[1];
    }
    auto set_viewport(const gawl::Rectangle& region) -> void {
        auto r = region;
        r.magnify(draw_scale);
        viewport[0][0] = r.a.x;
        viewport[0][1] = buffer_size.size[1] - r.height() - r.a.y;
        viewport[1][0] = r.width();
        viewport[1][1] = r.height();
    }
    auto unset_viewport() -> void {
        viewport[0]    = {0, 0};
        viewport[1]    = buffer_size.size;
        window_size[0] = viewport[1][0] / draw_scale;
        window_size[1] = viewport[1][1] / draw_scale;
    }
    auto prepare() -> gawl::internal::FramebufferBinder {
        auto binder = gawl::internal::FramebufferBinder(0);
        glViewport(viewport[0][0], viewport[0][1], viewport[1][0], viewport[1][1]);
        print(viewport[0][0], " ", viewport[0][1], " ", viewport[1][0], " ", viewport[1][1], " ");
        return binder;
    }
    auto get_state() const -> internal::WindowState {
        return state;
    }
    auto set_state(const internal::WindowState new_state) -> void {
        state = new_state;
    }
    auto get_window_size() const -> const std::array<int, 2>& {
        return window_size;
    }
    auto set_follow_buffer_scale(const bool flag) -> void {
        if(flag == follow_buffer_scale) {
            return;
        }
        follow_buffer_scale = flag;
        on_buffer_resize(0, 0, buffer_size.scale);
    }
    auto get_follow_buffer_scale() const -> bool {
        return follow_buffer_scale;
    }
    auto get_scale() const -> double {
        return draw_scale;
    }
    auto set_scale(const double scale) -> void {
        specified_scale = scale;
        on_buffer_resize(0, 0, buffer_size.scale);
    }
    auto set_event_driven(const bool flag) -> void {
        if(event_driven == flag) {
            return;
        }
        if(event_driven) {
            event_driven = false;
        } else {
            event_driven = true;
            impl()->refresh();
        }
    }
    auto get_event_driven() const -> bool {
        return event_driven;
    }
};
} // namespace gawl::internal
