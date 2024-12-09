#include "eglobject.hpp"
#include "../macros/assert.hpp"

namespace gawl {
auto EGLSubObject::flush() -> void {
    glFlush();
}

auto EGLSubObject::wait() -> void {
    glFinish();
}

EGLSubObject::EGLSubObject(const EGLDisplay display, const EGLContext context) : display(display), context(context) {
    ASSERT(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) != EGL_FALSE);
}

EGLSubObject::~EGLSubObject() {
    ASSERT(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE);
    ASSERT(eglDestroyContext(display, context) != EGL_FALSE);
}

namespace impl {
auto EGLObject::fork() const -> EGLSubObject {
    return {display, eglCreateContext(display, config, context, context_attribs.data())};
}

EGLObject::EGLObject(towl::Display& wl_display) {
    display = eglGetDisplay(wl_display.native());
    ASSERT(display != EGL_NO_DISPLAY);

    auto major = EGLint(0);
    auto minor = EGLint(0);
    ASSERT(eglInitialize(display, &major, &minor) != EGL_FALSE);
    ASSERT((major == 1 && minor >= 4) || major >= 2);
    ASSERT(eglBindAPI(EGL_OPENGL_API) != EGL_FALSE);

    constexpr auto config_attribs = std::array<EGLint, 15>{EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                                           EGL_RED_SIZE, 8,
                                                           EGL_GREEN_SIZE, 8,
                                                           EGL_BLUE_SIZE, 8,
                                                           EGL_ALPHA_SIZE, 8,
                                                           EGL_SAMPLES, 4,
                                                           EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                                           EGL_NONE};

    auto num = EGLint(0);
    ASSERT(eglChooseConfig(display, config_attribs.data(), &config, 1, &num) != EGL_FALSE && num != 0);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs.data());
    ASSERT(context != EGL_NO_CONTEXT);
}

EGLObject::~EGLObject() {
    ASSERT(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE);
    ASSERT(eglDestroyContext(display, context) != EGL_FALSE);
    ASSERT(eglTerminate(display) != EGL_FALSE);
    ASSERT(eglReleaseThread() != EGL_FALSE);
}
} // namespace impl
} // namespace gawl
