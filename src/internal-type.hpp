#pragma once
#include <array>

#include "type.hpp"

namespace gawl::internal {
enum class WindowState {
    Constructing,
    Running,
    Destructing,
};

struct Viewport {
    std::array<size_t, 2> base;
    std::array<size_t, 2> size;
    size_t                gl_base = 0; // coordinates of the origin of the viewport with the lower right as the coordinate origin

    auto set(gawl::Rectangle region, const std::array<size_t, 2>& buffer_size) -> void {
        if(region.a.x < 0) {
            region.a.x = 0;
        }
        if(region.a.y < 0) {
            region.a.y = 0;
        }
        if(region.b.x > buffer_size[0]) {
            region.b.x = buffer_size[0];
        }
        if(region.b.y > buffer_size[1]) {
            region.b.y = buffer_size[1];
        }

        base[0] = region.a.x;
        base[1] = region.a.y;
        gl_base = buffer_size[1] - region.height() - region.a.y;
        size[0] = region.width();
        size[1] = region.height();
    }

    auto unset(const std::array<size_t, 2>& buffer_size) -> void {
        base    = {0, 0};
        size    = buffer_size;
        gl_base = 0;
    }

    auto to_rectangle() -> Rectangle {
        return Rectangle{{1. * base[0], 1. * base[1]}, {1. + base[0] + size[0], 1. + base[1] + size[1]}};
    }
};

struct BufferSize {
    std::array<size_t, 2> size  = {800, 600};
    size_t                scale = 1;
};
} // namespace gawl::internal
