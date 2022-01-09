#pragma once
#include "empty-texture-data.hpp"

namespace gawl {
class EmptyTexture {
  private:
    std::shared_ptr<internal::EmptyTextureData> data;

  public:
    auto get_scale() const -> double {
        return 1.0;
    }
    auto get_viewport() const -> internal::Viewport {
        assert(data);
        return {{0, 0}, data->get_size()};
    }
    auto prepare() -> internal::FramebufferBinder {
        assert(data);
        auto        binder = internal::FramebufferBinder(data->get_frame_buffer_name());
        const auto& size   = data->get_size();
        glViewport(0, 0, size[0], size[1]);
        return binder;
    }

    auto get_width(const gawl::concepts::MetaScreen auto& screen) const -> int {
        assert(data);
        return data->get_width(screen);
    }
    auto get_height(const gawl::concepts::MetaScreen auto& screen) const -> int {
        assert(data);
        return data->get_height(screen);
    }
    auto draw(gawl::concepts::Screen auto& screen, const Point& point) -> void {
        assert(data);
        return data->draw(screen, point);
    }
    auto draw_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        assert(data);
        return data->draw_rect(screen, rect);
    }
    auto draw_fit_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        assert(data);
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
