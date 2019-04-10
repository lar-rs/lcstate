/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 * 
mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-saved-object.h"
#include "market-time.h"
#include "mkt-connection.h"

enum
{
	PROP_0,
	PROP_SAVED_OBJECT_PATH,
	PROP_SAVED_OBJECT_WIDGET,
	PROP_SAVED_OBJECT_WINDOW,
};


struct _GlSavedObjectPrivate
{

	gchar     *path;
	gchar     *window;
	gchar     *widget;
};


#define GL_SAVED_OBJECT_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_SAVED_OBJECT, GlSavedObjectPrivate))

static const gchar*
gl_saved_object_get_path                          ( GlSaved *saved )
{
	g_return_val_if_fail(saved != NULL , NULL);
	g_return_val_if_fail(GL_IS_SAVED_OBJECT(saved) , NULL);
	return GL_SAVED_OBJECT(saved)->priv->path;
}

static const gchar*
gl_saved_object_get_window                        ( GlSaved *saved )
{
	g_return_val_if_fail(saved != NULL , NULL);
	g_return_val_if_fail(GL_IS_SAVED_OBJECT(saved) , NULL);
	return GL_SAVED_OBJECT(saved)->priv->window;
}

static const gchar*
gl_saved_object_get_widget                        ( GlSaved *saved )
{
	g_return_val_if_fail(saved != NULL , NULL);
	g_return_val_if_fail(GL_IS_SAVED_OBJECT(saved) , NULL);
	return GL_SAVED_OBJECT(saved)->priv->widget;
}

static void
gl_saved_object_init_item_interface ( GlSavedInterface *iface )
{
	iface->saved_path=gl_saved_object_get_path;
	iface->saved_window=gl_saved_object_get_window;
	iface->saved_widget=gl_saved_object_get_widget;
}


G_DEFINE_TYPE_WITH_CODE (GlSavedObject, gl_saved_object, MKT_TYPE_ITEM_OBJECT,
										G_IMPLEMENT_INTERFACE (GL_TYPE_SAVED,
						                       gl_saved_object_init_item_interface) )

static void
gl_saved_object_init (GlSavedObject *gl_saved_object)
{
	GlSavedObjectPrivate *priv      = GL_SAVED_OBJECT_PRIVATE(gl_saved_object);
	priv->path            = g_strdup("0");
	priv->window          = g_strdup("null");
	priv->widget          = g_strdup("null");
	gl_saved_object->priv = priv;

	/* TODO: Add initialization code here */
}

static void
gl_saved_object_finalize (GObject *object)
{
	GlSavedObject *data = GL_SAVED_OBJECT(object);
	if(data->priv->path)   g_free(data->priv->path);
	if(data->priv->window) g_free(data->priv->window);
	if(data->priv->widget) g_free(data->priv->widget);
	G_OBJECT_CLASS (gl_saved_object_parent_class)->finalize (object);
}


static void
gl_saved_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SAVED_OBJECT (object));
	GlSavedObject *data = GL_SAVED_OBJECT(object);
	switch (prop_id)
	{
	case PROP_SAVED_OBJECT_PATH:
		if(data->priv->path!= NULL)g_free(data->priv->path);
		data->priv->path = g_value_dup_string(value);
		break;
	case PROP_SAVED_OBJECT_WINDOW:
		if(data->priv->window!= NULL)g_free(data->priv->window);
		data->priv->window = g_value_dup_string(value);
		break;
	case PROP_SAVED_OBJECT_WIDGET:
		if(data->priv->widget!= NULL)g_free(data->priv->widget);
		data->priv->widget = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_saved_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SAVED_OBJECT (object));
	GlSavedObject *data = GL_SAVED_OBJECT(object);
	switch (prop_id)
	{
	case PROP_SAVED_OBJECT_PATH:
		g_value_set_string(value , data->priv->path);
		break;
	case PROP_SAVED_OBJECT_WINDOW:
		g_value_set_string(value , data->priv->window);
		break;
	case PROP_SAVED_OBJECT_WIDGET:
		g_value_set_string(value , data->priv->widget);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_saved_object_class_init (GlSavedObjectClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (GlSavedObjectPrivate));
	object_class->finalize     = gl_saved_object_finalize;
	object_class->set_property = gl_saved_object_set_property;
	object_class->get_property = gl_saved_object_get_property;

	g_object_class_override_property(object_class,PROP_SAVED_OBJECT_PATH,"saved-tree-path");
	g_object_class_override_property(object_class,PROP_SAVED_OBJECT_WINDOW,"saved-window");
	g_object_class_override_property(object_class,PROP_SAVED_OBJECT_WIDGET,"saved-widget");
}

