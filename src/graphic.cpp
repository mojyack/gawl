#include <cstring>
#include <iostream>

#include "error.hpp"
#include "graphic-base.hpp"
#include "graphic.hpp"
#include "shader-source.hpp"

namespace gawl {
namespace internal {
GLObject* gl;

auto create_graphic_globject() -> GLObject* {
    gl = new GLObject(graphic_vertex_shader_source, graphic_fragment_shader_source, true);
    return gl;
}

// ====== GraphicData ====== //
class GraphicData : public GraphicBase {
  public:
    GraphicData(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop);
    GraphicData(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop);
    ~GraphicData() {}
};

GraphicData::GraphicData(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) : GraphicData(std::move(buffer), crop) {}
GraphicData::GraphicData(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop) : GraphicBase(internal::gl) {
    const auto txbinder = bind_texture();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    if(crop) {
        if((*crop)[0] < 0) {
            (*crop)[0] += buffer.get_width();
        }
        if((*crop)[1] < 0) {
            (*crop)[1] += buffer.get_height();
        }
        if((*crop)[2] < 0) {
            (*crop)[2] += buffer.get_width();
        }
        if((*crop)[3] < 0) {
            (*crop)[3] += buffer.get_height();
        }
    }

    if(crop) {
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, crop.value()[0]);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, crop.value()[1]);
    } else {
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    }
    width  = crop ? (*crop)[2] : buffer.get_width();
    height = crop ? (*crop)[3] : buffer.get_height();
    glPixelStorei(GL_UNPACK_ROW_LENGTH, buffer.get_width());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get_buffer());
}
} // namespace internal

// ====== Graphic ====== //
auto Graphic::get_width(const Screen* const screen) const -> int {
    ASSERT(graphic_data, "Texture not initialized")
    return graphic_data.get()->get_width(screen);
}
auto Graphic::get_height(const Screen* const screen) const -> int {
    ASSERT(graphic_data, "Texture not initialized")
    return graphic_data.get()->get_height(screen);
}
auto Graphic::draw(Screen* const screen, const Point& point) -> void {
    ASSERT(graphic_data, "Texture not initialized")
    graphic_data.get()->draw(screen, point);
}
auto Graphic::draw_rect(Screen* screen, const Rectangle& rect) -> void {
    ASSERT(graphic_data, "Texture not initialized")
    graphic_data.get()->draw_rect(screen, rect);
}
auto Graphic::draw_fit_rect(Screen* screen, const Rectangle& rect) -> void {
    ASSERT(graphic_data, "Texture not initialized")
    graphic_data.get()->draw_fit_rect(screen, rect);
}
auto Graphic::draw_transformed(Screen* screen, const std::array<Point, 4>& vertices) const -> void {
    ASSERT(graphic_data, "Texture not initialized")
    graphic_data.get()->draw_transformed(screen, vertices);
}
auto Graphic::clear() -> void {
    *this = Graphic();
}
Graphic::operator bool() const {
    return static_cast<bool>(graphic_data);
}
Graphic::Graphic(const Graphic& src) {
    *this = src;
}
Graphic::Graphic(Graphic&& src) {
    *this = src;
}
Graphic& Graphic::operator=(const Graphic& src) {
    this->graphic_data = src.graphic_data;
    return *this;
}
Graphic& Graphic::operator=(Graphic&& src) {
    this->graphic_data = src.graphic_data;
    return *this;
}
Graphic::Graphic(const char* const file, std::optional<std::array<int, 4>> crop) {
    try {
        const auto data = new internal::GraphicData(PixelBuffer(file), crop);
        graphic_data.reset(data);
    } catch(const std::exception&) {
        std::cerr << file << " is not a valid image file." << std::endl;
        return;
    }
}
Graphic::Graphic(std::vector<uint8_t>& buffer, std::optional<std::array<int, 4>> crop) {
    try {
        const auto data = new internal::GraphicData(PixelBuffer(buffer), crop);
        graphic_data.reset(data);
    } catch(const std::exception&) {
        std::cerr << "invalid buffer." << std::endl;
        return;
    }
}
Graphic::Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) {
    graphic_data.reset(new internal::GraphicData(buffer, crop));
}
Graphic::Graphic(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop) {
    graphic_data.reset(new internal::GraphicData(std::forward<decltype(buffer)>(buffer), crop));
}
Graphic::Graphic() {}
} // namespace gawl
