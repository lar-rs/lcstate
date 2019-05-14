## Process this file with automake to produce Makefile.in

## Author Sascha.Smolkov

SUBDIRS = axis-service pumps-service valves-service stirrers-service vessels_service airflow-service sequence-workings-service measurement-service

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


