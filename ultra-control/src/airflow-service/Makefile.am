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
	ultra_airflow_service

ultra_airflow_service_SOURCES =       \
	ultra-airflow-object.c               \
	airflow-application-object.c      \
	airflow-service.c
	


EXTRA_DIST =                             \
	ultra-airflow-object.h               \
	airflow-application-object.h
	
	




ultra_airflow_service_LDFLAGS = 
ultra_airflow_service_LDADD = $(MKT_LIBS)  $(GLIB_LIBS)

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = com.lar.tera.airflow.service

gsettings_SCHEMAS = \
	com.lar.tera.airflow.gschema.xml
		
SCHEMAS_CLEAN =  \
	com.lar.tera.airflow.gschema.xml.valid

EXTRA_DIST += $(gsettings_SCHEMAS)

@GSETTINGS_RULES@

gschemas.compiled: $(gsettings_SCHEMAS)
	$(GLIB_COMPILE_SCHEMAS) .
	
	
CLEANFILES=  \
	$(SCHEMAS_CLEAN)