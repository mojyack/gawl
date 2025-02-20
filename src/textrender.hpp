#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "align.hpp"
#include "color.hpp"
#include "graphic-base.hpp"

#include <freetype/freetype.h>

namespace gawl {
namespace impl {
class Character : public impl::GraphicBase {
  public:
    int left;
    int top;
    int advance_x;
    int advance_y;

    Character(char32_t code, const std::vector<FT_Face>& faces);
};

class CharacterCache {
  public:
    std::vector<FT_Face>                    faces;
    std::unordered_map<char32_t, Character> cache;

    auto get_character(char32_t c) -> Character&;

    CharacterCache(const std::vector<std::string>& font_names, int size);
    CharacterCache(CharacterCache&& o) = default;
    ~CharacterCache();
};

auto convert_utf8_to_unicode32(std::string_view utf8) -> std::u32string;
} // namespace impl

class WrappedText {
  private:
    double                      width        = 0;
    double                      screen_scale = 0;
    std::vector<std::u32string> lines;

  public:
    auto is_changed(double width, double screen_scale) const -> bool;
    auto reset() -> void;
    auto get_lines() const -> const std::vector<std::u32string>&;

    WrappedText() {}
    WrappedText(double width, double screen_scale, const std::vector<std::u32string> lines);
};

using Callback = std::function<bool(size_t, const gawl::Rectangle&, impl::Character&)>;

struct DrawParams {
    int      size     = 0;
    bool     dry      = false;
    Callback callback = nullptr;
};

struct DrawFitRectParams {
    int         size     = 0;
    gawl::Align align_x  = gawl::Align::Center;
    gawl::Align align_y  = gawl::Align::Center;
    Callback    callback = nullptr;
};

struct DrawWrappedParams {
    int         size    = 0;
    gawl::Align align_x = gawl::Align::Center;
    gawl::Align align_y = gawl::Align::Center;
};

struct GlyphMeta {
    double left;
    double top;
    double advance_x;
    double advance_y;
};

class TextRender {
  private:
    std::unordered_map<int, impl::CharacterCache> caches;
    std::vector<std::string>                      font_names;
    int                                           default_size;

    auto clear() -> void;
    auto get_chara(int size) -> impl::CharacterCache&;
    auto get_chara_graphic(int size, char32_t chara) -> impl::Character&;
    auto create_wrapped_text(const MetaScreen& screen, double width, std::string_view text, int size) -> WrappedText;

  public:
    auto init(std::vector<std::string> font_names, int default_size) -> void;
    auto get_default_size() const -> int;
    auto set_char_color(const Color& color) -> void;
    auto get_rect(const MetaScreen& screen, std::string_view text, int size = 0) -> Rectangle;
    auto get_rect(const MetaScreen& screen, std::u32string_view text, int size = 0) -> Rectangle;
    auto get_glyph_meta(const MetaScreen& screen, char character, int size = 0) -> GlyphMeta;
    auto draw(Screen& screen, const Point& point, const Color& color, std::string_view text, const DrawParams& params = {}) -> Rectangle;
    auto draw(Screen& screen, const Point& point, const Color& color, std::u32string_view text, const DrawParams& params = {}) -> Rectangle;
    auto draw_fit_rect(Screen& screen, const Rectangle& rect, const Color& color, std::string_view text, const DrawFitRectParams& params = {}) -> Rectangle;
    auto calc_wrapped_text_height(Screen& screen, double width, double line_height, std::string_view text, WrappedText& wrapped_text, int size = 0) -> double;
    auto draw_wrapped(Screen& screen, const Rectangle& rect, double line_height, const Color& color, std::string_view text, WrappedText& wrapped_text, const DrawWrappedParams& params = {}) -> void;

    TextRender() {}
    TextRender(std::vector<std::string> font_names, int default_size);
};
} // namespace gawl
