#pragma once
#include "graphic-shader.hpp"
#include "screen.hpp"

namespace gawl::impl {
class GraphicBase {
  private:
    GraphicShader* shader;
    GLuint         texture = 0;

    auto do_draw(Screen& screen) const -> void;

  protected:
    int  width;
    int  height;
    bool invert_top_bottom = false;

    auto get_texture() const -> GLuint;
    auto bind_texture() const -> TextureBinder;
    auto release_texture() -> void;

  public:
    auto get_width(const MetaScreen& screen) const -> int;
    auto get_height(const MetaScreen& screen) const -> int;
    auto draw(Screen& screen, const Point& point) -> void;
    auto draw_rect(Screen& screen, const Rectangle& rect) -> void;
    auto draw_fit_rect(Screen& screen, const Rectangle& rect) -> void;
    auto draw_transformed(Screen& screen, const std::array<Point, 4>& vertices) -> void;

    operator bool() const;
    auto operator=(GraphicBase&& o) -> GraphicBase&;

    GraphicBase() {};
    GraphicBase(GraphicShader& shader);
    GraphicBase(GraphicBase&& o);
    ~GraphicBase();
};
} // namespace gawl::impl
