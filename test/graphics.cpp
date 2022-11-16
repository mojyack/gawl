#include <gawl/wayland/gawl.hpp>

class Impl;

using Gawl = gawl::Gawl<Impl>;

class Impl {
  private:
    Gawl::Window<Impl>& window;
    gawl::Graphic       graphic;

  public:
    auto refresh_callback() -> void {
        gawl::clear_screen({0, 0, 0, 1});
        graphic.draw(window, {0, 0});
        graphic.draw_fit_rect(window, {{0, 170}, {170, 340}});
        graphic.draw_rect(window, {{170, 0}, {340, 340}});
        graphic.draw_transformed(window, {{{510, 0}, {680, 170}, {510, 340}, {340, 170}}});
        graphic.draw_fit_rect(window, {{680, 0}, {1020, 340}});
    }
    Impl(Gawl::Window<Impl>& window) : window(window), graphic(gawl::PixelBuffer::from_file("image.png").unwrap()) {}
};

auto main() -> int {
    auto app = Gawl::Application();
    app.open_window<Impl>({.manual_refresh = false});
    app.run();
    return 0;
}
