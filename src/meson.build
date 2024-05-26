subdir('wayland')

freetype_dep = dependency('freetype2')
magick_dep = dependency('Magick++')
jxl_dep = dependency('libjxl')
xkbcommon_dep = dependency('xkbcommon')
fc_dep = dependency('fontconfig')

gawl_deps = gawl_wayland_deps + [freetype_dep, magick_dep, jxl_dep]
gawl_keysym_deps = [xkbcommon_dep]
gawl_fc_deps = [fc_dep]
gawl_files = files(
  'window-callbacks.cpp',
  'pixelbuffer.cpp',
  'shader.cpp',
  'polygon.cpp',
  'graphic-shader.cpp',
  'textrender-shader.cpp',
  'polygon-shader.cpp',
  'graphic-base.cpp',
  'graphic.cpp',
  'misc.cpp',
  'jxl-decoder.cpp',
)
