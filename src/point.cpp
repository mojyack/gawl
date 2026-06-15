#include <cmath>
#include <numbers>

#include "point.hpp"

namespace gawl {
auto Point::rotate(const Point& origin, const double angle) -> void {
    const auto a  = angle * 2 * std::numbers::pi;
    const auto s  = std::sin(a);
    const auto c  = std::cos(a);
    const auto rx = x - origin.x;
    const auto ry = y - origin.y;
    x             = origin.x + rx * c - ry * s;
    y             = origin.y + rx * s + ry * c;
}

auto Point::operator+(const Point& o) const -> Point {
    return Point{x + o.x, y + o.y};
}

auto Point::operator+=(const Point& o) -> Point& {
    *this = *this + o;
    return *this;
}

auto Point::operator-(const Point& o) const -> Point {
    return Point{x - o.x, y - o.y};
}

auto Point::operator-=(const Point& o) -> Point& {
    *this = *this - o;
    return *this;
}

auto Point::operator*(const double n) const -> Point {
    return Point{x * n, y * n};
}

auto Point::operator*=(const double n) -> Point& {
    *this = *this * n;
    return *this;
}

auto rotate(const std::span<Point> points, const Point& origin, const double angle) -> void {
    for(auto& p : points) {
        p.rotate(origin, angle);
    }
}
} // namespace gawl
