## Process this file with automake to produce Makefile.in

## Author Sascha.Smolkov


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DDATA_DIR=\""$(datadir)"\" \
	$(GLIB_CFLAGS) \
	$(MKT_CFLAGS)

AM_CFLAGS =$(WARN_CFLAGS) \
	 -Wall\
	 -g   \
	 -lm


bin_PROGRAMS =          \
	ultra_sequence_workers_service

ultra_sequence_workers_service_SOURCES =    \
	sequence-process.c                      \
	ultra-sequence-process.c                \
	ultra-axishold-process.c                \
	ultra-allhold-process.c                 \
	ultra-injection-process.c               \
	ultra-injectiontic-process.c            \
	ultra-injectioncod-process.c            \
	ultra-rinsing-process.c                 \
	ultra-sampling-process.c                \
	ultra-samplingcod-process.c             \
	ultra-dilution-process.c                \
	sequence-workers-application-object.c   \
	sequence-workers-service.c



EXTRA_DIST =                                \
	sequence-process.h                      \
	ultra-sequence-process.h                \
	ultra-axishold-process.h                \
	ultra-allhold-process.h                 \
	ultra-injection-process.h               \
	ultra-injectiontic-process.h            \
	ultra-injectioncod-process.h            \
	ultra-rinsing-process.h                 \
	ultra-sampling-process.h                \
	ultra-samplingcod-process.h             \
	ultra-dilution-process.h                \
	sequence-workers-application-object.h






ultra_sequence_workers_service_LDFLAGS =
ultra_sequence_workers_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.lar.tera.sequence.workers.service
