/*
 * gui-control.h
 *
 *  Created on: 28.03.2011
 *      Author: asmolkov
 */

#ifndef GUICONTROL_H_
#define GUICONTROL_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gdk/gdk.h>




typedef struct
{
	GtkFunction  func;
	gboolean     done;
	gboolean     run;
}GuiControlFunk_t;

typedef struct
{
	GList       *op_func; // GList GuiControlFunk_t..
	GList       *op_func_stop;
	char        *name;
	gpointer     data;
	gboolean     done;
	gboolean     run;
	gint         tag;
	gint         timeout;
}GlOperations_t;


gboolean         gl_control_operations_is_run       (  );
GlOperations_t*  gl_control_operation_get_operation (char *name);
gboolean         gl_control_operation_name_is_run   (char *name);
gboolean         gl_control_operation_name_is_done  (char *name);
gboolean         gl_control_operation_name_stop     (char *name);
gboolean         gl_control_operation_name_run      (char *name);


GlOperations_t*  gl_control_operation_new          (char *name, gpointer data );
gboolean         gl_control_operation_add_run_func (GlOperations_t *op,GtkFunction func);
gboolean         gl_control_operation_add_stop_func(GlOperations_t *op,GtkFunction func);
void             gl_control_operation_free         (GlOperations_t *op);
gboolean         gl_control_operation_run          (GlOperations_t *op);
gboolean         gl_control_operation_stop         (GlOperations_t *op);

gboolean         gl_control_operation_stop_last    (  );
gboolean         gl_control_operation_stop_all     (  );

#endif /* GUICONTROL_H_ */
