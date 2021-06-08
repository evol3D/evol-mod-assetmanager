project('libshaderc', 'cpp')

fs = import('fs')
cxxc = meson.get_compiler('cpp')

shaderc_inc = include_directories('install/include')

shaderc_dep = declare_dependency(
  dependencies: cxxc.find_library('shaderc_combined', dirs: meson.current_source_dir()/'install/lib'),
  include_directories: shaderc_inc,
)

meson.override_dependency('shaderc', shaderc_dep)
