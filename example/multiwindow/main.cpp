#include <gawl/wayland/gawl.hpp>

class Red;
class Green;
class Blue;

using Gawl = gawl::Gawl<Red, Green, Blue>;

template <class T, class N, bool red, bool green, bool blue>
class Color : public Gawl::Window<T> {
  protected:
    Gawl::Application& app;
    int                count = 0;
    const char* const  next_title;

  public:
    auto refresh_callback() -> void {
        count += 1;
        auto color = count % 60 < 30 ? (count % 60) / 30.0 : (30 - count % 30) / 30.0;
        gawl::clear_screen({red ? 1 : color, green ? 1 : color, blue ? 1 : color, 1});
    }
    auto keyboard_callback(const uint32_t key, const gawl::ButtonState state) -> void {
        if(state != gawl::ButtonState::Press) {
            return;
        }
        switch(key) {
        case KEY_N:
            app.open_window<N>(Gawl::WindowCreateHint{.title = next_title}, app);
            break;
        case KEY_M:
            this->close_window();
            break;
        case KEY_Q:
            this->quit_application();
            break;
        }
    }
    Color(Gawl::WindowCreateHint& hint, Gawl::Application& app, const char* const next_title) : Gawl::Window<T>(hint), app(app), next_title(next_title) {}
};

class Red : public Color<Red, Green, true, false, false> {
  public:
    Red(Gawl::WindowCreateHint& hint, Gawl::Application& app) : Color(hint, app, "Green") {}
};
class Green : public Color<Green, Blue, false, true, false> {
  public:
    Green(Gawl::WindowCreateHint& hint, Gawl::Application& app) : Color(hint, app, "Blue") {}
};
class Blue : public Color<Blue, Red, false, false, true> {
  public:
    Blue(Gawl::WindowCreateHint& hint, Gawl::Application& app) : Color(hint, app, "Red") {}
};

auto main(const int /*argc*/, const char* const /*argv*/[]) -> int {
    auto app = Gawl::Application();
    app.open_window<Red>(Gawl::WindowCreateHint{.title = "Red", .manual_refresh = false}, app);
    app.run();
    return 0;
}
