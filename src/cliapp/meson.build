
config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
configure_file(
  output: 'cli-config.h',
  configuration: config_h,
)

cli_sources = [
  'main.c',
]

cliapp_deps = [
  dependency('glib-2.0'),
]

executable('cliapp', cliapp_sources,
  dependencies: cliapp_deps,
  install: true,
)


