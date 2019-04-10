/*
 * gl-update.h
 *
 *  Created on: 22.02.2013
 *      Author: sascha
 */

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * dbusexample
 * Copyright (C) sascha 2012 <sascha@sascha-ThinkPad-X61>
 *
dbusexample is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dbusexample is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-plugin-manager.h"

#include "../lgdm-status.h"
#include "gl-indicate.h"
#include "gl-system.h"
#include "gl-connection.h"
#include "gl-action-widget.h"
#include "gl-translation.h"


struct _GlPluginManagerPrivate
{

};


#define GL_PLUGIN_MANAGER_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_PLUGIN_MANAGER, GlPluginManagerPrivate))

G_DEFINE_TYPE (GlPluginManager, gl_plugin_manager, MKT_TYPE_WINDOW);

enum
{
	GL_PLUGIN_MANAGER_PROP0,

};

enum
{
	GL_PLUGIN_MANAGER_LAST_SIGNAL
};


//static guint gl_log_signals[GL_LOG_LAST_SIGNAL] = { 0 };



static void
gl_plugin_manager_init (GlPluginManager *pm)
{
    GlPluginManagerPrivate *priv = GL_PLUGIN_MANAGER_GET_PRIVATE(pm);
    pm->priv   = priv;
}

static void
gl_plugin_manager_finalize (GObject *object)
{
	GlPluginManager *update = GL_PLUGIN_MANAGER(object);

	G_OBJECT_CLASS (gl_plugin_manager_parent_class)->finalize (object);
}



void
gl_plugin_manager_set_property(  GObject        *object,
		guint           prop_id,
		const GValue   *value,
		GParamSpec     *pspec)
{
	GlPluginManager *pm = GL_PLUGIN_MANAGER(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}
void
gl_plugin_manager_get_property(  GObject        *object,
		guint           prop_id,
		GValue         *value,
		GParamSpec     *pspec)
{
	GlPluginManager *pm = GL_PLUGIN_MANAGER(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
gl_plugin_manager_class_init (GlPluginManagerClass *klass)
{
	GObjectClass*   object_class  = G_OBJECT_CLASS (klass);
	//MktWindowClass *mktdraw_class = MKT_WINDOW_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlPluginManagerPrivate));
	object_class->finalize          = gl_plugin_manager_finalize;
	object_class->set_property      = gl_plugin_manager_set_property;
	object_class->get_property      = gl_plugin_manager_get_property;

}

void
gl_plugin_manager_realize_update_package_tree (  MktWindow *window, GtkWidget *tree )
{
	//TEST:g_debug( "gl_log_system_realize_system_log for %s %s",gl_widget_option_get_name(tree),G_OBJECT_TYPE_NAME(tree));
	g_return_if_fail(window != NULL );
	g_return_if_fail(GL_IS_PLUGIN_MANAGER(window));
	g_return_if_fail(tree != NULL);

}

