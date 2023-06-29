#pragma once
#include <xkbcommon/xkbcommon.h>

#include "towl.hpp"

namespace gawl::wl::internal {
class WaylandWindow {
  public:
    virtual auto wl_get_surface() -> towl::SurfaceTag                                                                       = 0;
    virtual auto wl_get_output() const -> towl::OutputTag                                                                   = 0;
    virtual auto wl_on_keycode_enter(const towl::Array<uint32_t>& keys) -> void                                             = 0;
    virtual auto wl_on_keysym_enter(const std::vector<xkb_keycode_t>& keys, xkb_state* xkb_state) -> void                   = 0;
    virtual auto wl_on_key_leave() -> void                                                                                  = 0;
    virtual auto wl_on_keycode_input(uint32_t keycode, uint32_t state) -> void                                              = 0;
    virtual auto wl_on_keysym_input(xkb_keycode_t keysym, uint32_t state, bool enable_repeat, xkb_state* xkb_state) -> void = 0;
    virtual auto wl_on_click(uint32_t button, uint32_t state) -> void                                                       = 0;
    virtual auto wl_on_pointer_motion(double x, double y) -> void                                                           = 0;
    virtual auto wl_on_pointer_axis(uint32_t axis, double value) -> void                                                    = 0;
    virtual auto wl_on_touch_down(uint32_t id, double x, double y) -> void                                                  = 0;
    virtual auto wl_on_touch_up(uint32_t id) -> void                                                                        = 0;
    virtual auto wl_on_touch_motion(uint32_t id, double x, double y) -> void                                                = 0;
    virtual auto wl_set_output_scale(uint32_t scale) -> void                                                                = 0;

    virtual ~WaylandWindow() {}
};
} // namespace gawl::wl::internal
