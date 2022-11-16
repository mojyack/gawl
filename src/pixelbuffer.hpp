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
    std::array<size_t, 2> size;
    std::vector<uint8_t>  data;

    auto load_texture_imagemagick(Magick::Image&& image) -> void {
        size = {image.columns(), image.rows()};
        data.resize(size[0] * size[1] * 4);
        image.write(0, 0, image.columns(), image.rows(), "RGBA", Magick::CharPixel, data.data());
        return;
    }

    PixelBuffer(std::array<size_t, 2> size, std::vector<uint8_t> data) : size(std::move(size)), data(std::move(data)) {}

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

    static auto from_raw(const size_t width, const size_t height, const uint8_t* const buffer) -> PixelBuffer {
        const auto len  = size_t(width * height * 4);
        auto       data = std::vector<uint8_t>(len);
        std::memcpy(data.data(), buffer, len);
        return PixelBuffer({width, height}, std::move(data));
    }

    static auto from_raw(const size_t width, const size_t height, std::vector<uint8_t> buffer) -> PixelBuffer {
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
            return PixelBuffer({jxl.width, jxl.height}, std::move(jxl.buffer));
        }

        try {
            auto result = PixelBuffer();
            result.load_texture_imagemagick(Magick::Image(file));
            return result;
        } catch(const Magick::Exception& e) {
            return Error(e.what());
        }
    }

    static auto from_blob(const uint8_t* const data, const size_t size) -> Result<PixelBuffer> {
        try {
            auto blob   = Magick::Blob(data, size);
            auto result = PixelBuffer();
            result.load_texture_imagemagick(Magick::Image(blob));
            return result;
        } catch(const Magick::Exception& e) {
            return Error(e.what());
        }
    }

    static auto from_blob(const std::vector<uint8_t>& buffer) -> Result<PixelBuffer> {
        return from_blob(buffer.data(), buffer.size());
    }
};
} // namespace gawl
