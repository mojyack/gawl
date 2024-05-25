subdir('wayland')

freetype_dep = dependency('freetype2')
magick_dep = dependency('Magick++')
jxl_dep = dependency('libjxl')
xkbcommon_dep = dependency('xkbcommon')
fc_dep = dependency('fontconfig')

gawl_deps = gawl_wayland_deps + [freetype_dep, magick_dep, jxl_dep]
gawl_keysym_deps = [xkbcommon_dep]
gawl_fc_deps = [fc_dep]

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

gawl_files = files(
)
