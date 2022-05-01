project('gawltest', 'cpp', version : '1.0.0', default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++20'])

executable('01-polygon', ['polygon.cpp'], dependencies : [dependency('gawl')])
executable('02-graphics', ['graphics.cpp'], dependencies : [dependency('gawl')])
executable('03-textrender', ['textrender.cpp'], dependencies : [dependency('gawl'), dependency('fontconfig')])
executable('04-keyboard', ['keyboard.cpp'], dependencies : [dependency('gawl'), dependency('gawl-keysym')])
executable('05-mouse', ['mouse.cpp'], dependencies : [dependency('gawl'), dependency('fontconfig')])
executable('06-multiwindow', ['multiwindow.cpp'], dependencies : [dependency('gawl')])
