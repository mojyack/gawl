project('gawl', ['cpp', 'c'], version : '2.6.6', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++20'])
subdir('src')

common_files = gawl_files + gawl_wayland_files
common_deps = gawl_deps + gawl_wayland_deps

executable(
  'polygon',
  files('examples/polygon.cpp') + common_files,
  dependencies: common_deps
)

executable(
  'graphics',
  files('examples/graphics.cpp') + common_files,
  dependencies: common_deps
)

executable(
  'textrender',
  files('examples/textrender.cpp') + common_files,
  dependencies: common_deps + gawl_fc_deps
)

executable(
  'keyboard',
  files('examples/keyboard.cpp') + common_files,
  dependencies: common_deps + gawl_keysym_deps
)

executable(
  'mouse',
  files('examples/mouse.cpp') + common_files,
  dependencies: common_deps + gawl_fc_deps
)

executable(
  'multiwindow',
  files('examples/multiwindow.cpp') + common_files,
  dependencies: common_deps
)

executable(
  'mtloading',
  files('examples/background-texture-loading.cpp') + common_files,
  dependencies: common_deps
)

executable(
  'touch',
  files('examples/touch.cpp') + common_files,
  dependencies: common_deps + gawl_fc_deps
)
