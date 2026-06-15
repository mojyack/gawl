#pragma once
#include <array>

namespace gawl {
using Color = std::array<double, 4>;

constexpr auto parse_color_code(uint32_t num) -> Color {
    auto ret = Color();
    ret[0]   = (num >> 24 & 0xFF) / 255.0;
    ret[1]   = (num >> 16 & 0xFF) / 255.0;
    ret[2]   = (num >> 8 & 0xFF) / 255.0;
    ret[3]   = (num >> 0 & 0xFF) / 255.0;
    return ret;
}
} // namespace gawl
