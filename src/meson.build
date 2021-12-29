wl_deps = [
    dependency('wayland-client++'), 
    dependency('wayland-client-extra++'), 
    dependency('wayland-cursor++'), 
    dependency('wayland-egl++')]
egl_deps = [
    dependency('glx'),
    dependency('opengl'),
    dependency('egl')]
freetype_inc = include_directories('/usr/include/freetype2')
freetype_dep = dependency('freetype2')
thread_dep = dependency('threads')
magick_inc = include_directories('/usr/include/ImageMagick-7')
magick_dep = dependency('Magick++')

gawl_inc = [freetype_inc, magick_inc]
gawl_dep = [wl_deps, egl_deps, freetype_dep, thread_dep, magick_dep]
gawl_files = files(
    'empty-texture.cpp',
    'gawl-application.cpp',
    'gawl-window.cpp',
    'globject.cpp',
    'graphic-base.cpp',
    'graphic.cpp',
    'misc.cpp',
    'pixelbuffer.cpp',
    'polygon.cpp',
    'screen.cpp',
    'textrender.cpp',
    'wayland-application.cpp',
    'wayland-window.cpp',
)

gawl_headers = files(
    'binder.hpp',
    'earcut.hpp',
    'empty-texture.hpp',
    'error.hpp',
    'fd.hpp',
    'gawl-application.hpp',
    'gawl-window.hpp',
    'gawl.hpp',
    'globject.hpp',
    'graphic-base.hpp',
    'graphic.hpp',
    'misc.hpp',
    'pixelbuffer.hpp',
    'polygon.hpp',
    'screen.hpp',
    'textrender.hpp',
    'thread.hpp',
    'type.hpp',
    'variant-buffer.hpp',
    'variant.hpp',
    'wayland-application.hpp',
    'wayland-window.hpp',
)

gawl = shared_module(
    'gawl',
    gawl_files,
    dependencies: [gawl_dep],
    install: true
)

install_headers(gawl_headers, subdir: 'gawl')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libraries: [gawl, gawl_dep],
    name: 'gawl',
    filebase: 'gawl',
    description: 'libgawl'
)
