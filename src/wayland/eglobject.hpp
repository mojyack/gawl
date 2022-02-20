#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <EGL/egl.h>

#include "../util.hpp"
#include "towl.hpp"

namespace gawl::internal::wl {
struct EGLObject {
    EGLDisplay display = nullptr;
    EGLConfig  config  = nullptr;
    EGLContext context = nullptr;
    EGLObject(towl::Display& wl_display) {
        display = eglGetDisplay(wl_display.native());
        internal::dynamic_assert(display != EGL_NO_DISPLAY);

        auto major = EGLint(0);
        auto minor = EGLint(0);
        internal::dynamic_assert(eglInitialize(display, &major, &minor) != EGL_FALSE);
        internal::dynamic_assert((major == 1 && minor >= 4) || major >= 2);
        internal::dynamic_assert(eglBindAPI(EGL_OPENGL_API) != EGL_FALSE);

        constexpr auto config_attribs = std::array<EGLint, 15>{EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                                               EGL_RED_SIZE, 8,
                                                               EGL_GREEN_SIZE, 8,
                                                               EGL_BLUE_SIZE, 8,
                                                               EGL_ALPHA_SIZE, 8,
                                                               EGL_SAMPLES, 4,
                                                               EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                                               EGL_NONE};

        auto num = EGLint(0);
        internal::dynamic_assert(eglChooseConfig(display, config_attribs.data(), &config, 1, &num) != EGL_FALSE && num != 0);

        constexpr auto context_attribs = std::array<EGLint, 3>{EGL_CONTEXT_CLIENT_VERSION, 2,
                                                               EGL_NONE};

        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs.data());
        internal::dynamic_assert(context != EGL_NO_CONTEXT);
    }
    ~EGLObject() {
        internal::dynamic_assert(eglDestroyContext(display, context) != EGL_FALSE);
        internal::dynamic_assert(eglTerminate(display) != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
