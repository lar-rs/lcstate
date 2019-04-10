/*
 * gl-updade.h
 *
 *  Created on: 22.02.2013
 *      Author: sascha
 */

#ifndef GL_DATE_TIME_WINDOW_H_
#define GL_DATE_TIME_WINDOW_H_

#include "mkt-window.h"

G_BEGIN_DECLS

#define GL_TYPE_DATE_TIME_WINDOW                 (gl_date_time_window_get_type ())
#define GL_DATE_TIME_WINDOW(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_DATE_TIME_WINDOW,  GlDateTimeWindow))
#define GL_DATE_TIME_WINDOW_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_DATE_TIME_WINDOW,  GlDateTimeWindowClass))
#define GL_IS_DATE_TIME_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_DATE_TIME_WINDOW))
#define GL_IS_DATE_TIME_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_DATE_TIME_WINDOW))
#define GL_DATE_TIME_WINDOW_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_DATE_TIME_WINDOW,  GlDateTimeWindowClass))

typedef struct _GlDateTimeWindowClass   GlDateTimeWindowClass;
typedef struct _GlDateTimeWindow        GlDateTimeWindow;
typedef struct _GlDateTimeWindowPrivate GlDateTimeWindowPrivate;


struct _GlDateTimeWindowClass
{
	MktWindowClass                 parent_class;
	void                         (*changet_date_time)      ( GlDateTimeWindow *dt );
};

struct _GlDateTimeWindow
{
	MktWindow                      parent_instance;
	GlDateTimeWindowPrivate               *priv;
};

GType                        gl_date_time_window_get_type        ( void ) G_GNUC_CONST;

gdouble                      gl_date_time_window_get_from        ( GlDateTimeWindow *dt );
gdouble                      gl_date_time_window_get_to          ( GlDateTimeWindow *dt );
gboolean                     gl_date_time_window_is_set          ( GlDateTimeWindow *dt );
void                         gl_date_time_set_current            ( GlDateTimeWindow *dt );


#endif /* GL_UPDADE_H_ */
