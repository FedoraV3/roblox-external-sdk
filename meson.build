project('untitled5', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=c++23'])

fs = import('fs')

sources = files('main.cpp')
headers = files('offsets.h', 'structs.h')
incdirs = ['.']

# optional parts, so the setup survives folders being added or removed
foreach f : ['player/player.cpp', 'player/localplayer/player.cpp', 'player/team/team.cpp', 'memory/memory.cpp', 'engine/instance/instance.cpp', 'engine/datamodel/DataModel.cpp']
  if fs.exists(f)
    sources += files(f)
  endif
endforeach

foreach f : ['player/player.h', 'player/localplayer/player.h', 'player/team/team.h', 'memory/memory.h', 'engine/instance/instance.h', 'engine/datamodel/DataModel.h']
  if fs.exists(f)
    headers += files(f)
  endif
endforeach

foreach d : ['player', 'player/localplayer', 'player/team', 'memory', 'engine', 'engine/instance', 'engine/datamodel']
  if fs.exists(d)
    incdirs += d
  endif
endforeach

untitled5 = executable('untitled5', sources, headers, install : true, include_directories : include_directories(incdirs))

test('test', untitled5)
