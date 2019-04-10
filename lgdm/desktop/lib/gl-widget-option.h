/*
 * gl-widget-option.h
 *
 *  Created on: 03.11.2011
 *      Author: sasscha
 */

#ifndef GLWIDGETOPTION_H_
#define GLWIDGETOPTION_H_

#include <glib-object.h>
#include <gtk/gtk.h>


const char*         gl_widget_option_get_name                   (GObject *obj);
void                gl_widget_option_set_name                   (GObject *obj, const gchar *name);
void                gl_widget_option_set_id                     (GObject *obj ,const char *format,  ... )G_GNUC_PRINTF (2, 3);
char*               gl_widget_option_get_try_output_item_name   (GObject *widget);
gboolean            gl_widget_option_is_input                   (GObject *widget);
gboolean            gl_widget_option_is_output                  (GObject *widget);
gboolean            gl_widget_option_is_translate               (GObject *widget);
gboolean            gl_widget_option_set_translate              (GObject *widget ,const gchar *text);
const gchar*        gl_widget_option_get_translate              (GObject *widget);
gchar*              gl_widget_option_atk_name_get               (GObject *widget,gchar *option);
gchar*              gl_widget_option_get_translate_type         (GObject *widget);
gboolean            gl_widget_option_remove_all_childs          ( GtkWidget *container );
GtkWidget*          gl_widget_option_get_root_widow             ( GtkWidget *widget );

#endif /* GLWIDGETOPTION_H_ */
