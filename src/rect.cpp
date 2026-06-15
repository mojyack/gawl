#include <algorithm>

#include "rect.hpp"

namespace gawl {
auto Rectangle::expand(const double w, const double h) -> Rectangle& {
    a.x -= w;
    a.y -= h;
    b.x += w;
    b.y += h;
    return *this;
}

auto Rectangle::to_points() const -> std::array<Point, 4> {
    return {a, {b.x, a.y}, b, {a.x, b.y}};
}

auto Rectangle::width() const -> double {
    return b.x - a.x;
}

auto Rectangle::height() const -> double {
    return b.y - a.y;
}

auto Rectangle::operator+(const Point& o) const -> Rectangle {
    return {a + o, b + o};
}

auto Rectangle::operator+=(const Point& o) -> Rectangle& {
    *this = *this + o;
    return *this;
}

auto Rectangle::operator-(const Point& o) const -> Rectangle {
    return {a - o, b - o};
}

auto Rectangle::operator-=(const Point& o) -> Rectangle& {
    *this = *this - o;
    return *this;
}

auto Rectangle::operator*(double n) const -> Rectangle {
    return Rectangle{a * n, b * n};
}

auto Rectangle::operator*=(double n) -> Rectangle {
    *this = *this * n;
    return *this;
}

auto Rectangle::operator&=(const Rectangle& o) -> Rectangle& {
    a.x = std::max(a.x, o.a.x);
    a.y = std::max(a.y, o.a.y);
    b.x = std::min(b.x, o.b.x);
    b.y = std::min(b.y, o.b.y);
    return *this;
}
} // namespace gawl
