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
	ultra_axis_service

ultra_axis_service_SOURCES =      \
	axis-object.c                   \
	axisZ-object.c                  \
	move-axis.c                     \
	move-object.c                   \
	inj-valve.c                     \
	control-ptp.c                   \
	d3sensor-object.c               \
	d3go-object.c                   \
	ultra-axis.c                    \
	axis-application-object.c       \
	ultra-axis-service.c


EXTRA_DIST =                     \
	axis-object.h                  \
	axisZ-object.h                 \
	move-axis.h                    \
	move-object.h                  \
	d3sensor-object.h              \
	d3go-object.h                  \
	inj-valve.h                    \
	control-ptp.h                  \
	ultra-axis.h                   \
	axis-application-object.h




ultra_axis_service_LDFLAGS =
ultra_axis_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.lar.tera.axis.service
