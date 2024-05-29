#include "empty-texture.hpp"
#include "global.hpp"

namespace gawl {
auto EmptyTexture::get_scale() const -> double {
    return 1.0;
}

auto EmptyTexture::set_viewport(const gawl::Rectangle& region) -> void {
    viewport.set(region, {size_t(width), size_t(height)});
}

auto EmptyTexture::unset_viewport() -> void {
    viewport.unset({size_t(width), size_t(height)});
}

auto EmptyTexture::get_viewport() const -> const Viewport& {
    return viewport;
}

auto EmptyTexture::prepare() -> impl::FramebufferBinder {
    auto binder = impl::FramebufferBinder(frame_buffer);
    glViewport(0, 0, width, height);
    return binder;
}

EmptyTexture::EmptyTexture(const int width, const int height)
    : GraphicBase(impl::global->graphic_shader) {
    this->width             = width;
    this->height            = height;
    this->invert_top_bottom = true;
    unset_viewport();

    const auto txbinder = bind_texture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &frame_buffer);
    const auto fbbinder = impl::FramebufferBinder(frame_buffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, get_texture(), 0);
    const auto buffers = std::array{GLenum(GL_COLOR_ATTACHMENT0)};
    glDrawBuffers(1, buffers.data());
}

EmptyTexture::~EmptyTexture() {
    glDeleteFramebuffers(1, &frame_buffer);
}
} // namespace gawl
