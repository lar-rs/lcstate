## Process this file with automake to produce Makefile.in

## Author Sascha.Smolkov


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DDATA_DIR=\""$(datadir)"\" \
	$(GLIB_CFLAGS) \
	$(MKT_CFLAGS)

AM_CFLAGS = -Wall   \
	 -g    \
	 -lm


bin_PROGRAMS =          \
	ultra_measurement_service

ultra_measurement_service_SOURCES =          \
	ultraconfig.c                            \
	mkt-process-object.c                     \
	measurement-application-object.c         \
	temperatur-observer.c                    \
	measurement-service.c                    \
	ultra-integration-object.c               \
	ultimate-channel.c                       \
	ultra-channel-object.c                   \
	ultra-channel-diff.c                     \
	ultra-stream-object.c                    \
	prepare-task.c                           \
	analyze-task.c                           \
	psanalyse-task.c                         \
	ultimate-process-object.c                \
	ultimate-process-hold.c                  \
	measurement-process.c                    \
	calibration-process.c                    \
	ultra-control-process.c



EXTRA_DIST =                                 \
	measurement-application-object.h         \
	mkt-process-object.h                     \
	ultra-integration-object.h               \
	ultraconfig.h                            \
	temperatur-observer.h                    \
	ultimate-channel.h                       \
	ultra-channel-object.h                   \
	ultra-channel-diff.h                     \
	ultra-stream-object.h                    \
	prepare-task.h                           \
	analyze-task.h                           \
	psanalyse-task.h                         \
	ultimate-process-object.h                \
	ultimate-process-hold.h                  \
	measurement-process.h                    \
	calibration-process.h                    \
	ultra-control-process.h





ultra_measurement_service_LDFLAGS =
ultra_measurement_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = ultra.measurement.service \

SCHEMAS_CLEAN =  \
	ultra.measurement.gschema.xml.valid


gsettings_SCHEMAS = \
	ultra.measurement.gschema.xml

EXTRA_DIST += $(gsettings_SCHEMAS)

@GSETTINGS_RULES@


gschemas.compiled: $(gsettings_SCHEMAS)
	$(GLIB_COMPILE_SCHEMAS) .



CLEANFILES=  \
	$(SCHEMAS_CLEAN)
