#include "wl-object.hpp"
#include "../macros/unwrap.hpp"
#include "window.hpp"

namespace gawl::impl {

class WaylandCallbacks : public towl::KeyboardCallbacks,
                         public towl::PointerCallbacks,
                         public towl::TouchCallbacks,
                         public towl::OutputCallbacks {
  private:
    Critical<Windows>*              critical_windows;
    std::optional<KeyRepeatConfig>* keyboard_repeat_config;
    wl_surface*                     keyboard_focused = nullptr;
    wl_surface*                     pointer_focused  = nullptr;
    std::vector<wl_surface*>        touch_focused;

    auto find_focused_window(const wl_surface* const surface) -> WaylandWindow* {
        auto [lock, windows] = critical_windows->access();
        for(const auto& window : windows) {
            const auto wl_window = std::bit_cast<WaylandWindow*>(window.get());
            if(wl_window->wl_get_surface() == surface) {
                return wl_window;
            }
        }
        return nullptr;
    }

    auto get_touch_focused(const uint32_t id) -> wl_surface** {
        if(id + 1 >= touch_focused.size()) {
            touch_focused.resize(id + 1);
        }
        return &touch_focused[id];
    }

    // KeyboardCallbacks
    auto on_wl_keyboard_keymap(uint32_t /*format*/, int32_t /*fd*/, uint32_t /*size*/) -> void override {}

    auto on_wl_keyboard_enter(wl_surface* const surface, const towl::Array<uint32_t>& keys) -> void override {
        keyboard_focused = surface;
        unwrap_pn_mut(window, find_focused_window(keyboard_focused));
        window.wl_on_keycode_enter(keys);
    }

    auto on_wl_keyboard_leave(wl_surface* /*surface*/) -> void override {
        unwrap_pn_mut(window, find_focused_window(keyboard_focused));
        window.wl_on_key_leave();
        keyboard_focused = nullptr;
    }

    auto on_wl_keyboard_key(const uint32_t key, const uint32_t state) -> void override {
        unwrap_pn_mut(window, find_focused_window(keyboard_focused));
        window.wl_on_keycode_input(key, state);
    }

    auto on_wl_keyboard_modifiers(uint32_t /*mods_depressed*/, uint32_t /*mods_latched*/, uint32_t /*mods_locked*/, uint32_t /*group*/) -> void override {}

    auto on_wl_keyboard_repeat_info(const int32_t rate, const int32_t delay) -> void override {
        keyboard_repeat_config->emplace(KeyRepeatConfig{1000 / rate, delay});
    }

    // PointerCallbacks
    auto on_wl_pointer_enter(wl_surface* const surface, const double x, const double y) -> void override {
        pointer_focused = surface;
        on_wl_pointer_motion(x, y);
    }

    auto on_wl_pointer_motion(const double x, const double y) -> void override {
        assert_n(pointer_focused);
        unwrap_pn_mut(window, find_focused_window(pointer_focused));
        window.wl_on_pointer_motion(x, y);
    }

    auto on_wl_pointer_leave(wl_surface* const /*surface*/) -> void override {
        pointer_focused = nullptr;
    }

    auto on_wl_pointer_button(const uint32_t button, const uint32_t state) -> void override {
        assert_n(pointer_focused);
        unwrap_pn_mut(window, find_focused_window(pointer_focused));
        window.wl_on_pointer_button(button, state);
    }

    auto on_wl_pointer_axis(const uint32_t axis, const double value) -> void override {
        assert_n(pointer_focused);
        unwrap_pn_mut(window, find_focused_window(pointer_focused));
        window.wl_on_pointer_axis(axis, value);
    }

    auto on_wl_pointer_frame() -> void override {}
    auto on_wl_pointer_axis_source(uint32_t /*source*/) -> void override {}
    auto on_wl_pointer_axis_stop(uint32_t /*axis*/) -> void override {}
    auto on_wl_pointer_axis_discrete(uint32_t /*axis*/, int32_t /*discrete*/) -> void override {}
    auto on_wl_pointer_axis_value120(uint32_t /*axis*/, int32_t /*value120*/) -> void override {}
    // TouchCallbacks
    auto on_wl_touch_down(wl_surface* const surface, const uint32_t id, const double x, const double y) -> void override {
        auto& focused = *get_touch_focused(id);
        focused       = surface;
        unwrap_pn_mut(window, find_focused_window(focused));
        window.wl_on_touch_down(id, x, y);
    }

    auto on_wl_touch_motion(const uint32_t id, const double x, const double y) -> void override {
        auto& focused = *get_touch_focused(id);
        assert_n(focused);
        unwrap_pn_mut(window, find_focused_window(focused));
        window.wl_on_touch_motion(id, x, y);
    }

    auto on_wl_touch_up(const uint32_t id) -> void override {
        auto& focused = *get_touch_focused(id);
        assert_n(focused);
        unwrap_pn_mut(window, find_focused_window(focused));
        window.wl_on_touch_up(id);
    }

    auto on_wl_touch_frame() -> void override {}

    // OutputCallbacks

    auto on_wl_output_scale(wl_output* const output, const uint32_t scale) -> void override {
        auto [lock, windows] = critical_windows->access();
        for(const auto& window : windows) {
            const auto wl_window = std::bit_cast<WaylandWindow*>(window.get());
            if(wl_window->wl_get_output() == output) {
                wl_window->wl_set_output_scale(scale);
                return;
            }
        }
    }

  public:
    auto set_keyboard_repeat_config(std::optional<KeyRepeatConfig>* keyboard_repeat_config) -> void {
        this->keyboard_repeat_config = keyboard_repeat_config;
    }
    WaylandCallbacks(Critical<Windows>* critical_windows)
        : critical_windows(critical_windows) {}
};

auto delete_wayland_callbacks(WaylandCallbacks* callbacks) -> void {
    delete callbacks;
}

auto WaylandClientObjects::create(Critical<Windows>* const critical_windows) -> std::unique_ptr<WaylandClientObjects> {
    auto       display      = towl::Display();
    const auto registry_ptr = display.get_registry();
    const auto callbacks    = new WaylandCallbacks(critical_windows);

    const auto wl = new WaylandClientObjects{
        .display            = std::move(display),
        .registry           = towl::Registry(registry_ptr),
        .callbacks          = AutoWaylandCallbacks(callbacks),
        .compositor_binder  = towl::CompositorBinder(4),
        .xdg_wm_base_binder = towl::XDGWMBaseBinder(2),
        .seat_binder        = towl::SeatBinder(4, callbacks, callbacks, callbacks),
        .output_binder      = towl::OutputBinder(2, callbacks),
    };

    wl->registry.set_binders({&wl->compositor_binder, &wl->xdg_wm_base_binder, &wl->seat_binder, &wl->output_binder});
    callbacks->set_keyboard_repeat_config(&wl->repeat_config);

    return std::unique_ptr<WaylandClientObjects>(wl);
}
} // namespace gawl::impl
