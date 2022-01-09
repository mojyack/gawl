#pragma once
#include <array>
#include <concepts>

#include "binder.hpp"

namespace gawl {
namespace concepts {
template <class S>
concept MetaScreen = requires(const S& c) {
    { c.get_scale() } -> std::same_as<double>;
    { c.get_screen_size() } -> std::convertible_to<std::array<std::size_t, 2>>;
};
template <class S>
concept Screen = requires(S& m) {
    { m.prepare() } -> std::same_as<internal::FramebufferBinder>;
} && MetaScreen<S>;
} // namespace concepts
} // namespace gawl
