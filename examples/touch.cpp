#include "gawl/misc.hpp"
#include "gawl/polygon.hpp"
#include "gawl/wayland/application.hpp"

struct Touch {
    gawl::Point pos;
    uint32_t    state = 0;
};

class Callbacks : public gawl::WindowCallbacks {
  private:
    std::vector<Touch> touchs;

    auto get_touch(const uint32_t id) -> Touch& {
        if(id >= touchs.size()) {
            touchs.resize(id + 1);
        }
        return touchs[id];
    }

  public:
    auto refresh() -> void override {
        const auto [screen_w, screen_h] = window->get_window_size();

        gawl::clear_screen({0, 0, 0, 1});
        for(auto i = 0u; i < touchs.size(); i += 1) {
            const auto& t = touchs[i];
            if(t.state == 0) {
                continue;
            }

            static const auto colors = std::array{
                gawl::Color{0, 0, 1, 1},
                gawl::Color{0, 1, 0, 1},
                gawl::Color{0, 1, 1, 1},
                gawl::Color{1, 0, 0, 1},
                gawl::Color{1, 0, 1, 1},
                gawl::Color{1, 1, 0, 1},
                gawl::Color{1, 1, 1, 1},
            };

            const auto& c      = colors[i % colors.size()];
            const auto  line_h = std::array{gawl::Point{0, t.pos.y}, gawl::Point{1. * screen_w, t.pos.y}};
            const auto  line_v = std::array{gawl::Point{t.pos.x, 0}, gawl::Point{t.pos.x, 1. * screen_h}};
            gawl::draw_lines(*window, line_h, c, 3);
            gawl::draw_lines(*window, line_v, c, 3);
            gawl::draw_rect(*window, {{t.pos.x - 8, t.pos.y - 8}, {t.pos.x + 8, t.pos.y + 8}}, c);
        }
    }

    auto close() -> void override {
        application->quit();
    }

    auto on_touch_down(const uint32_t id, const gawl::Point& pos) -> void override {
        get_touch(id) = {pos, 1};
    }

    auto on_touch_up(const uint32_t id) -> void override {
        get_touch(id) = {{}, 0};
    }

    auto on_touch_motion(const uint32_t id, const gawl::Point& pos) -> void override {
        get_touch(id) = {pos, 2};
    }
};

auto main() -> int {
    auto app = gawl::WaylandApplication();
    app.open_window({.manual_refresh = true}, std::shared_ptr<Callbacks>(new Callbacks()));
    app.run();
    return 0;
}
