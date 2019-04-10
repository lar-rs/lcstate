cc = meson.get_compiler('c')

ultra_sources = [
    'main.c',
    'ultra-state.c',
]


config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', 'ultra')
config_h.set_quoted('VERSION', meson.project_version())
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
configure_file(
  output: 'ultra-config.h',
  configuration: config_h,
)

ultra_deps = [
  glib_dep,
#  libcandbus_dep,
  core_dep
]

gnome = import('gnome')

#can_sources += gnome.compile_resources('can-resources',
#  'can.gresource.xml',
#  c_name: 'can'
#)

executable (
  'ultra',
  ultra_sources,
	dependencies: ultra_deps,
	install:      true
)
