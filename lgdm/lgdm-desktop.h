/**
 * file  lgdm-desktop.h object header
 * Copyright (C) LAR 2013-2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef LGDM_LARDESKTOP_H_
#define LGDM_LARDESKTOP_H_
#include "lgdm-desktop-place.h"
#include "lgdm-sidebar.h"
#include "lgdm-status.h"
#include "lgdm-state.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>

G_BEGIN_DECLS

#define LGDM_TYPE_DESKTOP (lgdm_desktop_get_type())
#define LGDM_DESKTOP(obj)                                                        \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), LGDM_TYPE_DESKTOP, LgdmDesktop))
#define LGDM_DESKTOP_CLASS(klass)                                                \
  (G_TYPE_CHECK_CLASS_CAST((klass), LGDM_TYPE_DESKTOP, LgdmDesktopClass))
#define LGDM_IS_DESKTOP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), LGDM_TYPE_DESKTOP))
#define LGDM_IS_DESKTOP_CLASS(klass)                                             \
  (G_TYPE_CHECK_CLASS_TYPE((klass), LGDM_TYPE_DESKTOP))

typedef struct _LgdmDesktop LgdmDesktop;
typedef struct _LgdmDesktopClass LgdmDesktopClass;
typedef struct _LgdmDesktopPrivate LgdmDesktopPrivate;

struct _LgdmDesktopClass {
  GtkWindowClass parent_class;
};

struct _LgdmDesktop {
  GtkWindow desktop;
  LgdmDesktopPrivate *priv;
};

GType lgdm_desktop_get_type(void);
void lgdm_desktop_local_init(void);
LgdmDesktop *lgdm_desktop_local_get(void);
GList *lgdm_desktop_launcher(void);
gboolean lgdm_desktop_local_show_app(const gchar *name);

void lgdm_desktop_local_add_app(GtkWidget *app, const gchar *name);
void lgdm_desktop_local_remove_app(const gchar *name);

LgdmStatus *lgdm_desktop_local_get_status(void);
LgdmSidebar *lgdm_desktop_local_get_sidebar(void);
LgdmDesktopPlace *lgdm_desktop_local_get_action_place(void);


void  lgdm_desktop_system_error();
void  lgdm_desktop_session_error();


G_END_DECLS
#endif
