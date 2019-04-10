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

#ifndef _GL_PLOT_TAB_H_
#define _GL_PLOT_TAB_H_

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkextra.h"
#include "gl-translation.h"
#include "gl-widget-option.h"

G_BEGIN_DECLS

#define GL_TYPE_PLOT_TAB             (gl_plot_tab_get_type ())
#define GL_PLOT_TAB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PLOT_TAB, GlPlotTab))
#define GL_PLOT_TAB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PLOT_TAB, GlPlotTabClass))
#define GL_IS_PLOT_TAB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PLOT_TAB))
#define GL_IS_PLOT_TAB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PLOT_TAB))
#define GL_PLOT_TAB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_PLOT_TAB, GlPlotTabClass))

typedef struct _GlPlotTabClass GlPlotTabClass;
typedef struct _GlPlotTab GlPlotTab;
typedef struct _GlPlotTabPrivate GlPlotTabPrivate;


struct _GlPlotTabClass
{
	GtkToggleButtonClass parent_class;
};

struct _GlPlotTab
{
	GtkToggleButton parent_instance;

    GlPlotTabPrivate *priv; 
};

GType                                     gl_plot_tab_get_type                   (void) G_GNUC_CONST;

GtkWidget*                                gl_plot_tab_new                        ( const gchar *name );
GList*                                    gl_plot_tab_get_datasets               ( GlPlotTab *tab );
gboolean                                  gl_plot_tab_add_dataset                ( GlPlotTab *tab, GtkPlotData *dataset );
GtkPlotData*                              gl_plot_tab_get_dataset                ( GlPlotTab *tab, const gchar *data_name );

G_END_DECLS

#endif /* _GL_PLOT_TAB_H_ */
