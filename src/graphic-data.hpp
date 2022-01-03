#pragma once
#include "graphic-base.hpp"
#include "pixelbuffer.hpp"

namespace gawl::internal {
class GraphicData : public GraphicBase<GraphicGLObject> {
  public:
    GraphicData(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop) : GraphicBase<GraphicGLObject>(get_global()->graphic_shader) {
        const auto txbinder = this->bind_texture();
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
        this->width  = crop ? (*crop)[2] : buffer.get_width();
        this->height = crop ? (*crop)[3] : buffer.get_height();
        glPixelStorei(GL_UNPACK_ROW_LENGTH, buffer.get_width());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get_buffer());
    }
    ~GraphicData() {}
};
} // namespace gawl::internal
