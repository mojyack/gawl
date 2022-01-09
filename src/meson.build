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

gawl_dep = [wl_deps, egl_deps, freetype_dep, thread_dep, magick_dep]

gawl_headers = files(
  'application.hpp',
  'binder.hpp',
  'earcut.hpp',
  'error.hpp',
  'empty-texture-data.hpp',
  'empty-texture.hpp',
  'fd.hpp',
  'global.hpp',
  'globject.hpp',
  'graphic-base.hpp',
  'graphic-data.hpp',
  'graphic-globject.hpp',
  'graphic.hpp',
  'include.hpp',
  'internal-type.hpp',
  'misc.hpp',
  'null-screen.hpp',
  'pixelbuffer.hpp',
  'polygon-globject.hpp',
  'polygon.hpp',
  'screen.hpp',
  'shader-source.hpp',
  'textrender-data.hpp',
  'textrender-globject.hpp',
  'textrender.hpp',
  'thread.hpp',
  'type.hpp',
  'variant-buffer.hpp',
  'variant.hpp',
  'window-creat-hint.hpp',
  'window.hpp',
  'window-impl-concept.hpp',
)

install_headers(gawl_headers, subdir: 'gawl')

subdir('wayland')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libraries: [gawl_dep],
    name: 'gawl',
    filebase: 'gawl',
    description: 'libgawl'
)
