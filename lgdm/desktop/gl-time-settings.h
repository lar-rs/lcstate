/**
 * @defgroup LgdmLibrary
 * @defgroup GlTimeSettings
 * @ingroup  GlTimeSettings
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARTIME_SETTINGS_H_
#define GL_LARTIME_SETTINGS_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_TIME_SETTINGS    			     (gl_time_settings_get_type())
#define GL_TIME_SETTINGS(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_TIME_SETTINGS, GlTimeSettings))
#define GL_TIME_SETTINGS_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_TIME_SETTINGS, GlTimeSettingsClass))
#define GL_IS_TIME_SETTINGS(obj)		         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_TIME_SETTINGS))
#define GL_IS_TIME_SETTINGS_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_TIME_SETTINGS))

typedef struct _GlTimeSettings			          GlTimeSettings;
typedef struct _GlTimeSettingsClass		      GlTimeSettingsClass;
typedef struct _GlTimeSettingsPrivate            GlTimeSettingsPrivate;

struct _GlTimeSettingsClass
{
	GtkWindowClass                      parent_class;
};

struct _GlTimeSettings
{
	GtkWindow                           action;
	GlTimeSettingsPrivate              *priv;
};


GType 		         gl_time_settings_get_type                ( void );
GtkWidget*  	     gl_time_settings_new                     (  );
void                 gl_time_settings_start                   ( GlTimeSettings *time );
void                 gl_time_settings_stop                    ( GlTimeSettings *time );

void                 gl_time_settings_set_hour_interval       ( GlTimeSettings *time , gdouble min , gdouble max );
gdouble              gl_time_settings_get_total_seconds       ( GlTimeSettings *time );




//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARTIME_SETTINGS_H_ */

/** @} */
