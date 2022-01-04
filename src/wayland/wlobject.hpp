#pragma once
#include <unordered_map>

#include <wayland-client-core.h>
#include <wayland-client-protocol-extra.hpp>
#include <wayland-client-protocol.hpp>
#include <wayland-cursor.hpp>
#include <wayland-egl.hpp>

namespace gawl::internal::wl {
struct WaylandClientObject {
    wayland::display_t  display;
    wayland::registry_t registry;

    wayland::compositor_t  compositor;
    wayland::xdg_wm_base_t xdg_wm_base;
    wayland::seat_t        seat;
    wayland::shm_t         shm;
    wayland::output_t      output;

    wayland::pointer_t  pointer;
    wayland::keyboard_t keyboard;

    std::unordered_map<std::string, wayland::proxy_t&> registry_map = {
        {wayland::compositor_t::interface_name, compositor},
        {wayland::xdg_wm_base_t::interface_name, xdg_wm_base},
        {wayland::seat_t::interface_name, seat},
        {wayland::shm_t::interface_name, shm},
        {wayland::output_t::interface_name, output},
    };

    struct KeyRepeatConfig {
        uint32_t interval;
        uint32_t delay_in_milisec;
    };
    std::optional<KeyRepeatConfig> repeat_config;

    int buffer_scale = -1;
};

struct WaylandWindowObject {
    wayland::surface_t       surface;
    wayland::shell_surface_t shell_surface;
    wayland::xdg_surface_t   xdg_surface;
    wayland::xdg_toplevel_t  xdg_toplevel;
    wayland::callback_t      frame_cb;
    wayland::cursor_image_t  cursor_image;
    wayland::buffer_t        cursor_buffer;
    wayland::surface_t       cursor_surface;
};
} // namespace gawl::internal::wl
