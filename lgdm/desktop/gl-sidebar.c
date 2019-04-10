/*
 * @ingroup GlSidebar
 * @{
 * @file  gl-sidebar.c	LGDM desktop side bar
 * @brief LGDM desktop side bar.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include <mktlib.h>
#include <mktbus.h>

#include "gl-sidebar.h"
#include <string.h>



#include "../config.h"
#include <glib/gi18n-lib.h>



#define SIDEBAR_MAX_PARAMS  15




//static GlSidebar *__gui_process_sidebar = NULL;

struct _GlSidebarPrivate
{
	GtkButton               *online;
	GtkButton               *offline;
	GtkButton               *larkey;
	GtkButton               *screenshot;
	GtkImage                *key_image;
	GtkSpinner              *screenshot_processed;

	guint                    level;
	gint                     main_tag;

	//  sidebar windows ----------------------------
	GSList                  *last_plugin;
	GList                   *indicate;
	GList                   *widgets;
	GTimer                  *info_timer;
	LarpcDevice             *pc_device;
	GSubprocess             *process;
};


enum {
	GL_SIDEBAR_PROP_NULL,
	GL_SIDEBAR_PROP_LEVEL,
	GL_SIDEBAR_PROP_SCREENSHOT_PROCESSED,
};


enum
{
	GL_SIDEBAR_ONLINE_SIGNAL,
	GL_SIDEBAR_OFFLINE_SIGNAL,
	GL_SIDEBAR_LARKEY_SIGNAL,
	GL_SIDEBAR_SCREENSHOT_SIGNAL,
	GL_SIDEBAR_LAST_SIGNAL
};


static guint gl_sidebar_signals[GL_SIDEBAR_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlSidebar, gl_sidebar, GTK_TYPE_BOX);



static gboolean
online_clicked_cb (GlSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(sidebar),FALSE);
	ultra_control_call_online(ultra_control_client_object(),NULL,NULL,NULL);
	return FALSE;
}

static gboolean
offline_clicked_cb (GlSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(sidebar),FALSE);
	ultra_control_call_offline(ultra_control_client_object(),NULL,NULL,NULL);
	return FALSE;
}

static gboolean
larkey_clicked_cb (GlSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(sidebar),FALSE);
	g_signal_emit(sidebar, gl_sidebar_signals[GL_SIDEBAR_LARKEY_SIGNAL],0);
	return FALSE;
}

static void
gl_app_launcher_waite_process (GObject *source_object,   GAsyncResult *res,  gpointer user_data)
{
	GlSidebar *sidebar = GL_SIDEBAR(user_data);
	g_object_unref(source_object);
	sidebar->priv->process = NULL;
	gl_sidebar_screenshot_processed(sidebar,FALSE);
}

static gboolean
screenshot_clicked_cb (GlSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(sidebar),FALSE);
	if(sidebar->priv->process   == NULL &&sidebar->priv->pc_device!=NULL)
	{
		gl_sidebar_screenshot_processed(sidebar,TRUE);
		GString *file_path = g_string_new("");
		g_string_append_printf(file_path,"%s/%s",larpc_device_get_usb_path(sidebar->priv->pc_device),security_device_get_device_name(TERA_GUARD()));
		file_path = g_string_ascii_down(file_path);
//		g_debug("Create directory = %s",file_path->str);
		if(!mkt_is_dir(file_path->str))
		{
			mkt_make_dir(file_path->str);
		}
		g_string_append_printf(file_path,"/%s-screen-%s.png",security_device_get_device_name(TERA_GUARD()),market_db_get_date_file(market_db_time_now()));
		GError *error = NULL;
		file_path = g_string_ascii_down(file_path);
//		g_debug("Screenshot path = %s",file_path->str);
		sidebar->priv->process            = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE,&error,"gnome-screenshot","-f",file_path->str,NULL);
		if(sidebar->priv->process )
			g_subprocess_wait_async(sidebar->priv->process,NULL,gl_app_launcher_waite_process,sidebar);

		//g_signal_emit(sidebar, gl_sidebar_signals[GL_SIDEBAR_SCREENSHOT_SIGNAL],0);
		g_string_free(file_path,TRUE);
	}
	return FALSE;
}

/*
static void
gl_sidebar_change_level(GlManager *manager,GlLevelManager *level)
{

}


*/
static void
gl_sidebar_application_change (GlSidebar *sidebar,GParamSpec *pspec, gpointer    user_data)
{

}




static void
gl_sidebar_online_real ( GlSidebar *sidebar )
{

}

static void
gl_sidebar_offline_real ( GlSidebar *sidebar )
{

}

static void
gl_sidebar_larkey_real ( GlSidebar *sidebar )
{
	gboolean is_done = FALSE;
	security_device_call_logout_sync(TERA_GUARD(),&is_done,NULL,NULL);
}

static void
gl_sidebar_screenshot_real ( GlSidebar *sidebar )
{

}

static void
gl_sidebar_new_larpc_device ( GlSidebar *sidebar )
{
	sidebar->priv->pc_device = mkt_pc_manager_client_get_device();
	if(sidebar->priv->pc_device !=NULL)
	{
		g_object_bind_property(sidebar->priv->pc_device,"has-usb",sidebar->priv->screenshot,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	}
}

static void
gl_sidebar_init(GlSidebar *sidebar)
{
	g_return_if_fail (sidebar != NULL);
	g_return_if_fail (GL_IS_SIDEBAR(sidebar));
	sidebar->priv = gl_sidebar_get_instance_private (sidebar);
	gtk_widget_init_template (GTK_WIDGET (sidebar));
	sidebar->priv = G_TYPE_INSTANCE_GET_PRIVATE(sidebar,GL_TYPE_SIDEBAR,GlSidebarPrivate);
	sidebar->priv->level              = 0;
	sidebar->priv->indicate           = NULL;
	sidebar->priv->last_plugin        = NULL;
	sidebar->priv->info_timer         = g_timer_new();
	g_signal_connect (sidebar ,"notify::application",G_CALLBACK(gl_sidebar_application_change),NULL);

	//gtk_widget_set_app_paintable(GTK_WIDGET(sidebar), TRUE);
	//gtk_widget_set_opacity(GTK_WIDGET(sidebar),0.1 );


//  Sidebar window -----------------------------------------------------------
}

static void
gl_sidebar_finalize (GObject *object)
{
	GlSidebar* sidebar = GL_SIDEBAR(object);
	if(sidebar->priv->info_timer)g_timer_destroy(sidebar->priv->info_timer);
	G_OBJECT_CLASS (gl_sidebar_parent_class)->finalize(object);
}



static void
change_ultra_control_property ( GObject *object, GParamSpec *pspec, GlSidebar *sidebar )
{
	gtk_widget_set_state_flags(GTK_WIDGET(sidebar->priv->offline),GTK_STATE_FLAG_NORMAL,TRUE);
	gtk_widget_set_state_flags(GTK_WIDGET(sidebar->priv->online),GTK_STATE_FLAG_NORMAL,TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->offline),ultra_control_get_red_button(ultra_control_client_object()));
	gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->online),ultra_control_get_green_button(ultra_control_client_object()));

}


static void
gl_sidebar_constructed  (GObject *object)
{
	GlSidebar* sidebar = GL_SIDEBAR(object);
	//g_object_bind_property(ultra_control_client_object(),"red-button",sidebar->priv->offline,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	//g_object_bind_property(ultra_control_client_object(),"green-button",sidebar->priv->online,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->screenshot),FALSE);
	gl_sidebar_new_larpc_device(sidebar);
	g_signal_connect(ultra_control_client_object(),"notify::red-button",G_CALLBACK(change_ultra_control_property),sidebar);
	g_signal_connect(ultra_control_client_object(),"notify::green-button",G_CALLBACK(change_ultra_control_property),sidebar);

	if(G_OBJECT_CLASS (gl_sidebar_parent_class)->constructed)
		G_OBJECT_CLASS (gl_sidebar_parent_class)->constructed(object);
}


static void
gl_sidebar_larkey_changed              ( GlSidebar *bar )
{
	GdkPixbuf *buf          = NULL;
	GtkIconTheme  *theme              = gtk_icon_theme_get_default();
	GError *error = NULL ;
	if(security_device_get_security_usb(TERA_GUARD()))
	{
		buf       = gtk_icon_theme_load_icon(theme,"usb-lock",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
	}
	else
	{
		switch( bar->priv->level)
		{
		case 1:
			buf       = gtk_icon_theme_load_icon(theme,"lock-safe",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
			break;
		case 2:
			buf       = gtk_icon_theme_load_icon(theme,"unlock-unsafe",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
			break;
		case 3:
			buf       = gtk_icon_theme_load_icon(theme,"unlock-unsafe",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
			break;
		case 4:
			buf       = gtk_icon_theme_load_icon(theme,"unlock-unsafe",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
			break;
		case 5:
			buf       = gtk_icon_theme_load_icon(theme,"unlock-unsafe",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
			break;
		default:
			buf       = gtk_icon_theme_load_icon(theme,"unlock-unsafe",48,GTK_ICON_LOOKUP_FORCE_SVG,NULL);
			break;
		}

	}

	if(error )
		g_warning ("Load icon buf : %s",error->message);
	gtk_image_set_from_pixbuf(GTK_IMAGE(bar->priv->key_image),buf);
	//g_object_unref(buf);
}

static void
gl_sidebar_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_SIDEBAR(object));
	GlSidebar* sidebar = GL_SIDEBAR(object);
	switch (prop_id)
	{
	case GL_SIDEBAR_PROP_LEVEL:
		sidebar->priv->level = g_value_get_uint(value);
		gl_sidebar_larkey_changed(sidebar);
		break;
	case GL_SIDEBAR_PROP_SCREENSHOT_PROCESSED:
		gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->screenshot_processed),g_value_get_boolean(value));
		gtk_widget_set_state_flags(GTK_WIDGET(sidebar->priv->screenshot),GTK_STATE_FLAG_NORMAL,TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->screenshot),!g_value_get_boolean(value));
		if(g_value_get_boolean(value))
		{

			gtk_widget_show(GTK_WIDGET(sidebar->priv->screenshot_processed));
			gtk_spinner_start(sidebar->priv->screenshot_processed);
		}
		else
		{
			gtk_spinner_stop(sidebar->priv->screenshot_processed);
			gtk_widget_hide(GTK_WIDGET(sidebar->priv->screenshot_processed));
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_sidebar_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_SIDEBAR(object));
	GlSidebar* sidebar = GL_SIDEBAR(object);
	switch (prop_id)
	{
	case GL_SIDEBAR_PROP_LEVEL:
		g_value_set_uint(value,sidebar->priv->level);
		break;
	case GL_SIDEBAR_PROP_SCREENSHOT_PROCESSED:
		g_value_set_boolean(value,gtk_widget_get_sensitive(GTK_WIDGET(sidebar->priv->screenshot_processed)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_sidebar_class_init(GlSidebarClass *klass)
{
	GObjectClass*         object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/sidebar.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlSidebar, online);
	gtk_widget_class_bind_template_child_private (widget_class, GlSidebar, offline);
	gtk_widget_class_bind_template_child_private (widget_class, GlSidebar, larkey);
	gtk_widget_class_bind_template_child_private (widget_class, GlSidebar, screenshot);
	gtk_widget_class_bind_template_child_private (widget_class, GlSidebar, key_image);
	gtk_widget_class_bind_template_child_private (widget_class, GlSidebar, screenshot_processed);


	gtk_widget_class_bind_template_callback (widget_class, online_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, offline_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, larkey_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, screenshot_clicked_cb);

	object_class -> finalize           =  gl_sidebar_finalize;
	object_class -> set_property       =  gl_sidebar_set_property;
	object_class -> get_property       =  gl_sidebar_get_property;
	object_class -> constructed        =  gl_sidebar_constructed;

	klass->online                      = gl_sidebar_online_real;
	klass->offline                     = gl_sidebar_offline_real;
	klass->larkey                      = gl_sidebar_larkey_real;
	klass->screenshot                  = gl_sidebar_screenshot_real;


	g_object_class_install_property (object_class,GL_SIDEBAR_PROP_LEVEL,
						g_param_spec_uint  ("lar-level",
								"Desktop action button name",
								"Desktop action button name",
								0,4,0,
								G_PARAM_WRITABLE | G_PARAM_READABLE ));
	g_object_class_install_property (object_class,GL_SIDEBAR_PROP_SCREENSHOT_PROCESSED,
							g_param_spec_boolean  ("screenshot-processed",
									"Desktop action button name",
									"Desktop action button name",
									FALSE,
									G_PARAM_WRITABLE | G_PARAM_READABLE ));


	gl_sidebar_signals[GL_SIDEBAR_ONLINE_SIGNAL] =
			g_signal_new ("sidebar-online",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlSidebarClass, online),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_sidebar_signals[GL_SIDEBAR_OFFLINE_SIGNAL] =
			g_signal_new ("sidebar-offline",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlSidebarClass, offline),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_sidebar_signals[GL_SIDEBAR_LARKEY_SIGNAL] =
			g_signal_new ("sidebar-larkey",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlSidebarClass, larkey),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_sidebar_signals[GL_SIDEBAR_SCREENSHOT_SIGNAL] =
			g_signal_new ("sidebar-screenshot",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlSidebarClass, screenshot),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);



}


//FIXME : plug in info window (last plugin ) Start last plugin from list..
GtkWidget*
gl_sidebar_new ( )
{
	GlSidebar  *sidebar;
	sidebar   = GL_SIDEBAR(g_object_new( GL_TYPE_SIDEBAR,NULL));
	return     GTK_WIDGET(sidebar);
}

gboolean
gl_sidebar_online_sensetive         ( GlSidebar *bar , gboolean sensetive )
{
	g_return_val_if_fail(bar!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(bar),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(bar->priv->online),sensetive);
	return FALSE;
}

gboolean
gl_sidebar_offline_sensetive        ( GlSidebar *bar , gboolean sensetive )
{
	g_return_val_if_fail(bar!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(bar),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(bar->priv->offline),sensetive);
	return FALSE;
}

gboolean
gl_sidebar_larkey_sensetive         ( GlSidebar *bar , gboolean sensetive )
{
	g_return_val_if_fail(bar!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(bar),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(bar->priv->larkey),sensetive);
	return FALSE;
}

gboolean
gl_sidebar_screenshot_sensetive     ( GlSidebar *bar , gboolean sensetive )
{
	g_return_val_if_fail(bar!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_SIDEBAR(bar),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(bar->priv->screenshot),sensetive);
	return FALSE;
}

void
gl_sidebar_screenshot_processed     ( GlSidebar *bar , gboolean processed)
{
	g_return_if_fail(bar!=NULL);
	g_return_if_fail(GL_IS_SIDEBAR(bar));
	g_object_set(bar,"screenshot-processed",processed,NULL);

}



/** @} */
