/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * src
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
src is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * src is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-action-Offline.h"
#include "gl-system.h"
#include "gl-draganddrop.h"
#include "gl-level-manager.h"
#include "gl-connection.h"
#include "mkt-collector.h"
#include <stdlib.h>



struct _GlActionOfflinePrivate
{
	gint temp;

};



#define GL_ACTION_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_ACTION_OFFLINE, GlActionOfflinePrivate))



G_DEFINE_TYPE (GlActionOffline, gl_action_Offline, GL_TYPE_ACTION_WIDGET );



static void
gl_action_Offline_action_start ( GlActionWidget *action )
{
	mkIset(gui_controlControl__control,1000);
}

gboolean
gl_plugin_go_offline_status_change_cb   (  GlBinding *binding ,GlActionOffline *action  )
{
	//TEST:g_debug ( "gl_plugin_go_online_status_change_idle");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(action  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_ACTION_OFFLINE(action),FALSE);

	guint status =mkIget(control_subscription__internalStatus);
	if( status < 1000 )
	{
		mkt_window_set_sensetive(MKT_WINDOW(action) , FALSE);
	}
	else if ( mkIget(control_gui__enable_red_button))
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
gl_action_Offline_init (GlActionOffline *object)
{
	object -> priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_ACTION_OFFLINE,GlActionOfflinePrivate);
	gl_connection_connect_binding_signal("control_subscription__internalStatus",G_CALLBACK(gl_plugin_go_offline_status_change_cb),object);
	gl_connection_connect_binding_signal("control_gui__enable_red_button",G_CALLBACK(gl_plugin_go_offline_status_change_cb),object);
}

static void
gl_action_Offline_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlActionOffline *widget = GL_ACTION_OFFLINE(object);


	G_OBJECT_CLASS (gl_action_Offline_parent_class)->finalize (object);
}

static void
gl_action_Offline_class_init ( GlActionOfflineClass *klass )
{
	GObjectClass*        object_class     =  G_OBJECT_CLASS (klass);
	GlActionWidgetClass* parent_class     =  GL_ACTION_WIDGET_CLASS(klass);
	object_class -> finalize              =  gl_action_Offline_finalize;
	parent_class -> action_start          =  gl_action_Offline_action_start;
	g_type_class_add_private (klass, sizeof (GlActionOfflinePrivate));
}

