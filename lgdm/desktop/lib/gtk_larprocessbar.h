/*
 * gtk_larprocessbar.h
 *
 *  Created on: 23.11.2010
 *      Author: asmolkov
 */

#ifndef GTK_LARPROCESSBAR_H_
#define GTK_LARPROCESSBAR_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GTK_LARPROCESS_BAR_TYPE				(gtk_larprocess_bar_get_type())
#define GTK_LARPROCESS_BAR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),GTK_LARPROCESS_BAR_TYPE, GtkLarProcessBar))
#define GTK_LARPROCESS_BAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass) ,GTK_LARPROCESS_BAR_TYPE, GtkLarProcessBarClass))
#define GTK_IS_LARPROCESS_BAR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),GTK_LARPROCESS_BAR_TYPE))
#define GTK_IS_LARPROCESS_BAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass) ,GTK_LARPROCESS_BAR_TYPE))

typedef struct _GtkLarProcessBar			GtkLarProcessBar;
typedef struct _GtkLarProcessBarClass		GtkLarProcessBarClass;


struct _GtkLarProcessBar
{
	GtkEventBox    container;
	GtkWidget      *mvbox;
};

struct _GtkLarProcessBarClass
{
	GtkEventBoxClass        parent_class;
	void (*larprocessbar ) (GtkLarProcessBar *larProcessBar);

};

GtkType		     gtk_larprocess_bar_get_type(void);
GtkWidget*	     gtk_larprocess_bar_new();
gboolean         gtk_larprocess_bar_destroy(GtkLarProcessBar *processbar);


//---------------------------info ------------------------------------------------------------------
gboolean gtk_larprocess_bar_is_exist(GtkLarProcessBar *processbar,gchar *name);


//-------------------------------progress bar control ----------------------------------------------
gboolean         gtk_larprocess_bar_add_process(GtkLarProcessBar *processbar,gchar *name);
gboolean         gtk_larprocess_bar_remove_process(GtkLarProcessBar *processbar,gchar *name);
gboolean         gtk_larprocess_bar_set_process_value(GtkLarProcessBar *processbar,gchar *name,gdouble value);
gboolean         gtk_larprocess_bar_set_process_name(GtkLarProcessBar *processbar,gchar *name,gchar *lname);


G_END_DECLS


#endif /* GTK_LARPROCESSBAR_H_ */
