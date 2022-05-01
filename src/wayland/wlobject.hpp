#pragma once
#include <list>
#include <unordered_map>

#include <sys/mman.h>

#include "../util.hpp"
#include "../window-impl-concept.hpp"
#include "towl.hpp"

namespace gawl::internal::wl {
template <template <class, class...> class Backend, class... Impls>
struct Wl {
    struct KeyRepeatConfig {
        int32_t interval;
        int32_t delay_in_milisec;
    };

    struct GlueParameter {
        std::list<Variant<Backend<Impls, Impls...>...>>* windows;
        std::optional<KeyRepeatConfig>*                  config;
    };

    class SeatGlue {
      public:
        static auto proc_window(std::list<Variant<Backend<Impls, Impls...>...>>& windows, const towl::SurfaceTag surface, auto&& proc) -> void {
            for(auto& w : windows) {
                const auto matched = w.visit(
                    [surface, proc](auto& w) -> bool {
                        if(w.wl_get_surface() == surface) {
                            proc(w);
                            return true;
                        }
                        return false;
                    });
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
            std::list<Variant<Backend<Impls, Impls...>...>>* windows;
            towl::SurfaceTag                                 active;

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
            PointerGlue(GlueParameter& parameter) : windows(parameter.windows) {}
        };

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

            std::list<Variant<Backend<Impls, Impls...>...>>* windows;
            std::optional<KeyRepeatConfig>*                  config;
            towl::SurfaceTag                                 active;

            [[no_unique_address]] std::conditional_t<Implement<Impls...>::keysym, std::optional<XKB>, towl::Empty> xkb;

            auto calc_modifiers() const -> ModifierFlags {
                if(xkb) {
                    return (xkb_state_mod_name_is_active(xkb->state.get(), XKB_MOD_NAME_SHIFT, xkb_state_component(1)) ? ModifierFlags::Control : ModifierFlags::None) |
                           (xkb_state_mod_name_is_active(xkb->state.get(), XKB_MOD_NAME_CAPS, xkb_state_component(1)) ? ModifierFlags::Lock : ModifierFlags::None) |
                           (xkb_state_mod_name_is_active(xkb->state.get(), XKB_MOD_NAME_CTRL, xkb_state_component(1)) ? ModifierFlags::Control : ModifierFlags::None) |
                           (xkb_state_mod_name_is_active(xkb->state.get(), XKB_MOD_NAME_ALT, xkb_state_component(1)) ? ModifierFlags::Mod1 : ModifierFlags::None) |
                           (xkb_state_mod_name_is_active(xkb->state.get(), XKB_MOD_NAME_NUM, xkb_state_component(1)) ? ModifierFlags::Mod2 : ModifierFlags::None) |
                           (xkb_state_mod_name_is_active(xkb->state.get(), XKB_MOD_NAME_LOGO, xkb_state_component(1)) ? ModifierFlags::Mod4 : ModifierFlags::None);
                }
                return ModifierFlags::None;
            }

          public:
            auto on_keymap(const uint32_t format, const int32_t fd, const uint32_t size) -> void {
                if constexpr(Implement<Impls...>::keysym) {
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
                if constexpr(Implement<Impls...>::keycode) {
                    proc_window(*windows, active, [&keys](auto& impl) -> void { impl.wl_on_keycode_enter(keys); });
                }
                if constexpr(Implement<Impls...>::keysym) {
                    if(xkb) {
                        auto syms = std::vector<xkb_keysym_t>(keys.size);
                        for(auto i = size_t(0); i < keys.size; i += 1) {
                            syms[i] = xkb_state_key_get_one_sym(xkb->state.get(), keys.data[i] + 8);
                        }
                        proc_window(*windows, active, [this, &syms](auto& impl) -> void { impl.wl_on_keysym_enter(syms, calc_modifiers()); });
                    }
                }
            }
            auto on_leave(const towl::SurfaceTag surface) -> void {
                proc_window(*windows, surface, [](auto& impl) -> void { impl.wl_on_key_leave(); });
                active = towl::nulltag;
            }
            auto on_key(const uint32_t key, const uint32_t state) -> void {
                if constexpr(Implement<Impls...>::keyboard) {
                    if constexpr(Implement<Impls...>::keysym) {
                        if(xkb) {
                            proc_window(*windows, active, [this, key, state](auto& impl) -> void {
                                impl.wl_on_key_input(key, state, xkb_state_key_get_one_sym(xkb->state.get(), key + 8), calc_modifiers(), xkb_keymap_key_repeats(xkb->keymap.get(), key + 8));
                            });
                        }
                    } else {
                        proc_window(*windows, active, [key, state](auto& impl) -> void {
                            impl.wl_on_key_input(key, state, 0, ModifierFlags::None, 0);
                        });
                    }
                }
            }
            auto on_modifiers(const uint32_t mods_depressed, const uint32_t mods_latched, const uint32_t mods_locked, const uint32_t group) -> void {
                if constexpr(Implement<Impls...>::keysym) {
                    if(xkb) {
                        xkb_state_update_mask(xkb->state.get(), mods_depressed, mods_latched, mods_locked, group, 0, 0);
                    }
                }
            }
            auto on_repeat_info(const int32_t rate, const int32_t delay) -> void {
                config->emplace(KeyRepeatConfig{1000 / rate, delay});
            };
            KeyboardGlue(GlueParameter& parameter) : windows(parameter.windows), config(parameter.config) {}
        };

        PointerGlue  pointer_glue;
        KeyboardGlue keyboard_glue;

        SeatGlue(GlueParameter& parameter) : pointer_glue(parameter), keyboard_glue(parameter) {}
    };

    class OutputGlue {
      private:
        std::list<Variant<Backend<Impls, Impls...>...>>* windows;

      public:
        auto on_scale(const towl::OutputTag output, const int32_t scale) -> void {
            for(auto& w : *windows) {
                w.visit([output, scale](auto& w) {
                    if(w.wl_get_output() == output) {
                        w.resize_buffer(-1, -1, scale);
                    }
                });
            }
        }
        OutputGlue(GlueParameter& parameter) : windows(parameter.windows) {}
    };

    using Compositor = towl::Compositor<4>;
    using WMBase     = towl::WMBase<2>;
    using Seat       = towl::Seat<4, SeatGlue>;
    using Output     = towl::Output<2, OutputGlue>;
    using Registry   = towl::Registry<GlueParameter, Compositor, WMBase, Seat, Output>;

    struct WaylandClientObject {
        towl::Display                  display;
        Registry                       registry;
        std::optional<KeyRepeatConfig> repeat_config;

        WaylandClientObject(std::list<Variant<Backend<Impls, Impls...>...>>& windows) : registry(display.get_registry<GlueParameter, Compositor, WMBase, Seat, Output>({&windows, &repeat_config})) {}
    };
};
} // namespace gawl::internal::wl
