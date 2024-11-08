#include "window.hpp"

namespace gawl {
auto Window::on_buffer_resize(const std::optional<std::array<size_t, 2>> size, const std::optional<size_t> scale) -> void {
    constexpr auto MIN_SCALE = 0.01;
    if(size) {
        buffer_size.size = *size;
    }
    if(scale) {
        buffer_size.scale = *scale;
        if(specified_scale >= MIN_SCALE) {
            draw_scale = specified_scale;
        } else if(follow_buffer_scale) {
            draw_scale = buffer_size.scale;
        } else {
            draw_scale = 1;
        }
    }
    viewport.unset(buffer_size.size);
    window_size[0] = viewport.size[0] / draw_scale;
    window_size[1] = viewport.size[1] / draw_scale;
}

auto Window::get_buffer_size() const -> const impl::BufferSize& {
    return buffer_size;
}

auto Window::get_scale() const -> double {
    return draw_scale;
}

auto Window::set_scale(const double scale) -> void {
    specified_scale = scale;
    on_buffer_resize(std::nullopt, std::nullopt);
}

auto Window::prepare() -> impl::FramebufferBinder {
    auto binder = impl::FramebufferBinder(0);
    glViewport(viewport.base[0], viewport.gl_y, viewport.size[0], viewport.size[1]);
    return binder;
}

auto Window::get_viewport() const -> const Viewport& {
    return viewport;
}

auto Window::set_viewport(const gawl::Rectangle& region) -> void {
    viewport.set(Rectangle(region).magnify(draw_scale), buffer_size.size);
}

auto Window::unset_viewport() -> void {
    viewport.unset(buffer_size.size);
}

auto Window::get_window_size() const -> const std::array<int, 2>& {
    return window_size;
}

auto Window::set_callbacks(std::shared_ptr<WindowCallbacks> callbacks) -> void {
    this->callbacks         = std::move(callbacks);
    this->callbacks->window = this;
}

auto Window::set_follow_buffer_scale(const bool flag) -> void {
    if(flag == follow_buffer_scale) {
        return;
    }
    follow_buffer_scale = flag;
    on_buffer_resize(std::nullopt, std::nullopt);
}

auto Window::get_follow_buffer_scale() const -> bool {
    return follow_buffer_scale;
}

auto Window::set_event_driven(const bool flag) -> void {
    event_driven = flag;
}

auto Window::get_event_driven() const -> bool {
    return event_driven;
}
} // namespace gawl
