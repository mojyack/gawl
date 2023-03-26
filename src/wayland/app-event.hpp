#pragma once
#include "../variant-buffer.hpp"

namespace gawl::wl::internal {
class WindowBase;

namespace app_event {
struct HandleEventArgs {
    ::gawl::wl::internal::WindowBase* window;
};

struct CloseWindowArgs {
    ::gawl::wl::internal::WindowBase* window;
};

struct QuitApplicationArgs {};

using Queue = ::gawl::internal::VariantEventBuffer<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs>;
} // namespace app_event
} // namespace gawl::wl::internal
