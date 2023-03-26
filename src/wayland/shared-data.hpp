#pragma once
#include "app-event.hpp"
#include "eglobject.hpp"
#include "wlobject.hpp"

namespace gawl::wl::internal {
struct SharedData {
    WaylandClientObject* wl;
    EGLObject*           egl;
    app_event::Queue*    application_events;
};
} // namespace gawl::wl::internal
