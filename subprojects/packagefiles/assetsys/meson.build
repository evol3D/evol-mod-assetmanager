project('assetsys', 'c',
  default_options: [
    'default_library=static',
  ],
)

assetsys_include = include_directories('.')

# All other targets should follow the same template
assetsys_src = files('assetsys.c')
assetsys_lib = library('assetsys', assetsys_src, include_directories: assetsys_include)
assetsys_dep = declare_dependency(link_with: assetsys_lib, include_directories: assetsys_include)

if meson.version().version_compare('>= 0.54.0')
  meson.override_dependency('assetsys', assetsys_dep)
endif
