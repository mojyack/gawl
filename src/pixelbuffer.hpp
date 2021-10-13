#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace gawl {
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
} // namespace gawl
