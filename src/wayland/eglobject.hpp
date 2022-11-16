#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <EGL/egl.h>

#include "../util.hpp"
#include "towl.hpp"

namespace gawl {
class EGLSubObject {
  private:
    const EGLDisplay display;
    const EGLContext context;

  public:
    auto flush() -> void {
        glFlush();
    }

    auto wait() -> void {
        glFinish();
    }

    EGLSubObject(const EGLDisplay display, const EGLContext context) : display(display), context(context) {
        dynamic_assert(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) != EGL_FALSE);
    }

    ~EGLSubObject() {
        dynamic_assert(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE);
        dynamic_assert(eglDestroyContext(display, context) != EGL_FALSE);
    }
};
} // namespace gawl

namespace gawl::internal::wl {
struct EGLObject {
    constexpr static auto context_attribs = std::array<EGLint, 3>{EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    EGLDisplay display = nullptr;
    EGLConfig  config  = nullptr;
    EGLContext context = nullptr;

    auto fork() const -> EGLSubObject {
        return {display, eglCreateContext(display, config, context, context_attribs.data())};
    }

    EGLObject(towl::Display& wl_display) {
        display = eglGetDisplay(wl_display.native());
        dynamic_assert(display != EGL_NO_DISPLAY);

        auto major = EGLint(0);
        auto minor = EGLint(0);
        dynamic_assert(eglInitialize(display, &major, &minor) != EGL_FALSE);
        dynamic_assert((major == 1 && minor >= 4) || major >= 2);
        dynamic_assert(eglBindAPI(EGL_OPENGL_API) != EGL_FALSE);

        constexpr auto config_attribs = std::array<EGLint, 15>{EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                                               EGL_RED_SIZE, 8,
                                                               EGL_GREEN_SIZE, 8,
                                                               EGL_BLUE_SIZE, 8,
                                                               EGL_ALPHA_SIZE, 8,
                                                               EGL_SAMPLES, 4,
                                                               EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                                               EGL_NONE};

        auto num = EGLint(0);
        dynamic_assert(eglChooseConfig(display, config_attribs.data(), &config, 1, &num) != EGL_FALSE && num != 0);
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs.data());
        dynamic_assert(context != EGL_NO_CONTEXT);
    }

    ~EGLObject() {
        dynamic_assert(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE);
        dynamic_assert(eglDestroyContext(display, context) != EGL_FALSE);
        dynamic_assert(eglTerminate(display) != EGL_FALSE);
        dynamic_assert(eglReleaseThread() != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
