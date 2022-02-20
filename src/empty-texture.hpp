#pragma once
#include <memory>

#include "empty-texture-data.hpp"

namespace gawl {
class EmptyTexture {
  private:
    std::shared_ptr<internal::EmptyTextureData> data;
    internal::Viewport                          viewport;

  public:
    auto get_scale() const -> double {
        return 1.0;
    }
    auto set_viewport(const gawl::Rectangle& region) -> void {
        internal::dynamic_assert(static_cast<bool>(data));
        viewport.set(region, data->get_size());
    }
    auto unset_viewport() -> void {
        internal::dynamic_assert(static_cast<bool>(data));
        viewport.unset(data->get_size());
    }
    auto get_viewport() const -> internal::Viewport {
        return viewport;
    }
    auto prepare() -> internal::FramebufferBinder {
        internal::dynamic_assert(static_cast<bool>(data));
        auto        binder = internal::FramebufferBinder(data->get_frame_buffer_name());
        const auto& size   = data->get_size();
        glViewport(0, 0, size[0], size[1]);
        return binder;
    }

    auto get_width(const gawl::concepts::MetaScreen auto& screen) const -> int {
        internal::dynamic_assert(static_cast<bool>(data));
        return data->get_width(screen);
    }
    auto get_height(const gawl::concepts::MetaScreen auto& screen) const -> int {
        internal::dynamic_assert(static_cast<bool>(data));
        return data->get_height(screen);
    }
    auto draw(gawl::concepts::Screen auto& screen, const Point& point) -> void {
        internal::dynamic_assert(static_cast<bool>(data));
        return data->draw(screen, point);
    }
    auto draw_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        internal::dynamic_assert(static_cast<bool>(data));
        return data->draw_rect(screen, rect);
    }
    auto draw_fit_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        internal::dynamic_assert(static_cast<bool>(data));
        return data->draw_fit_rect(screen, rect);
    }
    auto clear() -> void {
        data.reset();
    }
    operator bool() const {
        return static_cast<bool>(data);
    }
    EmptyTexture(){};
    EmptyTexture(const int width, const int height) {
        data.reset(new internal::EmptyTextureData(width, height));
    }
};
} // namespace gawl
