#pragma once
#include <array>

#include "point.hpp"

namespace gawl {
struct Rectangle {
    Point a;
    Point b;

    auto expand(double w, double h) -> Rectangle&;
    auto to_points() const -> std::array<Point, 4>;
    auto width() const -> double;
    auto height() const -> double;

    auto operator+(const Point& p) const -> Rectangle;
    auto operator+=(const Point& p) -> Rectangle&;
    auto operator-(const Point& p) const -> Rectangle;
    auto operator-=(const Point& p) -> Rectangle&;
    auto operator&=(const Rectangle& o) -> Rectangle&;
    auto operator*(double n) const -> Rectangle;
    auto operator*=(double n) -> Rectangle;
};
} // namespace gawl
