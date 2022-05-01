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

class Keysym {
  public:
    auto keysym_callback(const xkb_keysym_t keysym, const gawl::ButtonState state, const gawl::ModifierFlags modifiers) -> void {
        static auto buffer = std::array<char, 128>();
        xkb_keysym_get_name(keysym, buffer.data(), buffer.size());
        std::cout << buffer.data() << "(" << keysym << ")" << " " << static_cast<int>(state) << " " << std::bitset<8>(static_cast<int>(modifiers)) << std::endl;
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
