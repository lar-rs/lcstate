
gen_include = include_directories('.')

gnome = import('gnome')

gen_sources=[

  gnome.gdbus_codegen (
    'airflow',
    'airflow.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),
  gnome.gdbus_codegen (
    'analog',
    'analog.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),
 gnome.gdbus_codegen (
    'axis',
    'axis.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),
   gnome.gdbus_codegen (
    'can',
    'can.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),
   gnome.gdbus_codegen (
    'digin',
    'digin.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),
  gnome.gdbus_codegen (
    'furnace',
    'furnace.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),

  gnome.gdbus_codegen (
    'pressure',
    'pressure.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'Wqm'
  ),
  gnome.gdbus_codegen (
    'pump',
    'pump.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'Wqm'
  ),
  gnome.gdbus_codegen (
    'relay',
    'relay.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
  gnome.gdbus_codegen (
    'sensor',
    'sensor.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
   gnome.gdbus_codegen (
    'status',
    'status.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
   gnome.gdbus_codegen (
    'stepper',
    'stepper.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
  gnome.gdbus_codegen (
    'stirrer',
    'stirrer.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
 gnome.gdbus_codegen (
    'stream',
    'stream.xml',
    interface_prefix: 'lar.wqm',
    namespace: 'Wqm'
  ),
  gnome.gdbus_codegen (
    'temperatur',
    'temperatur.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
  gnome.gdbus_codegen (
    'uart',
    'uart.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),
  gnome.gdbus_codegen (
    'valve',
    'valve.xml',
    interface_prefix: 'lar.ams.',
    namespace: 'wqm'
  ),

]
gen_lib = static_library(
  'gen',
  sources: gen_sources,
  include_directories: gen_include,
  dependencies: glib_dep
)

gen_dep = declare_dependency(
	sources:   gen_sources,
	include_directories: gen_include,
	link_with: gen_lib,
  dependencies: glib_dep
)

