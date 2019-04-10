/*
 * gl-model.h
 *
 *  Created on: 25.09.2013
 *      Author: sascha
 */

#ifndef GL_MODEL_H_
#define GL_MODEL_H_


#include "mkt-value.h"
#include "mkt-model.h"
#include <gtk/gtk.h>
#include "mkt-window.h"



gboolean                    gl_model_register_signal             ( MktWindow *window , GtkWidget *widget );
void                        gl_model_incomming_data              ( MktWindow *window , GtkWidget *widget ,  MktModel *model);
void                        gl_model_register                    ( MktWindow *window , GtkWidget *widget );
MktModel*                   gl_model_select                      ( GtkWidget *widget );
GType                       gl_model_interface_type              ( const gchar *wid );
gboolean                    gl_model_to_widget_container         ( MktWindow *window ,MktModel *model , GtkWidget *widget );

gboolean                    gl_model_set_combobox_model_uint     ( GtkWidget *widget , guint value );
gboolean                    gl_model_is_combobox_model           ( GtkWidget *widget );
guint                       gl_model_get_combobox_uint           ( GtkWidget *widget );
GValue*                     gl_model_get_combobox_value          ( GtkWidget *widget, gint column);
void                        gl_model_create_param_model_uint_list_store ( GtkComboBox *combobox , const gchar *model_type );
gboolean                    gl_model_set_combobox_value          ( GtkWidget *widget , gint column ,  const GValue *in_value );
GValue*                     gl_model_get_widget_value            ( GtkWidget *widget , GType vorgabe );
#endif /* GL_MODEL_H_ */
