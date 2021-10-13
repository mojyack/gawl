#include "gawl-window.hpp"
#include "gawl-application.hpp"
#include "graphic.hpp"
#include "polygon.hpp"
#include "textrender.hpp"

namespace gawl {
namespace internal {
GLObject* default_gl_object;
struct GlobalVar {
    std::shared_ptr<GLObject> graphic_shader;
    std::shared_ptr<GLObject> textrender_shader;
    std::shared_ptr<GLObject> polygon_shader;
    GlobalVar() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        graphic_shader.reset(create_graphic_globject());
        textrender_shader.reset(create_text_globject());
        polygon_shader.reset(create_polygon_globject());

        default_gl_object = graphic_shader.get();
    }
    ~GlobalVar() {}
};
static GlobalVar* global;
} // namespace internal
static int              global_count = 0;
static constexpr double MIN_SCALE    = 0.01;

auto GawlWindow::on_buffer_resize(const size_t width, const size_t height, const size_t scale) -> void {
    if(width != 0 || height != 0) {
        buffer_size.size = {width, height};
    }
    buffer_size.scale = scale;
    draw_scale        = specified_scale >= MIN_SCALE ? specified_scale : follow_buffer_scale ? buffer_size.scale
                                                                                             : 1;
    window_size[0]    = buffer_size.size[0] / draw_scale;
    window_size[1]    = buffer_size.size[1] / draw_scale;
    window_resize_callback();
    if(app.is_running()) {
        refresh();
    }
}
auto GawlWindow::is_running() const -> bool {
    return status == Status::READY;
}
auto GawlWindow::init_global() -> void {
    if(global_count == 0) {
        internal::global = new internal::GlobalVar();
    }
    global_count += 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}
auto GawlWindow::init_complete() -> void {
    status = Status::READY;
}
auto GawlWindow::get_window_size() const -> const std::array<int, 2>& {
    return window_size;
}
auto GawlWindow::set_follow_buffer_scale(const bool flag) -> void {
    if(flag == follow_buffer_scale) {
        return;
    }
    follow_buffer_scale = flag;
    on_buffer_resize(0, 0, buffer_size.scale);
}
auto GawlWindow::get_follow_buffer_scale() const -> bool {
    return follow_buffer_scale;
}
auto GawlWindow::set_scale(const double scale) -> void {
    specified_scale = scale;
    on_buffer_resize(0, 0, buffer_size.scale);
}
auto GawlWindow::set_event_driven(const bool flag) -> void {
    if(event_driven == flag) {
        return;
    }
    if(event_driven) {
        event_driven = false;
    } else {
        event_driven = true;
        if(app.is_running()) {
            refresh();
        }
    }
}
auto GawlWindow::get_event_driven() const -> bool {
    return event_driven;
}
auto GawlWindow::close_request_callback() -> void {
    quit_application();
}
auto GawlWindow::get_scale() const -> double {
    return draw_scale;
}
auto GawlWindow::get_size() const -> std::array<std::size_t, 2> {
    return buffer_size.size;
}
auto GawlWindow::is_close_pending() const -> bool {
    return status == Status::CLOSE;
}
auto GawlWindow::close_window() -> void {
    app.close_window(this);
}
auto GawlWindow::quit_application() -> void {
    app.quit();
}
GawlWindow::GawlWindow(GawlApplication& app) : app(app) {}
GawlWindow::~GawlWindow() {
    if(global_count == 1) {
        delete internal::global;
    }
    global_count -= 1;
}
} // namespace gawl
