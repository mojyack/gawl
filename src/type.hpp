#pragma once
#include <array>
#include <condition_variable>
#include <mutex>

#include <linux/input-event-codes.h>

namespace gawl {
struct Area {
    std::array<double, 4> data;

    auto magnify(const double scale) -> void {
        for(auto& p : data) {
            p *= scale;
        }
    }
    auto width() const -> double {
        return data[2] - data[0];
    }
    auto height() const -> double {
        return data[3] - data[1];
    }
    auto operator[](size_t i) const -> double {
        return data[i];
    }
    auto operator[](size_t i) -> double& {
        return data[i];
    }
};
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
struct SafeVar {
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
    SafeVar(T src) : data(src) {}
    SafeVar() {}
};

class ConditionalVariable {
  private:
    std::condition_variable condv;
    SafeVar<bool>           waked;

  public:
    void wait() {
        waked.store(false);
        auto lock = std::unique_lock<std::mutex>(waked.mutex);
        condv.wait(lock, [this]() { return waked.data; });
    }
    template <typename D>
    bool wait_for(D duration) {
        waked.store(false);
        auto lock = std::unique_lock<std::mutex>(waked.mutex);
        return condv.wait_for(lock, duration, [this]() { return waked.data; });
    }
    void wakeup() {
        waked.store(true);
        condv.notify_all();
    }
};
} // namespace gawl
