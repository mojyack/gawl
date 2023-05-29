wl_deps = [
    dependency('towl', version : ['>=1.2', '<1.3']),
]
egl_deps = [
    dependency('glx'),
    dependency('opengl'),
    dependency('egl')
]
freetype_dep = dependency('freetype2')
magick_dep = dependency('Magick++')
xkbcommon_dep = dependency('xkbcommon')
fc_dep = dependency('fontconfig')
jxl_dep = dependency('libjxl')

gawl_dep = [wl_deps, egl_deps, freetype_dep, magick_dep, jxl_dep]

gawl_headers = files(
  'application.hpp',
  'binder.hpp',
  'earcut.hpp',
  'empty-texture.hpp',
  'fc.hpp',
  'global.hpp',
  'globject.hpp',
  'graphic-base.hpp',
  'graphic-globject.hpp',
  'graphic.hpp',
  'include.hpp',
  'internal-type.hpp',
  'jxl-decoder.hpp',
  'misc.hpp',
  'null-screen.hpp',
  'pixelbuffer.hpp',
  'polygon-globject.hpp',
  'polygon.hpp',
  'screen.hpp',
  'shader-source.hpp',
  'textrender-internal.hpp',
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
pkgconfig.generate(
    requires: [gawl_dep],
    name: 'gawl',
    filebase: 'gawl',
    description: 'libgawl',
)
pkgconfig.generate(
    requires: [xkbcommon_dep],
    name: 'gawl-keysym',
    filebase: 'gawl-keysym',
    description: 'libgawl keysym dependency',
)
pkgconfig.generate(
    requires: [fc_dep],
    name: 'gawl-fc',
    filebase: 'gawl-fc',
    description: 'libgawl fontconfig dependency',
)
