#pragma once
#include "internal-type.hpp"
#include "screen.hpp"

namespace gawl::internal {
template <class GL>
requires concepts::GraphicGLObject<GL>
class GraphicBase {
  private:
    GL&    gl;
    GLuint texture;

    auto do_draw(gawl::concepts::Screen auto& screen) const -> void {
        const auto vabinder = gl.bind_vao();
        const auto ebbinder = gl.bind_ebo();
        const auto shbinder = gl.use_shader();
        const auto fbbinder = screen.prepare();
        const auto txbinder = bind_texture();
        if constexpr(concepts::GLObjectWithParameter<GL>) {
            gl.set_shader_parameters(shbinder.get());
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

  protected:
    int  width, height;
    bool invert_top_bottom = false;
    auto get_texture() const -> GLuint {
        return texture;
    }
    auto bind_texture() const -> TextureBinder {
        return texture;
    }

  public:
    auto get_width(const gawl::concepts::MetaScreen auto& screen) const -> int {
        return width / screen.get_scale();
    }
    auto get_height(const gawl::concepts::MetaScreen auto& screen) const -> int {
        return height / screen.get_scale();
    }
    auto draw(gawl::concepts::Screen auto& screen, const Point& point) -> void {
        draw_rect(screen, {{point.x, point.y}, {point.x + width, point.y + height}});
    }
    auto draw_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        auto r = rect;
        r.magnify(screen.get_scale());
        gl.move_vertices(screen, r, invert_top_bottom);
        do_draw(screen);
    }
    auto draw_fit_rect(gawl::concepts::Screen auto& screen, const Rectangle& rect) -> void {
        draw_rect(screen, calc_fit_rect(rect, width, height));
    }
    auto draw_transformed(gawl::concepts::Screen auto& screen, const std::array<Point, 4>& vertices) -> void {
        auto       v = vertices;
        const auto s = screen.get_scale();
        for(auto& p : v) {
            p.magnify(s);
        }
        gl.move_vertices(screen, v, invert_top_bottom);
        do_draw(screen);
    }
    GraphicBase(GL& gl) : gl(gl) {
        glGenTextures(1, &texture);
        const auto txbinder = bind_texture();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    ~GraphicBase() {
        glDeleteTextures(1, &texture);
    }
};
} // namespace gawl::internal
