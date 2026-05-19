#include <coop/task-handle.hpp>
#include <coop/timer.hpp>

#include "../macros/unwrap.hpp"
#include "window.hpp"
#include "wl-object.hpp"

namespace gawl::impl {

class WaylandCallbacks : public towl::KeyboardCallbacks,
                         public towl::PointerCallbacks,
                         public towl::TouchCallbacks {
  private:
    Windows*                        windows;
    std::optional<KeyRepeatConfig>* keyboard_repeat_config;
    wl_surface*                     keyboard_focused = nullptr;
    wl_surface*                     pointer_focused  = nullptr;
    std::vector<wl_surface*>        touch_focused;

    auto find_focused_window(const wl_surface* const surface) -> WaylandWindow* {
        for(const auto& window : *windows) {
            const auto wl_window = std::bit_cast<WaylandWindow*>(window.get());
            if(wl_window->wayland_surface.native() == surface) {
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
        unwrap_mut(window, find_focused_window(keyboard_focused));
        for(const auto key : std::span{keys.data, keys.size}) {
            window.pending_callbacks.emplace_back(window.callbacks->on_keycode(key, gawl::ButtonState::Enter));
        }
    }

    auto on_wl_keyboard_leave(wl_surface* /*surface*/) -> void override {
        unwrap_mut(window, find_focused_window(keyboard_focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_keycode(-1, gawl::ButtonState::Leave));
        window.key_repeater.cancel();
        keyboard_focused = nullptr;
    }

    auto on_wl_keyboard_key(const uint32_t key, const uint32_t state) -> void override {
        unwrap_mut(window, find_focused_window(keyboard_focused));
        const auto key_state = state == WL_KEYBOARD_KEY_STATE_PRESSED ? ButtonState::Press : ButtonState::Release;
        window.pending_callbacks.emplace_back(window.callbacks->on_keycode(key, key_state));

        window.key_repeater.cancel();
        if(!window.wl->repeat_config || key_state != ButtonState::Press) {
            return;
        }
        window.runner->push_task([](WaylandWindow& self, uint32_t key) -> coop::Async<void> {
            co_await coop::sleep(std::chrono::milliseconds(self.wl->repeat_config->delay_in_milisec));
            const auto interval = std::chrono::milliseconds(self.wl->repeat_config->interval);
            while(true) {
                const auto begin = std::chrono::system_clock::now();
                co_await self.callbacks->on_keycode(key, ButtonState::Repeat);
                const auto elapsed = std::chrono::system_clock::now() - begin;
                if(elapsed < interval) {
                    co_await coop::sleep(interval - elapsed);
                }
            }
        }(window, key),
                                 &window.key_repeater);
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
        ensure(pointer_focused);
        unwrap_mut(window, find_focused_window(pointer_focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_pointer({x, y}));
    }

    auto on_wl_pointer_leave(wl_surface* const /*surface*/) -> void override {
        pointer_focused = nullptr;
    }

    auto on_wl_pointer_button(const uint32_t button, const uint32_t state) -> void override {
        ensure(pointer_focused);
        unwrap_mut(window, find_focused_window(pointer_focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_click(button, state == WL_POINTER_BUTTON_STATE_PRESSED ? ButtonState::Press : ButtonState::Release));
    }

    auto on_wl_pointer_axis(const uint32_t axis, const double value) -> void override {
        ensure(pointer_focused);
        unwrap_mut(window, find_focused_window(pointer_focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_scroll(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? WheelAxis::Horizontal : WheelAxis::Vertical, value));
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
        unwrap_mut(window, find_focused_window(focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_touch_down(id, {x, y}));
    }

    auto on_wl_touch_motion(const uint32_t id, const double x, const double y) -> void override {
        auto& focused = *get_touch_focused(id);
        ensure(focused);
        unwrap_mut(window, find_focused_window(focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_touch_motion(id, {x, y}));
    }

    auto on_wl_touch_up(const uint32_t id) -> void override {
        auto& focused = *get_touch_focused(id);
        ensure(focused);
        unwrap_mut(window, find_focused_window(focused));
        window.pending_callbacks.emplace_back(window.callbacks->on_touch_up(id));
    }

    auto on_wl_touch_frame() -> void override {}

  public:
    auto set_keyboard_repeat_config(std::optional<KeyRepeatConfig>* keyboard_repeat_config) -> void {
        this->keyboard_repeat_config = keyboard_repeat_config;
    }
    WaylandCallbacks(Windows& windows)
        : windows(&windows) {}
};

auto delete_wayland_callbacks(WaylandCallbacks* callbacks) -> void {
    delete callbacks;
}

auto WaylandClientObjects::create(Windows& windows, std::vector<towl::impl::InterfaceBinder*> binders) -> std::unique_ptr<WaylandClientObjects> {
    auto       display      = towl::Display();
    const auto registry_ptr = display.get_registry();
    const auto callbacks    = new WaylandCallbacks(windows);

    const auto wl = new WaylandClientObjects{
        .display            = std::move(display),
        .registry           = towl::Registry(registry_ptr),
        .callbacks          = AutoWaylandCallbacks(callbacks),
        .compositor_binder  = towl::CompositorBinder(6),
        .xdg_wm_base_binder = towl::XDGWMBaseBinder(2),
        .seat_binder        = towl::SeatBinder(4, callbacks, callbacks, callbacks),
        .repeat_config      = {},
    };

    binders.push_back(&wl->compositor_binder);
    binders.push_back(&wl->xdg_wm_base_binder);
    binders.push_back(&wl->seat_binder);
    wl->registry.set_binders(std::move(binders));
    callbacks->set_keyboard_repeat_config(&wl->repeat_config);

    return std::unique_ptr<WaylandClientObjects>(wl);
}
} // namespace gawl::impl
