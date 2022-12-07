#include <gawl/wayland/gawl.hpp>

class Red;
class Green;
class Blue;

using Gawl = gawl::Gawl<Red, Green, Blue>;

auto open_window(void* app, int number) -> void;

template <class T, int next_window, bool red, bool green, bool blue>
class Color {
  protected:
    Gawl::Window<T>& window;
    void*            app; // Gawl::Application is incomplete type here
    int              count = 0;

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
            open_window(app, next_window);
            break;
        case KEY_M:
            window.close_window();
            break;
        case KEY_Q:
            window.quit_application();
            break;
        }
    }

    Color(Gawl::Window<T>& window, void* const app) : window(window), app(app) {}
};

class Red : public Color<Red, 2, true, false, false> {
  public:
    Red(Gawl::Window<Red>& window, void* const app) : Color(window, app) {}
};

class Green : public Color<Green, 3, false, true, false> {
  public:
    Green(Gawl::Window<Green>& window, void* const app) : Color(window, app) {}
};

class Blue : public Color<Blue, 1, false, false, true> {
  public:
    Blue(Gawl::Window<Blue>& window, void* const app) : Color(window, app) {}
};

static_assert(gawl::concepts::WindowImplWithKeycodeCallback<Red>);

auto open_window(void* const app, const int number) -> void {
    auto& a = *std::bit_cast<Gawl::Application*>(app);
    switch(number) {
    case 1:
        a.open_window<Red>({.title = "Red"}, app);
        break;
    case 2:
        a.open_window<Green>({.title = "Green"}, app);
        break;
    case 3:
        a.open_window<Blue>({.title = "Blue"}, app);
        break;
    }
}

auto main() -> int {
    auto app = Gawl::Application();
    open_window(&app, 1);
    app.run();
    return 0;
}
