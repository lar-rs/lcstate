/*
 * gtk_larprocessbar.c
 *
 *  Created on: 23.11.2010
 *      Author: asmolkov
 */

#ifndef GTK_LARPROCESSBAR_C_
#define GTK_LARPROCESSBAR_C_

#include "gtk_larprocessbar.h"

#include <string.h>


static void gtk_larprocess_bar_class_init(GtkLarProcessBarClass *klass);
static void gtk_larprocess_bar_init(GtkLarProcessBar *larprocess_bar);

typedef struct
{
	char     name[1024];
	gboolean value;
}LarProcessBarValue;


GtkType gtk_larprocess_bar_get_type()
{
	static gchar __name[] = "GtkLarProcessBar";
	static guint larprocess_bar_type = 0;
	if( ! larprocess_bar_type )
	{
		GtkTypeInfo larprocess_bar_info =
		{
			__name,
			sizeof (GtkLarProcessBar),
			sizeof (GtkLarProcessBarClass),
			(GtkClassInitFunc)  gtk_larprocess_bar_class_init,
			(GtkObjectInitFunc) gtk_larprocess_bar_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL

		};

		larprocess_bar_type = gtk_type_unique(gtk_event_box_get_type(),&larprocess_bar_info);
	}
	return 	larprocess_bar_type;
}





void gtk_larprocess_bar_class_init(GtkLarProcessBarClass *klass)
{


}
void gtk_larprocess_bar_init(GtkLarProcessBar *processbar)
{
	printf("gtk_larprocess_bar_init .... \n");
	g_return_if_fail (processbar != NULL);
	g_return_if_fail (GTK_IS_LARPROCESS_BAR(processbar));
	processbar->mvbox =gtk_vbox_new(FALSE,1);
	gtk_container_add (GTK_CONTAINER (processbar), processbar->mvbox);
	//GtkWidget *label = gtk_label_new("peocess info");
	//gtk_box_pack_start(GTK_BOX(processbar->mvbox),label,TRUE,TRUE,2);
	//gtk_widget_show(label);
	gtk_widget_show(processbar->mvbox);
	gtk_widget_hide(GTK_WIDGET(processbar));
}

GtkWidget*  gtk_larprocess_bar_new()
{
	GtkLarProcessBar *processbar;
	processbar = g_object_new (gtk_larprocess_bar_get_type(), NULL);
	return GTK_WIDGET (processbar);
}

gboolean
gtk_larprocess_bar_destroy(GtkLarProcessBar *processbar)
{
	g_return_val_if_fail (processbar != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_LARPROCESS_BAR(processbar),FALSE);
	GList *children = NULL;
	children = gtk_container_get_children(GTK_CONTAINER(processbar->mvbox));
	while(children)
	{
		GtkWidget *widget = GTK_WIDGET(children->data);
		gtk_container_remove(GTK_CONTAINER(processbar->mvbox),widget);
		children = children->next;
	}
	gtk_widget_destroy(processbar->mvbox);
	gtk_widget_destroy(GTK_WIDGET(processbar));
	return TRUE;
}

//------------------------------------info -----------------------------------------

gboolean __intername_is_name_exist = FALSE;
void  __internal_larprocess_bar_is_name_exist(GtkWidget *widget,   gpointer data)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (data != NULL);

	gchar *name = (gchar*) data;
	if(0==strcmp(gtk_widget_get_name(widget),name))
		__intername_is_name_exist = TRUE;

}



gboolean
gtk_larprocess_bar_is_exist(GtkLarProcessBar *processbar,gchar *name)
{
	g_return_val_if_fail (processbar != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_LARPROCESS_BAR(processbar),FALSE);
	g_return_val_if_fail (name != NULL,FALSE);
	__intername_is_name_exist = FALSE;
	gtk_container_foreach(GTK_CONTAINER(processbar->mvbox),__internal_larprocess_bar_is_name_exist,name);
	return __intername_is_name_exist;
}


//-------------------------------------progress bar control ------------------------



gboolean
gtk_larprocess_bar_add_process(GtkLarProcessBar *processbar,gchar *name)
{
	printf("gtk_larprocess_bar_add_process ....%s \n",name);
	g_return_val_if_fail (processbar != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_LARPROCESS_BAR(processbar),FALSE);
	g_return_val_if_fail (name != NULL,FALSE);
	if(gtk_larprocess_bar_is_exist(processbar,name) ){return FALSE; }
	GtkWidget *hbox  = gtk_hbox_new(FALSE,1);
	gtk_widget_set_name(hbox,name);
	GtkWidget *label = gtk_label_new(name);
	gtk_widget_modify_font(label,pango_font_description_from_string("Oreal 8"));
	gtk_misc_set_alignment(GTK_MISC(label),0.01,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,1);
	GtkWidget *process = gtk_progress_bar_new();
	gtk_widget_set_size_request(process,100,6);
	gtk_box_pack_start(GTK_BOX(hbox),process,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(processbar->mvbox),hbox,FALSE,FALSE,1);
	gtk_widget_show(label);
	gtk_widget_show(process);
	gtk_widget_show(hbox);
	gtk_widget_show(GTK_WIDGET(processbar));
	return TRUE;
}
gboolean
gtk_larprocess_bar_remove_process(GtkLarProcessBar *processbar,gchar *name)
{
	g_return_val_if_fail (processbar != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_LARPROCESS_BAR(processbar),FALSE);
	g_return_val_if_fail (name != NULL,FALSE);
	if(!gtk_larprocess_bar_is_exist(processbar,name) ){printf("ERRRORRRRR........\n"); return FALSE; }
	GList *children = NULL;
	children = gtk_container_get_children(GTK_CONTAINER(processbar->mvbox));
	while(children)
	{
		GtkWidget *widget = GTK_WIDGET(children->data);
		printf("Set Value To %s\n",gtk_widget_get_name(widget));
		if(0==strcmp(gtk_widget_get_name(widget),name))
		{
			gtk_container_remove(GTK_CONTAINER(processbar->mvbox),widget);
		}
		children = children->next;

	}

	return TRUE;
}




gboolean  gtk_larprocess_bar_set_process_value(GtkLarProcessBar *processbar,gchar *name,gdouble value)
{
	printf("gtk_larprocess_bar_set_process_value ....%s \n",name);
	g_return_val_if_fail (processbar != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_LARPROCESS_BAR(processbar),FALSE);
	g_return_val_if_fail (name != NULL,FALSE);
	if(!gtk_larprocess_bar_is_exist(processbar,name) ){printf("ERRRORRRRR........\n"); return FALSE; }
	GList *children = NULL;
	children = gtk_container_get_children(GTK_CONTAINER(processbar->mvbox));
	while(children)
	{
		GtkWidget *widget = GTK_WIDGET(children->data);

		if(0==strcmp(gtk_widget_get_name(widget),name))
		{
			if(GTK_IS_CONTAINER(widget))
			{
				GList *parend =  gtk_container_get_children(GTK_CONTAINER(widget));
				while(parend)
				{
					GtkWidget *pwid = GTK_WIDGET(parend->data);
					if(GTK_IS_PROGRESS_BAR(pwid))
					{
						printf("Set Value %f To %s\n",value,gtk_widget_get_name(pwid));
						gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pwid),value);

					}
					parend = parend->next;
				}

			}
		}
		children = children->next;

	}
	return TRUE;
//	gtk_container_foreach(GTK_CONTAINER(processbar->mvbox),__internal_larprocess_bar_set_value,&val);

}

gboolean
gtk_larprocess_bar_set_process_name(GtkLarProcessBar *processbar,gchar *name,gchar *lname)
{

	printf("gtk_larprocess_bar_set_process_name ....%s -%s \n",name,lname);
	g_return_val_if_fail (processbar != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_LARPROCESS_BAR(processbar),FALSE);
	g_return_val_if_fail (name != NULL,FALSE);
	if(!gtk_larprocess_bar_is_exist(processbar,name) ){printf("ERRRORRRRR........\n"); return FALSE; }
	GList *children = NULL;
	children = gtk_container_get_children(GTK_CONTAINER(processbar->mvbox));
	while(children)
	{
		GtkWidget *widget = GTK_WIDGET(children->data);
		if(0==strcmp(gtk_widget_get_name(widget),name))
		{
			if(GTK_IS_CONTAINER(widget))
			{
				GList *parend =  gtk_container_get_children(GTK_CONTAINER(widget));
				while(parend)
				{
					GtkWidget *pwid = GTK_WIDGET(parend->data);
					if(GTK_IS_LABEL(pwid))
					{
						gtk_label_set_text(GTK_LABEL(pwid),lname);
					}
					parend = parend->next;
				}
			}
		}
		children = children->next;
	}
	return TRUE;
}



#endif /* GTK_LARPROCESSBAR_C_ */
