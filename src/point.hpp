#pragma once
#include <span>

namespace gawl {
struct Point {
    double x;
    double y;

    auto rotate(const Point& origin, double angle) -> void;

    auto operator+(const Point& o) const -> Point;
    auto operator+=(const Point& o) -> Point&;
    auto operator-(const Point& o) const -> Point;
    auto operator-=(const Point& o) -> Point&;
    auto operator*(double n) const -> Point;
    auto operator*=(double n) -> Point&;
};

auto rotate(const std::span<Point> points, const Point& origin, const double angle) -> void;
} // namespace gawl
