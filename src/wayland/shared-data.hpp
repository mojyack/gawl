#pragma once
#include <wayland-client.hpp>

#include "../variant-buffer.hpp"
#include "eglobject.hpp"
#include "wlobject.hpp"

namespace gawl::internal::wl {
template <class... Impls>
struct SharedData {
    struct HandleEventArgs {
        Variant<Impls*...> window;
    };
    struct CloseWindowArgs {
        Variant<Impls*...> window;
    };
    struct QuitApplicationArgs {};
    using BufferType = VariantEventBuffer<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs>;

    WaylandClientObject* wl;
    EGLObject*           egl;
    BufferType*          application_events;
};
}; // namespace gawl::internal::wl
