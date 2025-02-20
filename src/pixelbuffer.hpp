#pragma once
#include <optional>
#include <span>
#include <vector>

namespace gawl {
struct PixelBuffer {
    size_t                 width;
    size_t                 height;
    std::vector<std::byte> data;

    static auto from_raw(size_t width, size_t height, const std::byte* buffer) -> PixelBuffer;
    static auto from_file(const char* file) -> std::optional<PixelBuffer>;
    static auto from_blob(const std::byte* data, size_t size) -> std::optional<PixelBuffer>;
    static auto from_blob(std::span<const std::byte> buffer) -> std::optional<PixelBuffer>;
};
} // namespace gawl
