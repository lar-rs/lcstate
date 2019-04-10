/*
 * gl-plot.h
 *
 *  Created on: 30.10.2012
 *      Author: sascha
 */

#ifndef GL_PLOT_H_
#define GL_PLOT_H_

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkextra.h"


G_BEGIN_DECLS

#define GL_TYPE_PLOTS             (gl_plots_get_type ())
#define GL_PLOTS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PLOTS, GlPlots))
#define GL_PLOTS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PLOTS, GlPlotsClass))
#define GL_IS_PLOTS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PLOTS))
#define GL_IS_PLOTS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PLOTS))
#define GL_PLOTS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_PLOTS, GlPlotsClass))

typedef struct _GlPlotsClass     GlPlotsClass;
typedef struct _GlPlots          GlPlots;
typedef struct _GlPlotsPrivate   GlPlotsPrivate;


struct _GlPlotsClass
{
	GtkVBoxClass                 parent_class;
};

struct _GlPlots
{
	GtkVBox                      parent_instance;
	GlPlotsPrivate              *priv;
};

GType                       gl_plots_get_type          ( void ) G_GNUC_CONST;

GlPlots*                    gl_plots_new               ( gint width, gint height, gdouble magnification);

GtkPlotData*                gl_plots_get_plot_data     ( GlPlots *gPlots , const gchar *tab,const gchar *name );
gboolean                    gl_plots_clean_dataset     ( GlPlots *gPlots , const gchar *tab,const gchar *name );
gboolean                    gl_plots_clean             ( GlPlots *gPlots );
gboolean                    gl_plots_add_dataset       ( GlPlots *gPlots, const gchar *tab,const gchar *name ,gdouble *x , gdouble *y ,glong size );

gboolean                    gl_plots_add_dataset_func_lineal  ( GlPlots *gPlots,const gchar *tab ,const gchar *name ,gdouble  slope , gdouble intercept,guint color);

void                        gl_plots_dataset_set_symbol                     ( GlPlots *gPlots , const gchar *tab, const gchar *name,
		                                                                      GtkPlotSymbolType type,
		                                                                      GtkPlotSymbolStyle style,
                                                                              gint size, gfloat line_width ,
                                                                              const GdkColor *color, const GdkColor *border_color);

void                        gl_plots_dataset_set_line_attributes            ( GlPlots *gPlots ,  const gchar *tab,const gchar *name,
		                                                                      GtkPlotLineStyle style,
		                                                                      GdkCapStyle cap_style,
		                                                                      GdkJoinStyle join_style,
		                                                                      gfloat width,const GdkColor *color);

void                        gl_plots_dataset_set_legend                     ( GlPlots *gPlots , const gchar *tab, const gchar *name,const gchar *legend );
void                        gl_plots_dataset_set_connector                  ( GlPlots *gPlots , const gchar *tab, const gchar *name,GtkPlotConnector connector);

void                        gl_plots_set_axis_title                         ( GlPlots *gPlots , GtkPlotAxisPos axis , const gchar *title  );

G_END_DECLS


#endif /* GL_PLOT_H_ */
