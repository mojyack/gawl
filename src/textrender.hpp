#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "screen.hpp"
#include "graphic-base.hpp"
#include "type.hpp"

namespace gawl {
class TextRenderPrivate;
class Character;
class TextRender {
  private:
    std::shared_ptr<TextRenderPrivate> data;

    Character* get_chara_graphic(int size, char32_t c);

  public:
    using DrawFunc = std::function<bool(size_t, Area const&, GraphicBase&)>;
    static auto set_char_color(Color const& color) -> void;

    auto draw(Screen* screen, double x, double y, Color const& color, const char* text, DrawFunc func = nullptr) -> Area;
    auto draw(Screen* screen, double x, double y, Color const& color, const char32_t* text, DrawFunc func = nullptr) -> Area;
    auto draw_fit_rect(Screen* screen, Area rect, Color const& color, const char* text, Align alignx = Align::center, Align aligny = Align::center, DrawFunc func = nullptr) -> Area;
    auto get_rect(const Screen* screen, Area& rect, const char* text) -> void;
    auto get_rect(const Screen* screen, Area& rect, const char32_t* text) -> void;
    auto draw_wrapped(Screen* screen, Area& rect, int line_spacing, const Color& color, const char* text, Align alignx = Align::center, Align aligny = Align::center) -> void;
         operator bool() const;
    TextRender(const std::vector<const char*>&& font_names, int size);
    TextRender();
    ~TextRender();
};
} // namespace gawl
