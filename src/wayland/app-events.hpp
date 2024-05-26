#pragma once
#include "../variant-buffer.hpp"

namespace gawl {
class Window;
namespace impl {
struct HandleEventArgs {
    Window* window;
};

struct CloseWindowArgs {
    Window* window;
};

struct QuitApplicationArgs {};

using AppEventQueue = ::gawl::impl::VariantEventBuffer<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs>;
} // namespace impl
} // namespace gawl
