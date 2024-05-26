#include "gawl/polygon.hpp"
#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    int count = 0;

  public:
    auto refresh() -> void override {
        gawl::clear_screen({0, 0, 0, 1});
        gawl::mask_alpha();
        const auto& size  = window->get_window_size();
        const auto  min   = std::min(size[0], size[1]);
        auto        color = count % 120 < 60 ? (count % 120) / 60.0 : (60 - count % 60) / 60.0;
        gawl::draw_polygon(*window, std::vector<gawl::Point>{{10.0, 10.0}, {10.0, 10.0 + min / 10.0}, {10.0 + min / 10.0, 10.0}}, {1, 1, 1, color});
        gawl::draw_polygon(*window, std::vector<gawl::Point>{{size[0] - 10.0, size[1] - 10.0}, {size[0] - 10.0, size[1] - 10.0 - min / 10.0}, {size[0] - 10.0 - min / 10.0, size[1] - 10.0}}, {1, 1, 1, color});
        gawl::draw_polygon_fan(*window, gawl::triangulate_circle_angle({size[0] - 10.0, 10.0}, min / 10.0 + color * 5, {0.25, 0.25}), {1, 1, 1, 1});
        gawl::draw_lines(*window, std::vector<gawl::Point>{{10.0, size[1] - 10.0 - min / 10.0}, {10.0 + min / 10.0, size[1] - 10.0}}, {1, 1, 1, 1}, 3);
        gawl::draw_rect(*window, {{size[0] / 2.0 - 100, size[1] / 2.0 - 200}, {size[0] / 2.0 + 100, size[1] / 2.0 + 200}}, {1, 1, 1, 1});

        gawl::mask_alpha();
        gawl::draw_outlines(*window, gawl::triangulate_circle({size[0] / 2.0, size[1] / 2.0 - 80}, 30), {0, 0, 0, 0.5}, 3);
        gawl::unmask_alpha();

        gawl::draw_outlines(*window, gawl::triangulate_circle({size[0] / 2.0, size[1] / 2.0 + 80}, 30), {0, 0, 0, 0.1}, 3);

        count += 1;
    }
};

auto main() -> int {
    auto app = gawl::WaylandApplication();
    app.open_window({}, new Callbacks());
    app.run();
    return 0;
}
