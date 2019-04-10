/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-plot-tab.c
 * Copyright (C) sascha 2012 <sascha@sascha-VirtualBox>
 * 
gl-plot-tab.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-plot-tab.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-plot-tab.h"



struct _GlPlotTabPrivate
{
	GList *dataset;
};

#define GL_PLOT_TAB_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_PLOT_TAB, GlPlotTabPrivate))



G_DEFINE_TYPE (GlPlotTab, gl_plot_tab, GTK_TYPE_TOGGLE_BUTTON);

static void
gl_plot_tab_init (GlPlotTab *gl_plot_tab)
{
    GlPlotTabPrivate *priv = GL_PLOT_TAB_GET_PRIVATE(gl_plot_tab);
    gl_plot_tab->priv = priv;
    gl_plot_tab->priv->dataset = NULL;

}

static void
gl_plot_tab_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	//TEST:	g_debug("gl_plot_tab_finalize....");
	GlPlotTab *tab = GL_PLOT_TAB(object);
	GList *curr    = tab->priv->dataset;
	while( curr )
	{
		if(curr->data && GTK_PLOT_DATA(curr->data))
		{
			if(GTK_PLOT_DATA(curr->data)->plot)
			{
				 gtk_plot_remove_data(GTK_PLOT_DATA(curr->data)->plot, GTK_PLOT_DATA(curr->data));
			}
			gtk_widget_unref(curr->data);
		}
		curr = curr -> next;
	}
	g_list_free(tab->priv->dataset);
	G_OBJECT_CLASS (gl_plot_tab_parent_class)->finalize (object);
}

static void
gl_plot_tab_class_init (GlPlotTabClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkToggleButtonClass* parent_class = GTK_TOGGLE_BUTTON_CLASS (klass);
	g_type_class_add_private (klass, sizeof (GlPlotTabPrivate));
	object_class->finalize = gl_plot_tab_finalize;
}


GtkWidget*
gl_plot_tab_new (const gchar *name)
{
	GtkWidget *tab = g_object_new(GL_TYPE_PLOT_TAB,NULL);
	gl_widget_option_set_name(G_OBJECT(tab),name);
	return tab;
}


gboolean
gl_plot_tab_add_dataset( GlPlotTab *tab,GtkPlotData *dataset )
{
	g_return_val_if_fail(tab != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_PLOT_TAB(tab) ,FALSE);
	g_return_val_if_fail(dataset!=NULL,FALSE);
	g_return_val_if_fail(GTK_IS_PLOT_DATA(dataset),FALSE);
	tab->priv->dataset = g_list_append( tab->priv->dataset , dataset );
	return TRUE;
}

GList*
gl_plot_tab_get_datasets ( GlPlotTab *tab )
{
	g_return_val_if_fail(tab != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOT_TAB(tab) ,NULL);
	return tab->priv->dataset;
}

GtkPlotData*
gl_plot_tab_get_dataset ( GlPlotTab *tab , const gchar *data_name )
{
	g_return_val_if_fail(tab != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOT_TAB(tab) ,NULL);
	g_return_val_if_fail(data_name != NULL ,NULL);
	GList * curr = tab->priv->dataset;
	while(curr)
	{
		if(curr->data && GTK_IS_PLOT_DATA(curr->data) && GTK_PLOT_DATA(curr->data)->name != NULL)
		{
			if(0==g_strcmp0(data_name,GTK_PLOT_DATA(curr->data)->name)) return GTK_PLOT_DATA(curr->data);
		}
		curr= curr->next;
	}
	return NULL;
}


