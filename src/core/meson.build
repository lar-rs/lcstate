
include_core = include_directories('.')

gnome = import('gnome')
sources = [
  'market-time.c',
  'mkt-log.c',
  'mkt-value.c',
  'mkt-utils.c',
  'ultra-can-api.c',
  'ultra-can-dbus.c',
  'ultra-ams-dbus.c',
  'ultra-panel-pc.c',
]


core_lib = static_library(
  'statelib',
  sources: sources,
  include_directories: include_core,
  dependencies: glib_dep
)

core_dep = declare_dependency(
	sources:   sources,
	include_directories: include_core,
	link_with: core_lib,
  dependencies: glib_dep
)

