#pragma once
#include <array>

#include "point.hpp"

namespace gawl {
struct Rectangle {
    Point a;
    Point b;

    auto magnify(double scale) -> Rectangle&;
    auto expand(double w, double h) -> Rectangle&;
    auto to_points() const -> std::array<Point, 4>;
    auto width() const -> double;
    auto height() const -> double;
    auto operator+=(const Point& o) -> Rectangle&;
    auto operator&=(const Rectangle& o) -> Rectangle&;
};
} // namespace gawl
