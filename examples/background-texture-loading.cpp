#include <coop/thread.hpp>
#include <coop/timer.hpp>

#include "gawl/graphic.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "macros/unwrap.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    coop::Runner*                runner;
    std::array<gawl::Graphic, 3> graphics;
    coop::TaskHandle             worker;

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
        for(auto i = 0uz; i < graphics.size(); i += 1) {
            if(!graphics[i]) {
                break;
            }
            graphics[i].draw(*window, {170.0 * i, 0});
        }
    }

    auto close() -> void override {
        application->quit();
    }

    auto on_created(gawl::Window* /*window*/) -> coop::Async<bool> override {
        runner->push_task(worker_main(), &worker);
        co_return true;
    }

    auto worker_main() -> coop::Async<void> {
        const auto loader = [this](const char* const path) -> std::optional<gawl::Graphic> {
            constexpr auto error_value = std::nullopt;

            auto context = std::bit_cast<gawl::WaylandWindow*>(window)->fork_context();
            unwrap_v(pixbuf, gawl::PixelBuffer::from_file(path));
            auto graphic = gawl::Graphic(pixbuf);
            context.wait();
            return graphic;
        };

        for(auto& graphic : graphics) {
            co_await coop::sleep(std::chrono::seconds(1));
            co_unwrap_v_mut(result, co_await coop::run_blocking(loader, "examples/image.png"));
            graphic = std::move(result);
            window->refresh();
        }
    }

    Callbacks(coop::Runner& runner)
        : runner(&runner) {
    }

    ~Callbacks() {
        worker.cancel();
    }
};

auto main() -> int {
    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<Callbacks>(new Callbacks(runner));
    runner.push_task(app.run());
    runner.push_task(app.open_window({.manual_refresh = true}, std::move(cbs)));
    runner.run();
    return 0;
}
