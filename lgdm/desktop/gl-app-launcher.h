/**
 * @defgroup LgdmLibrary
 * @defgroup GlAppLauncher
 * @ingroup  GlAppLauncher
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARAPP_LAUNCHER_H_
#define GL_LARAPP_LAUNCHER_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gdesktopappinfo.h>
#include "lgdm-app-generated-code.h"

G_BEGIN_DECLS


#define GL_TYPE_APP_LAUNCHER    			     (gl_app_launcher_get_type())
#define GL_APP_LAUNCHER(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_APP_LAUNCHER, GlAppLauncher))
#define GL_APP_LAUNCHER_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_APP_LAUNCHER, GlAppLauncherClass))
#define GL_IS_APP_LAUNCHER(obj)		         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_APP_LAUNCHER))
#define GL_IS_APP_LAUNCHER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_APP_LAUNCHER))

typedef struct _GlAppLauncher		              GlAppLauncher;
typedef struct _GlAppLauncherClass		          GlAppLauncherClass;
typedef struct _GlAppLauncherPrivate              GlAppLauncherPrivate;

struct _GlAppLauncherClass
{
	LgdmObjectSkeletonClass                     parent_class;
	void                                      (*action_start)       ( GlAppLauncher *action );
};

struct _GlAppLauncher
{
	LgdmObjectSkeleton                          object;
	GlAppLauncherPrivate                       *priv;
};


GType 		         gl_app_launcher_get_type                ( void );

gboolean             gl_app_launcher_stop                    ( GlAppLauncher *launcher );
gboolean             gl_app_launcher_start                   ( GlAppLauncher *launcher );


GAppInfo*            gl_app_launcher_app_info                ( GlAppLauncher *launcher );
gboolean             gl_app_launcher_is_autostart            ( GlAppLauncher *launcher );
gint                 gl_app_launcher_level                   ( GlAppLauncher *launcher );

const gchar*         gl_app_launcher_get_id                  ( GlAppLauncher *launcher );



gboolean             gl_app_launcher_register_client_dbus_object_manager ( GlAppLauncher *launcher , GDBusConnection *connection );

//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARAPP_LAUNCHER_H_ */

/** @} */
