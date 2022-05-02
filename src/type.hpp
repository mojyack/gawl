#pragma once
#include <array>
#include <cmath>
#include <concepts>
#include <numbers>
#include <ranges>

#include <linux/input-event-codes.h>

namespace gawl {
struct Point {
    double x;
    double y;

    auto magnify(const double scale) -> void {
        x *= scale;
        y *= scale;
    }
    auto rotate(const Point& origin, const double angle) -> void {
        const auto a  = angle * 2 * std::numbers::pi;
        const auto s  = std::sin(a);
        const auto c  = std::cos(a);
        const auto rx = x - origin.x;
        const auto ry = y - origin.y;
        x             = origin.x + rx * c - ry * s;
        y             = origin.y + rx * s + ry * c;
    }
    auto operator+=(const Point& o) -> Point& {
        x += o.x;
        y += o.y;
        return *this;
    }
};

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
};

template <typename T>
concept PointArray = requires(T c) {
    std::same_as<typename T::value_type, Point>;
    c[0];
}
&&std::ranges::range<T>;

template <PointArray T>
auto rotate(T& points, const Point& origin, const double angle) -> void {
    for(auto& p : points) {
        p.rotate(origin, angle);
    }
}

using Color = std::array<double, 4>;

enum class ButtonState {
    Press,
    Release,
    Repeat,
    Enter,
    Leave,
};

enum class WheelAxis {
    Vertical,
    Horizontal,
};

enum class Align {
    Left,
    Center,
    Right,
};

enum class ModifierFlags {
    None    = 0,
    Shift   = 1 << 0,
    Lock    = 1 << 1,
    Control = 1 << 2,
    Mod1    = 1 << 3,
    Mod2    = 1 << 4,
    Mod4    = 1 << 5,

};

constexpr auto operator|(const ModifierFlags a, const ModifierFlags b) -> ModifierFlags {
    return static_cast<ModifierFlags>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr auto operator&(const ModifierFlags a, const ModifierFlags b) -> ModifierFlags {
    return static_cast<ModifierFlags>(static_cast<int>(a) & static_cast<int>(b));
}
} // namespace gawl
