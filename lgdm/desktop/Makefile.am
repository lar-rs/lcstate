## Process this file with automake to produce Makefile.in

## Author Sascha.Smolkov

SUBDIRS =


AM_CPPFLAGS = -Ilib \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-I/usr/include/hiredis \
	$(GLIB_CFLAGS)  \
	$(MKT_CFLAGS)

AM_CFLAGS =        \
	-lm            \
	-Wall          \
	$(WARN_CFLAGS) \
	-g

lib_LTLIBRARIES = liblargdm.la


databox_source= \
			gtkdatabox.c\
			gtkdatabox_marshal.c\
			gtkdatabox_scale.c\
			gtkdatabox_graph.c\
			gtkdatabox_xyc_graph.c\
			gtkdatabox_xyyc_graph.c\
 			gtkdatabox_points.c\
            gtkdatabox_lines.c\
            gtkdatabox_bars.c\
            gtkdatabox_offset_bars.c\
            gtkdatabox_regions.c\
			gtkdatabox_markers.c\
            gtkdatabox_cross_simple.c\
            gtkdatabox_grid.c\
			gtkdatabox_ruler.c


databox_header		= \
			gtkdatabox_typedefs.h\
			gtkdatabox.h\
			gtkdatabox_marshal.h\
			gtkdatabox_scale.h\
			gtkdatabox_graph.h\
			gtkdatabox_xyc_graph.h\
			gtkdatabox_xyyc_graph.h\
 			gtkdatabox_points.h\
            gtkdatabox_lines.h\
            gtkdatabox_bars.h\
            gtkdatabox_offset_bars.h\
            gtkdatabox_regions.h\
			gtkdatabox_markers.h\
            gtkdatabox_cross_simple.h\
            gtkdatabox_grid.h\
			gtkdatabox_ruler.h


gtk_312_sources =         \
	gtkactionmuxer.c      \
	gtkactionobservable.c \
	gtkactionobserver.c   \
	gtkmenutracker.c      \
	gtkmenutrackeritem.c  \
	gtkpopoveraccessible.c





EXTRA_DIST = gtkdatabox_marshal.list

BUILT_SOURCES = gtkdatabox_marshal.c gtkdatabox_marshal.h

$(srcdir)/gtkdatabox_marshal.c: $(srcdir)/gtkdatabox_marshal.list
	glib-genmarshal --body --prefix=gtk_databox_marshal --internal $< > $@

$(srcdir)/gtkdatabox_marshal.h: $(srcdir)/gtkdatabox_marshal.list
	glib-genmarshal --header --prefix=gtk_databox_marshal --internal $< > $@

GDBUS_MESSAGES = \
	messages-generated-code.c \
	messages-generated-code.h  \
	$(NULL)



$(GDBUS_MESSAGES) : $(srcdir)/gl-messages-objectmanager.xml
	gdbus-codegen \
		--interface-prefix com.lar.lgdm. \
		--c-namespace lg_messages \
		--c-generate-object-manager \
		--generate-c-code messages-generated-code \
		$< \
		$(NULL)

GDBUS_GENERATED = \
	lgdm-app-generated-code.c \
	lgdm-app-generated-code.h  \
	$(NULL)



$(GDBUS_GENERATED) : $(srcdir)/gl-application-objectmanager.xml
	gdbus-codegen \
		--interface-prefix com.lar.lgdm. \
		--c-namespace Lgdm \
		--c-generate-object-manager \
		--generate-c-code lgdm-app-generated-code \
		$< \
		$(NULL)

GDBUS_DESKTOP_GENERATED = \
	desktop-generated-code.c \
	desktop-generated-code.h \
	$(NULL)



$(GDBUS_DESKTOP_GENERATED) : desktop-objectmanager.xml
	gdbus-codegen \
		--interface-prefix com.lar.lgdm. \
		--c-namespace Desktop \
		--c-generate-object-manager \
		--generate-c-code desktop-generated-code \
		 $< \
		$(NULL)


GDBUS_APP_GENERATED = \
	guiapp-generated-code.c \
	guiapp-generated-code.h \
	$(NULL)



$(GDBUS_APP_GENERATED) : app-interface.xml
	gdbus-codegen                                  \
		--interface-prefix com.lar.tera.           \
		--c-namespace GuiApp                       \
		--generate-c-code guiapp-generated-code   \
		 $< \
		$(NULL)


EXTRA_DIST += \
	gl-application-objectmanager.xml

resource_xml = $(srcdir)/ldm-ui-resources.xml

EXTRA_DIST += \
	$(resource_xml)   \
	$(resource_files)



RESOURCE_GENERATED = \
	ldm-uiresources.h \
	ldm-uiresources.c

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir)/resources --generate-dependencies $(srcdir)/ldm-ui-resources.xml)

ldm-uiresources.h: ldm-ui-resources.xml
	$(AM_V_GEN) glib-compile-resources $(srcdir)/ldm-ui-resources.xml \
		--target=$@ --sourcedir=$(srcdir)/resources --c-name ldm --generate-header --manual-register

ldm-uiresources.c:ldm-uiresources.h ldm-ui-resources.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources $(srcdir)/ldm-ui-resources.xml \
		--target=$@ --sourcedir=$(srcdir)/resources --c-name ldm --generate-source --manual-register





liblargdm_la_SOURCES =          \
	lgdm-app-generated-code.c   \
	guiapp-generated-code.c     \
	desktop-generated-code.c    \
	$(databox_source)           \
	ldm-uiresources.c           \
	gl-application-object.c     \
	gl-time-settings.c          \
	gl-layout.c                 \
	gl-layout-manager.c         \
	gl-docking-plug.c           \
	gl-statistic-dialog.c       \
	gl-string-dialog.c          \
	gl-combo-dialog.c           \
	gl-combo-row.c              \
	gl-spinner-dialog.c         \
	gl-date-dialog.c            \
	gl-date-settings.c          \
	gl-date-complete-dialog.c   \
	gl-interval-dialog.c



liblargdm_la_HEADERS =         \
	lgdm-app-generated-code.h  \
	guiapp-generated-code.h    \
	desktop-generated-code.h   \
	$(databox_header)          \
	ldm-uiresources.h          \
	gl-application-object.h    \
	gl-time-settings.h         \
	gl-layout.h                \
	gl-layout-manager.h        \
	gl-docking-plug.h          \
	gl-statistic-dialog.h      \
	gl-string-dialog.h         \
	gl-spinner-dialog.h        \
	gl-date-settings.h         \
	gl-combo-row.h             \
	gl-combo-dialog.h          \
	gl-date-dialog.h           \
	gl-date-complete-dialog.h  \
	gl-interval-dialog.h       \
	largdm.h




liblargdm_ladir = $(includedir)/$(PACKAGE)

liblargdm_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
liblargdm_la_LIBADD = $(GLIB_LIBS) $(MKT_LIBS)

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA =  largdm.pc

EXTRA_DIST += \
	largdm.pc.in






bin_PROGRAMS = largdm

largdm_LDFLAGS = $(GLIB_LIBS) $(MKT_LIBS) -static liblargdm.la
largdm_LDADD = $(GLIB_LIBS) $(MKT_LIBS)

largdm_SOURCES =             \
	lgdm-ui-collection.c     \
	lgdm-status.c              \
	lgdm-desktop.c             \
	lgdm-desktop-place.c       \
	lgdm-sidebar.c             \
	gl-desktop-action.c      \
	gl-status-action.c       \
	lgdm-app-launcher.c        \
	gl-service-log.c         \
	gl-select-device.c       \
	lgdm-desktop-app.c       \
	row-service.c            \
	row-user.c               \
	row-fake-user.c          \
	largdm-main.c

largladecatalogdir = $(datarootdir)/glade/catalogs

dist_largladecatalog_DATA = \
	gdmcatalog.xml


EXTRA_DIST += \
	lgdm-ui-collection.h       \
	lgdm-desktop-app.h         \
	lgdm-app-launcher.h        \
	lgdm-status.h              \
	lgdm-desktop.h             \
	lgdm-desktop-place.h       \
	lgdm-sidebar.h             \
	gl-service-log.h           \
	gl-select-device.h         \
	gl-desktop-action.h        \
	gl-status-action.h         \
	row-service.h              \
	row-fake-user.h            \
	row-user.h



ldmthemesdir = $(datarootdir)/themes
nobase_ldmthemes_DATA = $(light_thema_files)

ldmiconsdir = $(datarootdir)/themes
nobase_ldmicons_DATA = $(light_icons_files)

gtksettingsdir = $(sysconfdir)/gtk-3.0
dist_gtksettings_DATA = settings.ini

gsettings_SCHEMAS = \
	com.lar.LGDM.gschema.xml

EXTRA_DIST += $(gsettings_SCHEMAS)

@GSETTINGS_RULES@


gschemas.compiled: com.lar.LGDM.gschema.xml \
	$(GLIB_COMPILE_SCHEMAS) .

largdminitdir = $(sysconfdir)/init
dist_largdminit_SCRIPTS = nodm.conf

CLEANFILES=                     \
	$(GDBUS_GENERATED)          \
	$(GDBUS_APP_GENERATED)      \
	$(GDBUS_DESKTOP_GENERATED)  \
	$(RESOURCE_GENERATED)       \
	$(BUILT_SOURCES)

largdmdefaultdir = $(sysconfdir)/default
dist_largdmdefault_DATA = nodm

datadir =  $(datarootdir)
dist_data_DATA = lar-background.png


xsessiondir = /home/lar
xsession_SCRIPTS = .xsession
