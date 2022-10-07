#include <bitset>
#include <iostream>

#include <gawl/wayland/gawl.hpp>

class Keycode;

using GawlKeycode = gawl::Gawl<Keycode>;

class Keycode {
  public:
    auto keycode_callback(const uint32_t keycode, const gawl::ButtonState state) -> void {
        std::cout << keycode << " " << static_cast<int>(state) << std::endl;
    }
    Keycode(GawlKeycode::Window<Keycode>& /*window*/) {}
};

class Keysym;

using GawlKeysym = gawl::Gawl<Keysym>;

enum class Modifiers {
    None    = 0,
    Shift   = 1 << 0,
    Lock    = 1 << 1,
    Control = 1 << 2,
    Mod1    = 1 << 3,
    Mod2    = 1 << 4,
    Mod4    = 1 << 5,
};

constexpr auto operator|(const Modifiers a, const Modifiers b) -> Modifiers {
    return static_cast<Modifiers>(static_cast<int>(a) | static_cast<int>(b));
}

class Keysym {
  private:
    auto calc_modifiers(xkb_state* const state) const -> Modifiers {
        return (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, xkb_state_component(1)) ? Modifiers::Shift : Modifiers::None) |
               (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CAPS, xkb_state_component(1)) ? Modifiers::Lock : Modifiers::None) |
               (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL, xkb_state_component(1)) ? Modifiers::Control : Modifiers::None) |
               (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT, xkb_state_component(1)) ? Modifiers::Mod1 : Modifiers::None) |
               (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_NUM, xkb_state_component(1)) ? Modifiers::Mod2 : Modifiers::None) |
               (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_LOGO, xkb_state_component(1)) ? Modifiers::Mod4 : Modifiers::None);
        return Modifiers::None;
    }

  public:
    auto keysym_callback(const xkb_keycode_t keycode, const gawl::ButtonState state, xkb_state* const xkb_state) -> void {
        if(state == gawl::ButtonState::Leave) {
            return;
        }

        const auto keysym = xkb_state_key_get_one_sym(xkb_state, keycode);
        static auto buffer = std::array<char, 128>();
        xkb_keysym_get_name(keysym, buffer.data(), buffer.size());
        std::cout << buffer.data() << "(" << keysym << ")"
                  << " " << static_cast<int>(state) << " " << std::bitset<8>(static_cast<int>(calc_modifiers(xkb_state))) << std::endl;
    }
    Keysym(GawlKeysym::Window<Keysym>& /*window*/) {}
};

auto main() -> int {
    {
        std::cout << "keycode:" << std::endl;
        auto app = GawlKeycode::Application();
        app.open_window<Keycode>({.manual_refresh = false});
        app.run();
    }
    {
        std::cout << "keysym:" << std::endl;
        auto app = GawlKeysym::Application();
        app.open_window<Keysym>({.manual_refresh = false});
        app.run();
    }
    return 0;
}
