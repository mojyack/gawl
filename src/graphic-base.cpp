#include "graphic-base.hpp"
#include "misc.hpp"

namespace gawl::internal {
auto GraphicBase::move_vertices(const Screen* const screen, const Rectangle& rect, const bool invert) -> void {
    auto r = rect;
    gawl::convert_screen_to_viewport(screen, r);
    vertices[0][0] = r.a.x;
    vertices[0][1] = invert ? r.b.y : r.a.y;
    vertices[1][0] = r.b.x;
    vertices[1][1] = invert ? r.b.y : r.a.y;
    vertices[2][0] = r.b.x;
    vertices[2][1] = invert ? r.a.y : r.b.y;
    vertices[3][0] = r.a.x;
    vertices[3][1] = invert ? r.a.y : r.b.y;
    auto vbbinder  = gl->bind_vbo();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}
auto GraphicBase::move_vertices(const Screen* const screen, const std::array<Point, 4>& points, const bool invert) -> void {
    auto v = points;
    gawl::convert_screen_to_viewport(screen, v);
    vertices[0][0] = v[0].x;
    vertices[0][1] = invert ? v[3].y : v[0].y;
    vertices[1][0] = v[1].x;
    vertices[1][1] = invert ? v[2].y : v[1].y;
    vertices[2][0] = v[2].x;
    vertices[2][1] = invert ? v[1].y : v[2].y;
    vertices[3][0] = v[3].x;
    vertices[3][1] = invert ? v[0].y : v[3].y;
    auto vbbinder  = gl->bind_vbo();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}
auto GraphicBase::do_draw(Screen* const screen) const -> void {
    const auto vabinder = gl->bind_vao();
    const auto ebbinder = gl->bind_ebo();
    const auto shbinder = gl->use_shader();
    const auto fbbinder = screen->prepare();
    const auto txbinder = bind_texture();
    gl->set_shader_parameters(shbinder.get());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
auto GraphicBase::get_texture() const -> GLuint {
    return texture;
}
auto GraphicBase::bind_texture() const -> TextureBinder {
    return texture;
}
auto GraphicBase::get_width(const Screen* const screen) const -> int {
    return width / screen->get_scale();
}
auto GraphicBase::get_height(const Screen* const screen) const -> int {
    return height / screen->get_scale();
}
auto GraphicBase::draw(Screen* const screen, const Point& point) -> void {
    draw_rect(screen, {{point.x, point.y}, {point.x + width, point.y + height}});
}
auto GraphicBase::draw_rect(Screen* const screen, const Rectangle& rect) -> void {
    auto r = rect;
    r.magnify(screen->get_scale());
    move_vertices(screen, r, invert_top_bottom);
    do_draw(screen);
}
auto GraphicBase::draw_fit_rect(Screen* const screen, const Rectangle& rect) -> void {
    draw_rect(screen, calc_fit_rect(rect, width, height));
}
auto GraphicBase::draw_transformed(Screen* const screen, const std::array<Point, 4>& vertices) -> void {
    auto       v = vertices;
    const auto s = screen->get_scale();
    for(auto& p : v) {
        p.magnify(s);
    }
    move_vertices(screen, v, invert_top_bottom);
    do_draw(screen);
}
GraphicBase::GraphicBase(internal::GLObject* gl) : gl(gl) {
    auto vbbinder = gl->bind_vbo();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);

    auto ebbinder = gl->bind_ebo();

    const static GLuint elements[] = {
        0, 1, 2,
        2, 3, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    vertices[0][2] = 0.0;
    vertices[0][3] = 0.0;
    vertices[1][2] = 1.0;
    vertices[1][3] = 0.0;
    vertices[2][2] = 1.0;
    vertices[2][3] = 1.0;
    vertices[3][2] = 0.0;
    vertices[3][3] = 1.0;

    glGenTextures(1, &texture);
    const auto txbinder = bind_texture();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}
GraphicBase::~GraphicBase() {
    glDeleteTextures(1, &texture);
}
} // namespace gawl::internal
