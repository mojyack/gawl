#pragma once
#include <vector>

#include "util.hpp"

namespace gawl::internal {
template <class... Ts>
class VariantBuffer {
  public:
    using Item = Variant<Ts...>;

  private:
    Critical<std::vector<Item>> buffer[2];
    std::atomic_int             flip = 0;

  public:
    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        auto [lock, data] = buffer[flip].access();
        data.emplace_back(Item(std::in_place_type<T>, std::forward<Args>(args)...));
    }

    auto push(Item item) -> void {
        auto [lock, data] = buffer[flip].access();
        data.emplace_back(std::move(item));
    }

    auto swap() -> std::vector<Item>& {
        buffer[!flip].unsafe_access().clear();
        flip ^= 1;
        auto [lock, data] = buffer[!flip].access();
        return data;
    }

    template <class T>
    constexpr static auto index_of() -> size_t {
        return Item::template index_of<T>();
    }
};

template <class... Ts>
class VariantEventBuffer {
  public:
    using Item = Variant<Ts...>;

  private:
    Critical<std::vector<Item>> buffer[2];
    std::atomic_int             flip = 0;
    Event                       event;

  public:
    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        auto [lock, data] = buffer[flip].access();
        data.emplace_back(Item(std::in_place_type<T>, std::forward<Args>(args)...));
        event.wakeup();
    }

    auto push(Item item) -> void {
        auto [lock, data] = buffer[flip].access();
        data.emplace_back(std::move(item));
        event.wakeup();
    }

    auto swap() -> std::vector<Item>& {
        buffer[!flip].unsafe_access().clear();
        flip ^= 1;
        auto [lock, data] = buffer[!flip].access();
        return data;
    }

    auto wait() -> void {
        event.wait();
    }

    template <class T>
    constexpr static auto index_of() -> size_t {
        return Item::template index_of<T>();
    }
};
} // namespace gawl::internal
