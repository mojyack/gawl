#define GAWL_KEYCODE
#include <gawl/wayland/gawl.hpp>

template <class T, class U, bool red, bool green, bool blue>
class Color {
  protected:
    gawl::Window<T>&   window;
    gawl::Application& app;
    int                    count = 0;

  public:
    auto refresh_callback() -> void {
        auto color = count % 60 < 30 ? (count % 60) / 30.0 : (30 - count % 30) / 30.0;
        gawl::clear_screen({red ? 1 : color, green ? 1 : color, blue ? 1 : color, 1});
        count += 1;
    }

    auto keycode_callback(const uint32_t key, const gawl::ButtonState state) -> void {
        if(state != gawl::ButtonState::Press) {
            return;
        }
        switch(key) {
        case KEY_N:
            app.open_window<U>({}, app);
            break;
        case KEY_M:
            window.close_window();
            break;
        case KEY_Q:
            window.quit_application();
            break;
        }
    }

    Color(gawl::Window<T>& window, gawl::Application& app)
        : window(window),
          app(app) {}
};

class Red;
class Green;
class Blue;

class Red : public Color<Red, Green, true, false, false> {
  public:
    Red(gawl::Window<Red>& window, gawl::Application& app) : Color(window, app) {}
};

class Green : public Color<Green, Blue, false, true, false> {
  public:
    Green(gawl::Window<Green>& window, gawl::Application& app) : Color(window, app) {}
};

class Blue : public Color<Blue, Red, false, false, true> {
  public:
    Blue(gawl::Window<Blue>& window, gawl::Application& app) : Color(window, app) {}
};

auto main() -> int {
    auto app = gawl::Application();
    app.open_window<Red>({}, app);
    app.run();
    return 0;
}
