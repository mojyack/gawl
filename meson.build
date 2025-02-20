project('gawl', ['cpp', 'c'], version : '3.0.0', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++23'])
add_project_arguments('-Wfatal-errors', language: 'cpp')
add_project_arguments('-Wno-missing-field-initializers', language: 'cpp')
subdir('src')

executable(
  'polygon',
  files('examples/polygon.cpp') + gawl_core_files + gawl_polygon_files,
  dependencies: gawl_core_deps + gawl_polygon_deps,
)

executable(
  'graphics',
  files('examples/graphics.cpp') + gawl_core_files + gawl_graphic_files,
  dependencies: gawl_core_deps + gawl_graphic_deps,
)

executable(
  'empty-texture',
  files('examples/empty-texture.cpp') + gawl_core_files + gawl_graphic_files + gawl_empty_texture_files,
  dependencies: gawl_core_deps + gawl_graphic_deps,
)

executable(
  'textrender',
  files('examples/textrender.cpp') + gawl_core_files + gawl_textrender_files + gawl_fc_files,
  dependencies: gawl_core_deps + gawl_textrender_deps + gawl_fc_deps,
)

executable(
  'keyboard',
  files('examples/keyboard.cpp') + gawl_core_files,
  dependencies: gawl_core_deps,
)

executable(
  'mouse',
  files('examples/mouse.cpp') + gawl_core_files + gawl_textrender_files + gawl_fc_files + gawl_no_touch_callbacks_file,
  dependencies: gawl_core_deps + gawl_textrender_deps + gawl_fc_deps,
)

executable(
  'multiwindow',
  files('examples/multiwindow.cpp') + gawl_core_files,
  dependencies: gawl_core_deps,
)

executable(
  'mtloading',
  files('examples/background-texture-loading.cpp') + gawl_core_files + gawl_graphic_files + gawl_fc_files,
  dependencies: gawl_core_deps + gawl_graphic_deps + gawl_fc_deps,
)

executable(
  'touch',
  files('examples/touch.cpp') + gawl_core_files + gawl_polygon_files,
  dependencies: gawl_core_deps + gawl_polygon_deps,
)
