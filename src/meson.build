sources = ['main.cc', 'app.cc', 'graphics.cc', 'rasterize.cc', 'lsystem.cc',
  'imgui_impl_sdlrenderer3.cpp', 'imgui_impl_sdl3.cpp']

libs_header_only = include_directories('.', 'libs_header_only')

executable('app', sources, include_directories : libs_header_only, 
          dependencies: [sdl3_dep, sdl3_image_dep, fmt_dep, imgui_dep])
