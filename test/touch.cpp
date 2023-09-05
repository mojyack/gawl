#define GAWL_MOUSE
#include <gawl/fc.hpp>
#include <gawl/textrender.hpp>
#include <gawl/wayland/gawl.hpp>
#include <gawl/polygon.hpp>

struct Touch {
    gawl::Point pos;
    uint32_t    state = 0;
};

class Impl {
  private:
    gawl::Window<Impl>& window;
    std::vector<Touch>  touchs;

    auto get_touch(const uint32_t id) -> Touch& {
        if(id >= touchs.size()) {
            touchs.resize(id + 1);
        }
        return touchs[id];
    }

  public:
    auto refresh_callback() -> void {
        const auto [screen_w, screen_h] = window.get_window_size();

        gawl::clear_screen({0, 0, 0, 1});
        for(auto i = 0u; i < touchs.size(); i += 1) {
            const auto& t = touchs[i];
            if(t.state == 0) {
                continue;
            }
            
            static const auto colors = std::array {
                gawl::Color{0,0,1,1},
                gawl::Color{0,1,0,1},
                gawl::Color{0,1,1,1},
                gawl::Color{1,0,0,1},
                gawl::Color{1,0,1,1},
                gawl::Color{1,1,0,1},
                gawl::Color{1,1,1,1},
            };

            const auto& c = colors[i % colors.size()];
            const auto line_h = std::array{gawl::Point{0, t.pos.y}, gawl::Point{1. * screen_w, t.pos.y}};
            const auto line_v = std::array{gawl::Point{t.pos.x, 0}, gawl::Point{t.pos.x, 1. * screen_h}};
            gawl::draw_lines(window, line_h, c, 3);
            gawl::draw_lines(window, line_v, c, 3);
            gawl::draw_rect(window, {{t.pos.x - 8, t.pos.y - 8}, {t.pos.x + 8, t.pos.y + 8}}, c);
        }
    }

    auto touch_down_callback(const uint32_t id, const gawl::Point pos) -> void {
        get_touch(id) = {pos, 1};
    }

    auto touch_up_callback(const uint32_t id) -> void {
        get_touch(id) = {{}, 0};
    }

    auto touch_motion_callback(const uint32_t id, const gawl::Point pos) -> void {
        get_touch(id) = {pos, 2};
    }

    Impl(gawl::Window<Impl>& window)
        : window(window) {}
};

auto main() -> int {
    auto app = gawl::Application();
    app.open_window<Impl>({.manual_refresh = false});
    app.run();
    return 0;
}
