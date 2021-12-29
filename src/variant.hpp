#pragma once
#include <variant>

namespace gawl {
template <typename... Ts>
class Variant {
  private:
    template <typename>
    struct Tag {};

    using Finder = std::variant<Tag<Ts>...>;

    std::variant<Ts...> data;

  public:
    auto index() const -> size_t {
        return data.index();
    }
    template <typename T>
    constexpr static auto index_of() -> size_t {
        return Finder(Tag<T>{}).index();
    }
    template <typename T>
    auto get() -> T& {
        return std::get<T>(data);
    }
    template <typename T>
    auto get() const -> const T& {
        return std::get<T>(data);
    }
    template <typename T, typename... Args>
    auto emplace(Args&&... args) -> Variant<Ts...>& {
        data.template emplace<T>(std::forward<Args>(args)...);
        return *this;
    }
    auto emplace(auto&& o) -> Variant<Ts...>& {
        data = std::move(o);
        return *this;
    }
    Variant() {}
    Variant(auto&& o) : data(std::move(o)) {}
    template <typename T, typename... Args>
    Variant(std::in_place_type_t<T>, Args&&... args) : data(std::in_place_type<T>, std::forward<Args>(args)...) {}
};
} // namespace gawl
