#pragma once
#include <array>

#include "point.hpp"

namespace gawl {
struct Rectangle {
    Point a;
    Point b;

    auto magnify(const double scale) -> Rectangle& {
        a.magnify(scale);
        b.magnify(scale);
        return *this;
    }

    auto expand(const double w, const double h) -> Rectangle& {
        a.x -= w;
        a.y -= h;
        b.x += w;
        b.y += h;
        return *this;
    }

    auto to_points() const -> std::array<Point, 4> {
        return {a, {b.x, a.y}, b, {a.x, b.y}};
    }

    auto width() const -> double {
        return b.x - a.x;
    }

    auto height() const -> double {
        return b.y - a.y;
    }

    auto operator+=(const Point& o) -> Rectangle& {
        a += o;
        b += o;
        return *this;
    }

    auto operator&=(const Rectangle& o) -> Rectangle& {
        a.x = std::max(a.x, o.a.x);
        a.y = std::max(a.y, o.a.y);
        b.x = std::min(b.x, o.b.x);
        b.y = std::min(b.y, o.b.y);
        return *this;
    }
};
} // namespace gawl
