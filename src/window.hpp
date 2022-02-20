#pragma once
#include <array>

#include "binder.hpp"
#include "internal-type.hpp"
#include "type.hpp"
#include "window-creat-hint.hpp"

namespace gawl::internal {
template <class Impl>
class Window {
  private:
    WindowState          state               = WindowState::Constructing;
    bool                 follow_buffer_scale = true;
    double               specified_scale     = 0;
    double               draw_scale          = 1.0;
    Critical<BufferSize> buffer_size;
    std::array<int, 2>   window_size;
    Viewport             viewport = {{0, 0}, {800, 600}};

  protected:
    bool event_driven = false;

    auto impl() -> Impl* {
        return reinterpret_cast<Impl*>(this);
    }
    auto on_buffer_resize(const std::optional<std::array<size_t, 2>> size, const std::optional<size_t> scale) -> void {
        constexpr auto MIN_SCALE = 0.01;
        const auto     lock      = buffer_size.get_lock();
        if(size) {
            buffer_size->size = *size;
            viewport.unset(buffer_size->size);
        }
        if(scale) {
            buffer_size->scale = *scale;
            draw_scale         = specified_scale >= MIN_SCALE ? specified_scale : follow_buffer_scale ? buffer_size->scale
                                                                                                      : 1;
        }
        window_size[0] = viewport.size[0] / draw_scale;
        window_size[1] = viewport.size[1] / draw_scale;
    }
    auto get_buffer_size() const -> const Critical<BufferSize>& {
        return buffer_size;
    }

  public:
    auto get_viewport() const -> const Viewport& {
        return viewport;
    }
    auto set_viewport(const gawl::Rectangle& region) -> void {
        auto r = region;
        r.magnify(draw_scale);
        const auto lock = buffer_size.get_lock();
        viewport.set(r, buffer_size->size);
    }
    auto unset_viewport() -> void {
        const auto lock = buffer_size.get_lock();
        viewport.unset(buffer_size->size);
    }
    auto prepare() -> FramebufferBinder {
        auto binder = FramebufferBinder(0);
        glViewport(viewport.base[0], viewport.gl_base, viewport.size[0], viewport.size[1]);
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
        on_buffer_resize(std::nullopt, std::nullopt);
    }
    auto get_follow_buffer_scale() const -> bool {
        return follow_buffer_scale;
    }
    auto get_scale() const -> double {
        return draw_scale;
    }
    auto set_scale(const double scale) -> void {
        specified_scale = scale;
        on_buffer_resize(std::nullopt, std::nullopt);
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
