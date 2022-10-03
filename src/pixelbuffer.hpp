#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include <ImageMagick-7/Magick++.h>

namespace gawl {
class PixelBuffer {
  private:
    std::array<size_t, 2> size;
    std::vector<uint8_t>  data;

  public:
    auto empty() const -> bool {
        return data.empty();
    }

    auto get_width() const -> size_t {
        return size[0];
    }

    auto get_height() const -> size_t {
        return size[1];
    }

    auto get_buffer() const -> const uint8_t* {
        return data.data();
    }

    auto clear() -> void {
        size = {0, 0};
        data.clear();
    }

    PixelBuffer() = default;

    PixelBuffer(const size_t width, const size_t height, const uint8_t* const buffer) : size{width, height} {
        const auto len = size_t(size[0] * size[1] * 4);
        data.resize(len);
        std::memcpy(data.data(), buffer, len);
    }

    PixelBuffer(const size_t width, const size_t height, std::vector<uint8_t>& buffer) : size{width, height} {
        data = std::move(buffer);
    }

    PixelBuffer(const char* const file);

    PixelBuffer(const std::vector<uint8_t>& buffer);

    PixelBuffer(const uint8_t* data, size_t size);
};

namespace internal {
inline auto load_texture_imagemagick(Magick::Image&& image) -> PixelBuffer {
    auto buffer = std::vector<uint8_t>(image.columns() * image.rows() * 4);
    image.write(0, 0, image.columns(), image.rows(), "RGBA", Magick::CharPixel, buffer.data());
    return PixelBuffer(image.columns(), image.rows(), buffer);
}
} // namespace internal

inline PixelBuffer::PixelBuffer(const char* file) {
    auto buf = internal::load_texture_imagemagick(Magick::Image(file));
    if(!buf.empty()) {
        *this = std::move(buf);
    }
}

inline PixelBuffer::PixelBuffer(const std::vector<uint8_t>& buffer) : PixelBuffer(buffer.data(), buffer.size()) {}

inline PixelBuffer::PixelBuffer(const uint8_t* const data, const size_t size) {
    auto blob = Magick::Blob(data, size);
    *this     = internal::load_texture_imagemagick(Magick::Image(blob));
}
} // namespace gawl
