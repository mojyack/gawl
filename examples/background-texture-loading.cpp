#include <thread>

#include "gawl/graphic.hpp"
#include "gawl/wayland/gawl.hpp"

class Impl {
  private:
    gawl::Window<Impl>&            window;
    std::mutex                     mutex;
    std::unique_ptr<gawl::Graphic> graphic1;
    std::unique_ptr<gawl::Graphic> graphic2;
    std::unique_ptr<gawl::Graphic> graphic3;

    std::thread worker;

  public:
    auto refresh_callback() -> void {
        gawl::clear_screen({0, 0, 0, 1});

        const auto lock = std::lock_guard(mutex);
        if(graphic1) {
            graphic1->draw(window, {170 * 0, 0});
        }
        if(graphic2) {
            graphic2->draw(window, {170 * 1, 0});
        }
        if(graphic3) {
            graphic3->draw(window, {170 * 2, 0});
        }
    }

    Impl(gawl::Window<Impl>& window) : window(window) {
        worker = std::thread([this]() {
            auto       context      = this->window.fork_context();
            const auto load_graphic = [this, &context](std::unique_ptr<gawl::Graphic>& storage) -> void {
                auto graphic = new gawl::Graphic(gawl::PixelBuffer::from_file("examples/image.png").unwrap());
                context.flush();
                const auto lock = std::lock_guard(mutex);
                storage         = std::unique_ptr<gawl::Graphic>(graphic);
            };

            std::this_thread::sleep_for(std::chrono::seconds(1));
            load_graphic(graphic1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            load_graphic(graphic2);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            load_graphic(graphic3);
        });
    }

    ~Impl() {
        worker.join();
    }
};

auto main() -> int {
    auto app = gawl::Application();
    app.open_window<Impl>({});
    app.run();
    return 0;
}
