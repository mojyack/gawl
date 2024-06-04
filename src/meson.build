subdir('wayland')

freetype_dep = dependency('freetype2')
magick_dep = dependency('Magick++')
jxl_dep = dependency('libjxl')
fc_dep = dependency('fontconfig')

gawl_core_deps = gawl_wayland_deps + [freetype_dep]
gawl_core_files = files(
  # core
  'point.cpp',
  'rect.cpp',
  'application.cpp',
  'window-callbacks.cpp',
  'window.cpp',
  'misc.cpp',
  'graphic-base.cpp',
  # shader
  'shader.cpp',
  'graphic-shader.cpp',
  'textrender-shader.cpp',
  'polygon-shader.cpp',
) + gawl_wayland_files

gawl_graphic_deps = [magick_dep, jxl_dep]
gawl_graphic_files = files(
  'pixelbuffer.cpp',
  'graphic.cpp',
  'jxl-decoder.cpp',
)

gawl_textrender_deps = []
gawl_textrender_files = files(
  'textrender.cpp',
)

gawl_polygon_deps = []
gawl_polygon_files = files(
  'polygon.cpp',
)

gawl_fc_deps = [fc_dep]
gawl_fc_files = files(
  'fc.cpp',
)

# optional files
gawl_empty_texture_files = files('empty-texture.cpp')
gawl_no_touch_callbacks_file = files('window-no-touch-callbacks.cpp')
