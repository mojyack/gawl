#include "eglobject.hpp"
#include "../util/assert.hpp"

namespace gawl {
auto EGLSubObject::flush() -> void {
    glFlush();
}

auto EGLSubObject::wait() -> void {
    glFinish();
}

EGLSubObject::EGLSubObject(const EGLDisplay display, const EGLContext context) : display(display), context(context) {
    line_assert(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) != EGL_FALSE);
}

EGLSubObject::~EGLSubObject() {
    line_assert(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE);
    line_assert(eglDestroyContext(display, context) != EGL_FALSE);
}

namespace impl {
auto EGLObject::fork() const -> EGLSubObject {
    return {display, eglCreateContext(display, config, context, context_attribs.data())};
}

EGLObject::EGLObject(towl::Display& wl_display) {
    display = eglGetDisplay(wl_display.native());
    line_assert(display != EGL_NO_DISPLAY);

    auto major = EGLint(0);
    auto minor = EGLint(0);
    line_assert(eglInitialize(display, &major, &minor) != EGL_FALSE);
    line_assert((major == 1 && minor >= 4) || major >= 2);
    line_assert(eglBindAPI(EGL_OPENGL_API) != EGL_FALSE);

    constexpr auto config_attribs = std::array<EGLint, 15>{EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                                           EGL_RED_SIZE, 8,
                                                           EGL_GREEN_SIZE, 8,
                                                           EGL_BLUE_SIZE, 8,
                                                           EGL_ALPHA_SIZE, 8,
                                                           EGL_SAMPLES, 4,
                                                           EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                                           EGL_NONE};

    auto num = EGLint(0);
    line_assert(eglChooseConfig(display, config_attribs.data(), &config, 1, &num) != EGL_FALSE && num != 0);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs.data());
    line_assert(context != EGL_NO_CONTEXT);
}

EGLObject::~EGLObject() {
    line_assert(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE);
    line_assert(eglDestroyContext(display, context) != EGL_FALSE);
    line_assert(eglTerminate(display) != EGL_FALSE);
    line_assert(eglReleaseThread() != EGL_FALSE);
}
} // namespace impl
} // namespace gawl
