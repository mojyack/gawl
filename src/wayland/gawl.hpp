#pragma once
#include "../include.hpp"
#include "application.hpp"
#include "window.hpp"

namespace gawl {
template <class... Impls>
struct Gawl {
    using Application = internal::wl::ApplicationBackend<Impls...>;

    template <class Impl>
    using Window = gawl::wl::Window<Impl, Impls...>;
};
} // namespace gawl
