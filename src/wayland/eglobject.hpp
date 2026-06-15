#pragma once
#include <array>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <EGL/egl.h>

#include "towl/display.hpp"

namespace gawl {
struct EGLSubObject {
    EGLDisplay display;
    EGLContext context = nullptr;

    auto flush() -> void;
    auto wait() -> void;
    auto destroy() -> void;

    auto operator=(EGLSubObject&& o) -> EGLSubObject&;

    EGLSubObject() = default;
    EGLSubObject(EGLSubObject&& o);
    EGLSubObject(const EGLDisplay display, const EGLContext context);
    ~EGLSubObject();
};
} // namespace gawl

namespace gawl::impl {
struct EGLObject {
    constexpr static auto context_attribs = std::array<EGLint, 3>{EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    EGLDisplay display = nullptr;
    EGLConfig  config  = nullptr;
    EGLContext context = nullptr;

    auto fork() const -> EGLSubObject;

    EGLObject(towl::Display& wl_display);
    ~EGLObject();
};
} // namespace gawl::impl
