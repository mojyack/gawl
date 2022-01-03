#pragma once
#include <wayland-client.hpp>

#include "../variant-buffer.hpp"
#include "../window.hpp"
#include "eglobject.hpp"

namespace gawl::internal::wl {
template <class Backend>
struct SharedData {
    struct HandleEventArgs {
        Backend& window;
    };
    struct CloseWindowArgs {
        Backend& window;
    };
    struct QuitApplicationArgs {};
    using BufferType = VariantEventBuffer<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs>;

    wayland::display_t* display;
    BufferType*         application_events;
    EGLObject*          egl;
};
}; // namespace gawl::internal::wl
