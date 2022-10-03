#pragma once
#include <array>
#include <concepts>

#include "binder.hpp"
#include "internal-type.hpp"
#include "type.hpp"

namespace gawl {
namespace concepts {
template <class S>
concept MetaScreen = requires(const S& c) {
    { c.get_scale() } -> std::same_as<double>;
    { c.get_viewport() } -> std::convertible_to<internal::Viewport>;
};

template <class S>
concept Screen = requires(S& m) {
    { m.prepare() } -> std::same_as<internal::FramebufferBinder>;
    { m.set_viewport(gawl::Rectangle{}) } -> std::same_as<void>;
    { m.unset_viewport() } -> std::same_as<void>;
}
&&MetaScreen<S>;
} // namespace concepts
} // namespace gawl
