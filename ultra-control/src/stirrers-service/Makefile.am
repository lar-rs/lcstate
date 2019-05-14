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
	ultra_stirrers_service

ultra_stirrers_service_SOURCES =          \
	stirrers-application-object.c         \
	stirrers-service.c 
	


EXTRA_DIST =    \
	stirrers-application-object.h
	
	




ultra_stirrers_service_LDFLAGS = 
ultra_stirrers_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.lar.tera.stirrers.service

