#pragma once
#include "global.hpp"
#include "graphic-base.hpp"

namespace gawl::internal {
class EmptyTextureData : public GraphicBase<GraphicGLObject> {
  private:
    GLuint                frame_buffer;
    std::array<size_t, 2> size;

  public:
    auto get_size() const -> const std::array<size_t, 2>& {
        return size;
    }
    auto get_frame_buffer_name() const -> GLuint {
        return frame_buffer;
    }
    EmptyTextureData(const size_t width, const size_t height) : GraphicBase<GraphicGLObject>(get_global()->graphic_shader), size{width, height} {
        const auto txbinder = bind_texture();

        this->width             = width;
        this->height            = height;
        this->invert_top_bottom = true;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenFramebuffers(1, &frame_buffer);
        auto fbbinder = FramebufferBinder(frame_buffer);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, get_texture(), 0);
        GLenum buffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, buffers);
    }
    ~EmptyTextureData() {
        glDeleteFramebuffers(1, &frame_buffer);
    }
};

} // namespace gawl::internal
