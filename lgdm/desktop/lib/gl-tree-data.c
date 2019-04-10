/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-tree-data.c
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
gl-tree-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-tree-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-tree-data.h"
#include <gtk/gtk.h>


struct _GlTreeDataPrivate
{
	gchar            *name;
	GtkTreeView      *tree;
	GtkTreeIter      *iter;
	GtkCellRenderer  *renderer;
	guint             colum;
	gchar            *ret_text;
	guint             type;
	gulong            tree_signal;
};

enum {
	GL_TREE_DATA_PROP_NULL,
	GL_TREE_DATA_PROP_NAME,
};

enum
{
	GL_TREE_DATA_DESTROY,
	GL_TREE_DATA_FOCUS_IN,
	GL_TREE_DATA_CHANGED,
	GL_TREE_DATA_LAST_SIGNAL
};


static guint gl_tree_data_signals[GL_TREE_DATA_LAST_SIGNAL] = { 0 };


#define GL_TREE_DATA_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_TREE_DATA, GlTreeDataPrivate))



G_DEFINE_TYPE (GlTreeData, gl_tree_data, G_TYPE_OBJECT);

static void
gl_tree_data_init (GlTreeData *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_TREE_DATA,GlTreeDataPrivate);
	object->priv->name       = g_strdup("noname");
	object->priv->ret_text   = NULL;
	object->priv->iter       = NULL;
	object->priv->tree       = NULL;
	object->priv->type       = 99;
	object->priv->tree_signal= 0;
	/* TODO: Add initialization code here */
}


static void
gl_tree_data_dispose ( GObject *object )
{

	//g_debug("TEST Dispose tree data start %d",object->ref_count);
	g_signal_emit(object,gl_tree_data_signals[GL_TREE_DATA_DESTROY],0);
	G_OBJECT_CLASS (gl_tree_data_parent_class)->dispose (object);
}

static void
gl_tree_data_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	//TEST:	g_debug("TEST Finalize tree data start %d",object->ref_count);
	GlTreeData *tdata = GL_TREE_DATA(object);
	if(tdata->priv->tree)g_signal_handler_disconnect (tdata->priv->tree ,tdata->priv->tree_signal );
	g_free(tdata->priv->name);
	if(tdata->priv->iter != NULL)     gtk_tree_iter_free(tdata->priv->iter);
	if(tdata->priv->ret_text != NULL) g_free(tdata->priv->ret_text );
	G_OBJECT_CLASS (gl_tree_data_parent_class)->finalize (object);
	//TEST:g_debug("TEST Delete tree data end");
}
/*
static void
gl_tree_data_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	//g_printf("Set (GL_MODUL) property \n");
	g_return_if_fail (GL_IS_TREE_DATA (object));
	GlTreeData *tdata = GL_TREE_DATA(object);
	switch (prop_id)
	{
	case GL_TREE_DATA_PROP_NAME:

		g_free(tdata->priv->name );
		tdata->priv->name     = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_tree_data_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	//g_printf("Get (GL_MODUL) property \n");
	g_return_if_fail (GL_IS_TREE_DATA (object));
	GlTreeData *tdata = GL_TREE_DATA(object);
	switch (prop_id)
	{
	case GL_TREE_DATA_PROP_NAME:

		g_value_set_string(value,tdata->priv->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}
*/

static void
gl_tree_data_class_init (GlTreeDataClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GlTreeDataPrivate));

	object_class->finalize          = gl_tree_data_finalize;
	object_class->dispose           = gl_tree_data_dispose;
	//object_class->set_property    = gl_tree_data_set_property;
	//object_class->get_property    = gl_tree_data_get_property;

	/*GParamSpec *pspec;
	pspec = g_param_spec_string ("name",
			"Data tree name",
			"Set data tree name",
			"noname",
			G_PARAM_READABLE | G_PARAM_WRITABLE);
	g_object_class_install_property (object_class,
			GL_TREE_DATA_PROP_NAME,pspec);*/

	gl_tree_data_signals[GL_TREE_DATA_DESTROY]  =
			g_signal_new ("tree-data-destroy",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION ,
					G_STRUCT_OFFSET (GlTreeDataClass, destroy),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_tree_data_signals[GL_TREE_DATA_FOCUS_IN]  =
			g_signal_new ("tree-data-focus-in",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlTreeDataClass, focus_in),
					NULL, NULL,
					g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1 , GTK_TYPE_ENTRY);
	gl_tree_data_signals[GL_TREE_DATA_CHANGED]  =
				g_signal_new ("tree-data-changed",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET (GlTreeDataClass, changed),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

void gl_tree_data_renderer_editing_start_cb(GtkCellRenderer *renderer,
											GtkCellEditable *editable,
											gchar           *path,
											gpointer         user_data)
{
	//g_debug("gl_tree_data_renderer_editing_start_cb %s",path);
	g_return_if_fail(user_data!= NULL);
	g_return_if_fail(GL_IS_TREE_DATA(user_data));
	GlTreeData *data = GL_TREE_DATA(user_data);
//	GtkTreeViewColumn *column    =  gtk_tree_view_get_column(GTK_TREE_VIEW(data->priv->tree),data->priv->colum);
	GtkTreeModel      *treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data->priv->tree));
	//g_debug("Set tree data %s , column = %d",gl_tree_data_get_name(data),data->priv->colum);
	//g_return_if_fail(column != NULL);
	g_return_if_fail(treemodel != NULL);
	GtkTreePath *data_path  = gtk_tree_model_get_path(treemodel,data->priv->iter);
	GtkTreePath *cell_path  = gtk_tree_path_new_from_string(path);
	if(data_path && cell_path &&  0 == gtk_tree_path_compare(data_path,cell_path))
	{
		if (GTK_IS_ENTRY (editable))
		{
			g_signal_emit(data,gl_tree_data_signals[GL_TREE_DATA_FOCUS_IN],0, GTK_ENTRY (editable));
			//gtk_tree_view_column_focus_cell (column,renderer);
		}
	}
	if(cell_path)gtk_tree_path_free(cell_path);
	if(data_path)gtk_tree_path_free(data_path);

}

void gl_tree_data_cell_edited_cb(GtkCellRendererText *cell,
        gchar               *path,
        gchar               *new_text,
        gpointer             user_data)
{
	//g_debug("gl_tree_data_cell_edited_cb");
	g_return_if_fail(user_data!= NULL);
	g_return_if_fail(GL_IS_TREE_DATA(user_data));
	GlTreeData *data = GL_TREE_DATA(user_data);
	//GtkTreeViewColumn *column    =  gtk_tree_view_get_column(GTK_TREE_VIEW(data->priv->tree),data->priv->colum);
	GtkTreeModel      *treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data->priv->tree));
	//g_debug("Set tree data %s , column = %d",gl_tree_data_get_name(data),data->priv->colum);
	//g_return_if_fail(column != NULL);
	g_return_if_fail(treemodel != NULL);
	GtkTreePath *data_path  = gtk_tree_model_get_path(treemodel,data->priv->iter);
	GtkTreePath *cell_path  = gtk_tree_path_new_from_string(path);
	if(0 == gtk_tree_path_compare(data_path,cell_path))
	{
		gtk_tree_store_set(GTK_TREE_STORE ( treemodel ),data->priv->iter,data->priv->colum,new_text,-1);
		g_signal_emit(data,gl_tree_data_signals[GL_TREE_DATA_CHANGED],0);
	}
	if(cell_path)gtk_tree_path_free(cell_path);
	if(data_path)gtk_tree_path_free(data_path);
}
void
gl_tree_data_cell_toggled_callback (GtkCellRendererToggle *cell, gchar *path, gpointer  user_data)
{
	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GL_IS_TREE_DATA(user_data));
	GlTreeData *data = GL_TREE_DATA(user_data);
	//GtkTreeViewColumn *column    =  gtk_tree_view_get_column(GTK_TREE_VIEW(data->priv->tree),data->priv->colum);
	GtkTreeModel      *treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data->priv->tree));
	//g_debug("Set tree data %s , column = %d",gl_tree_data_get_name(data),data->priv->colum);
	//g_return_if_fail(column != NULL);
	g_return_if_fail(treemodel != NULL);
	GtkTreePath *data_path  = gtk_tree_model_get_path(treemodel,data->priv->iter);
	GtkTreePath *cell_path  = gtk_tree_path_new_from_string(path);
	if(0 == gtk_tree_path_compare(data_path,cell_path))
	{
		gboolean fixed;
		gtk_tree_model_get (treemodel,data->priv->iter,data->priv->colum,&fixed,-1);
		gboolean __ON = !fixed;
		gtk_tree_store_set(GTK_TREE_STORE ( treemodel ),data->priv->iter,data->priv->colum,__ON,-1);
		//TEST:g_debug("gl_tree_data_cell_toggled_callback change data");

		g_signal_emit(data,gl_tree_data_signals[GL_TREE_DATA_CHANGED],0);
	}
	if(cell_path)gtk_tree_path_free(cell_path);
	if(data_path)gtk_tree_path_free(data_path);
}

void
gl_tree_data_change_tree_model_cd (GtkTreeView *tree,  GParamSpec *pspec ,GlTreeData *tdata )
{
	//g_debug("TEST TREE_DATA_CHANGE MODEL");
	g_object_unref(tdata);

}

GlTreeData*
gl_tree_data_new( GtkTreeView *tree,GtkCellRenderer   *renderer, GtkTreeIter *iter,guint colum )
{
	GlTreeData *tdata        = GL_TREE_DATA(g_object_new(GL_TYPE_TREE_DATA , NULL ));
	tdata->priv->iter        = gtk_tree_iter_copy(iter);
	tdata->priv->tree        = tree;
	tdata->priv->renderer    = renderer;
	tdata->priv->colum       = colum;
	tdata->priv->tree_signal = g_signal_connect(tree,"notify::model",G_CALLBACK(gl_tree_data_change_tree_model_cd),tdata);
	if(GTK_IS_CELL_RENDERER_TEXT(renderer))
	{
		g_signal_connect(renderer, "editing-started", (GCallback) gl_tree_data_renderer_editing_start_cb, tdata);
		g_signal_connect(renderer, "edited", (GCallback) gl_tree_data_cell_edited_cb, tdata);
		tdata->priv->type = GL_TREE_DATA_TYPE_TEXT;
	}
	else if(GTK_IS_CELL_RENDERER_TOGGLE(renderer))
	{
		g_signal_connect(renderer,"toggled",G_CALLBACK(gl_tree_data_cell_toggled_callback),tdata);
		tdata->priv->type = GL_TREE_DATA_TYPE_TOGGLE;
	}
	return tdata;
}

GlTreeData*
gl_tree_data_new_full( GtkTreeView *tree,GtkCellRenderer   *renderer, GtkTreeIter *iter,guint colum ,gboolean editable)
{
	GlTreeData *tdata     = GL_TREE_DATA(g_object_new(GL_TYPE_TREE_DATA , NULL ));
	tdata->priv->iter     = gtk_tree_iter_copy(iter);
	tdata->priv->tree     = tree;
	tdata->priv->renderer = renderer;
	tdata->priv->colum    = colum;

	tdata->priv->tree_signal = g_signal_connect(tree,"notify::model",G_CALLBACK(gl_tree_data_change_tree_model_cd),tdata);

	if(GTK_IS_CELL_RENDERER_TEXT(renderer) )
	{
		if(editable)
		{
			g_signal_connect(renderer, "editing-started", (GCallback) gl_tree_data_renderer_editing_start_cb, tdata);
			g_signal_connect(renderer, "edited", (GCallback) gl_tree_data_cell_edited_cb, tdata);
		}
		tdata->priv->type = GL_TREE_DATA_TYPE_TEXT;
	}
	else if(GTK_IS_CELL_RENDERER_TOGGLE(renderer) )
	{
		if(editable)
		{
			g_signal_connect(renderer,"toggled",G_CALLBACK(gl_tree_data_cell_toggled_callback),tdata);
		}
		tdata->priv->type = GL_TREE_DATA_TYPE_TOGGLE;
	}
	return tdata;
}


gchar*
gl_tree_data_get_name (GlTreeData *data)
{
	g_return_val_if_fail(data!= NULL,NULL);
	g_return_val_if_fail(GL_IS_TREE_DATA(data),NULL);
	return data->priv->name;
}
void
gl_tree_data_set_name (GlTreeData *data,const gchar *name)
{
	g_return_if_fail(data!= NULL);
	g_return_if_fail(GL_IS_TREE_DATA(data));
	g_return_if_fail(name != NULL);
	if(data->priv->name)g_free(data->priv->name);
	data->priv->name = g_strdup(name);
}
const gchar*
gl_tree_data_get_text (GlTreeData *data)
{
	g_return_val_if_fail(data!= NULL,NULL);
	g_return_val_if_fail(GL_IS_TREE_DATA(data),NULL);
	g_return_val_if_fail(data->priv->tree != NULL,NULL);
	g_return_val_if_fail(data->priv->iter != NULL,NULL);
	//g_debug("Set tree data %s , column = %d",gl_tree_data_get_name(data),data->priv->colum);
	//GtkTreeViewColumn *column    =  gtk_tree_view_get_column(GTK_TREE_VIEW(data->priv->tree),data->priv->colum);
	GtkTreeModel      *treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data->priv->tree));
	//g_return_val_if_fail(column != NULL,NULL);
	g_return_val_if_fail(treemodel != NULL,NULL);
	if(data->priv->ret_text != NULL) g_free(data->priv->ret_text );
	data->priv->ret_text = NULL;
	//g_debug("Get Tree data%s",gl_tree_data_get_name(data));
	g_return_val_if_fail(gtk_tree_store_iter_is_valid(GTK_TREE_STORE(treemodel),data->priv->iter),NULL);
	gboolean toggle_val = FALSE;
	switch(data->priv->type)
	{
		case GL_TREE_DATA_TYPE_TEXT:
			gtk_tree_model_get (treemodel,data->priv->iter,data->priv->colum,&data->priv->ret_text,-1);
			break;
		case GL_TREE_DATA_TYPE_TOGGLE:
			gtk_tree_model_get (treemodel,data->priv->iter,data->priv->colum,&toggle_val,-1);
			if(toggle_val)
				data->priv->ret_text = g_strdup("1");
			else
				data->priv->ret_text = g_strdup("0");
			break;
		default:
			data->priv->ret_text = g_strdup("0");
			break;
	}
	return data->priv->ret_text;
}

gboolean
gl_tree_data_set_text (GlTreeData *data ,gchar *text)
{
	g_return_val_if_fail(data!= NULL,FALSE);
	g_return_val_if_fail(GL_IS_TREE_DATA(data),FALSE);
	g_return_val_if_fail(text != NULL,FALSE);
	g_return_val_if_fail(data->priv->tree != NULL,FALSE);
	g_return_val_if_fail(data->priv->iter != NULL,FALSE);
	//GtkTreeViewColumn *column    =  gtk_tree_view_get_column(GTK_TREE_VIEW(data->priv->tree),data->priv->colum);
	GtkTreeModel      *treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data->priv->tree));
	//g_return_val_if_fail(column != NULL,FALSE);
	g_return_val_if_fail(treemodel != NULL,FALSE);
	//TEST:
	//g_debug("Set Tree data %s",gl_tree_data_get_name(data));
	g_return_val_if_fail(gtk_tree_store_iter_is_valid(GTK_TREE_STORE(treemodel),data->priv->iter),FALSE);

	gboolean toggle_val = FALSE;

	switch(data->priv->type)
	{
	case GL_TREE_DATA_TYPE_TEXT:
		if(data->priv->colum>0)
		{
			GType type1 = gtk_tree_model_get_column_type(treemodel,data->priv->colum-1);
			GType type2 = gtk_tree_model_get_column_type(treemodel,data->priv->colum+1);
			if( type1 == G_TYPE_BOOLEAN && type2 == G_TYPE_STRING )
			{
				//GtkTreeViewColumn *column    =  gtk_tree_view_get_column(GTK_TREE_VIEW(data->priv->tree),data->priv->colum+1);
				gboolean isOutliers = TRUE;
				gtk_tree_model_get(treemodel,data->priv->iter,data->priv->colum-1,&isOutliers,-1);
				if(!isOutliers)
					gtk_tree_store_set (GTK_TREE_STORE(treemodel),data->priv->iter,data->priv->colum+1,"yellow",-1);
				else
					gtk_tree_store_set (GTK_TREE_STORE(treemodel),data->priv->iter,data->priv->colum+1,"white",-1);
			}
			//g_debug("Set Tree data text to column %d value %s",data->priv->colum,text);
			gtk_tree_store_set (GTK_TREE_STORE(treemodel),data->priv->iter,data->priv->colum,text,-1);
		}
		break;
	case GL_TREE_DATA_TYPE_TOGGLE:
		if     (0==g_strcmp0(text,"on"   ))  toggle_val = TRUE;
		else if(0==g_strcmp0(text,"1"    ))  toggle_val = TRUE;
		else if(0==g_strcmp0(text,"TRUE" ))  toggle_val = TRUE;
		else if(0==g_strcmp0(text,"true" ))  toggle_val = TRUE;
		else toggle_val = FALSE;
		gtk_tree_store_set (GTK_TREE_STORE(treemodel),data->priv->iter,data->priv->colum,toggle_val,-1);
		GType type1 = gtk_tree_model_get_column_type(treemodel,data->priv->colum+1);
		GType type2 = gtk_tree_model_get_column_type(treemodel,data->priv->colum+2);
		if(type1 == G_TYPE_STRING && type2 == G_TYPE_STRING)
		{
			if(!toggle_val)
			{
				gtk_tree_store_set (GTK_TREE_STORE(treemodel),data->priv->iter,data->priv->colum+2,"yellow",-1);
			}
			else
			{
				gtk_tree_store_set (GTK_TREE_STORE(treemodel),data->priv->iter,data->priv->colum+2,"white",-1);
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

GtkWidget*
gl_tree_data_get_tree (GlTreeData *data)
{
	g_return_val_if_fail(data!= NULL,NULL);
	g_return_val_if_fail(GL_IS_TREE_DATA(data),NULL);
	return GTK_WIDGET(data->priv->tree);
}

guint
gl_tree_data_get_cell_type (GlTreeData *data)
{
	g_return_val_if_fail(data!= NULL,99);
	g_return_val_if_fail(GL_IS_TREE_DATA(data),99);
	return data->priv->type;
}

