/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gtk-foobar.h
 * Copyright (C) 2014 doseus <doseus@doseus-ThinkPad-T430s>
 *
 */

#ifndef _GL_APPLICATION_
#define _GL_APPLICATION_

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gtk/gtkx.h>

#include "gl-layout-manager.h"
#include "lgdm-app-generated-code.h"


G_BEGIN_DECLS

#define GL_TYPE_APPLICATION             (gl_application_get_type ())
#define GL_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_APPLICATION, GlApplication))
#define GL_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_APPLICATION, GlApplicationClass))
#define GL_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_APPLICATION))
#define GL_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_APPLICATION))
#define GL_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_APPLICATION, GlApplicationClass))

typedef struct _GlApplicationClass GlApplicationClass;
typedef struct _GlApplication GlApplication;
typedef struct _GlApplicationPrivate GlApplicationPrivate;

struct _GlApplicationClass
{
	GtkApplicationClass parent_class;
	gboolean            (*load_window)        ( GlApplication *self );
	gboolean            (*start)              ( GlApplication *self );
	gboolean            (*hide)               ( GlApplication *self );
	gboolean            (*show)               ( GlApplication *self );
};

struct _GlApplication
{
	GtkApplication parent_instance;

	GlApplicationPrivate *priv;

};

GType                gl_application_get_type                               ( void ) G_GNUC_CONST;
GlApplication*       gl_application_get                                    ( void );
GlApplication*       gl_application_new                                    ( GType app_type, const gchar *application_id , GApplicationFlags flag );
gboolean             gl_application_show                                   ( GlApplication *application );
gboolean             gl_application_hide                                   ( GlApplication *application );
gboolean             gl_application_apply_css                              ( GlApplication *application );
GlLayoutManager*     gl_application_default_layout_manager                 ( void );
LgdmApp*             gl_application_get_gapp                               ( void );
// void                 gl_application_add_watch_client                       ( TeraClientObject *object );

gboolean             gl_application_has_descktop                           ( void );



//Only server side ..





// Client dbus functions
gboolean             gl_application_register_client_dbus_object_manager    ( GDBusConnection *connection );
gboolean             gl_application_server_show                            ( GDBusObjectManager *manager, const gchar *object_path );
gboolean             gl_application_server_hide_all                        ( GDBusObjectManager *manager );
gboolean             gl_application_server_hide                            ( GDBusObjectManager *manager, const gchar *object_path );

/*
gboolean             gl_application_add_main_layout     ( GlApplication *application , GtkWidget *layout  );
gboolean             gl_application_add_layout          ( GlApplication *application , GtkWidget *layout , const gchar *name , const gchar *title  );
gboolean             gl_application_add_sub_layout      ( GlApplication *application , GtkWidget *layout , GtkWidget *sublayout ,  const gchar *name , const gchar *title  );
gboolean             gl_application_remove_layout       ( GlApplication *application  , const gchar *name );
gboolean             gl_application_switch_layout       ( GlApplication *application ,  const gchar *name  ,GtkStackTransitionType transition );
*/

#define UNBIND_ALL(b)do{if(b!=NULL&&G_IS_BINDING(b)){g_object_unref(b);b=NULL;}}while(0)

G_END_DECLS

#endif /* _APPLICATION_H_ */

