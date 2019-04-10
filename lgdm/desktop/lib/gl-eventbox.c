/*
 * @ingroup GlEventbox
 * @{
 * @file  gl-desktop-action.c	LGDM desktop action button
 * @brief LGDM desktop action button.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-eventbox.h"
#include <string.h>
#include <glib.h>
#include <glib/gnode.h>

#define GETTEXT_PACKAGE "gdm"
#include <glib/gi18n-lib.h>



//static GlEventbox *__gui_process_desktop = NULL;

struct _GlEventboxPrivate
{
	GTimer      *press_timer;
};


enum {
	GL_EVENTBOX_PROP_NULL,
	GL_EVENTBOX_ID,
	GL_EVENTBOX_MENU,
	GL_EVENTBOX_MANAGER,
};


enum
{
	GL_EVENTBOX_ADD_MANAGER,
	GL_EVENTBOX_LAST_SIGNAL
};


static guint gl_eventbox_signals[GL_EVENTBOX_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlEventbox, gl_eventbox, GTK_TYPE_SCROLLED_WINDOW);



static void
gl_eventbox_init(GlEventbox *eventbox)
{
	g_return_if_fail (eventbox != NULL);
	g_return_if_fail (GL_IS_EVENTBOX(eventbox));
	g_signale_connect()
	eventbox->priv = gl_eventbox_get_instance_private (eventbox);
	eventbox->priv->id = g_strdup("com.lar.Eventbox.Unknown");
	eventbox->priv->contaiments_eventboxs = NULL;
	eventbox->priv->child_eventboxs       = NULL;

}



static void
gl_eventbox_finalize (GObject *object)
{
	GlEventbox* eventbox = GL_EVENTBOX(object);
	if(eventbox->priv->id)g_free(eventbox->priv->id);
	if(eventbox->priv->menu)g_object_unref(eventbox->priv->menu);
	G_OBJECT_CLASS (gl_eventbox_parent_class)->finalize(object);
}


static void
gl_eventbox_activate_real ( GlEventbox *eventbox )
{
	g_debug("Activate eventbox %s",eventbox->priv->id);
}

static void
gl_eventbox_deactivate_real ( GlEventbox *eventbox )
{
	g_debug("Deactivate eventbox %s",eventbox->priv->id);
}


static void
gl_eventbox_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_EVENTBOX(object));
	GlEventbox* eventbox = GL_EVENTBOX(object);
	switch (prop_id)
	{
	case GL_EVENTBOX_ID:
		if(eventbox->priv->id)g_free(eventbox->priv->id);
		eventbox->priv->id = g_value_dup_string(value);
		break;
	case GL_EVENTBOX_MENU:
		if(eventbox->priv->menu)g_object_unref(eventbox->priv->menu);
		eventbox->priv->menu = g_value_dup_object(value);
		break;
	case GL_EVENTBOX_MANAGER:
		eventbox->priv->eventbox_manager = g_value_get_object(value);
		g_signal_emit (eventbox, gl_eventbox_signals[GL_EVENTBOX_ADD_MANAGER], 0,eventbox->priv->eventbox_manager);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_eventbox_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_EVENTBOX(object));
	GlEventbox* eventbox = GL_EVENTBOX(object);
	switch (prop_id)
	{
	case GL_EVENTBOX_ID:
		g_value_set_string(value,eventbox->priv->id);
		break;
	case GL_EVENTBOX_MENU:
		g_value_set_object(value,eventbox->priv->menu);
		break;
	case GL_EVENTBOX_MANAGER:
		g_value_set_object(value,eventbox->priv->eventbox_manager);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_eventbox_class_init(GlEventboxClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	//GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_eventbox_finalize;
	object_class -> set_property           =  gl_eventbox_set_property;
	object_class -> get_property           =  gl_eventbox_get_property;
	klass->activate                        =  gl_eventbox_activate_real;
	klass->deactivate                      =  gl_eventbox_deactivate_real;
	klass->menu                            =  NULL;

	g_object_class_install_property (object_class,GL_EVENTBOX_ID,
			g_param_spec_string  ("eventbox-id",
					_("Eventbox id"),
					_("Eventbox identification name"),
					"Application",
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));
	g_object_class_install_property (object_class,GL_EVENTBOX_MENU,
			g_param_spec_object  ("eventbox-menu",
					_("Eventbox menu info"),
					_("Eventbox menu info"),
					GTK_TYPE_MENU,
					G_PARAM_WRITABLE | G_PARAM_READABLE ));
	g_object_class_install_property (object_class,GL_EVENTBOX_MANAGER,
			g_param_spec_object  ("eventbox-manager",
					_("Eventbox manager object"),
					_("Eventbox manager object"),
					GL_TYPE_EVENTBOX_MANAGER,
					G_PARAM_WRITABLE | G_PARAM_READABLE ));
	gl_eventbox_signals[GL_EVENTBOX_ADD_MANAGER] =
			g_signal_new ("eventbox-add-manager",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					0,
					NULL, NULL,
					g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, GTK_TYPE_WIDGET);


}


const gchar*
gl_eventbox_get_id                              ( GlEventbox *eventbox )
{
	g_return_val_if_fail(eventbox!=NULL,NULL);
	g_return_val_if_fail(GL_IS_EVENTBOX(eventbox),NULL);
	return eventbox->priv->id;
}


void
gl_eventbox_activate                            ( GlEventbox *eventbox )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	GSList *l = NULL;
	for(l=eventbox->priv->contaiments_eventboxs;l!=NULL;l=l->next)
	{
		gl_eventbox_activate(GL_EVENTBOX(l->data));
	}
	if(GL_EVENTBOX_GET_CLASS(eventbox)->activate)
		GL_EVENTBOX_GET_CLASS(eventbox)->activate(eventbox);
	gtk_widget_show(GTK_WIDGET(eventbox));
}

void
gl_eventbox_deactivate                            ( GlEventbox *eventbox )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	GSList *l = NULL;
	for(l=eventbox->priv->contaiments_eventboxs;l!=NULL;l=l->next)
	{
		gl_eventbox_deactivate(GL_EVENTBOX(l->data));
	}
	if(GL_EVENTBOX_GET_CLASS(eventbox)->deactivate)
		GL_EVENTBOX_GET_CLASS(eventbox)->deactivate(eventbox);
}


void
gl_eventbox_add_manager                     ( GlEventbox *eventbox ,GtkWidget *manager )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	g_return_if_fail(eventbox->priv->eventbox_manager==NULL);
	g_return_if_fail(manager!=NULL);
	g_object_set(eventbox,"eventbox-manager",manager,NULL);
}
GtkWidget*
gl_eventbox_get_manager                     ( GlEventbox *eventbox )
{
	g_return_val_if_fail(eventbox!=NULL,NULL);
	g_return_val_if_fail(GL_IS_EVENTBOX(eventbox),NULL);
	return eventbox->priv->eventbox_manager;
}


GtkMenu*
gl_eventbox_get_menu                        ( GlEventbox *eventbox )
{
	g_return_val_if_fail(eventbox!=NULL,NULL);
	g_return_val_if_fail(GL_IS_EVENTBOX(eventbox),NULL);
	return eventbox->priv->menu;
}


static void
eventbox_destroy_child  ( GlEventbox *child, GlEventbox *eventbox )
{
	eventbox->priv->child_eventboxs  = g_slist_remove_all(eventbox->priv->child_eventboxs , child);
}

void
gl_eventbox_add_child_eventbox     ( GlEventbox *eventbox , GlEventbox *child )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	g_return_if_fail(child!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(child));
	g_return_if_fail(gtk_widget_get_parent(GTK_WIDGET(child))==NULL);
	eventbox->priv->child_eventboxs = g_slist_append(eventbox->priv->child_eventboxs,child);
	g_signal_connect(child,"destroy",G_CALLBACK(eventbox_destroy_child),eventbox);
	if(eventbox->priv->eventbox_manager)
	{
		gl_eventbox_manager_add_eventbox(GL_EVENTBOX_MANAGER(eventbox->priv->eventbox_manager),eventbox,child);
	}
}

static void
eventbox_destroy_contaiment  ( GlEventbox *contaiment, GlEventbox *eventbox )
{
	eventbox->priv->contaiments_eventboxs  = g_slist_remove_all(eventbox->priv->contaiments_eventboxs , contaiment);
}

void
gl_eventbox_add_contaiment_eventbox     ( GlEventbox *eventbox , GlEventbox *contaiment )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	g_return_if_fail(contaiment!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(contaiment));
	eventbox->priv->contaiments_eventboxs = g_slist_append(eventbox->priv->contaiments_eventboxs,contaiment);
	g_signal_connect(contaiment,"destroy",G_CALLBACK(eventbox_destroy_contaiment),eventbox);
	if(eventbox->priv->eventbox_manager)
	{
		g_object_set(contaiment,"eventbox-manager",eventbox->priv->eventbox_manager,NULL);
	}
}

static gint
eventbox_compare_eventbox_name        (gconstpointer  a, gconstpointer  b)
{
	return g_strcmp0(gl_eventbox_get_id(GL_EVENTBOX(a)),(const gchar*)b);
}

GlEventbox*
gl_eventbox_find_child                ( GlEventbox *eventbox , const gchar *id )
{
	g_return_val_if_fail(eventbox!=NULL,NULL);
	g_return_val_if_fail(GL_IS_EVENTBOX(eventbox),NULL);
	GSList *l = g_slist_find_custom(eventbox->priv->child_eventboxs,id,eventbox_compare_eventbox_name);
	if(l==NULL)return NULL;
	return GL_EVENTBOX(l->data);
}


gboolean
gl_eventbox_activate_from_name              ( GlEventbox *eventbox,const gchar *name )
{
	GlEventboxManager *lmanager = GL_EVENTBOX_MANAGER(gl_eventbox_get_manager(GL_EVENTBOX(eventbox)));
	return gl_eventbox_manager_activate_named(lmanager,name);
}


void
gl_eventbox_button_clicked_signal_cb         ( GlEventbox *eventbox, GtkButton *button )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	g_return_if_fail(button!=NULL);
	g_return_if_fail(GTK_IS_BUTTON(button));
	const gchar *widget_name = gtk_widget_get_name(GTK_WIDGET(button));
	g_debug("Activate eventbox %s signal cb",widget_name);
	gl_eventbox_activate_from_name(eventbox,widget_name);
}

void
gl_eventbox_activate_list_box_signal_cb      ( GlEventbox *eventbox, GtkListBoxRow *row , GtkListBox *box )
{
	g_return_if_fail(eventbox!=NULL);
	g_return_if_fail(GL_IS_EVENTBOX(eventbox));
	g_return_if_fail(row!=NULL);
	g_return_if_fail(GTK_IS_LIST_BOX_ROW(row));
	const gchar *widget_name = gtk_widget_get_name(GTK_WIDGET(row));
	g_debug("Activate eventbox %s signal cb",widget_name);
	//g_debug("Activate eventbox=%s widget name =%s",eventbox_name,widget_name);
	gl_eventbox_activate_from_name(eventbox,widget_name);
}




/** @} */
