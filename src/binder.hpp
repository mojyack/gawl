#pragma once
#include <utility>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace gawl::impl {
template <typename F>
concept BindCaller = requires(F f) {
    f.bind(GLenum(), GLuint());
};

template <GLenum target, BindCaller F>
class [[nodiscard]] Binder {
  private:
    GLuint v = 0;

  public:
    auto get() const -> GLuint {
        return v;
    }

    auto unbind() -> void {
        v = 0;
        F::bind(target, v);
    }

    auto operator=(Binder&& o) -> Binder& {
        unbind();
        std::swap(v, o.v);
        return *this;
    }

    Binder() = default;

    Binder(const GLuint v) : v(v) {
        F::bind(target, v);
    }

    Binder(Binder&& o) {
        *this = std::move(o);
    }

    ~Binder() {
        unbind();
    }
};

namespace bindcaller {
struct FramebufferBindCaller {
    static auto bind(const GLenum target, const GLuint b) -> void {
        glBindFramebuffer(target, b);
    }
};

struct BufferBindCaller {
    static auto bind(const GLenum target, const GLuint b) -> void {
        glBindBuffer(target, b);
    }
};

struct VArrayBindCaller {
    static auto bind(const GLenum /*target*/, const GLuint b) -> void {
        glBindVertexArray(b);
    }
};

struct ShaderBindCaller {
    static auto bind(const GLenum /*target*/, const GLuint b) -> void {
        glUseProgram(b);
    }
};

struct TextureBindCaller {
    static auto bind(const GLenum target, const GLuint b) -> void {
        glBindTexture(target, b);
    }
};
} // namespace bindcaller

using FramebufferBinder   = Binder<GL_FRAMEBUFFER, bindcaller::FramebufferBindCaller>;
using VertexBufferBinder  = Binder<GL_ARRAY_BUFFER, bindcaller::BufferBindCaller>;
using ElementBufferBinder = Binder<GL_ELEMENT_ARRAY_BUFFER, bindcaller::BufferBindCaller>;
using VArrayBinder        = Binder<0, bindcaller::VArrayBindCaller>;
using ShaderBinder        = Binder<0, bindcaller::ShaderBindCaller>;
using TextureBinder       = Binder<GL_TEXTURE_2D, bindcaller::TextureBindCaller>;

} // namespace gawl::impl
