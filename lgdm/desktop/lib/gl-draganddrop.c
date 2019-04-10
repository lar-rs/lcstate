/*
 * gui-debug.c
 *
 *  Created on: 04.09.2011
 *      Author: asmolkov
 */



#include "gl-draganddrop.h"



#define __TARGET_LEVEL_1  { "LevelNotebook-level1-Box",  0   , GL_DRAG_AND_DROP_TARGET_LEVEL1_NOTEBOOK }
#define __TARGET_LEVEL_2  { "LevelNotebook-level2-Box",  1   , GL_DRAG_AND_DROP_TARGET_LEVEL2_NOTEBOOK }
#define __TARGET_LEVEL_3  { "LevelNotebook-level3-Box",  2   , GL_DRAG_AND_DROP_TARGET_LEVEL3_NOTEBOOK }
#define __TARGET_LEVEL_4  { "LevelNotebook-level4-Box",  3   , GL_DRAG_AND_DROP_TARGET_LEVEL4_NOTEBOOK }
#define __TARGET_LEVEL_5  { "ControlBottomBox", 4     , GL_DRAG_AND_DROP_TARGET_CONTROL_BOX    }

GtkTargetEntry gl_drag_and_drop_source_list_all[] = {
		__TARGET_LEVEL_1,
		__TARGET_LEVEL_2,
		__TARGET_LEVEL_3,
		__TARGET_LEVEL_4,
		__TARGET_LEVEL_5,

};
GtkTargetEntry gl_drag_and_drop_source_list_level1[] = {
		__TARGET_LEVEL_1,
		__TARGET_LEVEL_2,
		__TARGET_LEVEL_3,
		__TARGET_LEVEL_4,
		__TARGET_LEVEL_5,
};

GtkTargetEntry gl_drag_and_drop_source_list_level2[] = {
		__TARGET_LEVEL_1,
		__TARGET_LEVEL_2,
		__TARGET_LEVEL_3,
		__TARGET_LEVEL_4,
		__TARGET_LEVEL_5,
};
GtkTargetEntry gl_drag_and_drop_source_list_level3[] = {
		__TARGET_LEVEL_2,
		__TARGET_LEVEL_3,
		__TARGET_LEVEL_4,
		__TARGET_LEVEL_5,
};
GtkTargetEntry gl_drag_and_drop_source_list_level4[] = {
		__TARGET_LEVEL_3,
		__TARGET_LEVEL_4,
		__TARGET_LEVEL_5,
};


GtkTargetEntry gl_drag_and_drop_target_list_all[] = {
		__TARGET_LEVEL_1,
		__TARGET_LEVEL_2,
		__TARGET_LEVEL_3,
		__TARGET_LEVEL_4,
		__TARGET_LEVEL_5,

};
GtkTargetEntry gl_drag_and_drop_target_list_level1[] = {__TARGET_LEVEL_1 };
GtkTargetEntry gl_drag_and_drop_target_list_level2[] = {__TARGET_LEVEL_2 };
GtkTargetEntry gl_drag_and_drop_target_list_level3[] = {__TARGET_LEVEL_3 };
GtkTargetEntry gl_drag_and_drop_target_list_level4[] = {__TARGET_LEVEL_4 };
GtkTargetEntry gl_drag_and_drop_target_list_level5[] = {__TARGET_LEVEL_5 };


GtkTargetEntry*
gl_drag_and_drop_get_target_list (guint level)
{
	return (GtkTargetEntry*) gl_drag_and_drop_target_list_all;


	/*switch ( level )
	{
	case 0 : return (GtkTargetEntry*) gl_drag_and_drop_target_list_level1;
	case 1 : return (GtkTargetEntry*) gl_drag_and_drop_target_list_level2;
	case 2 : return (GtkTargetEntry*) gl_drag_and_drop_target_list_level3;
	case 3 : return (GtkTargetEntry*) gl_drag_and_drop_target_list_level4;
	case 4 : return (GtkTargetEntry*) gl_drag_and_drop_target_list_level5;
	default :  return (GtkTargetEntry*) gl_drag_and_drop_target_list_all;
	}*/
}
GtkTargetEntry*
gl_drag_and_drop_get_source_list (guint level)
{
	return (GtkTargetEntry*) gl_drag_and_drop_source_list_all;
	/*g_debug ( "Get Source #%d",level);
	switch ( level )
	{
	case 0 : return (GtkTargetEntry*) gl_drag_and_drop_source_list_level1;
	case 1 : return (GtkTargetEntry*) gl_drag_and_drop_source_list_level2;
	case 2 : return (GtkTargetEntry*) gl_drag_and_drop_source_list_level3;
	case 3 : return (GtkTargetEntry*) gl_drag_and_drop_source_list_level4;
	default :  return (GtkTargetEntry*) gl_drag_and_drop_source_list_all;
	}*/
}

guint
gl_drag_and_drop_get_n_targets   (guint level)
{
	return (guint )  sizeof(gl_drag_and_drop_target_list_all) / sizeof(gl_drag_and_drop_target_list_all[0]);
	/*switch ( level )
	{
	case 0 : return (guint )  sizeof(gl_drag_and_drop_target_list_level1) / sizeof(gl_drag_and_drop_target_list_level1[0]);
	case 1 : return (guint )  sizeof(gl_drag_and_drop_target_list_level2) / sizeof(gl_drag_and_drop_target_list_level2[0]);
	case 2 : return (guint )  sizeof(gl_drag_and_drop_target_list_level3) / sizeof(gl_drag_and_drop_target_list_level3[0]);
	case 3 : return (guint )  sizeof(gl_drag_and_drop_target_list_level4) / sizeof(gl_drag_and_drop_target_list_level4[0]);
	case 4 : return (guint )  sizeof(gl_drag_and_drop_target_list_level5) / sizeof(gl_drag_and_drop_target_list_level5[0]);
	default:
	}*/
}

guint
gl_drag_and_drop_get_n_sources   (guint level)
{
	return (guint )  sizeof(gl_drag_and_drop_source_list_all) / sizeof(gl_drag_and_drop_source_list_all[0]);
	/*switch ( level )
	{
	case 0 : return (guint )  sizeof(gl_drag_and_drop_source_list_level1) / sizeof(gl_drag_and_drop_source_list_level1[0]);
	case 1 : return (guint )  sizeof(gl_drag_and_drop_source_list_level2) / sizeof(gl_drag_and_drop_source_list_level2[0]);
	case 2 : return (guint )  sizeof(gl_drag_and_drop_source_list_level3) / sizeof(gl_drag_and_drop_target_list_level3[0]);
	case 3 : return (guint )  sizeof(gl_drag_and_drop_source_list_level4) / sizeof(gl_drag_and_drop_target_list_level4[0]);
	default: return (guint )  sizeof(gl_drag_and_drop_source_list_all) / sizeof(gl_drag_and_drop_source_list_all[0]);
	}*/
}

void
drag_data_received_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
        GtkSelectionData *selection_data, guint target_type, guint time,
        gpointer data)
{
        //glong   *_idata;
        //gchar   *_sdata;

        gboolean dnd_success = FALSE;
        gboolean delete_selection_data = FALSE;

        const gchar *name = gtk_widget_get_name (widget);
        GtkWidget *wi = gtk_drag_get_source_widget(context);
        name = gtk_widget_get_name (wi);
        //  name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
        // g_print ("%s: drag_data_received_handl parent_instans\n", name);

        /* Deal with what we are given from source */
        if((selection_data != NULL) && (selection_data-> length >= 0))
        {

        	if (context-> action == GDK_ACTION_ASK)
        	{
        		/* Ask the user to move or copy, then set the context action. */
        	}

        	if (context-> action == GDK_ACTION_MOVE)
        		delete_selection_data = TRUE;


        }

        if (dnd_success == FALSE)
        {
        }

        gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}

/* Emitted when a drag is over the destination */
gboolean
drag_motion_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint t,
        gpointer user_data)
{
	 //= gtk_widget_get_name (widget);
	//g_print ("%s: drag_motion_handl", name);
	//GtkWidget *wi = gtk_drag_get_source_widget(context);
	//const gchar *name  = gtk_widget_get_name (wi);
	//g_print ("   CONTENT :%s\n", name);
	// Fancy stuff here. This signal spams the console something horrible.
        //const gchar *name = gtk_widget_get_name (widget);
        //g_print ("%s: drag_motion_handl\n", name);
    return  FALSE;
}

/* Emitted when a drag leaves the destination */
void
drag_leave_handl
(GtkWidget *widget, GdkDragContext *context, guint time, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        GtkWidget *wi = gtk_drag_get_source_widget(context);
        name = gtk_widget_get_name (wi);
        // name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
        // g_print ("%s: drag_leave_handl parent_instans\n", name);
}

/* Emitted when the user releases (drops) the selection. It should check that
 * the drop is over a valid part of the widget (if its a complex widget), and
 * itself to return true if the operation should continue. Next choose the
 * target type it wishes to ask the source for. Finally call gtk_drag_get_data
 * which will emit "drag-data-get" on the source. */
gboolean
drag_drop_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time,
        gpointer user_data)
{
      /*  gboolean        is_valid_drop_site;
        GdkAtom         target_type;

        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_drop_handl", name);
        GtkWidget *wi = gtk_drag_get_source_widget(context);
        name = gtk_widget_get_name (wi);
        g_print ("   CONTENT :%s\n", name);
        gtk_container_remove(GTK_CONTAINER(wi->parent),wi);
        if(GTK_IS_CONTROL_BOX(widget))
        {
        	gtk_control_box_add_widget(GTK_CONTROL_BOX(widget),wi,FALSE,FALSE,0);
        }
        else if(GTK_IS_BOX(widget))
        {
        	gtk_box_pack_start(GTK_BOX(widget),wi,TRUE,TRUE,0);
        	if(GTK_IS_CONTAINER(wi))gtk_container_set_border_width(GTK_CONTAINER(wi),0);
        }

        //  name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
        //  g_print ("%s: drag_drop_handl parent_instans\n", name); */

        /* Check to see if (x,y) is a valid drop site within widget */
       // is_valid_drop_site = TRUE;

        /* If the source offers a target */
        return TRUE;// is_valid_drop_site;
}


/******************************************************************************/
/* Signals receivable by source */

/* Emitted after "drag-data-received" is handled, and gtk_drag_finish is called
 * with the "delete" parameter set to TRUE (when DnD is GDK_ACTION_MOVE). */
void
drag_data_delete_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
        // We aren't moving or deleting anything here
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_data_delete_handl", name);

        GtkWidget *wi = gtk_drag_get_source_widget(context);
        name = gtk_widget_get_name (wi);
        g_print ("   CONTENT :%s\n", name);
        // name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
        // g_print ("%s: drag_data_delete_handl parent_instans\n", name);

}

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
        guint target_type, guint time, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
       // const gchar *string_data = "This is data from the source.";
       // const glong integer_data = 42;
        GtkWidget *wi = gtk_drag_get_source_widget(context);
        g_print ("%s: drag_data_get_handl", name);
        name = gtk_widget_get_name (wi);
        g_print ("   CONTENT :%s\n", name);
        //name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
        // g_print ("%s: drag_data_get_handl parent_instans\n", name);


        g_assert (selection_data != NULL);
        g_print (".\n");
}

/* Emitted when DnD begins. This is often used to present custom graphics. */
void
drag_begin_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_begin_handl", name);
        GtkWidget *wi = gtk_drag_get_source_widget(context);
        name = gtk_widget_get_name (wi);
        g_print ("   CONTENT :%s\n", name);

        //gtk_drag_set_icon_widget(context,wi,1,1);

       // name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
       // g_print ("%s: drag_begin_handl parent_instans\n", name);
}

/* Emitted when DnD ends. This is used to clean up any leftover data. */
void
drag_end_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_end_handl", name);

        GtkWidget *wi = gtk_drag_get_source_widget(context);
        name = gtk_widget_get_name (wi);
        g_print ("   CONTENT :%s\n", name);

        // name = gtk_widget_get_name (GTK_WIDGET(context->parent_instance));
        // g_print ("%s: drag_end_handl parent_instans\n", name);
}


void
drag_motion (GtkWidget *widget,
             GdkDragContext *context,
             gint x,
             gint y,
             guint time)
{
  GdkModifierType mask = 0;

  printf("Drag moution \n");
 /* gdk_window_get_pointer (gtk_widget_get_window (widget),
                          NULL, NULL, &mask);*/
  if (mask & GDK_CONTROL_MASK)
    gdk_drag_status (context, GDK_ACTION_COPY, time);
  else
    gdk_drag_status (context, GDK_ACTION_MOVE, time);
}
