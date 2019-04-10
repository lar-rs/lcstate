/*
 * gl-plot.c
 *
 *  Created on: 30.10.2012
 *      Author: sascha
 */


/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-data-curve.c
 * Copyright (C) sascha 2012 <sascha@sascha-ThinkPad-X61>
 *
 *  gl-data-curve.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gl-data-tree.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-plot.h"
#include "gl-translation.h"
#include "gl-widget-option.h"
#include "gtkextra.h"
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gtkextra.h"
#include <math.h>
#include "gl-plot-tab.h"


#define GL_DATA_PLOP_MAX_PLOTS  11


typedef struct
{
	gdouble min;
	gdouble max;

}GlPlotsRange;

typedef struct
{
	gchar     *format;
	gboolean   show_milisec;
	gint       millisec;

	gdouble    min;
	gdouble    max;
	gdouble    diff;


}GlPlotsAxeFormat;

#define GL_PLOTS_MAX_DATA_ARRAY 60

struct _GlPlotsPrivate
{
	GtkWidget           *canvas;
	gchar               *name;
	guint                format;
	GtkPlot             *main;
	GtkPlotCanvasChild  *canvas_main;

	GtkWidget           *scrollwin;
	GtkWidget           *top_crt;
	GtkWidget           *bottom_crt;
	GtkWidget           *bscrollwin;


	gdouble             width;
	gdouble             height;
	gdouble             magnification;

	GdkColor      color_line [GL_SYSTEM_MAX_MEASUREMENT_CHANNEL];
	GdkColor      color_point[GL_SYSTEM_MAX_MEASUREMENT_CHANNEL];
	GdkColor      color_axe  [GTK_PLOT_AXIS_BOTTOM+1];
	GdkColor      color_legend_fg;
	GdkColor      color_legend_bg;

	GtkWidget    *left;
	GtkWidget    *right;
	GtkWidget    *text_x;
	GtkWidget    *text_y;

   //GNode       *datasets;

	gdouble           x_pos;
	gdouble           y_pos;
	GtkPlotLine       info_line;



	GlPlotsRange      range_x;
	GlPlotsRange      range_y;
	gboolean          resize;
};







#define GL_PLOTS_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_PLOTS, GlPlotsPrivate))



G_DEFINE_TYPE (GlPlots, gl_plots, GTK_TYPE_VBOX);


#include <string.h>


void
gl_plots_activate_data_set( GlPlots *gPlots, GtkWidget *widget )
{
	g_return_if_fail(gPlots!=NULL);
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(widget!=NULL);

	GList *list = gl_plot_tab_get_datasets(GL_PLOT_TAB(widget));
	while(list)
	{
		if(list -> data && GTK_IS_PLOT_DATA(list ->data))
		{
			gtk_widget_show(GTK_WIDGET(list ->data));
			gtk_plot_data_show_legend(GTK_PLOT_DATA(list ->data));
		}
		list = list -> next;
	}
}
void
gl_plots_inactivate_data_set( GlPlots *gPlots, GtkWidget *widget )
{
	g_return_if_fail(gPlots!=NULL);
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(widget!=NULL);

	GList *list = gl_plot_tab_get_datasets(GL_PLOT_TAB(widget));

	while(list)
	{
		if(list -> data && GTK_IS_PLOT_DATA(list ->data))
		{
			gtk_widget_hide(GTK_WIDGET(list ->data));
			gtk_plot_data_hide_legend(GTK_PLOT_DATA(list ->data));
		}
		list = list -> next;
	}
}


GtkWidget*
gl_plots_get_active_button( GlPlots *gPlots )
{
	g_return_val_if_fail(gPlots != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,FALSE);
	GtkWidget *ab = NULL;
	gint i=0;
	GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
	GList *curr     = children;
	GtkWidget *widget = NULL;
	while(curr)
	{
		if(curr->data && GTK_IS_WIDGET(curr->data))
		{

			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(curr->data)))
			{
				widget = GTK_WIDGET (curr->data );
			}
		}
		curr = curr ->next;
	}
	g_list_free(children);
	return widget;
}

/*
 * Creaty newly GList
 * returned GList  need free after with g_list_free() or NULL .
 *
 */

GList *
gl_plots_get_activate_datasets( GlPlots *gPlots )
{
	GList *ret = NULL;
	GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
	GList *curr = children;
	while(curr)
	{
		if(curr->data && GTK_IS_TOGGLE_BUTTON(curr->data))
		{
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(curr->data)))
			{
				GList *temp = gl_plot_tab_get_datasets(GL_PLOT_TAB(curr->data));
				while(temp)
				{
					if(temp->data && GTK_IS_PLOT_DATA(temp->data))
					{
						//TEST:g_debug("Add Dataset %s",GTK_PLOT_DATA(temp->data)->name);
						ret = g_list_append(ret,temp->data);
					}
					temp=temp->next;
				}
			}
		}
		curr = curr ->next;
	}
	g_list_free(children);
	return ret;
}

GtkWidget*
gl_plots_get_control_button ( GlPlots *gPlots, const gchar *name  )
{
	g_return_val_if_fail(gPlots != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,NULL);
	g_return_val_if_fail(name != NULL ,NULL);
	GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
	GList *curr = children;
	while(curr)
	{
		if(curr->data && GTK_IS_WIDGET(curr->data))
		{
			GtkWidget *widget = GTK_WIDGET(curr->data);
			if(0==g_strcmp0(gl_widget_option_get_name(G_OBJECT(curr->data)),name)){g_list_free(children); return widget;}
		}
		curr = curr ->next;
	}
	g_list_free(children);
	return NULL;
}

GtkPlotData*
gl_plots_get_plot_dataset( GlPlots *gPlots ,const gchar *tab, const gchar *name)
{
	g_return_val_if_fail(gPlots != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,NULL);
	g_return_val_if_fail(tab != NULL ,NULL);
	g_return_val_if_fail(name != NULL,NULL);
	gint i;
	GtkWidget *widget = gl_plots_get_control_button(gPlots,tab);
	if(widget== NULL) return NULL;

	GtkPlotData *dataset = gl_plot_tab_get_dataset(GL_PLOT_TAB(widget),name);

	return dataset;
}

void
gl_plots_update_active_data_set ( GlPlots *gPlots,GtkWidget *activ )
{
	if(activ)
	{
		gl_plots_activate_data_set( gPlots,activ );
	}
	else
	{
		GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
		GList *curr = children;
		if(curr && curr->data !=NULL && GTK_IS_WIDGET(curr->data) )
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(curr->data),TRUE);
		}
		g_list_free(children);
	}
}



gboolean
gl_plots_activate_plot(GtkWidget *widget, gpointer data)
{
	//TEST:g_debug("SIGNAL gl_plots_activate_plot for %s button",gl_widget_option_get_name(G_OBJECT(widget)));
	g_return_val_if_fail(data != NULL,FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(data),FALSE);
	GlPlots *gPlots = GL_PLOTS(data) ;
	gint i=0;
	gPlots->priv->range_x.min = G_MAXDOUBLE;
	gPlots->priv->range_x.max = G_MINDOUBLE;
	gPlots->priv->range_y.min = G_MAXDOUBLE;
	gPlots->priv->range_y.max = G_MINDOUBLE;
	gboolean change_range = FALSE;
	GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
	GList *curr = children;
	while(curr)
	{
		if(curr->data && GTK_WIDGET(curr->data))
		{
			gtk_signal_handler_block_by_func(GTK_OBJECT(curr->data), GTK_SIGNAL_FUNC(gl_plots_activate_plot), data);
			if(curr->data != widget)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(curr->data), FALSE);
				gl_plots_inactivate_data_set(gPlots,GTK_WIDGET(curr->data));
			}
			gtk_signal_handler_unblock_by_func(GTK_OBJECT(curr->data), GTK_SIGNAL_FUNC(gl_plots_activate_plot), data);

		}
		curr = curr ->next;
	}
	g_list_free(children);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		gl_plots_update_active_data_set(gPlots,widget);
		change_range = gl_plots_refresh_active_plot_data(gPlots);
	}
	gtk_plot_canvas_paint   ( GTK_PLOT_CANVAS(gPlots->priv->canvas));
	gtk_plot_canvas_refresh ( GTK_PLOT_CANVAS(gPlots->priv->canvas));
	return FALSE;
}


GtkWidget*
gl_plots_create_control_button(   GlPlots *gPlots , const gchar *tab )
{
	g_return_val_if_fail(gPlots != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,NULL);
	g_return_val_if_fail(gPlots != NULL ,NULL);
	GtkWidget *widget = gl_plots_get_control_button(gPlots,tab);
	if(widget == NULL )
	{
	    GtkWidget   *button    = gl_plot_tab_new(tab);
	    //TEST: g_debug("CREATE BUTTON %s",tab);
		//gtk_widget_set_double_buffered(button,TRUE);
		gtk_signal_connect(GTK_OBJECT(button), "toggled",(GtkSignalFunc) gl_plots_activate_plot,gPlots);
		gtk_box_pack_start(GTK_BOX(gPlots->priv->bottom_crt),button,FALSE,FALSE,3);
		gtk_widget_show(button);
		widget = button;
	}
	const gchar *bname     = gl_system_get_input_item_text(tab);
	GdkFont     *font      = gtk_style_get_font(gtk_widget_get_style(widget));
	gint width = gdk_string_width( font ,bname);
	if(width<40)width = 40;
	width = width + 20;
	gtk_widget_set_size_request(widget,width,25);
	gtk_button_set_label(GTK_BUTTON(widget),bname);
	GtkWidget *parent = gtk_widget_get_parent(gPlots->priv->bottom_crt);
	gtk_container_set_reallocate_redraws(GTK_CONTAINER(gPlots->priv->bottom_crt),TRUE);
	gtk_container_check_resize(GTK_CONTAINER(gPlots->priv->bottom_crt));
	gtk_widget_queue_draw(gPlots->priv->bottom_crt);
	if(parent)
	{
		gtk_container_set_reallocate_redraws(GTK_CONTAINER(parent),TRUE);
		gtk_container_check_resize(GTK_CONTAINER(parent));
		gtk_widget_queue_draw(parent);
	}
	gtk_container_resize_children(GTK_CONTAINER(gPlots->priv->bscrollwin));
	gtk_widget_queue_draw(gPlots->priv->bscrollwin);
	return widget;
}

GtkPlotData*
gl_plots_append_dataset( GlPlots *gPlots , const gchar *name)
{
	g_return_val_if_fail(gPlots != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,NULL);
	g_return_val_if_fail(name != NULL,NULL);
	gint i;
	GtkPlotData      *datasets = GTK_PLOT_DATA(gtk_plot_data_new());
	gtk_plot_add_data(gPlots->priv->main, datasets);
	gtk_plot_data_set_name(datasets,name);
	gtk_plot_data_set_legend(datasets, name);
	gtk_widget_hide(GTK_WIDGET(datasets));
	gtk_plot_data_hide_legend(datasets);
	gtk_plot_data_set_line_attributes(datasets, GTK_PLOT_LINE_NONE, 0, 0, 3, &gPlots->priv->color_line[0]);
	gtk_plot_data_set_connector(datasets,GTK_PLOT_CONNECT_HV_STEP);
	gtk_plot_data_set_symbol(datasets,
			GTK_PLOT_SYMBOL_NONE,
			GTK_PLOT_SYMBOL_NONE,
			1, 2,
			&gPlots->priv->color_line[0],
			&gPlots->priv->color_line[0]);
	//TEST:g_debug("TEST_PLOTS_POS New dataset %s",name);
	return datasets;
}

GtkPlotData*
gl_plots_append_dataset_func( GlPlots *gPlots , const gchar *name,GtkPlotFunc function ,guint color )
{
	g_return_val_if_fail(gPlots != NULL ,NULL);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,NULL);
	g_return_val_if_fail(name != NULL,NULL);
	if ( color > GL_SYSTEM_MAX_MEASUREMENT_CHANNEL ) color = 0;
	gint i;
	GtkPlotData      *datasets =GTK_PLOT_DATA(gtk_plot_data_new_function(function));
	gtk_plot_add_data(gPlots->priv->main, datasets);
	gtk_plot_data_set_name(datasets,name);
	gtk_plot_data_set_legend(datasets, name);
	gtk_widget_hide(GTK_WIDGET(datasets));
	gtk_plot_data_hide_legend(datasets);
	gtk_plot_data_set_line_attributes(datasets, GTK_PLOT_LINE_SOLID, 0, 0, 3, &gPlots->priv->color_line[color]);
	gtk_plot_data_set_symbol(datasets,
			GTK_PLOT_SYMBOL_SQUARE,
			GTK_PLOT_SYMBOL_OPAQUE,
			2, 1,
			&gPlots->priv->color_line[0],
			&gPlots->priv->color_line[0]);
	gtk_widget_hide(GTK_WIDGET(datasets));
	gtk_plot_data_hide_legend(datasets);
	//TEST:g_debug("TEST_PLOTS_POS New dataset %s",name);
	return datasets;
}


gboolean
gl_plots_add_dataset_to_plot ( GlPlots *gPlots , const gchar *tab_name , GtkPlotData *pData )
{
	g_return_val_if_fail(gPlots != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) ,FALSE);
	g_return_val_if_fail(tab_name!=NULL,FALSE);
	g_return_val_if_fail(pData!=NULL,FALSE);
	g_return_val_if_fail(GTK_IS_PLOT_DATA(pData),FALSE);
	GtkWidget *widget = gl_plots_create_control_button(gPlots,tab_name);
	g_return_val_if_fail(widget!=NULL,FALSE);
	return gl_plot_tab_add_dataset(GL_PLOT_TAB(widget),pData);
}


static void
gl_data_curve_y_axe_format_set_label(GlPlots *gPlots)
{
	gint i = 0;
	gchar **labels = NULL;
	gdouble curr =  0;
	gint exp = 0;
	gint millisec = 0;
	glong sec     = 0;
	gchar *ms = NULL;
	GtkPlotArray *array;
	gchar *temp;
	gdouble float_1=FALSE;
	if(gPlots->priv->range_y.max - gPlots->priv->range_y.min <= 10.) float_1= TRUE;
	gdouble df =  (gPlots->priv->range_y.max - gPlots->priv->range_y.min)/10;
	curr = gPlots->priv->range_y.min;
	for (i=0;i<11;i++)
	{
			//temp = g_strdup_printf("%s%s",gl_system_get_time_format_string((long int) curr,format->format),ms);

		if(float_1)
			temp = g_strdup_printf(" %.1f ",curr);
		else
			temp = g_strdup_printf(" %.0f ",curr);
		labels = gl_strvadd_part_and_free(labels,temp);
	//	g_debug("curr = %f millisec =%d",curr,curr);
		g_free(temp);
		curr = curr + df;
	}
	//if(labels == NULL ) g_debug ("Labels == NULL ");
	//if(labels[0] == NULL ) g_debug ("Labels[0]== NULL ");
	//for( i=0; labels!=NULL &&  labels[i]!= NULL; i++ ) g_debug("Set Label %d %s",i,labels[i]);
	array = GTK_PLOT_ARRAY(gtk_plot_array_new(NULL, labels, 11, GTK_TYPE_STRING, TRUE));
	gtk_plot_axis_set_tick_labels(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT), array);
	g_object_unref(array);
	gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT), TRUE);
}


gboolean
gl_plots_refresh_active_plot_data(GlPlots *gPlots  )
{

	//TEST:g_debug("\ngl_plots_refresh_active_plot_data");
	gPlots->priv->range_y.min=  0.;
	gPlots->priv->range_y.max=  1.;
	gPlots->priv->range_x.min=  G_MAXDOUBLE;
	gPlots->priv->range_x.max=  0;
	GList *list        =  gl_plots_get_activate_datasets(gPlots);
	GList *curr        =  NULL;

	gtk_plot_axis_move_title(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT),90,0.02,0.5);
	gPlots->priv->resize      = FALSE;
	gint i;
	for ( curr = list ;curr!=NULL&&curr->data&&GTK_IS_PLOT_DATA(curr->data);curr=curr->next )
	{
		if(!GTK_PLOT_DATA(curr->data)->is_function)
		{

			gdouble min,max;
			gtk_plot_data_get_range(GTK_PLOT_DATA(curr->data),&min,&max,"x");
			//g_debug("DataSet X_RANGE %s Xmin=%f Xmax=%f",gPlots->priv->datasets[i]->name,min,max);
			if(min<gPlots->priv->range_x.min ){ gPlots->priv->resize= TRUE;gPlots->priv->range_x.min = min; }
			if(max>gPlots->priv->range_x.max ){ gPlots->priv->resize= TRUE;gPlots->priv->range_x.max = max; }


			gtk_plot_data_get_range(GTK_PLOT_DATA(curr->data),&min,&max,"y");
			//g_debug("DataSet Y_RANGE %s Ymin=%f Ymax=%f",gPlots->priv->datasets[i]->name,min,max);
			if(min<gPlots->priv->range_y.min ){ gPlots->priv->resize= TRUE;gPlots->priv->range_y.min = min; }
			if(max>gPlots->priv->range_y.max ){ gPlots->priv->resize= TRUE;gPlots->priv->range_y.max = max; }
		}

	}
	if(gPlots->priv->resize )
	{
		//TEST:g_debug("RESIZE X min %f max %f diff = %f",gPlots->priv->range_x.min,gPlots->priv->range_x.max,gPlots->priv->range_x.max-gPlots->priv->range_x.min);
		// Set X axe scaling value
		if(gPlots->priv->range_x.max == gPlots->priv->range_x.min) { gPlots->priv->range_x.max = gPlots->priv->range_x.max + 1.;}
		if(gPlots->priv->range_x.min > gPlots->priv->range_x.max)  { gPlots->priv->range_x.max = gPlots->priv->range_x.min + 1.;}
		gtk_plot_set_xrange(gPlots->priv->main,gPlots->priv->range_x.min,gPlots->priv->range_x.max );
		gtk_plot_set_ticks(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_X, (gPlots->priv->range_x.max-gPlots->priv->range_x.min)/8.0,1);


		// Set Y axe scaling value
		if(gPlots->priv->range_y.max == gPlots->priv->range_y.min) { gPlots->priv->range_y.max = gPlots->priv->range_y.max + 1.;}
		gtk_plot_set_ticks(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_Y, (gPlots->priv->range_y.max-gPlots->priv->range_y.min)/10.0,1);
		gtk_plot_set_yrange(gPlots->priv->main,gPlots->priv->range_y.min,gPlots->priv->range_y.max );
		gl_data_curve_y_axe_format_set_label(gPlots);
		//gl_plots_y_axe_format_set_label(gPlots,NULL);
		//gtk_plot_axis_ticks_autoscale(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT),gPlots->priv->range_x.min,gPlots->priv->range_x.max,&precis);
	}
	if(gPlots->priv->resize)
	{
		gtk_plot_set_range(GTK_PLOT(gPlots->priv->main), gPlots->priv->range_x.min ,gPlots->priv->range_x.max, gPlots->priv->range_y.min, gPlots->priv->range_y.max);
	}
	for ( curr = list; curr!=NULL&&curr->data&&GTK_IS_PLOT_DATA(curr->data); curr=curr->next )
	{
		gtk_widget_show(GTK_WIDGET(GTK_PLOT_DATA(curr->data)));
		gtk_plot_data_show_legend(GTK_PLOT_DATA(curr->data));
		gtk_plot_data_update(GTK_PLOT_DATA(curr->data));
		gtk_plot_data_paint(GTK_PLOT_DATA(curr->data));

	}
	if(gPlots->priv->resize )
	{
		gtk_plot_canvas_paint ( GTK_PLOT_CANVAS(gPlots->priv->canvas));
		gtk_plot_canvas_refresh ( GTK_PLOT_CANVAS(gPlots->priv->canvas));
		gtk_plot_paint(gPlots->priv->main);
	}
	else
	{
		gtk_plot_canvas_paint ( GTK_PLOT_CANVAS(gPlots->priv->canvas));
		gtk_plot_canvas_refresh ( GTK_PLOT_CANVAS(gPlots->priv->canvas));
		gtk_plot_paint(gPlots->priv->main);
	}
	if(list)g_list_free(list);
	return TRUE;
}


gboolean
gl_plots_left_clicked (GtkWidget *widget, gpointer data)
{
	//TEST:g_debug("Left clicked ");
	GlPlots *gPlots = GL_PLOTS(data);
//	GtkPlotAxis *axis = gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT);
	return TRUE;
}

gboolean
gl_plots_right_clicked (GtkWidget *widget, gpointer data)
{
	//TEST:g_debug("Right clicked ");
	GlPlots *gPlots = GL_PLOTS(data);

	return TRUE;
}

gint
gl_plots_select_item(GtkWidget *widget, GdkEvent *event, GtkPlotCanvasChild *child,
            gpointer data)
{
  GtkWidget **widget_list = NULL;
  GtkWidget *active_widget = NULL;
  GlPlots *gPlots = GL_PLOTS(data);
  gint n = 0;
  gdouble *x = NULL, *y = NULL;
  static gdouble y1,y2;
  if(GTK_IS_PLOT_CANVAS_TEXT(child))
  {
        printf("Item selected: TEXT\n");
        return FALSE;
  }
  if(GTK_IS_PLOT_CANVAS_PIXMAP(child))
  {
        printf("Item selected: PIXMAP\n");
        return FALSE;
  }
  if(GTK_IS_PLOT_CANVAS_RECTANGLE(child)){
	   printf("Item selected: RECTANGLE\n");
	   return FALSE;}
  if(GTK_IS_PLOT_CANVAS_ELLIPSE(child)){
        printf("Item selected: ELLIPSE\n");
        return FALSE;
  }
  if(GTK_IS_PLOT_CANVAS_LINE(child)){
        printf("Item selected: LINE\n");
        return FALSE;}
  if(GTK_IS_PLOT_CANVAS_PLOT(child)){
    switch(GTK_PLOT_CANVAS_PLOT(child)->pos){
      case GTK_PLOT_CANVAS_PLOT_IN_TITLE:
    	  printf("Item selected: TITLE\n");
    	  return FALSE;
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_LEGENDS:
        printf("Item selected: LEGENDS\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_PLOT:
    	  printf("Item selected: PLOT\n");
    	  return FALSE;
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_AXIS:
        printf("Item selected: AXIS\n");
        return FALSE;
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_MARKER:
        printf("Item selected: MARKER\n");
        return FALSE;
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_GRADIENT:
        printf("Item selected: GRADIENT\n");
        return FALSE;
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_DATA:

        /* = gtk_plot_data_get_x(GTK_PLOT_CANVAS_PLOT(child)->data, &n);
        gtk_plot_data_show_yerrbars(GTK_PLOT_CANVAS_PLOT(child)->data);
        y = gtk_plot_data_get_y(GTK_PLOT_CANVAS_PLOT(child)->data, &n);
        n = GTK_PLOT_CANVAS_PLOT(child)->datapoint;
        printf("Item selected: DATA\n");
        printf("Active point: %d -> %f %f\n",
                GTK_PLOT_CANVAS_PLOT(child)->datapoint, x[n], y[n]);
        gtk_plot_get_yrange(gPlots->priv->main,&y1,&y2);
        gtk_plot_draw_line(GTK_PLOT(gPlots->priv->main),gPlots->priv->info_line,x[n],y[n],x[n],y2);
        gtk_plot_data_remove_markers(GTK_PLOT_CANVAS_PLOT(child)->data);
        gtk_plot_data_add_marker(GTK_PLOT_CANVAS_PLOT(child)->data, n-1?n-1:0);*/
        return FALSE;
        break;
      default:
        break;
    }

    //widget_list = plots;
    active_widget = GTK_WIDGET(GTK_PLOT_CANVAS_PLOT(child)->plot);
  }

  return TRUE;
}

static void
gl_plots_create_canvas ( GlPlots *gPlots )
{
	//TEST:	g_debug("PLOTS_CREATE_CANVAS");
	g_return_if_fail(gPlots != NULL);
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
	GList *curr = children;
	while(curr)
	{
		if(curr->data && GTK_IS_WIDGET(curr->data))
		{
			gtk_container_remove(GTK_CONTAINER(gPlots->priv->bottom_crt),GTK_WIDGET(curr->data));
		}

		curr = curr->next;
	}
	g_list_free(children);
	gPlots->priv->range_x.min = G_MAXDOUBLE;
	gPlots->priv->range_x.max = G_MINDOUBLE;
	gPlots->priv->range_y.min = G_MAXDOUBLE;
	gPlots->priv->range_y.max = G_MINDOUBLE;

	/*if(gPlots->priv->left)  gtk_container_remove(GTK_CONTAINER(gPlots),gPlots->priv->left);
	if(gPlots->priv->right) gtk_container_remove(GTK_CONTAINER(gPlots),gPlots->priv->right);

	if(gPlots->priv->text_x)  gtk_container_remove(GTK_CONTAINER(gPlots),gPlots->priv->text_x);
	if(gPlots->priv->text_y) gtk_container_remove(GTK_CONTAINER(gPlots),gPlots->priv->text_y);



	gPlots->priv->left    = gtk_button_new_with_label("Left");
	gPlots->priv->right   = gtk_button_new_with_label("Right");

	gPlots->priv->text_x  = gtk_label_new("---");
	gPlots->priv->text_y  = gtk_label_new("---");
	gtk_widget_set_size_request(gPlots->priv->text_x,50,20);
	gtk_widget_set_size_request(gPlots->priv->text_y,50,20);

	gtk_widget_set_size_request(gPlots->priv->left,50,40);
	gtk_widget_set_size_request(gPlots->priv->right,50,40);

	gtk_signal_connect(GTK_OBJECT(gPlots->priv->left), "clicked",(GtkSignalFunc) gl_plots_left_clicked,gPlots);
	gtk_signal_connect(GTK_OBJECT(gPlots->priv->right), "clicked",(GtkSignalFunc) gl_plots_right_clicked,gPlots);

	GtkRequisition req;
	gtk_widget_size_request(GTK_WIDGET(gPlots), &req);
	gtk_fixed_put(GTK_FIXED(gPlots),gPlots->priv->left , req.width/4,req.height-40 );
	gtk_fixed_put(GTK_FIXED(gPlots),gPlots->priv->text_x , req.width/4+70,req.height-40 );
	gtk_fixed_put(GTK_FIXED(gPlots),gPlots->priv->text_y , req.width/4+70,req.height-20 );
	gtk_fixed_put(GTK_FIXED(gPlots),gPlots->priv->right , req.width/4+70+70,req.height-40 );
	gtk_widget_show(gPlots->priv->left);
	gtk_widget_show(gPlots->priv->right);
	gtk_widget_show(gPlots->priv->text_x);
	gtk_widget_show(gPlots->priv->text_y);*/


	GlTranslation *translation = GL_TRANSLATION(gl_system_get_object("translation"));
	gint channel_n =0;
	gint channels = 0;

	if(gPlots->priv->main != NULL ){gtk_plot_canvas_remove_child(GTK_PLOT_CANVAS(gPlots->priv->canvas), GTK_PLOT_CANVAS_CHILD(gPlots->priv->canvas_main));}
	//FIX:wenn nicht free gegeben wird



	gPlots->priv->main = GTK_PLOT(gtk_plot_new_with_size(NULL, .1, .085));


	 gtk_plot_set_legends_border(GTK_PLOT(gPlots->priv->main), 2, 3);
	 gtk_plot_set_transparent(gPlots->priv->main,TRUE);
	 GdkColor color;
	 gdk_color_parse("white", &color);
	 gdk_color_alloc(gtk_widget_get_colormap(GTK_WIDGET(gPlots)), &color);
	 gtk_plot_legends_set_attributes(GTK_PLOT(gPlots->priv->main),
	                                  NULL, 0,
	                 				  NULL,
	                                  &color);


/*	 gtk_plot_legends_set_attributes(GTK_PLOT(gPlots->priv->main),
			                                      NULL, 0,
			 	                                 NULL,
			 	                                 &gPlots->priv->color_legend_bg);*/

	 gtk_plot_axis_show_labels(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main),GTK_PLOT_AXIS_TOP),GTK_PLOT_LABEL_NONE);
	 gtk_plot_axis_show_labels(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main),GTK_PLOT_AXIS_RIGHT),GTK_PLOT_LABEL_NONE);

	 gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_BOTTOM), 0, 0);
	 gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT), 0, 0);
	 //gtk_plot_set_yscale (GTK_PLOT(gPlots->priv->main),GTK_PLOT_SCALE_LINEAR);

	 //gtk_plot_x0_set_visible(GTK_PLOT(gPlots->priv->main), TRUE);
	 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_BOTTOM), TRUE);
	// gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_RIGHT), FALSE);
	// gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_TOP), FALSE);
	 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT), TRUE);


	 //gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_RIGHT), TRUE);

	 gtk_plot_grids_set_visible(GTK_PLOT(gPlots->priv->main), TRUE, TRUE, TRUE, TRUE);
	// gtk_plot_axis_use_custom_tick_labels()


	 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_TOP));
	 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_RIGHT));

	 gtk_plot_set_range(GTK_PLOT(gPlots->priv->main), 0. ,1., 0., 0.1);
	 gtk_plot_set_ticks(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_X, 0.1, 1);
	 gtk_plot_set_ticks(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_Y, 0.01, 1);

	 gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_BOTTOM),"mg/l");
	 gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT),"FSR/sec");
	//gtk_plot_set_yscale(GTK_PLOT(gPlots->priv->main),GTK_PLOT_SCALE_LINEAR);

	// gtk_plot_axis_set_labels_offset(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT),0);


	 //gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT),.1,1.);
	 //gtk_plot_axis_set_attributes(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT),2.0,&color);


	 gtk_plot_legends_move(GTK_PLOT(gPlots->priv->main), .450, .05);
	 gtk_plot_set_legends_border(GTK_PLOT(gPlots->priv->main), 1, 1);
	 gtk_plot_clip_data(GTK_PLOT(gPlots->priv->main),TRUE);



	/*


	//gtk_plot_reflect_x (GTK_PLOT(gPlots->priv->main), TRUE);
	//gtk_plot_reflect_y (GTK_PLOT(gPlots->priv->main), TRUE);

	//gtk_plot_set_break(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_Y, 0.7, 0.72, .05, 4, GTK_PLOT_SCALE_LINEAR, .6);


	/*GtkPlotAxis *axis = gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_RIGHT);
	gtk_signal_connect(GTK_OBJECT(axis), "tick_label",
	                    GTK_SIGNAL_FUNC(my_tick_label_Y), gPlots);
	axis = gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_BOTTOM);
	gtk_signal_connect(GTK_OBJECT(axis), "tick_label",
		                    GTK_SIGNAL_FUNC(my_tick_label_X), gPlots);
	*/
	//gtk_plot_axis_show_ticks(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), 15, 3);
	/*gtk_plot_axis_set_labels_numbers(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT), GTK_PLOT_LABEL_FLOAT, 2);
	gtk_plot_set_range(GTK_PLOT(gPlots->priv->main), 0. ,1., 0., 1.);
	gtk_plot_set_ticks(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_X, 2, 1);
	gtk_plot_set_ticks(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_Y, .1, 1);

	gtk_plot_grids_set_visible(GTK_PLOT(gPlots->priv->main), TRUE, TRUE, TRUE, TRUE);
	//gtk_plot_axis_set_attributes(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_TOP),25.,&gPlots->priv->color_axe[GTK_PLOT_AXIS_BOTTOM]);
	gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_RIGHT), TRUE);
	gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_TOP), TRUE);
	gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_LEFT), TRUE);

	gtk_plot_axis_show_labels(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), GTK_PLOT_AXIS_TOP),GTK_PLOT_LABEL_NONE);
	gtk_plot_set_legends_border(GTK_PLOT(gPlots->priv->main), 2, 3);
	gtk_plot_set_transparent(gPlots->priv->main,TRUE);
	/*gtk_plot_legends_set_attributes(GTK_PLOT(gPlots->priv->main),
	                                 "Oreal", 0,
	                                 &gPlots->priv->color_legend_fg,
	                                 &gPlots->priv->color_legend_bg);*/

	//gtk_plot_axis_set_labels_offset(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT),gl_system_get_channel_name(channel_n));
	//gtk_plot_axis_set_labels_offset(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM),"time");

	//gtk_plot_axis_set_labels_suffix(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT), gl_system_get_channel_unit(channel_n));
	//gtk_plot_axis_set_labels_suffix(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT), gl_system_get_channel_unit(channel_n));


	/**/

	//gtk_plot_set_xscale(GTK_PLOT(gPlots->priv->main),GTK_PLOT_SCALE_LINEAR);
	//gtk_plot_set_yscale(GTK_PLOT(gPlots->priv->main),GTK_PLOT_SCALE_LINEAR);


	gPlots->priv->canvas_main = gtk_plot_canvas_plot_new(GTK_PLOT(gPlots->priv->main));
	gtk_plot_canvas_put_child( GTK_PLOT_CANVAS(gPlots->priv->canvas), gPlots->priv->canvas_main, .1, .05, .85, .85);
	//gtk_plots_resize(gPlots);
	gtk_widget_show(GTK_WIDGET(gPlots->priv->main));
	GTK_PLOT_CANVAS_PLOT(gPlots->priv->canvas_main)->flags |= GTK_PLOT_CANVAS_PLOT_SELECT_POINT;
	GTK_PLOT_CANVAS_PLOT(gPlots->priv->canvas_main)->flags |= GTK_PLOT_CANVAS_PLOT_DND_POINT;

	//Create cont
	gtk_signal_connect(GTK_OBJECT(gPlots->priv->canvas), "select_item",
	                    (GtkSignalFunc) gl_plots_select_item, gPlots);
	gtk_plot_canvas_paint(GTK_PLOT_CANVAS(gPlots->priv->canvas));
	gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(gPlots->priv->canvas));
	//TEST:g_debug("PLOTS_CREATE_CANVAS End ");
}



void
gl_plots_dataset_set_line_attributes(GlPlots *gPlots, const gchar *tab , const gchar *name,
		                                   GtkPlotLineStyle style,
		                                   GdkCapStyle cap_style,
		                                   GdkJoinStyle join_style,
		                                   gfloat width,const GdkColor *color)
{
	g_return_if_fail(gPlots != NULL );
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(name != NULL);
	GtkPlotData *pData = gl_plots_get_plot_dataset(gPlots,tab,name);
	g_return_if_fail(pData!=NULL);
	const GdkColor *col = color?color:&gPlots->priv->color_line[0];
	gtk_plot_data_set_line_attributes(pData, style,cap_style, join_style, width,col);
}

void
gl_plots_dataset_set_symbol(GlPlots *gPlots,  const gchar *tab , const gchar *name,
		                                      GtkPlotSymbolType type,
		                                      GtkPlotSymbolStyle style,
                                              gint size, gfloat line_width ,
                                              const GdkColor *color, const GdkColor *border_color)
{
	g_return_if_fail(gPlots != NULL );
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(name != NULL);
	GtkPlotData *pData = gl_plots_get_plot_dataset(gPlots,tab,name);
	g_return_if_fail(pData!=NULL);
	const GdkColor *col  = color?color:&gPlots->priv->color_point[0];
	const GdkColor *bcol = border_color?border_color:&gPlots->priv->color_point[0];
    gtk_plot_data_set_symbol(pData,type,style,size,line_width, col,bcol);
}

void
gl_plots_dataset_set_legend(GlPlots *gPlots, const gchar *tab , const gchar *name,const gchar *legend)
{
	g_return_if_fail(gPlots != NULL );
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(name != NULL);
	GtkPlotData *pData = gl_plots_get_plot_dataset(gPlots,tab,name);
	g_return_if_fail(pData!=NULL);
	gtk_plot_data_set_legend(pData, legend);

}

void
gl_plots_dataset_set_connector(GlPlots *gPlots, const gchar *tab , const gchar *name,GtkPlotConnector connector)
{
	g_return_if_fail(gPlots != NULL );
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(name != NULL);
	GtkPlotData *pData = gl_plots_get_plot_dataset(gPlots,tab,name);
	g_return_if_fail(pData!=NULL);
	gtk_plot_data_set_connector(pData,connector);

}

void
gl_plots_set_axis_title ( GlPlots *gPlots ,GtkPlotAxisPos axis , const gchar *title )
{
	gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(gPlots->priv->main), axis),title);


}

/*
 * x and y arrays must have the same length
 *
*/

gboolean
gl_plots_clean_dataset( GlPlots *gPlots, const gchar *tab  ,const gchar *name)
{
	g_return_val_if_fail(gPlots != NULL , FALSE);
    g_return_val_if_fail(GL_IS_PLOTS(gPlots) , FALSE);
    g_return_val_if_fail(name!=NULL,FALSE);
    //TEST:g_debug("gl_plots_clean_dataset");
	GtkPlotData *pData = gl_plots_get_plot_dataset(gPlots,tab,name);
	g_return_val_if_fail(pData != NULL,FALSE);
	pData->is_use = FALSE;
	pData->active = FALSE;
	gint len_x = 0;
	gint len_y = 0;
	gdouble *px = gtk_plot_data_get_x(pData,&len_x);
	gdouble *py = gtk_plot_data_get_x(pData,&len_y);
	if(len_x!=0 && len_x==len_y)
	{
		g_free(px);
		g_free(py);
	}
	GtkWidget *widget = gl_plots_get_active_button(gPlots);
	gl_plots_update_active_data_set   ( gPlots  , widget);
	gl_plots_refresh_active_plot_data ( gPlots  );
	gtk_widget_queue_draw(GTK_WIDGET(gPlots->priv->canvas));
	return TRUE;
}

gboolean
gl_plots_add_dataset (  GlPlots *gPlots,const gchar *tab ,const gchar *name ,gdouble *x , gdouble *y ,glong size )
{
	g_return_val_if_fail(gPlots != NULL , FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) , FALSE);
	g_return_val_if_fail(name!=NULL,FALSE);
	g_return_val_if_fail(x!=NULL,FALSE);
	g_return_val_if_fail(y!=NULL,FALSE);
	g_return_val_if_fail(size != 0,FALSE);
	GtkPlotData *pData = gl_plots_get_plot_dataset(gPlots,tab,name);

	if(pData == NULL)
	{
		pData = gl_plots_append_dataset(gPlots,name);
		g_return_val_if_fail(GTK_IS_PLOT_DATA(pData),FALSE);
		if(pData) gl_plots_add_dataset_to_plot(gPlots,tab,pData);
		g_return_val_if_fail(GTK_IS_PLOT_DATA(pData),FALSE);

	}
	g_return_val_if_fail(pData != NULL,FALSE);
	g_return_val_if_fail(GTK_IS_PLOT_DATA(pData),FALSE);

	gint len_x=0;
	gint len_y=0;
	gdouble *px = gtk_plot_data_get_x(pData,&len_x);
	gdouble *py = gtk_plot_data_get_y(pData,&len_y);

	px = (gdouble *)g_realloc(px, (len_x+size)*sizeof(gdouble));
    py = (gdouble *)g_realloc(py, (len_x+size)*sizeof(gdouble));

    gint i=0;
    for(;i<size;i++)
    {
    	px[len_x+i] = x[i];
    	py[len_x+i] = y[i];
    }
	 gtk_plot_data_set_numpoints(pData, len_x+size);
	 gtk_plot_data_set_x(pData, px);
	 gtk_plot_data_set_y(pData, py);
	 GtkWidget *widget = gl_plots_get_active_button(gPlots);
	 gl_plots_update_active_data_set   ( gPlots ,widget );
	 gl_plots_refresh_active_plot_data ( gPlots  );
	 gtk_widget_queue_draw(GTK_WIDGET(gPlots->priv->canvas));
     return TRUE;
}

gdouble
gl_plots_lineal_regression_function(GtkPlot *plot, GtkPlotData *data, gdouble x, gboolean *err)
{
	gdouble y;
    *err = FALSE;
    y = x*data->slope+data->intercept;
    return y;
}

gboolean
gl_plots_add_dataset_func_lineal (  GlPlots *gPlots,const gchar *tab ,const gchar *name , gdouble  slope , gdouble intercept ,guint color )
{
	//TEST:g_debug("gl_plots_add_dataset_func_lineal");
	g_return_val_if_fail(gPlots != NULL , FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) , FALSE);
	g_return_val_if_fail(name!=NULL,FALSE);

	GtkPlotData *dataset = gl_plots_get_plot_dataset(gPlots,tab,name);
    if(dataset == NULL)
    {
    	dataset = gl_plots_append_dataset_func(gPlots,name,gl_plots_lineal_regression_function,color);
    	if(dataset) gl_plots_add_dataset_to_plot(gPlots,tab,dataset);
    }
    g_return_val_if_fail(dataset!=NULL,FALSE);
    dataset->slope = slope ;
    dataset->intercept = intercept;
    GtkWidget *widget = gl_plots_get_active_button(gPlots);
    gl_plots_update_active_data_set   ( gPlots ,widget );
    gl_plots_refresh_active_plot_data ( gPlots  );
    gtk_widget_queue_draw(GTK_WIDGET(gPlots->priv->canvas));
	return TRUE;
}

void
gl_plots_set_new_file_data ( GlFile *file, GlDataArray *data , GlPlots *gPlots)
{
	g_return_if_fail(gPlots != NULL );
	g_return_if_fail(GL_IS_PLOTS(gPlots));
	g_return_if_fail(file != NULL);
	g_return_if_fail(GL_IS_FILE(file));
	gint bi = 0;

}



static gint    __width         =0;
static gint    __height        =0;
static gdouble __magnification =0;

static void
gl_plots_init (GlPlots *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_PLOTS,GlPlotsPrivate);
	object->priv->name         = g_strdup("measurement");
	object->priv->format       = GL_FILE_TYPE_UNKNOWN;
	object->priv->left         = NULL;
	object->priv->right        = NULL;
	object->priv->text_x       = NULL;
	object->priv->text_y       = NULL;
	object->priv->canvas       = NULL;
	object->priv->main         = NULL;
	object->priv->canvas_main  = NULL;



	object->priv->scrollwin    = NULL;
	object->priv->top_crt      = NULL;
	object->priv->bottom_crt   = NULL;


	object->priv->width        = 0.5;
	object->priv->height       = 0.5;
	object->priv->magnification= 1.;

	object->priv->x_pos        = 0.0;
	object->priv->y_pos        = 0.0;
	object->priv->range_x.min  = G_MAXDOUBLE;
	object->priv->range_x.max  = G_MINDOUBLE;
	object->priv->range_y.min  = G_MAXDOUBLE;
	object->priv->range_y.max  = G_MINDOUBLE;

	object->priv->resize       = FALSE;


	g_object_set(object,"homogeneous",FALSE,"spacing",0,NULL);


	object->priv->top_crt = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(object),object->priv->top_crt, FALSE, FALSE,2);
	gtk_widget_set_size_request(object->priv->top_crt,-1,50);
	gtk_widget_show(object->priv->top_crt);

	object->priv->scrollwin=gtk_scrolled_window_new(NULL, NULL);
	gtk_container_border_width(GTK_CONTAINER(object->priv->scrollwin),1);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(object->priv->scrollwin),
			GTK_POLICY_NEVER,GTK_POLICY_NEVER);
	gtk_box_pack_start(GTK_BOX(object),object->priv->scrollwin, TRUE, TRUE,0);
	gtk_widget_show(object->priv->scrollwin);
	object->priv->canvas = gtk_plot_canvas_new(__width, __height, __magnification);
	GTK_PLOT_CANVAS_UNSET_FLAGS(GTK_PLOT_CANVAS(object->priv->canvas), GTK_PLOT_CANVAS_DND_FLAGS);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(object->priv->scrollwin), object->priv->canvas);



	object->priv->bscrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(object->priv->bscrollwin),
				GTK_POLICY_AUTOMATIC,GTK_POLICY_NEVER);
	GtkWidget *viewport = gtk_viewport_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(object->priv->bscrollwin), viewport);
	GtkWidget *box = gtk_vbox_new(FALSE,0);
	object->priv->bottom_crt = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),object->priv->bottom_crt ,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(viewport),box);

	gtk_box_pack_start(GTK_BOX(object->priv->top_crt),object->priv->bscrollwin, TRUE, TRUE,0);
	gtk_widget_show(object->priv->bottom_crt);
	gtk_widget_show(box);
	gtk_widget_show(viewport);
	gtk_widget_show(object->priv->bscrollwin);

	//gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(object->priv->canvas), &color);

	/*GdkColor color;
	gdk_color_parse("light blue", &color);
	gdk_color_alloc(gtk_widget_get_colormap(object->priv->canvas), &color);
	gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(object->priv->canvas), &color);*/
	gtk_widget_show(object->priv->canvas);

	gint i;
	for(i=0;i<GL_SYSTEM_MAX_MEASUREMENT_CHANNEL;i++)
	{
		//XXXXX
		gdk_color_parse("red", &object->priv->color_point[i]);
		switch(i)
		{
		case 0: gdk_color_parse("dark blue", &object->priv->color_line[i]);
		//gdk_color_parse("dark slate blue", &object->priv->color_point[i]);
		break;
		case 1: gdk_color_parse("red", &object->priv->color_line[i]);
	//	gdk_color_parse("dark slate blue", &object->priv->color_point[i]);
		break;
		case 2: gdk_color_parse("medium blue", &object->priv->color_line[i]);
		//gdk_color_parse("dark slate blue", &object->priv->color_point[i]);
		break;
		case 3: gdk_color_parse("deep sky blue", &object->priv->color_line[i]);
		//gdk_color_parse("dark slate blue", &object->priv->color_point[i]);
		break;
		default:gdk_color_parse("dark grey", &object->priv->color_line[i]);
		//gdk_color_parse("red", &object->priv->color_point[i]);
		break;
		}
	}

	gdk_color_parse("dark grey", &object->priv->color_axe[GTK_PLOT_AXIS_BOTTOM] );
	gdk_color_parse("black",&object->priv->info_line.color);

	//gdk_color_parse("light grey", &object->priv->color_legend_fg );
	gdk_color_parse("white", &object->priv->color_legend_bg );
	object->priv->info_line.line_style = GTK_PLOT_LINE_SOLID;
	object->priv->info_line.cap_style  = 2;
	object->priv->info_line.join_style = 1;
	object->priv->info_line.line_width = 4;
	gl_plots_create_canvas ( object );
	//gtk_widget_set_double_buffered(GTK_WIDGET(object),TRUE);
	/* TODO: Add initialization code here */
}

gboolean
gl_plots_clean          ( GlPlots *gPlots )
{
	//TEST:g_debug("Clean .....");
	g_return_val_if_fail(gPlots != NULL , FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) , FALSE);
	GList *children = gtk_container_get_children(GTK_CONTAINER(gPlots->priv->bottom_crt));
	GList *curr = children;
	while(curr)
	{
		if(curr->data && GTK_IS_WIDGET(curr->data))
		{
			gtk_container_remove(GTK_CONTAINER(gPlots->priv->bottom_crt),GTK_WIDGET(curr->data));
		}

		curr = curr->next;
	}
	g_list_free(children);
	gtk_widget_queue_draw(gPlots->priv->bottom_crt);
	return TRUE;
}


static void
gl_plots_finalize (GObject *object)
{
	GlPlots *gPlots = GL_PLOTS(object);
	g_free(gPlots->priv->name);
	//gl_plots_remove_all_plot_data(gPlots);
	//g_debug("TEST ... gl_plots_finalize1");
	//gl_plots_clean(gPlots);
	//gtk_widget_destroy(gPlots->priv->bottom_crt);
	G_OBJECT_CLASS (gl_plots_parent_class)->finalize (object);
	//g_debug("TEST ... gl_plots_finalize2");
}

static void
gl_plots_class_init (GlPlotsClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
//	GtkTreeViewClass* parent_class = GTK_PLOT_CANVAS_CLASS(klass);

	g_type_class_add_private (klass, sizeof (GlPlotsPrivate));

	object_class->finalize = gl_plots_finalize;
}




GlPlots*
gl_plots_new(gint width, gint height, gdouble magnification)
{
	__width         = width;
	__height        = height;
	__magnification = magnification;
	//"width",width,"height",height,"magnification",magnification
	GlPlots *gPlots = GL_PLOTS(g_object_new(GL_TYPE_PLOTS,NULL));
	//g_debug("Construct canvas ... ");
	gtk_plot_canvas_construct(GTK_PLOT_CANVAS(gPlots->priv->canvas),   width, height, magnification);
	//g_debug("Construct canvas ... end");
	gPlots->priv->width  = width;
	gPlots->priv->height = height;
	gPlots->priv->magnification = magnification;
	GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(gPlots->priv->canvas), GTK_PLOT_CANVAS_DND_FLAGS);
	//TEST:g_debug(" Create data curve ...end ");
	return gPlots;
}

gboolean
gl_plots_translate( GlPlots *gPlots )
{
	g_return_val_if_fail(gPlots != NULL , FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) , FALSE);
	GlTranslation *translation = GL_TRANSLATION(gl_system_get_object("translation"));
	return TRUE;

}


gboolean
gl_plots_reload   ( GlPlots *gPlots )
{
	g_return_val_if_fail(gPlots != NULL , FALSE);
	g_return_val_if_fail(GL_IS_PLOTS(gPlots) , FALSE);
	gtk_plot_canvas_paint(GTK_PLOT_CANVAS(gPlots->priv->canvas));
	gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(gPlots->priv->canvas));
	return TRUE;
}





