#include <thread>

#include "gawl/graphic.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "macros/unwrap.hpp"
#include "util/assert.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    std::mutex    mutex;
    gawl::Graphic graphic1;
    gawl::Graphic graphic2;
    gawl::Graphic graphic3;

    std::thread worker;

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});

        const auto lock = std::lock_guard(mutex);
        if(graphic1) {
            graphic1.draw(*window, {170 * 0, 0});
        }
        if(graphic2) {
            graphic2.draw(*window, {170 * 1, 0});
        }
        if(graphic3) {
            graphic3.draw(*window, {170 * 2, 0});
        }
    }

    auto close() -> void override {
        application->quit();
    }

    Callbacks() {
        worker = std::thread([this]() {
            auto       context      = std::bit_cast<gawl::WaylandWindow*>(this->window)->fork_context();
            const auto load_graphic = [this, &context](gawl::Graphic& graphic) -> void {
                unwrap_on(pixbuf, gawl::PixelBuffer::from_file("examples/image.png"));
                auto new_graphic = gawl::Graphic(pixbuf);
                context.flush();
                const auto lock = std::lock_guard(mutex);
                graphic         = std::move(new_graphic);
            };

            std::this_thread::sleep_for(std::chrono::seconds(1));
            load_graphic(graphic1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            load_graphic(graphic2);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            load_graphic(graphic3);
        });
    }

    ~Callbacks() {
        worker.join();
    }
};

auto main() -> int {
    auto app = gawl::WaylandApplication();
    app.open_window({.manual_refresh = true}, std::shared_ptr<Callbacks>(new Callbacks()));
    app.run();
    return 0;
}
