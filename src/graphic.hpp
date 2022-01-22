#pragma once
#include "graphic-data.hpp"

namespace gawl {
class Graphic {
  private:
    std::shared_ptr<internal::GraphicData> graphic_data;

  public:
    auto get_width(const gawl::concepts::MetaScreen auto& screen) const -> int {
        assert(graphic_data);
        return graphic_data->get_width(screen);
    }
    auto get_height(const gawl::concepts::MetaScreen auto& screen) const -> int {
        assert(graphic_data);
        return graphic_data->get_height(screen);
    }
    auto draw(gawl::concepts::Screen auto& screen, const Point& point) -> void {
        assert(graphic_data);
        return graphic_data->draw(screen, point);
    }
    auto draw_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        assert(graphic_data);
        return graphic_data->draw_rect(screen, rect);
    }
    auto draw_fit_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        assert(graphic_data);
        return graphic_data->draw_fit_rect(screen, rect);
    }
    auto draw_transformed(gawl::concepts::Screen auto& screen, const std::array<Point, 4>& vertices) const -> void {
        assert(graphic_data);
        return graphic_data->draw_transformed(screen, vertices);
    }
    auto clear() -> void {
        *this = Graphic();
    }
    operator bool() const {
        return static_cast<bool>(graphic_data);
    }
    Graphic(const Graphic& o) {
        *this = o;
    }
    Graphic(Graphic&& o) {
        *this = o;
    }
    Graphic& operator=(const Graphic& o) {
        graphic_data = o.graphic_data;
        return *this;
    }
    Graphic& operator=(Graphic&& o) {
        graphic_data = o.graphic_data;
        return *this;
    }
    Graphic() {}
    Graphic(const char* const file, std::optional<std::array<int, 4>> crop = std::nullopt) {
        try {
            const auto data = new internal::GraphicData(PixelBuffer(file), crop);
            graphic_data.reset(data);
        } catch(const std::exception& e) {
            internal::warn(file, " is not a valid image file: ", e.what());
            return;
        }
    }
    Graphic(std::vector<uint8_t>& buffer, std::optional<std::array<int, 4>> crop = std::nullopt) {
        try {
            const auto data = new internal::GraphicData(PixelBuffer(buffer), crop);
            graphic_data.reset(data);
        } catch(const std::exception&) {
            internal::warn("invalid buffer");
            return;
        }
    }
    Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop = std::nullopt) {
        graphic_data.reset(new internal::GraphicData(buffer, crop));
    }
};
} // namespace gawl
