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
	ultra_vessels_service

ultra_vessels_service_SOURCES =        \
	ultra-vessel-object.c                \
	ultra-dilution-object.c              \
	open-furnace.c                       \
	close-furnace.c                      \
	open-tic.c                           \
	close-tic.c                          \
	ultra-furnace-object.c               \
	ultra-ticport-object.c               \
	vessels-application-object.c         \
	vessels-service.c



EXTRA_DIST =                           \
	ultra-vessel-object.h                \
	ultra-dilution-object.h              \
	open-furnace.h                       \
	close-furnace.h                      \
	open-tic.h                           \
	close-tic.h                          \
	ultra-furnace-object.h               \
	ultra-ticport-object.h               \
	vessels-application-object.h






ultra_vessels_service_LDFLAGS =
ultra_vessels_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.lar.tera.vessels.service
