/*
 * gui-draganddrop.h
 *
 *  Created on: 04.10.2011
 *      Author: sasscha
 */

#ifndef GUIDRAGANDDROP_H_
#define GUIDRAGANDDROP_H_

#include <gtk/gtk.h>


enum {
  GL_DRAG_AND_DROP_TARGET_LEVEL1_NOTEBOOK,
  GL_DRAG_AND_DROP_TARGET_LEVEL2_NOTEBOOK,
  GL_DRAG_AND_DROP_TARGET_LEVEL3_NOTEBOOK,
  GL_DRAG_AND_DROP_TARGET_LEVEL4_NOTEBOOK,
  GL_DRAG_AND_DROP_TARGET_CONTROL_BOX
};


GtkTargetEntry*                     gl_drag_and_drop_get_target_list (guint level);
guint                               gl_drag_and_drop_get_n_targets   (guint level);

GtkTargetEntry*                     gl_drag_and_drop_get_source_list (guint level);
guint                               gl_drag_and_drop_get_n_sources   (guint level);


void
drag_data_received_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
        GtkSelectionData *selection_data, guint target_type, guint time,
        gpointer data);
gboolean
drag_motion_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint t,
        gpointer user_data);
/* Emitted when a drag leaves the destination */
void
drag_leave_handl
(GtkWidget *widget, GdkDragContext *context, guint time, gpointer user_data);
gboolean
drag_drop_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time,
        gpointer user_data);

/******************************************************************************/
/* Signals receivable by source */

/* Emitted after "drag-data-received" is handled, and gtk_drag_finish is called
 * with the "delete" parameter set to TRUE (when DnD is GDK_ACTION_MOVE). */
void
drag_data_delete_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data);

/* Emitted when the destination requests data from the source via
 * gtk_drag_get_data. It should attempt to provide its data in the form
 * requested in the target_type passed to it from the destination. If it cannot,
 * it should default to a "safe" type such as a string or text, even if only to
 * print an error. Then use gtk_selection_data_set to put the source data into
 * the allocated selection_data object, which will then be passed to the
 * destination. This will cause "drag-data-received" to be emitted on the
 * destination. GdkSelectionData is based on X's selection mechanism which,
 * via X properties, is only capable of storing data in blocks of 8, 16, or
 * 32 bit units. */
void
drag_data_get_handl
(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data,
        guint target_type, guint time, gpointer user_data);

/* Emitted when DnD begins. This is often used to present custom graphics. */
void
drag_begin_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data);

void
drag_end_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data);

void
drag_motion (GtkWidget *widget,
             GdkDragContext *context,
             gint x,
             gint y,
             guint time);

#endif /* GUIDRAGANDDROP_H_ */
