#include "graphic.hpp"
#include "global.hpp"

namespace gawl {
auto Graphic::update_texture(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) -> void {
    const auto txbinder = this->bind_texture();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if(crop) {
        const auto& rect = *crop;
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect[0]);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, rect[1]);
        this->width  = rect[2];
        this->height = rect[3];
    } else {
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        this->width  = buffer.width;
        this->height = buffer.height;
    }
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
