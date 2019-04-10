/**
 * @defgroup LgdmLibrary
 * @defgroup GlSampleSettings
 * @ingroup  GlSampleSettings
 * @{
 * @file  gl-sample-settings.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_SAMPLE_SETTINGS_H_
#define GL_SAMPLE_SETTINGS_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SAMPLE_SETTINGS    			           (gl_sample_settings_get_type())
#define GL_SAMPLE_SETTINGS(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SAMPLE_SETTINGS, GlSampleSettings))
#define GL_SAMPLE_SETTINGS_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SAMPLE_SETTINGS, GlSampleSettingsClass))
#define GL_IS_SAMPLE_SETTINGS(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SAMPLE_SETTINGS))
#define GL_IS_SAMPLE_SETTINGS_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SAMPLE_SETTINGS))

typedef struct _GlSampleSettings			        GlSampleSettings;
typedef struct _GlSampleSettingsClass		        GlSampleSettingsClass;
typedef struct _GlSampleSettingsPrivate           GlSampleSettingsPrivate;

struct _GlSampleSettingsClass
{
	GlLayoutClass                           parent_class;
};

struct _GlSampleSettings
{
	GlLayout                                parent;
	GlSampleSettingsPrivate                       *priv;
};


GType 		         gl_sample_settings_get_type                ( void );



G_END_DECLS
#endif /* GL_SAMPLE_SETTINGS_H_ */

/** @} */
