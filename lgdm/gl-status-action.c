/*
 * @ingroup GlStatusAction
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



#include "gl-status-action.h"
#include "lgdm-app-launcher.h"

#include <string.h>

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>



//static GlStatusAction *__gui_process_desktop = NULL;

struct _GlStatusActionPrivate
{
	LgdmAppLauncher           *launcher;
	GtkImage                *action_icon;
	GtkButton               *start_button;
	GtkSpinner              *processed_spiner;

};


enum {
	GL_STATUS_ACTION_PROP_NULL,
	GL_STATUS_ACTION_LAUNCHER,

};


enum
{
	GL_STATUS_ACTION_START,
	GL_STATUS_ACTION_LAST_SIGNAL
};


static guint gl_status_action_signals[GL_STATUS_ACTION_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlStatusAction, gl_status_action, GTK_TYPE_BOX);



static void
start_button_clicked_cb  ( GlStatusAction *action , GtkButton *button )
{
	g_signal_emit(action,gl_status_action_signals[GL_STATUS_ACTION_START],0);
	lgdm_app_launcher_start(action->priv->launcher);
}


static void
gl_desktop_actiom_start_real  ( GlStatusAction *action ) {
}

static void
gl_status_action_init(GlStatusAction *desktop)
{
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (GL_IS_STATUS_ACTION(desktop));
	desktop->priv = gl_status_action_get_instance_private (desktop);
	gtk_widget_init_template (GTK_WIDGET (desktop));
}



static void
gl_status_action_finalize (GObject *object)
{
	GlStatusAction* desktop = GL_STATUS_ACTION(object);
	if(desktop->priv->launcher)      g_object_unref(desktop->priv->launcher);
	G_OBJECT_CLASS (gl_status_action_parent_class)->finalize(object);
}




static void
status_action_change_icon ( LgdmApp *app, GParamSpec *pspec, GlStatusAction *action )
{
	//gtk_image_set_from_icon_name(GTK_IMAGE(action->priv->action_icon),lgdm_app_get_icon(app),GTK_ICON_SIZE_LARGE_TOOLBAR);
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GIcon *icon         = g_icon_new_for_string(lgdm_app_get_icon(app),NULL);
	GtkIconInfo *info   = gtk_icon_theme_lookup_by_gicon(theme,icon,24,GTK_ICON_LOOKUP_FORCE_SVG);
	if(info)
	{
		GdkPixbuf *buf =gtk_icon_info_load_icon(info,NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(action->priv->action_icon),buf);
		g_object_unref(info);
		g_object_unref(buf);
	}
	g_object_unref (icon);

}
static void
gl_status_action_realize_icon ( GlStatusAction* action  )
{

	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GIcon *icon = g_app_info_get_icon(lgdm_app_launcher_app_info(action->priv->launcher));

	 GtkIconInfo *info = gtk_icon_theme_lookup_by_gicon(theme,icon,24,GTK_ICON_LOOKUP_FORCE_SVG);
	 if(info)
	 {
		 GdkPixbuf *buf =gtk_icon_info_load_icon(info,NULL);
		 gtk_image_set_from_pixbuf(GTK_IMAGE(action->priv->action_icon),buf);
		 g_object_unref(info);
		 g_object_unref(buf);
	 }
	g_object_unref (icon);
	g_signal_connect(lgdm_object_get_app(LGDM_OBJECT(action->priv->launcher)),"notify::icon",G_CALLBACK(status_action_change_icon),action);
}

static void
gl_status_action_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_STATUS_ACTION(object));
	GlStatusAction* action = GL_STATUS_ACTION(object);
	switch (prop_id)
	{
	case GL_STATUS_ACTION_LAUNCHER:
		if(action->priv->launcher) g_object_unref(action->priv->launcher);
		action->priv->launcher = g_value_dup_object(value);
		gl_status_action_realize_icon(action);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_status_action_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_STATUS_ACTION(object));
	GlStatusAction* action = GL_STATUS_ACTION(object);
	switch (prop_id)
	{
	case GL_STATUS_ACTION_LAUNCHER:
		g_value_set_object(value,action->priv->launcher);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void gl_status_action_class_init (GlStatusActionClass* klass)
{
	GObjectClass*   object_class = G_OBJECT_CLASS   (klass);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

	object_class -> finalize     = gl_status_action_finalize;
	object_class -> set_property = gl_status_action_set_property;
	object_class -> get_property = gl_status_action_get_property;
	klass        -> action_start = gl_desktop_actiom_start_real;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/status-action.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlStatusAction, action_icon);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatusAction, start_button);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatusAction, processed_spiner);

	gtk_widget_class_bind_template_callback (widget_class, start_button_clicked_cb);

	g_object_class_install_property (object_class,GL_STATUS_ACTION_LAUNCHER,
			g_param_spec_object ("action-launcher",
					"Desktop action button icon",
					"Desktop action button icon",
					LGDM_TYPE_APP_LAUNCHER,
					G_PARAM_WRITABLE |  G_PARAM_READABLE| G_PARAM_CONSTRUCT_ONLY  ));

	gl_status_action_signals[GL_STATUS_ACTION_START] =
				g_signal_new ("action-start",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET ( GlStatusActionClass, action_start),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}


/** @} */
