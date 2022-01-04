#pragma once
#include <unordered_map>
#include <vector>

#include "graphic-base.hpp"
#include "internal-type.hpp"

namespace gawl {
using TextRenderCharacterGraphic = internal::GraphicBase<internal::TextRenderGLObject>;
}
namespace gawl::internal {
class Character : public gawl::TextRenderCharacterGraphic {
  public:
    int offset[2];
    int advance;

    Character(char32_t code, const std::vector<FT_Face>& faces) : GraphicBase<TextRenderGLObject>(global->textrender_shader) {
        auto face        = FT_Face(nullptr);
        auto glyph_index = int(-1);
        for(auto f : faces) {
            const auto i = FT_Get_Char_Index(f, code);
            if(i != 0) {
                face        = f;
                glyph_index = i;
                break;
            }
        }
        if(glyph_index == -1) {
            // no font have the glygh. fallback to first font and remove character.
            code        = U' ';
            face        = faces[0];
            glyph_index = FT_Get_Char_Index(faces[0], code);
        }

        assert(!FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT));
        assert(!FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL));

        this->width  = face->glyph->bitmap.width;
        this->height = face->glyph->bitmap.rows;
        offset[0]    = face->glyph->bitmap_left;
        offset[1]    = face->glyph->bitmap_top;
        advance      = static_cast<int>(face->glyph->advance.x) >> 6;

        const auto txbinder = this->bind_texture();
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, this->width);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, this->width, this->height, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    }
};

class CharacterCache {
  public:
    std::vector<FT_Face>                     faces;
    std::unordered_map<char32_t, Character*> cache;

    auto get_character(const char32_t c) -> Character* {
        if(const auto f = cache.find(c); f != cache.end()) {
            return f->second;
        } else {
            const auto chara = new Character(c, faces);
            cache.insert(std::make_pair(c, chara));
            return chara;
        }
    }
    CharacterCache(const std::vector<std::string>& font_names, const int size) {
        for(const auto& path : font_names) {
            auto face = FT_Face();
            assert(!FT_New_Face(global->textrender_shader.freetype, path.data(), 0, &(face)));
            FT_Set_Pixel_Sizes(face, 0, size);
            faces.emplace_back(face);
        }
    }
    ~CharacterCache() {
        for(auto f : faces) {
            FT_Done_Face(f);
        }
    }
};

class TextRenderData {
  private:
    std::unordered_map<int, std::shared_ptr<CharacterCache>> caches;

    const std::vector<std::string> font_names;

  public:
    auto clear() -> void {
        for(auto& s : caches) {
            for(auto& c : s.second->cache) {
                delete c.second;
            }
        }
        caches.clear();
    }
    auto operator[](int size) -> CharacterCache& {
        if(!caches.contains(size)) {
            caches.insert(std::make_pair(size, new CharacterCache(font_names, size)));
        }
        return *caches.find(size)->second;
    }
    TextRenderData(const std::vector<std::string>&& font_names) : font_names(font_names) {}
    ~TextRenderData() {
        clear();
    }
};
} // namespace gawl::internal
