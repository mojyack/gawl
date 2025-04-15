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

    auto on_created(gawl::Window* /*window*/) -> coop::Async<bool> override {
        constexpr auto error_value = false;
        co_unwrap_v(pixbuf, gawl::PixelBuffer::from_file("examples/image.png"));
        graphic = gawl::Graphic(pixbuf);
        co_return true;
    }
};

auto main() -> int {
    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<Callbacks>(new Callbacks());
    runner.push_task(app.run());
    runner.push_task(app.open_window({.manual_refresh = true}, std::move(cbs)));
    runner.run();
    return 0;
}
