#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "screen.hpp"
#include "type.hpp"

namespace gawl {
class GraphicBase;
class GraphicData;
class GawlWindow;

class PixelBuffer {
  private:
    std::array<size_t, 2> size;
    std::vector<uint8_t>  data;

  public:
    auto empty() const -> bool;
    auto get_width() const -> size_t;
    auto get_height() const -> size_t;
    auto get_buffer() const -> const uint8_t*;
    auto clear() -> void;
    PixelBuffer(){};
    PixelBuffer(size_t width, size_t height, const uint8_t* buffer);
    PixelBuffer(size_t width, size_t height, std::vector<uint8_t>& buffer);
    PixelBuffer(const char* file);
    PixelBuffer(const std::vector<uint8_t>& buffer);
};

class Graphic {
  private:
    std::shared_ptr<GraphicData> graphic_data;

  public:
    auto get_width(const Screen* screen) const -> int;
    auto get_height(const Screen* screen) const -> int;
    auto draw(Screen* screen, const Point& point) -> void;
    auto draw_rect(Screen* screen, const Rectangle& rect) -> void;
    auto draw_fit_rect(Screen* screen, const Rectangle& rect) -> void;
    auto draw_transformed(Screen* screen, const std::array<Point, 4>& vertices) const -> void;
    auto clear() -> void;
         operator GraphicBase*() const;
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
