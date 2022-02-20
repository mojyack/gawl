#pragma once
#include <list>
#include <unordered_map>

#include "../util.hpp"
#include "../window-impl-concept.hpp"
#include "towl.hpp"

namespace gawl::internal::wl {
struct KeyRepeatConfig {
    int32_t interval;
    int32_t delay_in_milisec;
};

template <class... Impls>
struct GlueParameter {
    std::list<Variant<Impls...>>&   windows;
    std::optional<KeyRepeatConfig>& config;
};

template <class... Impls>
class SeatGlue {
  public:
    static auto proc_window(std::list<Variant<Impls...>>& windows, const towl::SurfaceTag surface, auto&& proc) -> void {
        for(auto& w : windows) {
            const auto matched = std::visit(
                [surface, proc](auto& w) -> bool {
                    if(w.wl_get_surface() == surface) {
                        proc(w);
                        return true;
                    }
                    return false;
                },
                w.as_variant());
            if(matched) {
                return;
            }
        }
        // the surface maybe destroyed
        // simply ignore this call
        return;
    }

    class PointerGlue {
      private:
        std::list<Variant<Impls...>>* windows;
        towl::SurfaceTag              active;

      public:
        auto on_enter(const towl::SurfaceTag surface, const double x, const double y) -> void {
            active = surface;
            on_motion(x, y);
        }
        auto on_leave(const towl::SurfaceTag /*surface*/) -> void {
            active = towl::nulltag;
        }
        auto on_motion(const double x, const double y) -> void {
            if(!active) {
                return;
            }
            proc_window(*windows, active, [x, y](auto& impl) -> void { impl.wl_on_pointer_motion(x, y); });
        }
        auto on_button(const uint32_t button, const uint32_t state) -> void {
            if(!active) {
                return;
            }
            proc_window(*windows, active, [button, state](auto& impl) -> void { impl.wl_on_click(button, state); });
        }
        auto on_axis(const uint32_t axis, const double value) -> void {
            if(!active) {
                return;
            }
            proc_window(*windows, active, [axis, value](auto& impl) -> void { impl.wl_on_pointer_axis(axis, value); });
        }
        PointerGlue(GlueParameter<Impls...>& parameter) : windows(&parameter.windows) {}
    };

    class KeyboardGlue {
      private:
        std::list<Variant<Impls...>>*   windows;
        std::optional<KeyRepeatConfig>* config;
        towl::SurfaceTag                active;

      public:
        auto on_enter(const towl::SurfaceTag surface, const towl::Array<uint32_t>& keys) -> void {
            active = surface;
            proc_window(*windows, active, [&keys](auto& impl) -> void { impl.wl_on_key_enter(keys); });
        }
        auto on_leave(const towl::SurfaceTag surface) -> void {
            active = towl::nulltag;
            proc_window(*windows, surface, [](auto& impl) -> void { impl.wl_on_key_leave(); });
        }
        auto on_key(const uint32_t key, const uint32_t state) -> void {
            if(!active) {
                return;
            }
            proc_window(*windows, active, [key, state](auto& impl) -> void { impl.wl_on_key_input(key, state); });
        }
        auto on_repeat_info(const int32_t rate, const int32_t delay) -> void {
            config->emplace(KeyRepeatConfig{1000 / rate, delay});
        };
        KeyboardGlue(GlueParameter<Impls...>& parameter) : windows(&parameter.windows), config(&parameter.config) {}
    };

    PointerGlue  pointer_glue;
    KeyboardGlue keyboard_glue;

    SeatGlue(GlueParameter<Impls...>& parameter) : pointer_glue(parameter), keyboard_glue(parameter) {}
};

template <class... Impls>
class OutputGlue {
  private:
    std::list<Variant<Impls...>>* windows;

  public:
    auto on_scale(const towl::OutputTag output, const int32_t scale) -> void {
        for(auto& w : *windows) {
            std::visit([output, scale](auto& w) {
                if(w.wl_get_output() == output) {
                    w.resize_buffer(-1, -1, scale);
                }
            },
                       w.as_variant());
        }
    }
    OutputGlue(GlueParameter<Impls...>& parameter) : windows(&parameter.windows) {}
};

using Compositor = towl::Compositor<4>;
using WMBase     = towl::WMBase<2>;
template <class... Impls>
using Seat = towl::Seat<4, SeatGlue<Impls...>>;
template <class... Impls>
using Output = towl::Output<2, OutputGlue<Impls...>>;

template <class... Impls>
using Registry = towl::Registry<GlueParameter<Impls...>, Compositor, WMBase, Seat<Impls...>, Output<Impls...>>;

template <class... Impls>
struct WaylandClientObject {
    towl::Display                  display;
    Registry<Impls...>             registry;
    std::optional<KeyRepeatConfig> repeat_config;

    WaylandClientObject(std::list<Variant<Impls...>>& windows) : registry(display.get_registry<GlueParameter<Impls...>, Compositor, WMBase, Seat<Impls...>, Output<Impls...>>({windows, repeat_config})) {}
};
} // namespace gawl::internal::wl
