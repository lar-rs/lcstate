project('wqm-candbus', 'c',
  version: '0.5.0',
)

i18n = import('i18n')

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('GETTEXT_PACKAGE', 'wqm-candbus')
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
config_h.set_quoted('DBUS_NAME', 'lar.wqm.candbus')

configure_file(
  output: 'ultimate-config.h',
  configuration: config_h,
)
add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

glib_dep = declare_dependency(
  dependencies: [
    dependency('glib-2.0'),
    dependency('gio-2.0'),
    dependency('gio-unix-2.0'),
    dependency('gmodule-2.0'),
    dependency('gobject-2.0')
  ],
  compile_args: [
#    '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_32',
#    '-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_32'
#
#	Herausgenommen von GS: Das Makro '_G_DEFINE_TYPE_EXTENDED_CLASS_INIT', defi-
#	niert in 'gtype.h', ignoriert bei
#	'GLIB_VERSION_MAX_ALLOWED < GLIB_VERSION_2_38' die dateilokale (static) Va-
#	riable 'TypeName##_private_offset'. Diese Variable wird jedoch in
#	'type_name##_get_instance_private' verwendet und führt zu einer fehlerhaften
#	Rückgabe.
  ]
)

subdir('data')
subdir('src')
subdir('po')

meson.add_install_script('build-aux/meson/postinstall.py')
