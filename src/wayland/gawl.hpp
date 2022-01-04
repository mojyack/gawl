#pragma once
#include "../include.hpp"
#include "application.hpp"
#include "window.hpp"

namespace gawl {
template <class... Impls>
struct Gawl {
    using Application = internal::wl::ApplicationBackend<Impls...>;
    using WindowCreateHint = internal::WindowCreateHint<internal::wl::SharedData<Impls...>>;

    template <class Impl>
    using Window = internal::wl::WindowBackend<Impl, Impls...>;
};
} // namespace gawl
