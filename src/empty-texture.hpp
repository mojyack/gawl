#pragma once
#include "graphic-base.hpp"

namespace gawl {
class EmptyTexture : public internal::GraphicBase<internal::GraphicGLObject> {
  private:
    GLuint                frame_buffer;
    std::array<size_t, 2> size;
    internal::Viewport    viewport;

    auto get_size() const -> const std::array<size_t, 2>& {
        return size;
    }

    auto get_frame_buffer_name() const -> GLuint {
        return frame_buffer;
    }

  public:
    auto get_scale() const -> double {
        return 1.0;
    }

    auto set_viewport(const gawl::Rectangle& region) -> void {
        viewport.set(region, get_size());
    }

    auto unset_viewport() -> void {
        viewport.unset(get_size());
    }

    auto get_viewport() const -> internal::Viewport {
        return viewport;
    }

    auto prepare() -> internal::FramebufferBinder {
        auto        binder = internal::FramebufferBinder(get_frame_buffer_name());
        const auto& size   = get_size();
        glViewport(0, 0, size[0], size[1]);
        return binder;
    }

    EmptyTexture(const size_t width, const size_t height) : internal::GraphicBase<internal::GraphicGLObject>(internal::global->graphic_shader), size{width, height} {
        const auto txbinder = bind_texture();

        this->width             = width;
        this->height            = height;
        this->invert_top_bottom = true;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenFramebuffers(1, &frame_buffer);
        const auto fbbinder = internal::FramebufferBinder(frame_buffer);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, get_texture(), 0);
        GLenum buffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, buffers);
    }

    ~EmptyTexture() {
        glDeleteFramebuffers(1, &frame_buffer);
    }
};

} // namespace gawl
