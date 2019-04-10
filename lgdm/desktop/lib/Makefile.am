#
#
#
# @author ASmolkov


AM_CPPFLAGS = -Idatabox   \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GLIB_CFLAGS)  \
	$(MKT_CFLAGS)
	


AM_CFLAGS =\
	 -Wall\
	 -g   \
	 -ldl \
	 -lrt \
	 -lm	

lib_LTLIBRARIES = liblargdm.la

GDBUS_GENERATED = \
	lgdm-app-generated-code.c \
	lgdm-app-generated-code.h  \
	lgdm-app-generated-docs-com.lar.GDM.ObjectManager.App.xml    \
	lgdm-app-generated-docs-com.lar.GDM.ObjectManager.Settings.xml	\
	$(NULL)

$(GDBUS_GENERATED) : gl-application-objectmanager.xml
	gdbus-codegen \
		--interface-prefix com.lar.GDM.ObjectManager. \
		--c-namespace Gldm \
		--c-generate-object-manager \
		--generate-c-code lgdm-app-generated-code \
		--generate-docbook lgdm-app-generated-docs \
		$< \
		$(NULL)


EXTRA_DIST = gl-application-objectmanager.xml


liblargdm_la_SOURCES =       \
	gl-model.c               \
	gl-window-main.c         \
	lgdm-app-generated-code.c   \
	gl-application-object.c  \
	gl-time-settings.c       \
	gl-status.c              \
	gl-desktop.c             \
	gl-sidebar.c             \
	gl-desktop-action.c      \
	lgdm-app-launcher.c        \
	gl-app-desktop.c         \
	gl-layout.c              \
	gl-layout-manager.c      \
	gl-statistic-dialog.c    \
	gl-string-dialog.c       \
	gl-spinner-dialog.c      \
	gl-app-object.c          \
	gl-action-Screenshot.c  \
	gl-action-widget.c  \
	gl-connection.c  \
	gl-controlbox.c   \
	gl-date-time-window.c  \
	gl-extern-process.c   \
	gl-indicate.c   \
	gl-layout.c   \
	gl-level-manager.c  \
	gl-level-notebook.c   \
	gl-log.c   \
	gl-model.c   \
	gl-module.c   \
	gl-plugin.c   \
	gl-string.c   \
	gl-saved.c   \
	gl-saved-object.c  \
	gl-status.c    \
	gl-translation.c   \
	gl-tree-data.c    \
	gl-update.c    \
	gl-user-question.c   \
	gl-widget-option.c    \
	gl-wizard.c   \
	gl-wizard-manager.c   \
	gl-xkbd.c   \
	gtk_larprocessbar.c  \
	gui-init.c     \
	gl-application.c   \
	mkt-module-manager.c   \
	mkt-window.c    \
	mkt-window-manager.c  \
	extra/gtkbordercombo.c  \
	extra/gtkcharsel.c   \
	extra/gtkcheckitem.c  \
	extra/gtkcolorcombo.c  \
	extra/gtkcombobutton.c   \
	extra/gtkextra.c   \
	extra/gtkextra-marshal.c  \
	extra/gtkextratypebuiltins.c  \
	extra/gtkfontcombo.c  \
	extra/gtkplot3d.c   \
	extra/gtkplotarray.c   \
	extra/gtkplotbar.c   \
	extra/gtkplotbox.c   \
	extra/gtkplotbubble.c   \
	extra/gtkplot.c   \
	extra/gtkplotcandle.c   \
	extra/gtkplotcanvas.c   \
	extra/gtkplotcanvasellipse.c   \
	extra/gtkplotcanvasline.c   \
	extra/gtkplotcanvaspixmap.c   \
	extra/gtkplotcanvasplot.c   \
	extra/gtkplotcanvasrectangle.c   \
	extra/gtkplotcanvastext.c   \
	extra/gtkplotcsurface.c   \
	extra/gtkplotdata.c   \
	extra/gtkplotdt.c   \
	extra/gtkplotflux.c   \
	extra/gtkplotgdk.c   \
	extra/gtkplotpc.c   \
	extra/gtkplotpixmap.c   \
	extra/gtkplotpolar.c   \
	extra/gtkplotprint.c   \
	extra/gtkplotps.c   \
	extra/gtkplotsegment.c   \
	extra/gtkplotsurface.c   \
	extra/gtkpsfont.c   \
	extra/gtktogglecombo.c   \
	databox/gtkdatabox_bars.c   \
	databox/gtkdatabox.c  \
	databox/gtkdatabox_cross_simple.c  \
	databox/gtkdatabox_graph.c  \
	databox/gtkdatabox_grid.c  \
	databox/gtkdatabox_lines.c   \
	databox/gtkdatabox_markers.c       \
	databox/gtkdatabox_marshal.c       \
	databox/gtkdatabox_offset_bars.c       \
	databox/gtkdatabox_points.c       \
	databox/gtkdatabox_regions.c       \
	databox/gtkdatabox_ruler.c       \
	databox/gtkdatabox_scale.c       \
	databox/gtkdatabox_xyc_graph.c       \
	databox/gtkdatabox_xyyc_graph.c
	

	
liblargdm_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
liblargdm_la_LIBADD =$(GLIB_LIBS) $(MKT_LIBS)

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA =  largdm.pc

EXTRA_DIST += \
	largdm.pc.in

largdmincludedir = $(includedir)/$(PACKAGE)
largdminclude_HEADERS =      \
	gl-model.h               \
	gl-window-main.h         \
	gl-application-object.h  \
	lgdm-app-generated-code.h  \
	gl-time-settings.h       \
	lgdm-status.h              \
	lgdm-desktop.h             \
	lgdm-sidebar.h             \
	gl-desktop-action.h      \
	lgdm-app-launcher.h        \
	gl-app-desktop.h         \
	gl-layout.h              \
	gl-layout-manager.h      \
	gl-statistic-dialog.h    \
	gl-string-dialog.h       \
	gl-spinner-dialog.h      \
	largdm.h

	
	
#	gl-action-LarKey.h             \
	gl-action-Screenshot.h             \
	gl-action-widget.h             \
	gl-connection.h             \
	gl-controlbox.h             \
	gl-date-time-window.h             \
	gl-extern-process.h             \
	gl-indicate.h             \
	gl-layout.h             \
	gl-level-manager.h             \
	gl-level-notebook.h             \
	gllib.h             \
	gl-log.h             \
	gl-manager.h             \
	gl-market-items.h             \
	gl-model.h             \
	gl-module.h             \
	gl-plot.h             \
	gl-plot-tab.h             \
	gl-plugin.h             \
	gl-plugin-manager.h             \
	gl-private.h             \
	gl-saved.h             \
	gl-saved-object.h             \
	lgdm-status.h             \
	gl-string.h             \
	gl-translation.h             \
	gl-tree-data.h             \
	gl-update.h             \
	gl-user-question.h             \
	gl-widget-option.h             \
	gl-wizard.h             \
	gl-wizard-manager.h             \
	gl-xkbd.h             \
	gtkdatabox.h             \
	gtkextra.h             \
	gtk_larprocessbar.h             \
	mkt-lib.h             \
	mkt-module-manager.h             \
	mkt-window.h             \
	mkt-window-manager.h     \
	gl-application.h



databoxdir = $(largdmincludedir)/databox

databox_HEADERS =

#	databox/gtkdatabox_bars.h         \
	databox/gtkdatabox_cross_simple.h \
	databox/gtkdatabox_graph.h         \
	databox/gtkdatabox_grid.h         \
	databox/gtkdatabox.h         \
	databox/gtkdatabox_lines.h         \
	databox/gtkdatabox_markers.h         \
	databox/gtkdatabox_marshal.h         \
	databox/gtkdatabox_offset_bars.h         \
	databox/gtkdatabox_points.h         \
	databox/gtkdatabox_regions.h         \
	databox/gtkdatabox_ruler.h         \
	databox/gtkdatabox_scale.h         \
	databox/gtkdatabox_typedefs.h         \
	databox/gtkdatabox_xyc_graph.h         \
	databox/gtkdatabox_xyyc_graph.h

extradir = $(largdmincludedir)/extra
	
extra_HEADERS =


#	extra/gtkbordercombo.h           \
	extra/gtkcharsel.h           \
	extra/gtkcheckitem.h           \
	extra/gtkcolorcombo.h           \
	extra/gtkcombobutton.h           \
	extra/gtkextrafeatures.h           \
	extra/gtkextraicons.h           \
	extra/gtkextra-marshal.h           \
	extra/gtkextratypebuiltins.h           \
	extra/gtkfontcombo.h           \
	extra/gtkiconlist.h           \
	extra/gtkplot3d.h           \
	extra/gtkplotarray.h           \
	extra/gtkplotbar.h           \
	extra/gtkplotbox.h           \
	extra/gtkplotbubble.h           \
	extra/gtkplotcandle.h           \
	extra/gtkplotcanvasellipse.h           \
	extra/gtkplotcanvas.h           \
	extra/gtkplotcanvasline.h           \
	extra/gtkplotcanvaspixmap.h           \
	extra/gtkplotcanvasplot.h           \
	extra/gtkplotcanvasrectangle.h           \
	extra/gtkplotcanvastext.h           \
	extra/gtkplotcsurface.h           \
	extra/gtkplotdata.h           \
	extra/gtkplotdt.h           \
	extra/gtkplotflux.h           \
	extra/gtkplotgdk.h           \
	extra/gtkplot.h           \
	extra/gtkplotpc.h           \
	extra/gtkplotpixmap.h           \
	extra/gtkplotpolar.h           \
	extra/gtkplotprint.h           \
	extra/gtkplotps.h           \
	extra/gtkplotsegment.h           \
	extra/gtkplotsurface.h           \
	extra/gtkpsfont.h           \
	extra/gtktogglecombo.h           \
	extra/icons.h           \
	extra/pixmaps.h


CLEANFILES=$(GDBUS_GENERATED)
