#pragma once
#include <concepts>
#include <optional>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "error.hpp"

namespace gawl::internal {
template <typename F>
concept BindCaller = requires(F f) {
    f.bind(GLenum(), GLuint());
};

template <GLenum E, BindCaller F>
class [[nodiscard]] Binder {
  private:
    std::optional<GLuint> b;

    auto unbind() -> void {
        if(b.has_value()) {
            F::bind(E, 0);
        }
    }

  public:
    auto get() const -> GLuint {
        ASSERT(b, "buffer name moved")
        return b.value();
    }
    auto operator=(Binder&& o) -> Binder& {
        unbind();
        b = std::move(o.b);
        o.b.reset();
        return *this;
    }
    Binder() {}
    Binder(const GLuint b) : b(b) {
        F::bind(E, b);
    }
    Binder(Binder& o) : b(o.b) {
        o.b.reset();
    }
    ~Binder() {
        unbind();
    }
};

namespace bindcaller {
struct FramebufferBindCaller {
    static auto bind(const GLenum e, const GLuint b) {
        glBindFramebuffer(e, b);
    }
};

struct BufferBindCaller {
    static auto bind(const GLenum e, const GLuint b) {
        glBindBuffer(e, b);
    }
};

struct VArrayBindCaller {
    static auto bind(const GLenum /* e */, const GLuint b) {
        glBindVertexArray(b);
    }
};

struct ShaderBindCaller {
    static auto bind(const GLenum /* e */, const GLuint b) {
        glUseProgram(b);
    }
};

struct TextureBindCaller {
    static auto bind(const GLenum e, const GLuint b) {
        glBindTexture(e, b);
    }
};
} // namespace bindcaller

using FramebufferBinder   = Binder<GL_FRAMEBUFFER, bindcaller::FramebufferBindCaller>;
using VertexBufferBinder  = Binder<GL_ARRAY_BUFFER, bindcaller::BufferBindCaller>;
using ElementBufferBinder = Binder<GL_ELEMENT_ARRAY_BUFFER, bindcaller::BufferBindCaller>;
using VArrayBinder        = Binder<0, bindcaller::VArrayBindCaller>;
using ShaderBinder        = Binder<0, bindcaller::ShaderBindCaller>;
using TextureBinder       = Binder<GL_TEXTURE_2D, bindcaller::TextureBindCaller>;

} // namespace gawl::internal
