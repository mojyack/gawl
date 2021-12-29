#pragma once
#include <vector>

#include "thread.hpp"
#include "variant.hpp"

namespace gawl {
template <class... Ts>
class VariantBuffer {
  private:
    using Item = Variant<Ts...>;

    Critical<std::vector<Item>> buffer;

  public:
    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        const auto lock = buffer.get_lock();
        buffer->emplace_back(Item(std::in_place_type<T>, std::forward<Args>(args)...));
    }
    auto push(auto&& args) -> void {
        const auto lock = buffer.get_lock();
        buffer->emplace_back(Item(std::move(args)));
    }
    auto exchange() -> std::vector<Item> {
        return buffer.replace();
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
        const auto lock = buffer.get_lock();
        buffer->emplace_back(Item(std::in_place_type<T>, std::forward<Args>(args)...));
        event.wakeup();
    }
    auto push(auto&& args) -> void {
        const auto lock = buffer.get_lock();
        buffer->emplace_back(Item(std::move(args)));
        event.wakeup();
    }
    auto exchange() -> std::vector<Item> {
        return buffer.replace();
    }
    auto wait() -> void {
        event.wait();
    }
    template <class T>
    constexpr static auto index_of() -> size_t {
        return Item::template index_of<T>();
    }
};
} // namespace gawl
