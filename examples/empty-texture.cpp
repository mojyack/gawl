#include "gawl/empty-texture.hpp"
#include "gawl/graphic.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "macros/unwrap.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    gawl::EmptyTexture texture1;
    gawl::EmptyTexture texture2;
    gawl::EmptyTexture texture3;
    gawl::Graphic      graphic;

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
        graphic.draw_rect(texture1, {{0, 0}, {32, 32}});
        graphic.draw_rect(texture2, {{0, 0}, {64, 64}});
        graphic.draw_rect(texture3, {{0, 0}, {128, 128}});
        texture1.draw_rect(*window, {{0, 0}, {128, 128}});
        texture2.draw_rect(*window, {{128, 0}, {256, 128}});
        texture3.draw_rect(*window, {{256, 0}, {384, 128}});
    }

    auto close() -> void override {
        application->quit();
    }

    auto on_created(gawl::Window* /*window*/) -> coop::Async<bool> override {
        constexpr auto error_value = false;
        co_unwrap_v(pixbuf, gawl::PixelBuffer::from_file("examples/image.png"));
        graphic = gawl::Graphic(pixbuf, std::array{50, 50, 120, 100});
        co_return true;
    }

    Callbacks()
        : texture1(32, 32),
          texture2(64, 64),
          texture3(128, 128) {}
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
