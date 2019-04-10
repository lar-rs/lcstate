/**
 * @defgroup LgdmLibrary
 * @defgroup GlAppInfo
 * @ingroup  GlAppInfo
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARAPP_INFO_H_
#define GL_LARAPP_INFO_H_
#include "lgdm-app-generated-code.h"
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_APP_INFO (gl_app_info_get_type())
#define GL_APP_INFO(obj)                                                       \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_APP_INFO, GlAppInfo))
#define GL_APP_INFO_CLASS(klass)                                               \
  (G_TYPE_CHECK_CLASS_CAST((klass), GL_TYPE_APP_INFO, GlAppInfoClass))
#define GL_IS_APP_INFO(obj)                                                    \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GL_TYPE_APP_INFO))
#define GL_IS_APP_INFO_CLASS(klass)                                            \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GL_TYPE_APP_INFO))

typedef struct _GlAppInfo GlAppInfo;
typedef struct _GlAppInfoClass GlAppInfoClass;
typedef struct _GlAppInfoPrivate GlAppInfoPrivate;

struct _GlAppInfoClass {
  GObjectClass parent_class;
  void (*action_start)(GlAppInfo *action);
};

struct _GlAppInfo {
  GObject object;
  GlAppInfoPrivate *priv;
};

GType gl_app_info_get_type(void);

gboolean gl_app_info_stop(GlAppInfo *info);
gboolean gl_app_info_start(GlAppInfo *info);

GAppInfo *gl_app_info_app_info(GlAppInfo *info);

const gchar *gl_app_info_get_id(GlAppInfo *info);

gboolean
gl_app_info_register_client_dbus_object_manager(GlAppInfo *info,
                                                GDBusConnection *connection);

GList *gl_app_info_all(GlAppInfo *info);

//---------------------------info
//------------------------------------------------------------------

//-------------------------------progress bar control
//----------------------------------------------

G_END_DECLS
#endif /* GL_LARAPP_INFO_H_ */

/** @} */
