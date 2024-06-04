#include "graphic.hpp"
#include "global.hpp"

namespace gawl {
auto Graphic::update_texture(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) -> void {
    const auto txbinder = this->bind_texture();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    if(crop) {
        if((*crop)[0] < 0) {
            (*crop)[0] += buffer.width;
        }
        if((*crop)[1] < 0) {
            (*crop)[1] += buffer.height;
        }
        if((*crop)[2] < 0) {
            (*crop)[2] += buffer.width;
        }
        if((*crop)[3] < 0) {
            (*crop)[3] += buffer.height;
        }
    }

    if(crop) {
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, crop.value()[0]);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, crop.value()[1]);
    } else {
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    }
    this->width  = crop ? (*crop)[2] : buffer.width;
    this->height = crop ? (*crop)[3] : buffer.height;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, buffer.width);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data.data());
}

Graphic::Graphic()
    : GraphicBase(impl::global->graphic_shader) {}

Graphic::Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop)
    : Graphic() {
    update_texture(buffer, crop);
}
} // namespace gawl
