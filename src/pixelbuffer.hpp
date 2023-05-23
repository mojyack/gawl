#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include <ImageMagick-7/Magick++.h>

#include "jxl-decoder.hpp"

namespace gawl {
class PixelBuffer {
  private:
    std::array<size_t, 2>  size;
    std::vector<std::byte> data;

    auto load_texture_imagemagick(Magick::Image&& image) -> void {
        size = {image.columns(), image.rows()};
        data.resize(size[0] * size[1] * 4);
        image.write(0, 0, image.columns(), image.rows(), "RGBA", Magick::CharPixel, data.data());
        return;
    }

    PixelBuffer(std::array<size_t, 2> size, std::vector<std::byte> data) : size(std::move(size)), data(std::move(data)) {}

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

    auto get_buffer() const -> const std::byte* {
        return data.data();
    }

    auto clear() -> void {
        size = {0, 0};
        data.clear();
    }

    auto operator=(PixelBuffer&& other) -> PixelBuffer& {
        size = other.size;
        std::swap(data, other.data);
        return *this;
    }

    PixelBuffer() = default;

    PixelBuffer(PixelBuffer&& other) {
        *this = std::move(other);
    }

    static auto from_raw(const size_t width, const size_t height, const std::byte* const buffer) -> PixelBuffer {
        const auto len  = size_t(width * height * 4);
        auto       data = std::vector<std::byte>(len);
        std::memcpy(data.data(), buffer, len);
        return PixelBuffer({width, height}, std::move(data));
    }

    static auto from_raw(const size_t width, const size_t height, std::vector<std::byte> buffer) -> PixelBuffer {
        return PixelBuffer({width, height}, std::move(buffer));
    }

    static auto from_file(const char* const file) -> Result<PixelBuffer> {
        // ImageMagick 7.1.0-44 can't decode grayscale jxl image properly
        // hook and decode it by hand
        if(std::string_view(file).ends_with(".jxl")) {
            auto jxl_result = internal::jxl::decode_jxl(file);
            if(!jxl_result) {
                return Error("failed to load jxl file");
            }
            auto& jxl = jxl_result.as_value();
            return PixelBuffer({jxl.width, jxl.height}, std::move(jxl.frames[0].buffer));
        }

        try {
            auto result = PixelBuffer();
            result.load_texture_imagemagick(Magick::Image(file));
            return result;
        } catch(const Magick::Exception& e) {
            return Error(e.what());
        }
    }

    static auto from_blob(const std::byte* const data, const size_t size) -> Result<PixelBuffer> {
        try {
            auto blob   = Magick::Blob(data, size);
            auto result = PixelBuffer();
            result.load_texture_imagemagick(Magick::Image(blob));
            return result;
        } catch(const Magick::Exception& e) {
            return Error(e.what());
        }
    }

    static auto from_blob(const std::vector<std::byte>& buffer) -> Result<PixelBuffer> {
        return from_blob(buffer.data(), buffer.size());
    }
};
} // namespace gawl
