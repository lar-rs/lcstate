lgdm_sources = [
  'main.c',
  'lgdm-state.c',
  'lgdm-desktop.c',
  'lgdm-app-launcher.c',
  'lgdm-desktop-place.c',
  'gl-desktop-action.c',
  'lgdm-status.c',
  'lgdm-sidebar.c',
  'gl-status-action.c',
  'gl-string-dialog.c',
]

lgdm_sources +=  gnome.gdbus_codegen (
    'lgdm-state-generated-code',
    'lgdm-state.xml',
    interface_prefix: 'lar.ams.lgdm.',
    namespace: 'Lgdm'
)
lgdm_sources +=  gnome.gdbus_codegen (
    'lgdm-app-generated-code',
    'lgdm-app.xml',
    interface_prefix: 'lar.ams.lgdm.',
    object_manager: true,
    namespace: 'Lgdm'
)
# lgdm_sources +=  gnome.gdbus_codegen (
#     'lgdm-application-generated-code',
#     'lgdm-application.xml',
#     interface_prefix: 'lar.ams.lgdm.',
#     object_manager: true,
#     namespace: 'Lgdm'
# )
lgdm_sources += gnome.compile_resources('lgdm-resources',
    'lgdm.resources.xml',
    c_name: 'lgdm'
)

lgdm_dep = [
  dependency('gio-2.0'),
  dependency('gtk+-3.0'),
  core_dep,
]

gnome = import('gnome')



executable('lgdm',
  lgdm_sources,
  dependencies: lgdm_dep,
  install: true,
)
