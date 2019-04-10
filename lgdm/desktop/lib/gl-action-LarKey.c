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

#include "gl-action-LarKey.h"
//#include "gl-system.h"
#include "gl-draganddrop.h"
#include "gl-level-manager.h"
//#include "gl-connection.h"
#include "mkt-collector.h"
#include <stdlib.h>



struct _GlActionLarKeyPrivate
{
	gchar *open_icon;
	gchar *close_icon;

};


enum {
	PROP_ACTION_LAR_KEY_0,
	PROP_ACTION_LAR_KEY_ICON_OPEN,
	PROP_ACTION_LAR_KEY_ICON_CLOSE
};


#define GL_ACTION_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_ACTION_LAR_KEY, GlActionLarKeyPrivate))



G_DEFINE_TYPE (GlActionLarKey, gl_action_LarKey, GL_TYPE_ACTION_WIDGET );



static void
gl_action_LarKey_action_start ( GlActionWidget *action )
{
	gl_level_manager_key_open_close();
}

static void
gl_action_LarKey_close_signal(GlLevelManager *level,GlActionLarKey *object)
{
	g_return_if_fail(GL_IS_ACTION_LAR_KEY(object));
	mktAPSet(object,"icon",object->priv->close_icon);
	//TEST:g_debug("TEST ........................CLOSE");
	mktAPSet(MKT_ATOM(object),"icon",object -> priv ->close_icon);
}

void gl_action_LarKey_open_signal(GlLevelManager *level,GlActionLarKey *object)
{
	g_return_if_fail(GL_IS_ACTION_LAR_KEY(object));
	mktAPSet(object,"icon",object->priv->open_icon);
	//TEST:g_debug("TEST ........................OPEN");
	mktAPSet(MKT_ATOM(object),"icon",object -> priv ->open_icon);
}

static void
gl_action_LarKey_init (GlActionLarKey *object)
{
	object -> priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_ACTION_LAR_KEY,GlActionLarKeyPrivate);
	object -> priv ->close_icon = g_strdup ( "/lar/gui/encrypted.png");
	object -> priv ->open_icon  = g_strdup ( "/lar/gui/unlock.png");
	if(gl_level_manager_get_static())
	{
		g_signal_connect( gl_level_manager_get_static() , "level_key_close" , G_CALLBACK(gl_action_LarKey_close_signal), object );
		g_signal_connect( gl_level_manager_get_static() , "level_key_open" , G_CALLBACK(gl_action_LarKey_open_signal) , object );
	}

}

static void
gl_action_LarKey_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlActionLarKey *widget = GL_ACTION_LAR_KEY(object);
	if(widget->priv->open_icon) g_free(widget->priv->open_icon);
	if(widget->priv->close_icon) g_free(widget->priv->close_icon);

	G_OBJECT_CLASS (gl_action_LarKey_parent_class)->finalize (object);
}
static void
gl_action_LarKey_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_ACTION_WIDGET (object));
	GlActionLarKey* widget = GL_ACTION_LAR_KEY(object);
	//g_debug ("Set Property ...id%d - %s ",prop_id,g_param_spec_get_name(pspec));
	switch (prop_id)
	{
	case PROP_ACTION_LAR_KEY_ICON_OPEN: // widget redraw
		if(widget -> priv ->open_icon)g_free ( widget -> priv ->open_icon );
		widget -> priv ->open_icon = mkt_atom_build_path(MKT_ATOM(widget),g_value_get_string(value));
		break;
	case PROP_ACTION_LAR_KEY_ICON_CLOSE: // widget redraw
		if(widget -> priv -> close_icon) g_free ( widget -> priv ->close_icon );
		widget -> priv -> close_icon   = mkt_atom_build_path(MKT_ATOM(widget),g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}
static void
gl_action_LarKey_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_ACTION_WIDGET (object));
	GlActionLarKey* widget = GL_ACTION_LAR_KEY(object);
	switch (prop_id)
	{
	case PROP_ACTION_LAR_KEY_ICON_OPEN:
		g_value_set_string(value,widget->priv->open_icon);
		break;
	case PROP_ACTION_LAR_KEY_ICON_CLOSE:
		g_value_set_string(value,widget->priv->close_icon);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_action_LarKey_class_init ( GlActionLarKeyClass *klass )
{
	GObjectClass*        object_class     =  G_OBJECT_CLASS (klass);
	GlActionWidgetClass* parent_class     =  GL_ACTION_WIDGET_CLASS(klass);
	object_class -> set_property          =  gl_action_LarKey_set_property;
	object_class -> get_property          =  gl_action_LarKey_get_property;
	object_class -> finalize              =  gl_action_LarKey_finalize;
	parent_class -> action_start          =  gl_action_LarKey_action_start;

	g_type_class_add_private (klass, sizeof (GlActionLarKeyPrivate));


	GParamSpec *pspec;

	pspec = g_param_spec_string("open_icon",
				"Action widget open icon",
				"Set/Get open icon path",
				"/lar/gui/unlock.png",
				G_PARAM_READABLE | G_PARAM_WRITABLE );
		g_object_class_install_property (object_class,
				PROP_ACTION_LAR_KEY_ICON_OPEN,pspec);

	pspec = g_param_spec_string("close_icon",
			"Action widget close icon",
			"Set/Get close icon path",
			"/lar/gui/encrypted.png",
			G_PARAM_READABLE | G_PARAM_WRITABLE  );
	g_object_class_install_property (object_class,
			PROP_ACTION_LAR_KEY_ICON_CLOSE,pspec);
}

