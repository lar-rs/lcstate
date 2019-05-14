#
#
#
# @author ASmolkov

SUBDIRS = eds dbus-access

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-I/usr/include/hiredis \
	$(GLIB_CFLAGS)  \
	$(MKT_CFLAGS)


AM_CFLAGS =         \
	 $(WARN_CFLAGS) \
	 -DSYSCONFDIR='"$(sysconfdir)"'  \
	 -g             \
	 -ldl           \
	 -lrt           \
	 -lm


EXTRA_DIST = \
	can4linux.h                \
	node-control-app-object.h  \
	node-analogext-object.h    \
	node-analog-object.h       \
	node-device-object.h       \
	node-digital-object.h      \
	node-index.h               \
	node-index-object.h        \
	node-motor-object.h        \
	node-motor3-object.h       \
	node-object.h


bin_PROGRAMS = candaemon
candaemon_LDFLAGS =
candaemon_LDADD = $(GLIB_LIBS) $(MKT_LIBS)

candaemon_SOURCES =             \
	node-device-object.c        \
	node-control-app-object.c   \
	node-digital-object.c       \
	node-analog-object.c        \
	node-analogext-object.c     \
	node-motor-object.c         \
	node-object.c               \
	node-index.c                \
	node-index-object.c         \
	node-motor3-object.c        \
	node-bus.c



dbusservicedir   = $(datadir)/dbus-1/system-services
dbusservice_DATA = com.lar.service.can.service


caninitdir = $(sysconfdir)/init
dist_caninit_SCRIPTS = candaemon.conf

