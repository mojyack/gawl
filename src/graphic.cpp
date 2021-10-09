#include <cstring>
#include <iostream>

#include <ImageMagick-7/Magick++.h>

#include "error.hpp"
#include "global.hpp"
#include "graphic.hpp"

namespace gawl {
namespace {
auto load_texture_imagemagick(Magick::Image&& image) -> PixelBuffer {
    auto buffer = std::vector<uint8_t>(image.columns() * image.rows() * 4);
    image.write(0, 0, image.columns(), image.rows(), "RGBA", Magick::CharPixel, buffer.data());
    return PixelBuffer(image.columns(), image.rows(), buffer);
}
} // namespace

namespace internal {
extern GlobalVar* global;
}

// ====== PixelBuffer ====== //
auto PixelBuffer::empty() const -> bool {
    return data.empty();
}
auto PixelBuffer::get_width() const -> size_t {
    return size[0];
}
auto PixelBuffer::get_height() const -> size_t {
    return size[1];
}
auto PixelBuffer::get_buffer() const -> const uint8_t* {
    return data.data();
}
auto PixelBuffer::clear() -> void {
    size = {0, 0};
    data.clear();
}
PixelBuffer::PixelBuffer(const size_t width, const size_t height, const char* const buffer) : size({width, height}) {
    const auto len = size_t(size[0] * size[1] * 4);
    data.resize(len);
    std::memcpy(data.data(), buffer, len);
}
PixelBuffer::PixelBuffer(const size_t width, const size_t height, std::vector<uint8_t>& buffer) : size({width, height}) {
    data = std::move(buffer);
}
PixelBuffer::PixelBuffer(const char* file) {
    auto buf = load_texture_imagemagick(Magick::Image(file));
    if(!buf.empty()) {
        *this = std::move(buf);
    }
}
PixelBuffer::PixelBuffer(const std::vector<uint8_t>& buffer) {
    auto blob = Magick::Blob(buffer.data(), buffer.size());
    *this     = load_texture_imagemagick(Magick::Image(blob));
}

// ====== GraphicData ====== //
class GraphicData : public GraphicBase {
  public:
    GraphicData(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop);
    GraphicData(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop);
    ~GraphicData() {}
};

GraphicData::GraphicData(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) : GraphicData(std::move(buffer), crop) {}
GraphicData::GraphicData(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop) : GraphicBase(*internal::global->graphic_shader) {
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

// ====== Graphic ====== //
auto Graphic::get_width(const Screen* const screen) const -> int {
    ASSERT(graphic_data, "Texture not initialized")
    return reinterpret_cast<GraphicData*>(graphic_data.get())->get_width(screen);
}
auto Graphic::get_height(const Screen* const screen) const -> int {
    ASSERT(graphic_data, "Texture not initialized")
    return reinterpret_cast<GraphicData*>(graphic_data.get())->get_height(screen);
}
auto Graphic::draw(Screen* const screen, const Point& point) -> void {
    ASSERT(graphic_data, "Texture not initialized")
    reinterpret_cast<GraphicData*>(graphic_data.get())->draw(screen, point);
}
auto Graphic::draw_rect(Screen* screen, const Rectangle& rect) -> void {
    ASSERT(graphic_data, "Texture not initialized")
    graphic_data.get()->draw_rect(screen, rect);
}
auto Graphic::draw_fit_rect(Screen* screen, const Rectangle& rect) -> void {
    ASSERT(graphic_data, "Texture not initialized")
    graphic_data.get()->draw_fit_rect(screen, rect);
}
auto Graphic::clear() -> void {
    *this = Graphic();
}
Graphic::operator GraphicBase*() const {
    return graphic_data.get();
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
Graphic::Graphic(const char* file, std::optional<std::array<int, 4>> crop) {
    try {
        const auto data = new GraphicData(PixelBuffer(file), crop);
        graphic_data.reset(data);
    } catch(const std::exception&) {
        std::cerr << file << " is not a valid image file." << std::endl;
        return;
    }
}
Graphic::Graphic(std::vector<uint8_t>& buffer, std::optional<std::array<int, 4>> crop) {
    try {
        const auto data = new GraphicData(PixelBuffer(buffer), crop);
        graphic_data.reset(data);
    } catch(const std::exception&) {
        std::cerr << "invalid buffer." << std::endl;
        return;
    }
}
Graphic::Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) {
    graphic_data.reset(new GraphicData(buffer, crop));
}
Graphic::Graphic(const PixelBuffer&& buffer, std::optional<std::array<int, 4>> crop) {
    graphic_data.reset(new GraphicData(std::forward<decltype(buffer)>(buffer), crop));
}
Graphic::Graphic() {}
} // namespace gawl
