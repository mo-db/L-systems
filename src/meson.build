sources = ['main.cc', 'app.cc', 'graphics.cc', 'rasterize.cc', 'lsystem.cc',
           'imgui_impl_sdl3.cpp', 'imgui_impl_sdlrenderer3.cpp']

include_dir = include_directories('header_only_libs')

executable('demo', sources, include_directories : include_dir, 
          dependencies: [sdl3_dep, sdl3_image_dep, fmt_dep, liblo_dep, imgui_dep])

executable('sdltest', 'sdltest.cc', include_directories : include_dir, 
          dependencies: [sdl3_dep, sdl3_image_dep, fmt_dep, liblo_dep, imgui_dep])

# sources = ['imgui-vulkan.cc']
# executable('imtest', sources,
#            dependencies: [imgui_dep, sdl3_dep])
