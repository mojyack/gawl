#pragma once
#include <cmath>
#include <numbers>
#include <span>

namespace gawl {
struct Point {
    double x;
    double y;

    auto magnify(double scale) -> void;
    auto rotate(const Point& origin, double angle) -> void;

    auto operator+=(const Point& o) -> Point&;
};

auto rotate(const std::span<Point> points, const Point& origin, const double angle) -> void;
} // namespace gawl
