/*
 *
 * 
 * Copyright (C) LAR Process analysis AG 2012
 *  Author : Sascha Smolkov    sascha.smolkov@gmail.com
 *
 *  Description :
 *               Start online measurement action object
 * 
 */

#include "gl-action-Online.h"
#include "gl-system.h"
#include "gl-draganddrop.h"
#include "gl-level-manager.h"
#include "gl-connection.h"
#include "mkt-collector.h"
#include <stdlib.h>



struct _GlActionOnlinePrivate
{
	gint temp;

};



#define GL_ACTION_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_ACTION_ONLINE, GlActionOnlinePrivate))



G_DEFINE_TYPE (GlActionOnline, gl_action_Online, GL_TYPE_ACTION_WIDGET );



static void
gl_action_Online_action_start ( GlActionWidget *action )
{
	mkIset(gui_controlControl__control,2000);
}

gboolean
gl_plugin_go_online_status_change_cb   (  GlBinding *binding ,GlActionOnline *action  )
{
	//TEST:g_debug ( "gl_plugin_go_online_status_change_idle");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(action  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_ACTION_ONLINE(action),FALSE);

	guint status =mkIget(control_subscription__internalStatus);
	//TEST:g_debug("Status %d",status);
	if( status < 100 || status > 1000 )
	{
		mkt_window_set_sensetive(MKT_WINDOW(action) , FALSE);
	}
	else if ( mkIget(control_gui__enable_green_button))
	{
		mkt_window_set_sensetive(MKT_WINDOW(action) , TRUE);
	}
	else
	{
		mkt_window_set_sensetive(MKT_WINDOW(action) , FALSE);
	}
	return TRUE;
}


static void
gl_action_Online_init (GlActionOnline *object)
{
	object -> priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_ACTION_ONLINE,GlActionOnlinePrivate);
	gl_connection_connect_binding_signal("control_subscription__internalStatus",G_CALLBACK(gl_plugin_go_online_status_change_cb),object);
	gl_connection_connect_binding_signal("control_gui__enable_green_button",G_CALLBACK(gl_plugin_go_online_status_change_cb),object);
	GlBinding *binding = GL_BINDING(mkt_collector_get_atom_static("control_subscription__internalStatus"));

}

static void
gl_action_Online_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlActionOnline *widget = GL_ACTION_ONLINE(object);
	G_OBJECT_CLASS (gl_action_Online_parent_class)->finalize (object);
}

static void
gl_action_Online_class_init ( GlActionOnlineClass *klass )
{
	GObjectClass*        object_class     =  G_OBJECT_CLASS (klass);
	GlActionWidgetClass* parent_class     =  GL_ACTION_WIDGET_CLASS(klass);
	object_class -> finalize              =  gl_action_Online_finalize;
	parent_class -> action_start          =  gl_action_Online_action_start;
	g_critical ( "Test Action widget add private  1");
	g_type_class_add_private (klass, sizeof (GlActionOnlinePrivate));
}

