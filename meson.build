project('evol-mod-assetmanager', ['c', 'cpp'],
version : '0.1',
default_options : ['warning_level=3', 'c_std=c11', 'default_library=shared'])

src = subproject('evol').get_variable('evolmod_subdir') + '/meson.build'
dst = meson.current_source_dir() + '/import/'
run_command( 'python3', meson.source_root() + '/subprojects/evol/buildscripts/copy.py', src, dst)
subdir('import')

cc = meson.get_compiler('c')
cc_id = cc.get_id()

subproject('evmod_ecs')

mod_src = [
  'src/mod.c',

  # Loaders
  'src/loaders/TextLoader/TextLoader.c',
  'src/loaders/JSONLoader/JSONLoader.c',
  'src/loaders/MeshLoader/MeshLoader.c',
  'src/loaders/ShaderLoader/ShaderLoader.c',
]

mod_incdir = [
  '..',
]

mod_deps = [
  evmod_deps,

  dependency('threads'),

  dependency('assetsys'),
  dependency('evmod_ecs'),
]

if cc_id == 'msvc'
  mod_deps += dependency('shaderc_msvc')
else
  mod_deps += dependency('shaderc_clang')
endif

module = shared_module(
  'evmodassetmanager', mod_src,
  include_directories: mod_incdir,
  dependencies: mod_deps,
)

mod_dep = declare_dependency(
  include_directories: mod_incdir,
)

meson.override_dependency('evmod_assets', mod_dep)
