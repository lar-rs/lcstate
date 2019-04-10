/*
 * @ingroup GlDesktopAction
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



#include "gl-desktop-action.h"
#include "lgdm-app-launcher.h"

#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>



//static GlDesktopAction *__gui_process_desktop = NULL;

struct _GlDesktopActionPrivate
{
	LgdmAppLauncher* launcher;
	GtkLabel*        action_name;
	GtkImage*        action_icon;
	GtkButton*       start_button;
	GtkSpinner*      processed_spiner;

};


enum {
	GL_DESKTOP_ACTION_PROP_NULL,
	GL_DESKTOP_ACTION_LAUNCHER,

};


enum
{
	GL_DESKTOP_ACTION_START,
	GL_DESKTOP_ACTION_LAST_SIGNAL
};


static guint gl_desktop_action_signals[GL_DESKTOP_ACTION_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlDesktopAction, gl_desktop_action, GTK_TYPE_BOX);



static void start_button_clicked_cb (GlDesktopAction* action, GtkButton* button)
{
	g_signal_emit           (action, gl_desktop_action_signals[GL_DESKTOP_ACTION_START], 0);
	lgdm_app_launcher_start (action -> priv -> launcher);
}


static void
gl_desktop_actiom_start_real  ( GlDesktopAction *action )
{
}

static void
gl_desktop_action_init(GlDesktopAction *desktop)
{
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (GL_IS_DESKTOP_ACTION(desktop));
	desktop->priv = gl_desktop_action_get_instance_private (desktop);
	gtk_widget_init_template (GTK_WIDGET (desktop));
}



static void
gl_desktop_action_finalize (GObject *object)
{
	GlDesktopAction* desktop = GL_DESKTOP_ACTION(object);
	if(desktop->priv->launcher)      g_object_unref(desktop->priv->launcher);
	G_OBJECT_CLASS (gl_desktop_action_parent_class)->finalize(object);
}



static void
gl_desktop_action_realize_name ( GlDesktopAction* action  )
{
	// TODO: translateion
	gtk_label_set_text(action->priv->action_name,g_app_info_get_name(lgdm_app_launcher_app_info(action->priv->launcher)));
}

static void
gl_desktop_action_realize_icon ( GlDesktopAction* action  )
{

	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GIcon *icon = g_app_info_get_icon(lgdm_app_launcher_app_info(action->priv->launcher));

	 GtkIconInfo *info = gtk_icon_theme_lookup_by_gicon(theme,icon,48,GTK_ICON_LOOKUP_FORCE_SVG);
	 if(info)
	 {
		 GdkPixbuf *buf =gtk_icon_info_load_icon(info,NULL);
		 gtk_image_set_from_pixbuf(GTK_IMAGE(action->priv->action_icon),buf);
		 g_object_unref(info);
		 g_object_unref(buf);
	 }
	g_object_unref (icon);
}

static void gl_desktop_action_set_property (GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
	g_return_if_fail (GL_IS_DESKTOP_ACTION(object));

	GlDesktopAction* action = GL_DESKTOP_ACTION(object);

	switch (prop_id) {
		case GL_DESKTOP_ACTION_LAUNCHER:
			if (action->priv->launcher)
				g_object_unref(action->priv->launcher);

			action -> priv -> launcher = g_value_dup_object (value);

			gl_desktop_action_realize_name (action);
			gl_desktop_action_realize_icon (action);

			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gl_desktop_action_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_DESKTOP_ACTION(object));
	GlDesktopAction* action = GL_DESKTOP_ACTION(object);
	switch (prop_id)
	{
	case GL_DESKTOP_ACTION_LAUNCHER:
		g_value_set_object(value,action->priv->launcher);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void gl_desktop_action_class_init (GlDesktopActionClass* klass)
{
	GObjectClass*   object_class = G_OBJECT_CLASS   (klass);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

	object_class -> finalize     = gl_desktop_action_finalize;
	object_class -> set_property = gl_desktop_action_set_property;
	object_class -> get_property = gl_desktop_action_get_property;
	klass        -> action_start = gl_desktop_actiom_start_real;

	gtk_widget_class_set_template_from_resource (widget_class, "/lgdm/ui/layout/desktop-action.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GlDesktopAction, action_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlDesktopAction, action_icon);
	gtk_widget_class_bind_template_child_private (widget_class, GlDesktopAction, start_button);
	gtk_widget_class_bind_template_child_private (widget_class, GlDesktopAction, processed_spiner);

	gtk_widget_class_bind_template_callback (widget_class, start_button_clicked_cb);

	g_object_class_install_property (
		object_class,
		GL_DESKTOP_ACTION_LAUNCHER,
		g_param_spec_object (
			"action-launcher",
			"Desktop action button icon",
			"Desktop action button icon",
			LGDM_TYPE_APP_LAUNCHER,
			G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY
		)
	);

	gl_desktop_action_signals [GL_DESKTOP_ACTION_START] = g_signal_new (
		"action-start",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (GlDesktopActionClass, action_start),
		NULL,
		NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE,
		0
	);
}

//FIXME : plug in info window (last plugin ) Start last plugin from list..
GtkWidget*
gl_desktop_action_new ( const gchar *dbus_name )
{
	GtkWidget *action;
	action   = GTK_WIDGET(g_object_new( GL_TYPE_DESKTOP_ACTION,NULL));
	gtk_buildable_set_name(GTK_BUILDABLE(action),dbus_name);
	return     action;
}

/** @} */
