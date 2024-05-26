#include "graphic-base.hpp"
#include "misc.hpp"

namespace gawl::impl {
auto GraphicBase::do_draw(Screen& screen) const -> void {
    const auto vabinder = shader->bind_vao();
    const auto ebbinder = shader->bind_ebo();
    const auto shbinder = shader->use_shader();
    const auto fbbinder = screen.prepare();
    const auto txbinder = bind_texture();
    shader->set_parameters(shbinder.get());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto GraphicBase::get_texture() const -> GLuint {
    return texture;
}

auto GraphicBase::bind_texture() const -> TextureBinder {
    return texture;
}

auto GraphicBase::release_texture() -> void {
    if(texture != 0) {
        glDeleteTextures(1, &texture);
    }
}

auto GraphicBase::get_width(const MetaScreen& screen) const -> int {
    return width / screen.get_scale();
}

auto GraphicBase::get_height(const MetaScreen& screen) const -> int {
    return height / screen.get_scale();
}

auto GraphicBase::draw(Screen& screen, const Point& point) -> void {
    draw_rect(screen, {{point.x, point.y}, {point.x + width, point.y + height}});
}

auto GraphicBase::draw_rect(Screen& screen, const Rectangle& rect) -> void {
    shader->move_vertices(screen, Rectangle(rect).magnify(screen.get_scale()), invert_top_bottom);
    do_draw(screen);
}

auto GraphicBase::draw_fit_rect(Screen& screen, const Rectangle& rect) -> void {
    draw_rect(screen, calc_fit_rect(rect, width, height));
}

auto GraphicBase::draw_transformed(Screen& screen, const std::array<Point, 4>& vertices) -> void {
    auto       v = vertices;
    const auto s = screen.get_scale();
    for(auto& p : v) {
        p.magnify(s);
    }
    shader->move_vertices(screen, v, invert_top_bottom);
    do_draw(screen);
}

auto GraphicBase::operator=(GraphicBase&& o) -> GraphicBase& {
    release_texture();
    shader            = o.shader;
    texture           = std::exchange(o.texture, 0);
    width             = o.width;
    height            = o.height;
    invert_top_bottom = o.invert_top_bottom;
    return *this;
}

GraphicBase::GraphicBase(GraphicShader& shader) : shader(&shader) {
    glGenTextures(1, &texture);
    const auto txbinder = bind_texture();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GraphicBase::GraphicBase(GraphicBase&& o) {
    *this = std::move(o);
}

GraphicBase::~GraphicBase() {
    release_texture();
}
} // namespace gawl::impl
