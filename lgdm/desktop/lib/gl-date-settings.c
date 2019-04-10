/*
 * @ingroup GlTimeSettings
 * @{
 * @file  gl-desktop-action.c	LGDM desktop action button
 * @brief LGDM desktop action button.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include <string.h>
#include "../gl-time-settings.h"

#define GETTEXT_PACKAGE "mktbus"
#include <glib/gi18n-lib.h>



//static GlTimeSettings *__gui_process_desktop = NULL;

struct _GlTimeSettingsPrivate
{
	GtkLabel                *action_name;
	GtkImage                *action_icon;
	GtkButton               *start_button;
	GtkButton               *service_button;
	GtkSpinner              *processed_spiner;
	gchar                   *icon_name;
	gchar                   *name;

};


enum {
	GL_TIME_SETTINGS_PROP_NULL,
	GL_TIME_SETTINGS_HOUR,
	GL_TIME_SETTINGS_MINUTES,
	GL_TIME_SETTINGS_SECONDS,
	GL_TIME_SETTINGS_TOTAL_SECONDS

};


enum
{
	GL_TIME_SETTINGS_LAST_SIGNAL
};


static guint gl_time_settings_signals[GL_TIME_SETTINGS_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlTimeSettings, gl_time_settings, GTK_TYPE_BOX);



static void
start_button_clicked_cb  ( GlTimeSettings *action , GtkButton *button )
{
	g_signal_emit(action,gl_time_settings_signals[GL_TIME_SETTINGS_START],0);
}

static void
service_button_clicked_cb ( GlTimeSettings *cation , GtkButton *button )
{
	//g_debug("Service button clicked ");
}

static void
gl_desktop_actiom_start_real  ( GlTimeSettings *action )
{
	//g_debug("Start action ");
}

static void
gl_time_settings_init(GlTimeSettings *desktop)
{
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (GL_IS_TIME_SETTINGS(desktop));
	desktop->priv = gl_time_settings_get_instance_private (desktop);
	gtk_widget_init_template (GTK_WIDGET (desktop));
}



static void
gl_time_settings_finalize (GObject *object)
{
	GlTimeSettings* desktop = GL_TIME_SETTINGS(object);
	if(desktop->priv->name)      g_free(desktop->priv->name);
	if(desktop->priv->icon_name) g_free(desktop->priv->icon_name);
	G_OBJECT_CLASS (gl_time_settings_parent_class)->finalize(object);
}



static void
gl_time_settings_realize_name ( GlTimeSettings* action  )
{
	// TODO: translateion
	gtk_label_set_text(action->priv->action_name,action->priv->name);
}

static void
gl_time_settings_realize_icon ( GlTimeSettings* action  )
{
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GdkPixbuf *buf = gtk_icon_theme_load_icon(theme,action->priv->icon_name,32,GTK_ICON_LOOKUP_NO_SVG,NULL);
	gtk_image_set_from_pixbuf(GTK_IMAGE(action->priv->action_icon),buf);
}

static void
gl_time_settings_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_TIME_SETTINGS(object));
	GlTimeSettings* action = GL_TIME_SETTINGS(object);
	switch (prop_id)
	{
	case GL_TIME_SETTINGS_NAME:
		if(action->priv->name)g_free(action->priv->name);
		action->priv->name = g_value_dup_string(value);
		gl_time_settings_realize_name(action);
		break;
	case GL_TIME_SETTINGS_ICON:
		if(action->priv->icon_name)g_free(action->priv->icon_name);
		action->priv->icon_name = g_value_dup_string(value);
		gl_time_settings_realize_icon(action);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_time_settings_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_TIME_SETTINGS(object));
	GlTimeSettings* desktop = GL_TIME_SETTINGS(object);
	switch (prop_id)
	{
	case GL_TIME_SETTINGS_NAME:
		g_value_set_string(value,desktop->priv->name);
		break;
	case GL_TIME_SETTINGS_ICON:
		g_value_set_string(value,desktop->priv->icon_name);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_time_settings_class_init(GlTimeSettingsClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_time_settings_finalize;
	object_class -> set_property           =  gl_time_settings_set_property;
	object_class -> get_property           =  gl_time_settings_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/time_setting.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, action_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, action_icon);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, start_button);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, service_button);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, processed_spiner);


	gtk_widget_class_bind_template_callback (widget_class, start_button_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, service_button_clicked_cb);

	g_object_class_install_property (object_class,GL_TIME_SETTINGS_HOUR,
			g_param_spec_string  ("hours",
					_("Time settings hours"),
					_("Time settings hours"),
					0.0,24.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_HOUR,
			g_param_spec_string  ("minutes",
					_("Time settings hours"),
					_("Time settings hours"),
					0.0,60.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_HOUR,
			g_param_spec_string  ("seconds",
					_("Time settings hours"),
					_("Time settings hours"),
					0.0,60.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_HOUR,
			g_param_spec_string  ("total-seconds",
					_("Time settings total seconds"),
					_("Time settings total seconds"),
					0.0,86400,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

}

//FIXME : plug in info window (last plugin ) Start last plugin from list..
GtkWidget*
gl_time_settings_new ( const gchar *dbus_name )
{
	GtkWidget *action;
	action   = GTK_WIDGET(g_object_new( GL_TYPE_TIME_SETTINGS,NULL));
	gtk_buildable_set_name(GTK_BUILDABLE(action),dbus_name);
	return     action;
}

/** @} */
