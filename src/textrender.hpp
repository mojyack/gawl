#pragma once
#include "align.hpp"
#include "color.hpp"
#include "graphic-base.hpp"
#include "viewport.hpp"

#include <freetype/freetype.h>

namespace gawl {
namespace impl {
class Character : public impl::GraphicBase {
  public:
    int offset[2];
    int advance;

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
    using Callback = std::function<bool(size_t, const gawl::Rectangle&, impl::Character&)>;

    auto init(std::vector<std::string> font_names, int default_size) -> void;
    auto get_default_size() const -> int;
    auto set_char_color(const Color& color) -> void;
    auto get_rect(const MetaScreen& screen, std::string_view text, int size = 0) -> Rectangle;
    auto get_rect(const MetaScreen& screen, std::u32string_view text, int size = 0) -> Rectangle;
    auto draw(Screen& screen, const Point& point, const Color& color, std::string_view text, int size = 0, Callback callback = nullptr) -> Rectangle;
    auto draw(Screen& screen, const Point& point, const Color& color, std::u32string_view text, int size = 0, Callback callback = nullptr) -> Rectangle;
    auto draw_fit_rect(Screen& screen, const Rectangle& rect, const Color& color, std::string_view text, int size = 0, gawl::Align alignx = gawl::Align::Center, gawl::Align aligny = gawl::Align::Center, Callback callback = nullptr) -> Rectangle;
    auto calc_wrapped_text_height(Screen& screen, double width, double line_height, std::string_view text, WrappedText& wrapped_text, int size = 0) -> double;
    auto draw_wrapped(Screen& screen, const Rectangle& rect, double line_height, const Color& color, std::string_view text, WrappedText& wrapped_text, int size = 0, gawl::Align alignx = gawl::Align::Center, gawl::Align aligny = gawl::Align::Center) -> void;

    TextRender() {}
    TextRender(std::vector<std::string> font_names, int default_size);
};
} // namespace gawl
