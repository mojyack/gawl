#include "gawl/graphic.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "macros/unwrap.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    gawl::Graphic graphic;

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
        graphic.draw(*window, {0, 0});
        graphic.draw_fit_rect(*window, {{0, 170}, {170, 340}});
        graphic.draw_rect(*window, {{170, 0}, {340, 340}});
        graphic.draw_transformed(*window, {{{510, 0}, {680, 170}, {510, 340}, {340, 170}}});
        graphic.draw_fit_rect(*window, {{680, 0}, {1020, 340}});
    }

    auto close() -> void override {
        application->quit();
    }

    auto init() -> bool {
        unwrap(pixbuf, gawl::PixelBuffer::from_file("examples/image.png"));
        graphic.update_texture(pixbuf);
        return true;
    }
};

auto main() -> int {
    auto app = gawl::WaylandApplication();
    auto cbs = std::shared_ptr<Callbacks>(new Callbacks());
    ensure(cbs->init());
    app.open_window({.manual_refresh = true}, std::move(cbs));
    app.run();
    return 0;
}
