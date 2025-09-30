project('imgui', 'cpp')

# include dir inside the subproject
imgui_include = include_directories('.', 'backends')

# list the core ImGui sources that must be in subprojects/imgui/
imgui_sources = [
  'imgui.cpp',
  'imgui_draw.cpp',
  'imgui_widgets.cpp',
  'imgui_tables.cpp',
  'imgui_demo.cpp',
]

# static library so the parent can link to it
imgui_lib = static_library(
  'imgui',
  imgui_sources,
  include_directories : imgui_include,
  install : false, # no system install needed
)

# provide a dependency object for fallback usage
imgui_dep = declare_dependency(
  link_with : imgui_lib,
  include_directories : imgui_include,
  version : '1.92.3'
)
