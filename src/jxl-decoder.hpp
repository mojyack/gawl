#pragma once
#include <optional>
#include <thread>
#include <vector>

namespace gawl::impl::jxl {
struct Animation {
    uint32_t tps_numerator;
    uint32_t tps_denominator;
    uint32_t loops;
};

struct Frame {
    std::vector<std::byte> buffer;
    uint32_t               duration;
};

struct JxlImage {
    uint32_t           width;
    uint32_t           height;
    std::vector<Frame> frames;
    Animation          animation;
    bool               have_animation;
};

auto decode_jxl(const char* path, uint32_t threads = std::thread::hardware_concurrency()) -> std::optional<JxlImage>;
} // namespace gawl::impl::jxl
