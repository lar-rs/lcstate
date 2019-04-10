
include_core = include_directories('.')

gnome = import('gnome')

introspection_sources=[

  gnome.gdbus_codegen (
    'airdlow',
    'airflow.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'WQM'
  ),
  gnome.gdbus_codegen (
    'analogsout',
    'analogsout.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'WQM'
  ),
  gnome.gdbus_codegen (
    'sensor',
    'sensors.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'WQM'
  ),
  gnome.gdbus_codegen (
    'ultra-pc-generated-code',
    'ultra-pc.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'WQM'
  ),
  gnome.gdbus_codegen (
    'ultra-can-generated-code',
    'ultra-can.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'WQM'
  ),
  gnome.gdbus_codegen (
    'ultra-ams-generated-code',
    'ultra-ams.xml',
    interface_prefix:'lar.ams.',
    namespace: 'WQM'
  ),
]
gen_lib = static_library(
  'gen',
  sources: sources,
  include_directories: include_gen,
  dependencies: glib_dep
)

gen_dep = declare_dependency(
	sources:   sources,
	include_directories: include_gen,
	link_with: gen_lib,
  dependencies: glib_dep
)

