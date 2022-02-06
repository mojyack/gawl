wl_deps = [
    dependency('towl', version : ['>=1.0', '<1.1']), 
]
egl_deps = [
    dependency('glx'),
    dependency('opengl'),
    dependency('egl')
]
freetype_dep = dependency('freetype2')
magick_dep = dependency('Magick++')

gawl_dep = [wl_deps, egl_deps, freetype_dep, magick_dep]

gawl_headers = files(
  'application.hpp',
  'binder.hpp',
  'earcut.hpp',
  'empty-texture-data.hpp',
  'empty-texture.hpp',
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
  'type.hpp',
  'util.hpp',
  'variant-buffer.hpp',
  'window-creat-hint.hpp',
  'window-impl-concept.hpp',
  'window.hpp',
)

install_headers(gawl_headers, subdir: 'gawl')

subdir('util')
install_headers(cutil_files, subdir: 'gawl/util')

subdir('wayland')

pkgconfig = import('pkgconfig')
cpp = meson.get_compiler('cpp')
pkgconfig.generate(
    requires: [gawl_dep],
    name: 'gawl',
    filebase: 'gawl',
    description: 'libgawl',
)
