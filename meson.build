project('lsys', 'c', 'cpp', 
  default_options : ['c_std=c11', 'cpp_std=c++23', 'buildtype=plain'])

# dependency method order: pkgconf -> cmake -> wrap
sdl3_dep = dependency('sdl3')
sdl3_image_dep = dependency('sdl3_image')
fmt_dep = dependency('fmt')
imgui_dep = dependency('imgui')

subdir('src')
