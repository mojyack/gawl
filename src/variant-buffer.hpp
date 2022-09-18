#pragma once
#include <vector>

#include "util.hpp"

namespace gawl::internal {
template <class... Ts>
class VariantBuffer {
  private:
    using Item = Variant<Ts...>;

    Critical<std::vector<Item>> buffer;

  public:
    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        auto [lock, data] = buffer.access();
        data.emplace_back(Item(std::in_place_type<T>, std::forward<Args>(args)...));
    }
    auto push(auto&& args) -> void {
        auto [lock, data] = buffer.access();
        data.emplace_back(Item(std::move(args)));
    }
    auto exchange() -> std::vector<Item> {
        auto [lock, data] = buffer.access();
        return std::move(data);
    }
    template <class T>
    constexpr static auto index_of() -> size_t {
        return Item::template index_of<T>();
    }
};

template <class... Ts>
class VariantEventBuffer {
  private:
    using Item = Variant<Ts...>;

    Critical<std::vector<Item>> buffer;
    Event                       event;

  public:
    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        auto [lock, data] = buffer.access();
        data.emplace_back(Item(std::in_place_type<T>, std::forward<Args>(args)...));
        event.wakeup();
    }
    auto push(auto&& args) -> void {
        auto [lock, data] = buffer.access();
        data.emplace_back(Item(std::move(args)));
        event.wakeup();
    }
    auto exchange() -> std::vector<Item> {
        auto [lock, data] = buffer.access();
        return std::move(data);
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
