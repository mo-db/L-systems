project('lsys', 'c', 'cpp', 
  default_options : ['c_std=c11', 'cpp_std=c++23', 'buildtype=plain'])

# # top-level meson.build snippet
# imgui_inc = include_directories('subprojects/imgui')
#
# imgui_sources = [
#   'subprojects/imgui/imgui.cpp',
#   'subprojects/imgui/imgui_draw.cpp',
#   'subprojects/imgui/imgui_widgets.cpp',
#   'subprojects/imgui/imgui_tables.cpp',
#   'subprojects/imgui/imgui_demo.cpp',
#   # 'src/imgui_impl_sdl3.cpp',          # your copy of the backend
#   # 'src/imgui_impl_sdlrenderer3.cpp',  # your renderer backend
# ]

# dependency method order: pkgconf -> cmake -> wrap
sdl3_dep = dependency('sdl3')
sdl3_image_dep = dependency('sdl3_image')
fmt_dep = dependency('fmt')
# imgui_dep = dependency('imgui')
liblo_dep = dependency('liblo')

# ask for imgui, but fall back to our subproject (subprojects/imgui/meson.build)
imgui_dep = dependency('imgui', fallback: ['imgui', 'imgui_dep'])

subdir('src')
