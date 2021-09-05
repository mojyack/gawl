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

#include "frame-buffer-info.hpp"
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
    static void set_char_color(Color const& color);

    Area draw(FrameBufferInfo info, double x, double y, Color const& color, const char* text, DrawFunc func = nullptr);
    Area draw(FrameBufferInfo info, double x, double y, Color const& color, const char32_t* text, DrawFunc func = nullptr);
    Area draw_fit_rect(FrameBufferInfo info, Area rect, Color const& color, const char* text, Align alignx = Align::center, Align aligny = Align::center, DrawFunc func = nullptr);
    void get_rect(FrameBufferInfo info, Area& rect, const char* text);
    void get_rect(FrameBufferInfo info, Area& rect, const char32_t* text);
    void draw_wrapped(FrameBufferInfo info, Area& rect, int line_spacing, Color const& color, const char* text, Align alignx = Align::center, Align aligny = Align::center);
         operator bool() const;
    TextRender(const std::vector<const char*>&& font_names, int size);
    TextRender();
    ~TextRender();
};
} // namespace gawl
