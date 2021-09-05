#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "frame-buffer-info.hpp"
#include "type.hpp"

namespace gawl {
enum class GraphicLoader {
    IMAGEMAGICK,
    DEVIL,
};

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
    PixelBuffer(size_t width, size_t height, const char* buffer);
    PixelBuffer(size_t width, size_t height, std::vector<uint8_t>& buffer);
    PixelBuffer(const char* file, GraphicLoader loader = GraphicLoader::IMAGEMAGICK);
    PixelBuffer(std::vector<uint8_t>& buffer);
};

class Graphic {
  private:
    std::shared_ptr<GraphicData> graphic_data;

  public:
    auto get_width(FrameBufferInfo info) const -> int;
    auto get_height(FrameBufferInfo info) const -> int;
    auto draw(FrameBufferInfo info, double x, double y) -> void;
    auto draw_rect(FrameBufferInfo info, Area area) -> void;
    auto draw_fit_rect(FrameBufferInfo info, Area area) -> void;
    auto clear() -> void;
         operator GraphicBase*() const;
         operator bool() const;
    Graphic(const Graphic&);
    Graphic(Graphic&&);
    Graphic& operator=(const Graphic&);
    Graphic& operator=(Graphic&&);
    Graphic();
    Graphic(const char* file, GraphicLoader loader = GraphicLoader::IMAGEMAGICK, std::optional<std::array<int, 4>> crop = std::nullopt);
    Graphic(std::vector<uint8_t>& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
    Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
    Graphic(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
};
} // namespace gawl
