#pragma once
#include <list>
#include <unordered_map>

#include <sys/mman.h>

#include "../util.hpp"
#include "window-base.hpp"

namespace gawl::wl::internal {
struct KeyRepeatConfig {
    int32_t interval;
    int32_t delay_in_milisec;
};

using Windows = std::vector<std::unique_ptr<::gawl::internal::Window>>;

struct GlueParameter {
    Critical<Windows>*              critical_windows;
    std::optional<KeyRepeatConfig>* config;
};

template <class C, class B>
    requires std::derived_from<C, B>
auto downcast(const B* const ptr) -> C* {
    return (C*)((std::byte*)(ptr) - ((std::byte*)(B*)(C*)(0x1000) - (std::byte*)(C*)(0x1000)));
}

auto proc_window(Critical<Windows>& critical_windows, const towl::SurfaceTag surface, auto proc) -> void {
    auto [lock, windows] = critical_windows.access();
    for(const auto& w_ : windows) {
        const auto w = downcast<WindowBase>(w_.get());
        if(w->wl_get_surface() == surface) {
            proc(*w);
            return;
        }
    }
}

class PointerGlue {
  private:
    Critical<Windows>* critical_windows;
    towl::SurfaceTag   active;

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
        proc_window(*critical_windows, active, [x, y](auto& impl) -> void { impl.wl_on_pointer_motion(x, y); });
    }

    auto on_button(const uint32_t button, const uint32_t state) -> void {
        if(!active) {
            return;
        }
        proc_window(*critical_windows, active, [button, state](auto& impl) -> void { impl.wl_on_click(button, state); });
    }

    auto on_axis(const uint32_t axis, const double value) -> void {
        if(!active) {
            return;
        }
        proc_window(*critical_windows, active, [axis, value](auto& impl) -> void { impl.wl_on_pointer_axis(axis, value); });
    }

    PointerGlue(GlueParameter& parameter) : critical_windows(parameter.critical_windows) {}
};

template <bool use_keycode, bool use_keysym>
class KeyboardGlue {
  private:
    struct XKB {
        struct KeymapDeleter {
            auto operator()(xkb_keymap* keymap) -> void {
                xkb_keymap_unref(keymap);
            }
        };

        struct StateDeleter {
            auto operator()(xkb_state* state) -> void {
                xkb_state_unref(state);
            }
        };

        std::unique_ptr<xkb_keymap, KeymapDeleter> keymap = nullptr;
        std::unique_ptr<xkb_state, StateDeleter>   state  = nullptr;

        XKB(xkb_keymap* const keymap, xkb_state* const state) : keymap(keymap), state(state) {}
    };

    Critical<Windows>*              critical_windows;
    std::optional<KeyRepeatConfig>* config;
    towl::SurfaceTag                active;

    [[no_unique_address]] std::conditional_t<use_keysym, std::optional<XKB>, towl::Empty> xkb;

  public:
    auto on_keymap(const uint32_t format, const int32_t fd, const uint32_t size) -> void {
        if constexpr(use_keysym) {
            auto file = FileDescriptor(fd);
            if(format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
                return;
            }

            const auto mapstr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
            if(mapstr == MAP_FAILED) {
                return;
            }

            const auto context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
            if(!context) {
                return;
            }

            const auto keymap = xkb_keymap_new_from_string(context, (const char*)mapstr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
            if(!keymap) {
                xkb_context_unref(context);
                return;
            }

            xkb = XKB(keymap, xkb_state_new(keymap));
            xkb_context_unref(context);

            munmap(mapstr, size);
        }
    }

    auto on_enter(const towl::SurfaceTag surface, const towl::Array<uint32_t>& keys) -> void {
        active = surface;
        if constexpr(use_keycode) {
            proc_window(*critical_windows, active, [&keys](auto& impl) -> void { impl.wl_on_keycode_enter(keys); });
        }
        if constexpr(use_keysym) {
            if(xkb) {
                auto syms = std::vector<xkb_keysym_t>(keys.size);
                for(auto i = size_t(0); i < keys.size; i += 1) {
                    syms[i] = keys.data[i] + 8;
                }
                proc_window(*critical_windows, active, [this, &syms](auto& impl) -> void { impl.wl_on_keysym_enter(syms, xkb->state.get()); });
            }
        }
    }

    auto on_leave(const towl::SurfaceTag surface) -> void {
        proc_window(*critical_windows, surface, [](auto& impl) -> void { impl.wl_on_key_leave(); });
        active = towl::nulltag;
    }

    auto on_key(const uint32_t key, const uint32_t state) -> void {
        if constexpr(use_keycode) {
            proc_window(*critical_windows, active, [key, state](auto& impl) -> void {
                impl.wl_on_keycode_input(key, state);
            });
        }
        if constexpr(use_keysym) {
            if(xkb) {
                proc_window(*critical_windows, active, [this, key, state](auto& impl) -> void {
                    impl.wl_on_keysym_input(key + 8, state, xkb_keymap_key_repeats(xkb->keymap.get(), key + 8), xkb->state.get());
                });
            }
        }
    }

    auto on_modifiers(const uint32_t mods_depressed, const uint32_t mods_latched, const uint32_t mods_locked, const uint32_t group) -> void {
        if constexpr(use_keysym) {
            if(xkb) {
                xkb_state_update_mask(xkb->state.get(), mods_depressed, mods_latched, mods_locked, group, 0, 0);
            }
        }
    }

    auto on_repeat_info(const int32_t rate, const int32_t delay) -> void {
        config->emplace(KeyRepeatConfig{1000 / rate, delay});
    }

    KeyboardGlue(GlueParameter& parameter) : critical_windows(parameter.critical_windows), config(parameter.config) {}
};

class OutputGlue {
  private:
    Critical<Windows>* critical_windows;

  public:
    auto on_scale(const towl::OutputTag output, const int32_t scale) -> void {
        auto [lock, windows] = critical_windows->access();
        for(const auto& w_ : windows) {
            const auto w = downcast<WindowBase>(w_.get());
            if(w->wl_get_output() == output) {
                w->wl_set_output_scale(scale);
            }
        }
    }

    OutputGlue(GlueParameter& parameter) : critical_windows(parameter.critical_windows) {}
};

// TODO:
// rewrite without macro
#if defined(GAWL_KEYCODE)
#if defined(GAWL_KEYSYM)
using KeyboardGlueOpt = KeyboardGlue<true, true>;
#else
using KeyboardGlueOpt = KeyboardGlue<true, false>;
#endif
#else
#if defined(GAWL_KEYSYM)
using KeyboardGlueOpt = KeyboardGlue<false, true>;
#else
using KeyboardGlueOpt = towl::Empty;
#endif
#endif

#if defined(GAWL_MOUSE)
using PointerGlueOpt = PointerGlue;
#else
using PointerGlueOpt  = towl::Empty;
#endif

using Compositor = towl::Compositor<4>;
using WMBase     = towl::WMBase<2>;
using Seat       = towl::Seat<4, KeyboardGlueOpt, PointerGlueOpt>;
using Output     = towl::Output<2, OutputGlue>;
using Registry   = towl::Registry<GlueParameter, Compositor, WMBase, Seat, Output>;

struct WaylandClientObject {
    towl::Display                  display;
    Registry                       registry;
    std::optional<KeyRepeatConfig> repeat_config;

    WaylandClientObject(Critical<Windows>& critical_windows)
        : registry(display.get_registry<GlueParameter, Compositor, WMBase, Seat, Output>({&critical_windows, &repeat_config})) {}
};
} // namespace gawl::wl::internal
