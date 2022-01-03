#pragma once
#include <cassert>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <wayland-client-protocol-extra.hpp>
#include <wayland-egl.hpp>

#include "../error.hpp"

namespace gawl::internal::wl {
struct EGLObject {
    EGLDisplay display = nullptr;
    EGLConfig  config  = nullptr;
    EGLContext context = nullptr;
    EGLObject(const wayland::display_t& wl_display) {
        display = eglGetDisplay(wl_display);
        assert(display != EGL_NO_DISPLAY);

        auto major = EGLint(0);
        auto minor = EGLint(0);
        assert(eglInitialize(display, &major, &minor) != EGL_FALSE);
        assert((major == 1 && minor >= 4) || major >= 2);
        assert(eglBindAPI(EGL_OPENGL_API) != EGL_FALSE);

        constexpr auto config_attribs = std::array<EGLint, 15>{EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                                               EGL_RED_SIZE, 8,
                                                               EGL_GREEN_SIZE, 8,
                                                               EGL_BLUE_SIZE, 8,
                                                               EGL_ALPHA_SIZE, 8,
                                                               EGL_SAMPLES, 4,
                                                               EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                                               EGL_NONE};

        auto num = EGLint(0);
        assert(eglChooseConfig(display, config_attribs.data(), &config, 1, &num) != EGL_FALSE && num != 0);

        constexpr auto context_attribs = std::array<EGLint, 3>{EGL_CONTEXT_CLIENT_VERSION, 2,
                                                               EGL_NONE};

        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs.data());
        assert(context != EGL_NO_CONTEXT);
    }
    ~EGLObject() {
        assert(eglDestroyContext(display, context) != EGL_FALSE);
        assert(eglTerminate(display) != EGL_FALSE);
    }
};
} // namespace gawl::internal::wl
