#pragma once
#include <array>
#include <cmath>
#include <concepts>
#include <condition_variable>
#include <mutex>

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
};

struct Rectangle {
    Point a;
    Point b;

    auto magnify(const double scale) -> void {
        a.magnify(scale);
        b.magnify(scale);
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
};

template <typename T>
concept PointArray = requires(T c) {
    std::same_as<typename T::value_type, Point>;
    c[0];
} && std::ranges::range<T>;

template <PointArray T>
auto rotate(T& points, const Point& origin, const double angle) -> void {
    for(auto& p : points) {
        p.rotate(origin, angle);
    }
}

using Color = std::array<double, 4>;

enum class ButtonState {
    press,
    release,
    repeat,
    leave,
};

enum class WheelAxis {
    vertical,
    horizontal,
};

enum class Align {
    left,
    center,
    right,
};

template <typename T>
struct Critical {
    mutable std::mutex mutex;
    T                  data;

    auto get_lock() const -> std::lock_guard<std::mutex> {
        return std::lock_guard<std::mutex>(mutex);
    }
    auto store(T src) -> void {
        auto lock = get_lock();
        data      = src;
    }
    auto load() const -> T {
        auto lock = get_lock();
        return data;
    }
    auto operator->() -> T* {
        return &data;
    }
    auto operator*() -> T& {
        return data;
    }
    Critical(T src) : data(src) {}
    Critical() {}
};

class Event {
  private:
    std::condition_variable condv;
    Critical<bool>          waked;

  public:
    auto wait() -> void {
        waked.store(false);
        auto lock = std::unique_lock<std::mutex>(waked.mutex);
        condv.wait(lock, [this]() { return waked.data; });
    }
    auto wait_for(auto duration) -> bool {
        waked.store(false);
        auto lock = std::unique_lock<std::mutex>(waked.mutex);
        return condv.wait_for(lock, duration, [this]() { return waked.data; });
    }
    auto wakeup() -> void {
        waked.store(true);
        condv.notify_all();
    }
};
} // namespace gawl
