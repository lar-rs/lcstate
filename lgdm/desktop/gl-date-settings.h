/**
 * @defgroup LgdmLibrary
 * @defgroup GlDateSettings
 * @ingroup  GlDateSettings
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARDATE_SETTINGS_H_
#define GL_LARDATE_SETTINGS_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_DATE_SETTINGS    			      (gl_date_settings_get_type())
#define GL_DATE_SETTINGS(obj)			          (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_DATE_SETTINGS, GlDateSettings))
#define GL_DATE_SETTINGS_CLASS(klass)		      (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_DATE_SETTINGS, GlDateSettingsClass))
#define GL_IS_DATE_SETTINGS(obj)		          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_DATE_SETTINGS))
#define GL_IS_DATE_SETTINGS_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_DATE_SETTINGS))

typedef struct _GlDateSettings			          GlDateSettings;
typedef struct _GlDateSettingsClass		          GlDateSettingsClass;
typedef struct _GlDateSettingsPrivate               GlDateSettingsPrivate;

struct _GlDateSettingsClass
{
	GtkWindowClass                               parent_class;
};

struct _GlDateSettings
{
	GtkWindow                                    action;
	GlDateSettingsPrivate                          *priv;
};


GType 		         gl_date_settings_get_type                ( void );

gdouble              gl_date_settings_get_total_sec           ( GlDateSettings *settings );
void                 gl_date_settings_set_total_sec           ( GlDateSettings *settings , gdouble total_sec);


//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARDATE_SETTINGS_H_ */

/** @} */
