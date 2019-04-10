/**
 * file  gl-desktop-action.h object header
 * brief This is LGDM action button object header file.
 * Copyright (C) LAR 2013
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef LGDM_LARAPP_LAUNCHER_H_
#define LGDM_LARAPP_LAUNCHER_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gdesktopappinfo.h>
#include "lgdm-app-generated-code.h"

G_BEGIN_DECLS


#define LGDM_TYPE_APP_LAUNCHER    			     (lgdm_app_launcher_get_type())
#define LGDM_APP_LAUNCHER(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),LGDM_TYPE_APP_LAUNCHER, LgdmAppLauncher))
#define LGDM_APP_LAUNCHER_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,LGDM_TYPE_APP_LAUNCHER, LgdmAppLauncherClass))
#define LGDM_IS_APP_LAUNCHER(obj)		         (G_TYPE_CHECK_INSTANCE_TYPE((obj),LGDM_TYPE_APP_LAUNCHER))
#define LGDM_IS_APP_LAUNCHER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE((klass) ,LGDM_TYPE_APP_LAUNCHER))

typedef struct _LgdmAppLauncher		              LgdmAppLauncher;
typedef struct _LgdmAppLauncherClass		          LgdmAppLauncherClass;
typedef struct _LgdmAppLauncherPrivate              LgdmAppLauncherPrivate;

struct _LgdmAppLauncherClass
{
	LgdmObjectSkeletonClass                     parent_class;
	void                                      (*action_start)       ( LgdmAppLauncher *action );
};

struct _LgdmAppLauncher
{
	LgdmObjectSkeleton                          object;
	LgdmAppLauncherPrivate                       *priv;
};


GType 		         lgdm_app_launcher_get_type                ( void );

gboolean             lgdm_app_launcher_stop                    ( LgdmAppLauncher *launcher );
gboolean             lgdm_app_launcher_start                   ( LgdmAppLauncher *launcher );


GAppInfo*            lgdm_app_launcher_app_info                ( LgdmAppLauncher *launcher );
gboolean             lgdm_app_launcher_is_autostart            ( LgdmAppLauncher *launcher );
gint                 lgdm_app_launcher_level                   ( LgdmAppLauncher *launcher );

const gchar*         lgdm_app_launcher_get_id                  ( LgdmAppLauncher *launcher );



gboolean             lgdm_app_launcher_register_client_dbus_object_manager ( LgdmAppLauncher *launcher , GDBusConnection *connection );

//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* LGDM_LARAPP_LAUNCHER_H_ */

/** @} */
