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
	ultra_valves_service

ultra_valves_service_SOURCES =          \
	valves-application-object.c         \
	valves-service.c 
	


EXTRA_DIST =    \
	valves-application-object.h
	
	




ultra_valves_service_LDFLAGS = 
ultra_valves_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.lar.tera.valves.service

