#include <cstring>

#include <ImageMagick-7/Magick++.h>

#include "pixelbuffer.hpp"

namespace gawl {
namespace {
auto load_texture_imagemagick(Magick::Image&& image) -> PixelBuffer {
    auto buffer = std::vector<uint8_t>(image.columns() * image.rows() * 4);
    image.write(0, 0, image.columns(), image.rows(), "RGBA", Magick::CharPixel, buffer.data());
    return PixelBuffer(image.columns(), image.rows(), buffer);
}
} // namespace
auto PixelBuffer::empty() const -> bool {
    return data.empty();
}
auto PixelBuffer::get_width() const -> size_t {
    return size[0];
}
auto PixelBuffer::get_height() const -> size_t {
    return size[1];
}
auto PixelBuffer::get_buffer() const -> const uint8_t* {
    return data.data();
}
auto PixelBuffer::clear() -> void {
    size = {0, 0};
    data.clear();
}
PixelBuffer::PixelBuffer(const size_t width, const size_t height, const uint8_t* const buffer) : size({width, height}) {
    const auto len = size_t(size[0] * size[1] * 4);
    data.resize(len);
    std::memcpy(data.data(), buffer, len);
}
PixelBuffer::PixelBuffer(const size_t width, const size_t height, std::vector<uint8_t>& buffer) : size({width, height}) {
    data = std::move(buffer);
}
PixelBuffer::PixelBuffer(const char* file) {
    auto buf = load_texture_imagemagick(Magick::Image(file));
    if(!buf.empty()) {
        *this = std::move(buf);
    }
}
PixelBuffer::PixelBuffer(const std::vector<uint8_t>& buffer) {
    auto blob = Magick::Blob(buffer.data(), buffer.size());
    *this     = load_texture_imagemagick(Magick::Image(blob));
}
} // namespace gawl
