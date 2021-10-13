#pragma once
#include <memory>
#include <optional>

#include "globject.hpp"
#include "pixelbuffer.hpp"
#include "screen.hpp"
#include "type.hpp"

namespace gawl {
namespace internal {
class GraphicData;
auto create_graphic_globject() -> GLObject*;
} // namespace internal

class Graphic {
  private:
    std::shared_ptr<internal::GraphicData> graphic_data;

  public:
    auto get_width(const Screen* screen) const -> int;
    auto get_height(const Screen* screen) const -> int;
    auto draw(Screen* screen, const Point& point) -> void;
    auto draw_rect(Screen* screen, const Rectangle& rect) -> void;
    auto draw_fit_rect(Screen* screen, const Rectangle& rect) -> void;
    auto draw_transformed(Screen* screen, const std::array<Point, 4>& vertices) const -> void;
    auto clear() -> void;
         operator bool() const;
    Graphic(const Graphic&);
    Graphic(Graphic&&);
    Graphic& operator=(const Graphic&);
    Graphic& operator=(Graphic&&);
    Graphic();
    Graphic(const char* file, std::optional<std::array<int, 4>> crop = std::nullopt);
    Graphic(std::vector<uint8_t>& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
    Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
    Graphic(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
};
} // namespace gawl
