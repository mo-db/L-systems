sources = ['minitest.cc']

libs_header_only = include_directories('.', 'libs_header_only')

executable('minitest', sources, include_directories : libs_header_only, 
          dependencies: [sdl3_dep, sdl3_image_dep, fmt_dep, imgui_dep])
