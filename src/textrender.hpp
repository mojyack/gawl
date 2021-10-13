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
    std::shared_ptr<internal::TextRenderPrivate> data;

    internal::Character* get_chara_graphic(int size, char32_t c);

  public:
    using DrawFunc = std::function<bool(size_t, const Rectangle&, internal::GraphicBase&)>;

    auto draw(Screen* screen, const Point& point, const Color& color, const char* text, DrawFunc func = nullptr) -> Rectangle;
    auto draw(Screen* screen, const Point& point, const Color& color, const char32_t* text, DrawFunc func = nullptr) -> Rectangle;
    auto draw_fit_rect(Screen* screen, const Rectangle& rect, Color const& color, const char* text, Align alignx = Align::center, Align aligny = Align::center, DrawFunc func = nullptr) -> Rectangle;
    auto set_char_color(const Color& color) -> void;
    auto get_rect(const Screen* screen, const Point& base, const char* text) -> Rectangle;
    auto get_rect(const Screen* screen, const Point& base, const char32_t* text) -> Rectangle;
    auto draw_wrapped(Screen* screen, const Rectangle& rect, int line_spacing, const Color& color, const char* text, Align alignx = Align::center, Align aligny = Align::center) -> void;
         operator bool() const;
    TextRender(const std::vector<const char*>& font_names, int size);
    TextRender();
    ~TextRender();
};
} // namespace gawl
