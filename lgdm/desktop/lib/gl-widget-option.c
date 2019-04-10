/*
 * gl-widget-option.c
 *
 *  Created on: 03.11.2011
 *      Author: sasscha
 */


#include <string.h>
#include <gtk/gtk.h>
#include <market-translation.h>

#include "gl-tree-data.h"
#include "gl-widget-option.h"
#include "gl-action-widget.h"
#include "gl-translation.h"


const char* gl_widget_option_get_name  (  GObject *obj)
{
	if(GTK_MAJOR_VERSION == 2)
	{
		if(GTK_MINOR_VERSION < 20)
		{
			if(GTK_IS_WIDGET(obj))return gtk_widget_get_name(GTK_WIDGET(obj));
			else return  gtk_buildable_get_name(GTK_BUILDABLE(obj));
			//else if(GL_IS_TREE_DATA(obj)) return gl_tree_data_get_name (GL_TREE_DATA(obj));
		}
		else if(GTK_MINOR_VERSION >= 20)
		{
			return  gtk_buildable_get_name(GTK_BUILDABLE(obj));
			//else if(GL_IS_TREE_DATA(obj)) return gl_tree_data_get_name (GL_TREE_DATA(obj));
		}
	}
	return NULL;
}


void   gl_widget_option_set_name  (GObject *obj, const gchar *name)
{
	g_return_if_fail(name != NULL);
	g_return_if_fail(obj  != NULL);

	if(GTK_MAJOR_VERSION == 2)
	{
		if(GTK_MINOR_VERSION < 20)
		{
			if(GTK_IS_WIDGET(obj)) gtk_widget_set_name(GTK_WIDGET(obj),name);
			//else if(GL_IS_TREE_DATA(obj)) return gl_tree_data_set_name (GL_TREE_DATA(obj),name);
		}
		else if(GTK_MINOR_VERSION >= 20)
		{
			if(GTK_IS_WIDGET(obj)) gtk_buildable_set_name(GTK_BUILDABLE(obj),name);
			//else if(GL_IS_TREE_DATA(obj)) return gl_tree_data_set_name (GL_TREE_DATA(obj),name);
		}
	}
}


void
gl_widget_option_set_id    (GObject *obj ,const char *format,  ... )
{
	gchar *id;
	va_list  args;

	va_start (args, format);
	id = g_strdup_vprintf (format, args);
	va_end (args);
	if(id)
	{
		if(GTK_MAJOR_VERSION == 2)
		{
			if(GTK_MINOR_VERSION < 20)
			{
				if(GTK_IS_WIDGET(obj)) gtk_widget_set_name(GTK_WIDGET(obj),id);
				else if(GL_IS_TREE_DATA(obj)) gl_tree_data_set_name (GL_TREE_DATA(obj),id);
			}
			else if(GTK_MINOR_VERSION >= 20)
			{
				if(GTK_IS_WIDGET(obj)) gtk_buildable_set_name(GTK_BUILDABLE(obj),id);
				else if(GL_IS_TREE_DATA(obj)) gl_tree_data_set_name (GL_TREE_DATA(obj),id);
			}
		}
		g_free(id);
	}
}


char* gl_widget_option_get_try_output_item_name(GObject *widget)
{
	static char ret[1024];
	char* p;
	size_t l;
	memset(ret,0,1024);
	p = (char*)gl_widget_option_get_name(widget);
	if(p==NULL ) return NULL;
	l = strcspn(p,":");
	if(strlen(p)>l)
	{
		p+=l;if(*p==':')p++;
	}
	if(g_str_has_prefix(p,"gui_"))
	{
		strncpy(ret , p , sizeof(ret) );
		return ret;
	}
	strcpy(ret,"gui_");
	l = strcspn(p,"_");
	char ts[100];memset(ts,0,100);
	memcpy(ts,p,l);
	p+=l;if(*p=='_')p++;
	strcat(ret,ts);
	strcat(ret,p);
//	printf("Conwert WidgettoOutput:|%s|\n",ret);
	return ret;
}


gchar*
gl_widget_option_atk_name_get(GObject *widget,gchar *option)
{
	g_return_val_if_fail(widget != NULL,NULL);
	g_return_val_if_fail(option != NULL,NULL);
	g_return_val_if_fail(GTK_IS_WIDGET(widget),NULL);
	char *ret = NULL;
    AtkObject *atk  = NULL;
	char **options  = NULL;
	atk = gtk_widget_get_accessible (GTK_WIDGET(widget));
	if(atk== NULL) return NULL;
	const char  *atk_buff = atk_object_get_name(atk);
	if(atk_buff == NULL ) return NULL;
	options = g_strsplit_set(atk_buff,"\n",-1);
	if(options == NULL) return NULL;
	int i;
	for(i=0;options[i]!= NULL;i++)
	{
		//printf("TEST_ATK:option %s\n",options[i]);

		char **op = g_strsplit_set((const char*)options[i],"=",-1);
		if((op!=NULL)&&(op[0]!=NULL))
		{

			if((0==strcmp(option,op[0]))&&(op[1]!=NULL)) ret = g_strdup(op[1]);
		}
		if(op!=NULL)g_strfreev(op);
	}
	g_strfreev(options);
	return ret;
}



gboolean  gl_widget_option_is_input (GObject *widget)
{
	if(GL_IS_TREE_DATA(widget))      return TRUE;
	if(GTK_IS_COMBO_BOX(widget))     return TRUE;
	if(GTK_IS_CHECK_BUTTON(widget))  return TRUE;
	if(GTK_IS_ENTRY(widget))         return TRUE;
	if(GTK_IS_LABEL(widget))         return TRUE;
	if(GTK_CONTAINER(widget))        return TRUE;
	return FALSE;
}
gboolean  gl_widget_option_is_output (GObject *widget)
{
	if(GL_IS_TREE_DATA(widget))      return TRUE;
	if(GTK_IS_COMBO_BOX(widget))     return TRUE;
	if(GTK_IS_RADIO_BUTTON(widget))  return TRUE;
	if(GTK_IS_CHECK_BUTTON(widget))  return TRUE;
	if(GTK_IS_SPIN_BUTTON(widget))   return TRUE;
	if(GTK_IS_ENTRY(widget))         return TRUE;
	return FALSE;
}
gboolean gl_widget_option_is_translate  (GObject *widget)
{
	if(GTK_IS_COMBO_BOX(widget))     return TRUE;
	if(GTK_IS_RADIO_BUTTON(widget))  return TRUE;
	if(GTK_IS_CHECK_BUTTON(widget))  return TRUE;
	if(GTK_IS_BUTTON(widget))        return TRUE;
	if(GTK_IS_LABEL(widget))         return TRUE;
	if(GTK_IS_TREE_STORE(widget))    return TRUE;
	//g_debug("Is no translation widget %s",gl_widget_option_get_name(G_OBJECT(widget)));
	return FALSE;
}

gboolean
gl_widget_option_set_translate (GObject *widget , const gchar *text)
{
	g_return_val_if_fail ( widget != NULL,FALSE);
	g_return_val_if_fail ( text != NULL,FALSE);
	g_return_val_if_fail ( GTK_IS_WIDGET(widget),FALSE);
	if(GTK_IS_COMBO_BOX(widget))
	{
		gchar **parts = g_strsplit_set(text,"\n",-1);
		GtkTreeModel *treemodel = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
		GtkTreeIter iter ;
		gint count = 0;
		gboolean valid = gtk_tree_model_get_iter_first(treemodel,&iter);
		while(valid)
		{
			if(parts && parts[count])
			{
				gtk_list_store_set  (GTK_LIST_STORE(treemodel),&iter,0, parts[count],-1);
			}
			else
			{
				break;
			}
			valid =  gtk_tree_model_iter_next (treemodel,&iter);
			count++;
		}
		g_strfreev(parts);
	}
	else if( GTK_IS_TREE_MODEL(widget))
	{
		g_debug("TRANSLATE tree model ");
		GtkTreeModel *treemodel = GTK_TREE_MODEL(widget);
		if(gtk_tree_model_get_column_type(treemodel,0) == G_TYPE_STRING)
		{
			GtkTreeIter iter ;
			gchar **parts = g_strsplit_set(text,"\n",-1);
			gint count = 0;
			gboolean valid = gtk_tree_model_get_iter_first ( treemodel,&iter );
			while(valid)
			{
				if(parts && parts[count])
				{
					gtk_list_store_set  (GTK_LIST_STORE(treemodel),&iter,0, parts[count],-1);
				}
				else
				{
					break;
				}
				valid =  gtk_tree_model_iter_next (treemodel,&iter);
				count++;
			}
		}
	}
/*	else if(GTK_IS_RADIO_BUTTON(widget))
	{
		//FIX: translate GtkRadioButton
	}
	else if (GTK_IS_CHECK_BUTTON(widget))
	{
		//FIX: translate GtkCheckBotton
		gtk_button_set_label(GTK_BUTTON(widget),text);
	}*/
	else if(GTK_IS_BUTTON(widget))
	{
		gtk_button_set_label(GTK_BUTTON(widget),text);
	}
	else if(GTK_IS_LABEL(widget))
	{
		//TEST:		g_debug("TRANSLATE label %s text %s",gl_widget_option_get_name(G_OBJECT(widget)),text);
		gtk_label_set_markup(GTK_LABEL(widget),text);
		//gtk_label_set_text(GTK_LABEL(widget),text);
	}
	else
	{
		g_warning ( "wodget options translate : unknown widget type");
	}
	return TRUE;
}
const gchar*
gl_widget_option_get_translate (GObject *widget)
{
	g_return_val_if_fail ( widget != NULL,NULL);
	g_return_val_if_fail ( GTK_IS_WIDGET(widget),NULL);
	const gchar *text = "notext";
	if(GTK_IS_COMBO_BOX(widget))
	{
		//FIX: translate GtkCombobox
	}
	else if(GTK_IS_RADIO_BUTTON(widget))
	{
		//FIX: translate GtkRadioButton
	}
	else if (GTK_IS_CHECK_BUTTON(widget))
	{
		//FIX: translate GtkCheckBotton
	}
	else if (GTK_IS_BUTTON ( widget))
	{
		text = gtk_button_get_label(GTK_BUTTON(widget));
	}
	else if(GTK_IS_LABEL(widget))
	{
		text = gtk_label_get_text(GTK_LABEL(widget));
	}
	else
	{
		g_warning ( "wodget options translate : unknown widget type");
	}
	return text;
}



gchar*
 gl_widget_option_get_translate_type ( GObject *widget )
{
	static  gchar *text;
	if(GL_IS_ACTION_WIDGET(widget))
	{
		text = "GlActionWidget";
	}
	else if(GTK_IS_COMBO_BOX(widget))
	{
		text = "GtkComboBox";
	}
	else if(GTK_IS_RADIO_BUTTON(widget))
	{
		text = "GtkRadioButton";
	}
	else if (GTK_IS_CHECK_BUTTON(widget))
	{
		text = "GtkCheckButton";
	}
	else if(GTK_IS_BUTTON(widget))
	{
		text = "GtkButton";
	}
	else if(GTK_IS_LABEL(widget))
	{
		text = "GtkLabel";
	}
	else
	{
		g_warning ( "wodget options translate : unknown widget type");
		text = "unknown";
	}
	return text;
}


void
gl_widget_option_set_scale_button ( GtkScaleButton *button , gdouble val )
{
	GList *lw = gtk_container_get_children(GTK_CONTAINER(button));
	GList *l = NULL;
	for(l=lw;l!=NULL;l=l->next)
	{
		if(l->data !=NULL && GTK_IS_LABEL(l->data))
		{
			gtk_label_set_text(GTK_LABEL(l->data),mkt_value_stringify_double(val));
		}
	}
	if(lw ) g_list_free(lw);
}

gboolean
gl_widget_option_remove_all_childs ( GtkWidget *container )
{
	g_return_val_if_fail(container != NULL , FALSE);
	g_return_val_if_fail(GTK_IS_CONTAINER(container) , FALSE);
	GList *childs = gtk_container_get_children(GTK_CONTAINER(container));
	GList *l;
	for(l=childs;l!=NULL;l= l->next)
	{
		if(l->data && GTK_IS_WIDGET(l->data))
		{
			gtk_container_remove(GTK_CONTAINER(container),GTK_WIDGET(l->data));
		}
	}
	return TRUE;
}

GtkWidget*
gl_widget_option_get_root_widow     ( GtkWidget *widget )
{
	if(widget==NULL)return NULL;
	GtkWidget *parent = gtk_widget_get_parent(widget);
	GtkWidget *ret = widget;
	while(parent!=NULL)
	{
		ret = parent;
		parent = gtk_widget_get_parent(parent);
	}
	return ret;
}

