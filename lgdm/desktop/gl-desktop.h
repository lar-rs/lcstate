/**
 * @defgroup LgdmLibrary
 * @defgroup GlDesktop
 * @ingroup  GlDesktop
 * @{
 * @file  gl-desktop.h object header
 * @brief This is LGDM desktop object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARDESKTOP_H_
#define GL_LARDESKTOP_H_
#include "gl-desktop-place.h"
#include "gl-sidebar.h"
#include "gl-status.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>

G_BEGIN_DECLS

#define GL_TYPE_DESKTOP (gl_desktop_get_type())
#define GL_DESKTOP(obj)                                                        \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_DESKTOP, GlDesktop))
#define GL_DESKTOP_CLASS(klass)                                                \
  (G_TYPE_CHECK_CLASS_CAST((klass), GL_TYPE_DESKTOP, GlDesktopClass))
#define GL_IS_DESKTOP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GL_TYPE_DESKTOP))
#define GL_IS_DESKTOP_CLASS(klass)                                             \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GL_TYPE_DESKTOP))

typedef struct _GlDesktop GlDesktop;
typedef struct _GlDesktopClass GlDesktopClass;
typedef struct _GlDesktopPrivate GlDesktopPrivate;

struct _GlDesktopClass {
  GtkWindowClass parent_class;
};

struct _GlDesktop {
  GtkWindow desktop;
  GlDesktopPrivate *priv;
};

GType gl_desktop_get_type(void);
void gl_desktop_local_init(void);
GlDesktop *gl_desktop_local_get(void);
GList *gl_desktop_launcher(void);
gboolean gl_desktop_local_show_app(const gchar *name);

void gl_desktop_local_add_app(GtkWidget *app, const gchar *name);
void gl_desktop_local_remove_app(const gchar *name);

GlStatus *gl_desktop_local_get_status(void);
GlSidebar *gl_desktop_local_get_sidebar(void);
GlDesktopPlace *gl_desktop_local_get_action_place(void);

//---------------------------info
//------------------------------------------------------------------

//-------------------------------progress bar control
//----------------------------------------------

G_END_DECLS
#endif /* GL_LARDESKTOP_H_ */

/** @} */
