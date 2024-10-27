#pragma once
#include "../window.hpp"
#include "towl/towl.hpp"

namespace gawl::impl {
using Windows = std::vector<std::unique_ptr<Window>>;

struct KeyRepeatConfig {
    int32_t interval;
    int32_t delay_in_milisec;
};

class WaylandCallbacks;
auto delete_wayland_callbacks(WaylandCallbacks* callbacks) -> void;
declare_autoptr(WaylandCallbacks, WaylandCallbacks, delete_wayland_callbacks);

struct WaylandClientObjects {
    towl::Display                  display;
    towl::Registry                 registry;
    AutoWaylandCallbacks           callbacks;
    towl::CompositorBinder         compositor_binder;
    towl::XDGWMBaseBinder          xdg_wm_base_binder;
    towl::SeatBinder               seat_binder;
    std::optional<KeyRepeatConfig> repeat_config;

    static auto create(Windows& windows) -> std::unique_ptr<WaylandClientObjects>;
};
} // namespace gawl::impl
