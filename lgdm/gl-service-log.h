/**
 * @defgroup LgdmLibrary
 * @defgroup GlServiceLog
 * @ingroup  GlServiceLog
 * @{
 * @file  lgdm-desktop-place.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_SERVICE_LOG_H_
#define GL_SERVICE_LOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SERVICE_LOG    			    (gl_service_log_get_type())
#define GL_SERVICE_LOG(obj)			        (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SERVICE_LOG, GlServiceLog))
#define GL_SERVICE_LOG_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SERVICE_LOG, GlServiceLogClass))
#define GL_IS_SERVICE_LOG(obj)		            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SERVICE_LOG))
#define GL_IS_SERVICE_LOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SERVICE_LOG))

typedef struct _GlServiceLog			        GlServiceLog;
typedef struct _GlServiceLogClass		        GlServiceLogClass;
typedef struct _GlServiceLogPrivate            GlServiceLogPrivate;

struct _GlServiceLogClass
{
	GtkWindowClass                                  parent_class;
};

struct _GlServiceLog
{
	GtkWindow                                       parent;
	GlServiceLogPrivate                            *priv;
};


GType 		         gl_service_log_get_type                ( void );


void gl_service_log_run  ( GlServiceLog *service_log );
void gl_service_log_copy_can_fail(const gchar *path, GCancellable *cancel, GAsyncReadyCallback callback, gpointer user_data);
void gl_service_log_copy_app_core_dump(const gchar *path, GCancellable *cancel, GAsyncReadyCallback callback, gpointer user_data);

G_END_DECLS
#endif /* GL_SERVICE_LOG_H_ */

/** @} */
