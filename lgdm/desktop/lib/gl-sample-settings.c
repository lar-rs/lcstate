/*
 * @ingroup GlSampleSettings
 * @{
 * @file  gl-sample-settings.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-sample-settings.h"

#define GETTEXT_PACKAGE "GlSampleSettings"
#include <glib/gi18n-lib.h>



//static GlSampleSettings *__gui_process_desktop = NULL;

struct _GlSampleSettingsPrivate
{
	gboolean example;
};


enum {
	GL_SAMPLE_SETTINGS_PROP_NULL,
};


enum
{
	GL_SAMPLE_SETTINGS_LAST_SIGNAL
};


//static guint gl_sample_settings_signals[GL_SAMPLE_SETTINGS_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlSampleSettings, gl_sample_settings, GL_TYPE_LAYOUT);

static void
gl_sample_settings_init(GlSampleSettings *gl_sample_settings)
{
	g_return_if_fail (gl_sample_settings != NULL);
	g_return_if_fail (GL_IS_SAMPLE_SETTINGS(gl_sample_settings));
	gl_sample_settings->priv = gl_sample_settings_get_instance_private (gl_sample_settings);
	gtk_widget_init_template (GTK_WIDGET (gl_sample_settings));
}



static void
gl_sample_settings_finalize (GObject *object)
{
	//GlSampleSettings* gl_sample_settings = GL_SAMPLE_SETTINGS(object);
	G_OBJECT_CLASS (gl_sample_settings_parent_class)->finalize(object);
}





static void
gl_sample_settings_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SAMPLE_SETTINGS(object));
	//GlSampleSettings* gl_sample_settings = GL_SAMPLE_SETTINGS(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_sample_settings_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SAMPLE_SETTINGS(object));
	//GlSampleSettings* gl_sample_settings = GL_SAMPLE_SETTINGS(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_sample_settings_class_init(GlSampleSettingsClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_sample_settings_finalize;
	object_class -> set_property           =  gl_sample_settings_set_property;
	object_class -> get_property           =  gl_sample_settings_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/sample-settings.ui");
	/*
	gtk_widget_class_bind_template_child_private (widget_class, GlSampleSettings, example_object);
	gtk_widget_class_bind_template_callback (widget_class, example_signal_callback);*/


}

/** @} */
