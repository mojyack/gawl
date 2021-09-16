#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "empty-texture.hpp"
#include "error.hpp"
#include "global.hpp"

namespace gawl {
extern GlobalVar* global;
// ====== EmptyTextureData ====== //
auto EmptyTextureData::get_size() const -> const std::array<int, 2>& {
    return size;
}
auto EmptyTextureData::get_frame_buffer_name() const -> GLuint {
    return frame_buffer;
}
EmptyTextureData::EmptyTextureData(const int width, const int height) : GraphicBase(*global->graphic_shader), size{width, height} {
    this->width             = width;
    this->height            = height;
    this->invert_top_bottom = true;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, get_texture(), 0);
    GLenum buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, buffers);
}
EmptyTextureData::~EmptyTextureData() {
    glDeleteFramebuffers(1, &frame_buffer);
}

// ====== EmptyTexture ====== //
auto EmptyTexture::get_width(FrameBufferInfo info) const -> int {
    ASSERT(data, "texture not initialized")
    return reinterpret_cast<EmptyTextureData*>(data.get())->get_width(info);
}
auto EmptyTexture::get_height(FrameBufferInfo info) const -> int {
    ASSERT(data, "texture not initialized")
    return reinterpret_cast<EmptyTextureData*>(data.get())->get_height(info);
}
auto EmptyTexture::draw(FrameBufferInfo info, double x, double y) -> void {
    ASSERT(data, "texture not initialized")
    reinterpret_cast<EmptyTextureData*>(data.get())->draw(info, x, y);
}
auto EmptyTexture::draw_rect(FrameBufferInfo info, Area area) -> void {
    ASSERT(data, "texture not initialized")
    data.get()->draw_rect(info, area);
}
auto EmptyTexture::draw_fit_rect(FrameBufferInfo info, Area area) -> void {
    ASSERT(data, "texture not initialized")
    data.get()->draw_fit_rect(info, area);
}
auto EmptyTexture::clear() -> void {
    *this = EmptyTexture();
}
EmptyTexture::operator EmptyTextureData*() const {
    return data.get();
}
EmptyTexture::operator GraphicBase*() const {
    return data.get();
}
EmptyTexture::operator bool() const {
    return static_cast<bool>(data);
}
EmptyTexture::EmptyTexture(const EmptyTexture& src) {
    *this = src;
}
EmptyTexture::EmptyTexture(EmptyTexture&& src) {
    *this = src;
}
EmptyTexture& EmptyTexture::operator=(const EmptyTexture& src) {
    this->data = src.data;
    return *this;
}
EmptyTexture& EmptyTexture::operator=(EmptyTexture&& src) {
    this->data = src.data;
    return *this;
}
EmptyTexture::EmptyTexture(const int width, const int height) {
    data.reset(new EmptyTextureData(width, height));
}
} // namespace gawl
