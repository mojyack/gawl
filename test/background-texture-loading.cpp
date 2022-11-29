#include <thread>

#include <gawl/wayland/gawl.hpp>

class Impl;

using Gawl = gawl::Gawl<Impl>;
class Impl {
  private:
    Gawl::Window<Impl>&            window;
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

    Impl(Gawl::Window<Impl>& window) : window(window) {
        worker = std::thread([this]() {
            auto context = this->window.fork_context();

            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                auto graphic = new gawl::Graphic(gawl::PixelBuffer::from_file("image.png").unwrap());
                context.flush();
                const auto lock = std::lock_guard(mutex);
                graphic1        = std::unique_ptr<gawl::Graphic>(graphic);
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                auto graphic = new gawl::Graphic(gawl::PixelBuffer::from_file("image.png").unwrap());
                context.flush();
                const auto lock = std::lock_guard(mutex);
                graphic2        = std::unique_ptr<gawl::Graphic>(graphic);
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                auto graphic = new gawl::Graphic(gawl::PixelBuffer::from_file("image.png").unwrap());
                context.flush();
                const auto lock = std::lock_guard(mutex);
                graphic3        = std::unique_ptr<gawl::Graphic>(graphic);
            }
        });
    }

    ~Impl() {
        worker.join();
    }
};

auto main() -> int {
    auto app = Gawl::Application();
    app.open_window<Impl>({.manual_refresh = false});
    app.run();
    return 0;
}
