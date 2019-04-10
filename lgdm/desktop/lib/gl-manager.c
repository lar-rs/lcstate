/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-manager.c
 * Copyright (C) Sascha 2011 <sascha@sascha-desktop>
 * 
gl-manager.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-manager.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-manager.h"

#define GL_MANAGER_PLACE_NAME "level_notebook"

struct _GlManagerPrivate
{
	gchar                  *place;
	GlPlugin               *active_plugin;
	GlPlugin               *active_menu;
	GlLevelManager         *level_manager;
	GlConnection           *connection;

	gulong                  start_plugin_signal_id;
	gulong                  stop_plugin_signal_id;
	gulong                  start_plugin_menu_signal_id;
	gulong                  stop_plugin_menu_signal_id;

};

enum {
	PROP_NULL,
	PROP_PLACE,
};

G_DEFINE_TYPE (GlManager, gl_manager, GTK_TYPE_EVENT_BOX);

static void
gl_manager_init (GlManager *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_MANAGER,GlManagerPrivate);
	object->priv->start_plugin_signal_id        = 0;
	object->priv->stop_plugin_signal_id         = 0;
	object->priv->start_plugin_menu_signal_id   = 0;
	object->priv->stop_plugin_menu_signal_id    = 0;

	object->priv->place = g_strdup("unknown_place");
	object->priv->level_manager                 = NULL;
	object->priv->connection                    = NULL;
	object->priv->active_menu                   = NULL;
	object->priv->active_plugin                 = NULL;
	/* TODO: Add initialization code here */
}

static void
gl_manager_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlManager *manager = GL_MANAGER(object);
	g_free(manager->priv->place);
	G_OBJECT_CLASS (gl_manager_parent_class)->finalize (object);
}

static void
gl_manager_change_level_real( GlManager *manager )
{
	if(GL_MANAGER_GET_CLASS(manager)->change_level != NULL)
		GL_MANAGER_GET_CLASS(manager)->change_level(manager,manager->priv->level_manager);
}

static void
gl_manager_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_MANAGER (object));
	GlManager* manager = GL_MANAGER(object);
	switch (prop_id)
	{
	case PROP_PLACE:
		/* TODO: Add setter for "mdule_path" property here */
		g_free(manager->priv->place );
		manager->priv->place     = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_manager_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_MANAGER (object));
	GlManager* manager = GL_MANAGER(object);
	switch (prop_id)
	{
	case PROP_PLACE:
		/* TODO: Add setter for "path" property here */
		g_value_set_string(value,manager->priv->place);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_manager_class_init (GlManagerClass *klass)
{
	g_type_class_add_private (klass, sizeof (GlManagerPrivate));

	GObjectClass*      object_class = G_OBJECT_CLASS (klass);
	GtkEventBoxClass*  parent_class = GTK_EVENT_BOX_CLASS(klass);


	object_class -> set_property       =  gl_manager_set_property;
	object_class -> get_property       =  gl_manager_get_property;
	object_class -> finalize           =  gl_manager_finalize;

	klass        -> pack_plugin        =  NULL;
	klass        -> remove_plugin      =  NULL;
	klass        -> change_level       =  NULL;
	klass        -> start_plugin       =  NULL;
	klass        -> stop_plugin        =  NULL;
	klass        -> start_plugin_menu  =  NULL;
	klass        -> stop_plugin_menu   =  NULL;

	klass        -> add_connection     =  NULL;
	klass        -> add_level_manager  =  NULL;

	GParamSpec *pspec;
	pspec = g_param_spec_string ("place",
			"Place",
			"Set/Get Manager place",
			GL_MANAGER_PLACE_NAME,
			G_PARAM_READABLE | G_PARAM_WRITABLE );
	g_object_class_install_property (object_class,
			PROP_PLACE,pspec);
}

GlManager*
gl_manager_new    ( gchar *place )
{
	return GL_MANAGER(g_object_new(GL_TYPE_MANAGER,"place",place,NULL));
}


gboolean
gl_manager_is_plugin_active          ( GlManager *manager )
{
	g_return_val_if_fail  (GL_IS_MANAGER(manager),FALSE);
	return (manager->priv->active_menu != NULL);
}

GlPlugin*
gl_manager_get_active_plugin ( GlManager *manager )
{
	g_return_val_if_fail   (GL_IS_MANAGER(manager),NULL);
	return manager->priv->active_plugin;
}

GlPlugin*
gl_manager_get_active_plugin_menu ( GlManager *manager )
{
	g_return_val_if_fail   (GL_IS_MANAGER(manager),NULL);
	return manager->priv->active_menu;
}

// level manager add ---------------------------------------------------------------
gboolean
gl_manager_change_level_signal (GlLevelManager *level , gpointer data)
{
	//TEST:g_debug("gl_manager_change_level_signal\n");
	g_return_val_if_fail   (GL_IS_MANAGER(data),FALSE);
	GlManager *manager = GL_MANAGER(data);
	gl_manager_change_level_real(manager);
}

void
gl_manager_add_level_manager  ( GlManager *manager , GlLevelManager *level )
{
	g_return_if_fail(GL_IS_MANAGER(manager));
	g_return_if_fail(GL_IS_LEVEL_MANAGER(level));
	manager->priv->level_manager = level;
	if(GL_MANAGER_GET_CLASS(manager)->add_level_manager != NULL)
			GL_MANAGER_GET_CLASS(manager)->add_level_manager(manager,level);
	g_signal_connect(manager->priv->level_manager,"change_gui_level",G_CALLBACK(gl_manager_change_level_signal),manager);
	gl_manager_change_level_real(manager);
}

GlLevelManager*
gl_manager_get_level_manager ( GlManager *manager )
{
	g_return_val_if_fail   (GL_IS_MANAGER(manager),NULL);
	return manager->priv->level_manager;
}


// connection add ---------------------------------------------------------------
void
gl_manager_add_connection     ( GlManager *manager ,  GlConnection   *connection )
{
	g_return_if_fail( GL_IS_MANAGER(manager) );
	g_return_if_fail( GL_IS_CONNECTION(connection) );
	manager->priv->connection = connection;
	if( GL_MANAGER_GET_CLASS(manager)->add_connection != NULL )
		GL_MANAGER_GET_CLASS(manager)->add_connection(manager,connection);
}

// plugin signal ----------------------------------------------------------

static void
gl_manager_start_plugin_signal(GlPlugin *plugin,gpointer data)
{
	g_return_if_fail(GL_IS_MANAGER(data));
    GlManager *manager = GL_MANAGER(data);
	manager->priv->active_plugin = plugin;
    if(GL_MANAGER_GET_CLASS(manager)->start_plugin != NULL)
			GL_MANAGER_GET_CLASS(manager)->start_plugin(manager,plugin);
}

static void
gl_manager_stop_plugin_signal(GlPlugin *plugin,gpointer data)
{
	g_return_if_fail(GL_IS_MANAGER(data));
	GlManager *manager = GL_MANAGER(data);
	if(GL_MANAGER_GET_CLASS(manager) ->stop_plugin != NULL)
		GL_MANAGER_GET_CLASS(manager)->stop_plugin(manager,plugin);
}

static void
gl_manager_start_plugin_menu_signal(GlPlugin *plugin,gpointer data)
{
	g_return_if_fail(GL_IS_MANAGER(data));
    GlManager *manager = GL_MANAGER(data);
    if( (manager->priv->active_menu  != NULL )
      &&(manager->priv->active_menu  != plugin) )
    {
    	gl_plugin_close_menu(manager->priv->active_menu);
    }
    manager->priv->active_menu = plugin;
    if(GL_MANAGER_GET_CLASS(manager)->start_plugin_menu != NULL)
			GL_MANAGER_GET_CLASS(manager)->start_plugin_menu(manager,plugin);
}

static void
gl_manager_stop_plugin_menu_signal(GlPlugin *plugin,gpointer data)
{
	g_return_if_fail(GL_IS_MANAGER(data));
	GlManager *manager = GL_MANAGER(data);
	manager->priv->active_menu =  NULL;
	if(GL_MANAGER_GET_CLASS(manager) ->stop_plugin_menu != NULL)
		GL_MANAGER_GET_CLASS(manager)->stop_plugin_menu(manager,plugin);
}

gboolean
gl_manager_pack_plugin_start  ( GlManager *manager , GlPlugin *plugin , gboolean user_move )
{
	g_return_val_if_fail   (GL_IS_MANAGER (manager),FALSE);
	g_return_val_if_fail   (GL_IS_PLUGIN(plugin),FALSE);

	if(GL_MANAGER_GET_CLASS(manager)->pack_plugin != NULL)
	{
		if(GL_MANAGER_GET_CLASS(manager)->pack_plugin(manager,plugin,user_move))
		{
			if( !g_signal_handler_is_connected(plugin,manager->priv->start_plugin_signal_id))
				manager->priv->start_plugin_signal_id = g_signal_connect(plugin,"plugin_start",G_CALLBACK(gl_manager_start_plugin_signal),manager);
			else
				g_signal_handler_unblock(plugin,manager->priv->start_plugin_signal_id);

			if( !g_signal_handler_is_connected(plugin,manager->priv->stop_plugin_signal_id))
				manager->priv->stop_plugin_signal_id = g_signal_connect(plugin,"plugin_stop",G_CALLBACK(gl_manager_stop_plugin_signal),manager);
			else
				g_signal_handler_unblock(plugin,manager->priv->stop_plugin_signal_id);

			if( !g_signal_handler_is_connected(plugin,manager->priv->start_plugin_menu_signal_id))
				manager->priv->start_plugin_menu_signal_id = g_signal_connect(plugin,"plugin_start_menu",G_CALLBACK(gl_manager_start_plugin_menu_signal),manager);
			if( !g_signal_handler_is_connected(plugin,manager->priv->stop_plugin_menu_signal_id))
				manager->priv->stop_plugin_menu_signal_id = g_signal_connect(plugin,"plugin_stop_menu",G_CALLBACK(gl_manager_stop_plugin_menu_signal),manager);

			return TRUE;
		}
	}
	return FALSE;
}

gboolean
gl_manager_remove_plugin ( GlManager *manager , GlPlugin *plugin , gboolean user_move)
{
	g_return_val_if_fail   (GL_IS_MANAGER (manager),FALSE);
	g_return_val_if_fail   (GL_IS_PLUGIN(plugin),FALSE);
	if(GL_MANAGER_GET_CLASS(manager)->remove_plugin != NULL)
	{
		if(GL_MANAGER_GET_CLASS(manager)->remove_plugin ( manager,plugin ,user_move))
		{
			g_signal_handler_block( plugin,manager->priv->start_plugin_signal_id );
			g_signal_handler_block( plugin,manager->priv->stop_plugin_signal_id  );
		}
	}
}

const gchar*
gl_manager_get_place ( GlManager *manager )
{
	g_return_val_if_fail   (GL_IS_MANAGER (manager),NULL);
	return manager->priv->place;
}


GlActionWidget*
gl_manager_get_action_widget         ( GlManager *manager , const char *action_nick )
{

}
