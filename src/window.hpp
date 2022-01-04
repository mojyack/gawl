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
        size_t                scale = 1.0;
    };

    WindowState        state               = WindowState::Constructing;
    bool               follow_buffer_scale = true;
    double             specified_scale     = 0;
    double             draw_scale          = 1.0;
    bool               event_driven        = false;
    BufferSize         buffer_size;
    std::array<int, 2> window_size;

  protected:
    auto impl() -> Impl* {
        return reinterpret_cast<Impl*>(this);
    }
    auto on_buffer_resize(const size_t width, const size_t height, const size_t scale) -> void {
        constexpr auto MIN_SCALE = 0.01;
        if(width != 0 || height != 0) {
            buffer_size.size = {width, height};
        }
        buffer_size.scale = scale;
        draw_scale        = specified_scale >= MIN_SCALE ? specified_scale : follow_buffer_scale ? buffer_size.scale
                                                                                                 : 1;
        window_size[0]    = buffer_size.size[0] / draw_scale;
        window_size[1]    = buffer_size.size[1] / draw_scale;
    }

  public:
    auto get_size() const -> const std::array<std::size_t, 2>& {
        return buffer_size.size;
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
