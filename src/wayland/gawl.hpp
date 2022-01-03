#pragma once
#include "../include.hpp"
#include "application.hpp"
#include "window.hpp"

namespace gawl {
template <class Impl>
struct Gawl {
    using Window           = internal::wl::WindowBackend<Impl>;
    using Application      = internal::wl::ApplicationBackend<Impl>;
    using WindowCreateHint = typename Window::WindowCreateHintType;
};
} // namespace gawl
