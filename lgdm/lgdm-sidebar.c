
/*
 * @ingroup LgdmSidebar
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



#include <market-time.h>
#include <mkt-utils.h>
#include "lgdm-sidebar.h"
#include "lgdm-state.h"
#include <string.h>



#include "ultimate-config.h"
#include <glib/gi18n-lib.h>



#define SIDEBAR_MAX_PARAMS  15




//static LgdmSidebar *__gui_process_sidebar = NULL;

struct _LgdmSidebarPrivate
{
	GtkButton               *online;
	GtkButton               *offline;
	GtkButton               *larkey;
	GtkButton               *screenshot;
	GtkImage                *key_image;
	GtkSpinner              *screenshot_processed;


	//  sidebar windows ----------------------------
	GSList                  *last_plugin;
GSubprocess             *process;

};


enum {
	LGDM_SIDEBAR_PROP_NULL,
};


G_DEFINE_TYPE_WITH_PRIVATE (LgdmSidebar, lgdm_sidebar, GTK_TYPE_BOX);



static gboolean
online_clicked_cb (LgdmSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(LGDM_IS_SIDEBAR(sidebar),FALSE);
	// ultra_control_call_online(ultra_control_client_object(),NULL,NULL,NULL);
	return FALSE;
}

static gboolean
offline_clicked_cb (LgdmSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(LGDM_IS_SIDEBAR(sidebar),FALSE);
	// ultra_control_call_offline(ultra_control_client_object(),NULL,NULL,NULL);
	return FALSE;
}
static gboolean
larkey_clicked_cb (LgdmSidebar *sidebar ,GtkWidget *widget)
{
    return FALSE;
}

static void
lgdm_app_launcher_waite_process (GObject *source_object,   GAsyncResult *res,  gpointer user_data)
{
	LgdmSidebar *sidebar = LGDM_SIDEBAR(user_data);
	g_object_unref(source_object);
	sidebar->priv->process = NULL;
    gtk_spinner_stop(sidebar->priv->screenshot_processed);
    gtk_widget_hide(GTK_WIDGET(sidebar->priv->screenshot_processed));
}



static gboolean
screenshot_clicked_cb (LgdmSidebar *sidebar ,GtkWidget *widget)
{
	g_return_val_if_fail(sidebar != NULL,FALSE);
	g_return_val_if_fail(LGDM_IS_SIDEBAR(sidebar),FALSE);
	if(sidebar->priv->process   == NULL)
	{
		// lgdm_sidebar_screenshot_processed(sidebar,TRUE);
		GString *file_path = g_string_new("");
		g_string_append_printf(file_path,"%s/%s",lgdm_state_get_usb_path(lgdm_state()),lgdm_state_get_device_name(lgdm_state()));
		file_path = g_string_ascii_down(file_path);
//		g_debug("Create directory = %s",file_path->str);
		if(!mkt_is_dir(file_path->str))
		{
			mkt_make_dir(file_path->str);
		}
		g_string_append_printf(file_path,"/%s-screen-%s.png",lgdm_state_get_device_name(lgdm_state()),market_db_get_date_file(market_db_time_now()));
		GError *error = NULL;
		file_path = g_string_ascii_down(file_path);
//		g_debug("Screenshot path = %s",file_path->str);
		sidebar->priv->process            = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE,&error,"gnome-screenshot","-f",file_path->str,NULL);
		if(sidebar->priv->process ){

            gtk_widget_show(GTK_WIDGET(sidebar->priv->screenshot_processed));
            gtk_spinner_start(sidebar->priv->screenshot_processed);
			g_subprocess_wait_async(sidebar->priv->process,NULL,lgdm_app_launcher_waite_process,sidebar);
        }

		//g_signal_emit(sidebar, lgdm_sidebar_signals[LGDM_SIDEBAR_SCREENSHOT_SIGNAL],0);
		g_string_free(file_path,TRUE);
	}
	return FALSE;
}


static void
lgdm_sidebar_init(LgdmSidebar *sidebar)
{
	g_return_if_fail (sidebar != NULL);
	g_return_if_fail (LGDM_IS_SIDEBAR(sidebar));
	sidebar->priv = lgdm_sidebar_get_instance_private (sidebar);
	gtk_widget_init_template (GTK_WIDGET (sidebar));
	sidebar->priv = G_TYPE_INSTANCE_GET_PRIVATE(sidebar,LGDM_TYPE_SIDEBAR,LgdmSidebarPrivate);
	// sidebar->priv->last_plugin        = NULL
	// sidebar->priv->info_timer         = g_timer_new();

	//gtk_widget_set_app_paintable(GTK_WIDGET(sidebar), TRUE);
	//gtk_widget_set_opacity(GTK_WIDGET(sidebar),0.1 );


//  Sidebar window -----------------------------------------------------------
}

static void
lgdm_sidebar_finalize (GObject *object)
{
	// LgdmSidebar* sidebar = LGDM_SIDEBAR(object);
	// if(sidebar->priv->info_timer)g_tismer_destroy(sidebar->priv->info_timer);
	G_OBJECT_CLASS (lgdm_sidebar_parent_class)->finalize(object);
}



static void
update_control_buttons(LgdmSidebar *sidebar) {
	gtk_widget_set_state_flags(GTK_WIDGET(sidebar->priv->offline),GTK_STATE_FLAG_NORMAL,TRUE);
	gtk_widget_set_state_flags(GTK_WIDGET(sidebar->priv->online),GTK_STATE_FLAG_NORMAL,TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->offline),lgdm_state_get_red_button(lgdm_state()));
    gtk_widget_set_sensitive(GTK_WIDGET(sidebar->priv->online),lgdm_state_get_green_button(lgdm_state()));
}




static void
change_level_property ( GObject *object, GParamSpec *pspec, LgdmSidebar *sidebar )
{
    switch(lgdm_state_get_level(lgdm_state()))
    {
		case 1: gtk_image_set_from_resource(GTK_IMAGE(sidebar->priv->key_image),"lgdm/image/lock.svg");
			break;
		case 2:
		case 3:
		case 4:
		case 5: gtk_image_set_from_resource(GTK_IMAGE(sidebar->priv->key_image),"lgdm/image/unlock.svg");
			break;
		default:
			gtk_image_set_from_resource(GTK_IMAGE(sidebar->priv->key_image),"lgdm/image/lock.svg");
			break;

	}
}

static void
lgdm_sidebar_constructed  (GObject *object)
{
	LgdmSidebar* sidebar = LGDM_SIDEBAR(object);
	//g_object_bind_property(ultra_control_client_object(),"red-button",sidebar->priv->offline,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	//g_object_bind_property(ultra_control_client_object(),"green-button",sidebar->priv->online,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    g_object_bind_property(lgdm_state(),"has-usb",sidebar->priv->screenshot,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    g_object_bind_property(lgdm_state(),"red-button",sidebar->priv->offline,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    g_object_bind_property(lgdm_state(),"green-button",sidebar->priv->offline,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    g_signal_connect(lgdm_state(),"notify::level",G_CALLBACK(change_level_property),sidebar);
	update_control_buttons(sidebar);

	if(G_OBJECT_CLASS (lgdm_sidebar_parent_class)->constructed)
		G_OBJECT_CLASS (lgdm_sidebar_parent_class)->constructed(object);
}



static void
lgdm_sidebar_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (LGDM_MANAGER) property \n");
	g_return_if_fail (LGDM_IS_SIDEBAR(object));
	// LgdmSidebar* sidebar = LGDM_SIDEBAR(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
lgdm_sidebar_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (LGDM_MANAGER) property \n");
	g_return_if_fail (LGDM_IS_SIDEBAR(object));
	// LgdmSidebar* sidebar = LGDM_SIDEBAR(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
lgdm_sidebar_class_init(LgdmSidebarClass *klass)
{
	GObjectClass*         object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/sidebar.ui");
	gtk_widget_class_bind_template_child_private (widget_class, LgdmSidebar, online);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmSidebar, offline);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmSidebar, larkey);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmSidebar, screenshot);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmSidebar, key_image);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmSidebar, screenshot_processed);


	gtk_widget_class_bind_template_callback (widget_class, online_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, offline_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, larkey_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, screenshot_clicked_cb);

	object_class -> finalize           =  lgdm_sidebar_finalize;
	object_class -> set_property       =  lgdm_sidebar_set_property;
	object_class -> get_property       =  lgdm_sidebar_get_property;
	object_class -> constructed        =  lgdm_sidebar_constructed;


 }


//FIXME : plug in info window (last plugin ) Start last plugin from list..
GtkWidget*
lgdm_sidebar_new ( )
{
	LgdmSidebar  *sidebar;
	sidebar   = LGDM_SIDEBAR(g_object_new( LGDM_TYPE_SIDEBAR,NULL));
	return     GTK_WIDGET(sidebar);
}


/** @} */
