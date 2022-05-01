#pragma once
#include "../variant-buffer.hpp"
#include "eglobject.hpp"
#include "wlobject.hpp"

namespace gawl::internal::wl {
template <template <class, class...> class Backend, class... Impls>
struct SharedData {
    struct HandleEventArgs {
        Variant<Backend<Impls, Impls...>*...> window;
    };
    struct CloseWindowArgs {
        Variant<Backend<Impls, Impls...>*...> window;
    };
    struct QuitApplicationArgs {};
    using BufferType = VariantEventBuffer<HandleEventArgs, CloseWindowArgs, QuitApplicationArgs>;

    using Wl = Wl<Backend, Impls...>;

    typename Wl::WaylandClientObject* wl;
    EGLObject*                        egl;
    BufferType*                       application_events;
};
}; // namespace gawl::internal::wl
