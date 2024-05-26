#pragma once
#include "application.hpp"

namespace gawl {
using Application = wl::Application;

template <class T>
using Window = wl::Window<T>;
} // namespace gawl
