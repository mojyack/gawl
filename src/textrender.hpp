#pragma once
#include <functional>

#include "graphic-base.hpp"

namespace gawl {
namespace internal {
class TextRenderPrivate;
class Character;
auto create_text_globject() -> GLObject*;
} // namespace internal
class TextRender {
  private:
    int                                          default_size;
    std::shared_ptr<internal::TextRenderPrivate> data;

    internal::Character* get_chara_graphic(int size, char32_t c);

  public:
    using DrawFunc = std::function<bool(size_t, const Rectangle&, internal::GraphicBase&)>;

    auto set_char_color(const Color& color) -> void;
    auto get_rect(const Screen* screen, const Point& base, const char* text, int size = 0) -> Rectangle;
    auto get_rect(const Screen* screen, const Point& base, const char32_t* text, int size = 0) -> Rectangle;

    // basic draw
    struct DrawArgs {
        int      size = 0;
        DrawFunc func = nullptr;
    };

    const static DrawArgs default_draw_args;

    auto draw(Screen* screen, const Point& point, const Color& color, const char* text, const DrawArgs& args = default_draw_args) -> Rectangle;
    auto draw(Screen* screen, const Point& point, const Color& color, const char32_t* text, const DrawArgs& args = DrawArgs{0, nullptr}) -> Rectangle;

    // draw fit rect
    struct DrawFitRectArgs {
        Align    alignx = Align::center;
        Align    aligny = Align::center;
        int      size   = 0;
        DrawFunc func   = nullptr;
    };

    const static DrawFitRectArgs default_draw_fit_rect_args;

    auto draw_fit_rect(Screen* screen, const Rectangle& rect, Color const& color, const char* text, const DrawFitRectArgs& args = default_draw_fit_rect_args) -> Rectangle;

    // draw wrapped
    struct DrawWrappedArgs {
        Align alignx = Align::center;
        Align aligny = Align::center;
        int   size   = 0;
    };

    const static DrawWrappedArgs default_draw_wrapped_args;

    auto draw_wrapped(Screen* screen, const Rectangle& rect, double line_spacing, const Color& color, const char* text, const DrawWrappedArgs& args = default_draw_wrapped_args) -> void;

         operator bool() const;
    auto operator=(const TextRender& o) noexcept -> TextRender&;
    TextRender() {}
    TextRender(const std::vector<const char*>& font_names, int default_size);
    TextRender(const TextRender& o);
    ~TextRender();
};
} // namespace gawl
