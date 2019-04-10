/*
 * file  gl-desktop-action.c	LGDM desktop action button
 * brief LGDM desktop action button.
 *
 *
 * Copyright (C) LAR 2014
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-layout-manager.h"

#include <string.h>
#include <glib.h>
#include <glib/gnode.h>
// #include "gl-application-object.h"
#include "lgdm-app-generated-code.h"

#include "../config.h"
#include <glib/gi18n-lib.h>


//static GlLayoutManager *__gui_process_desktop = NULL;

struct _GlLayoutManagerPrivate
{
	GtkButton               *back_button;
	GtkLabel                *parent_name;
	GtkLabel                *layout_name;
	GtkStack                *layouts_stack;
	GtkBox                  *stack_halter;

	guint                    parent_signal;
};


enum {
	GL_LAYOUT_MANAGER_PROP_NULL,
	GL_LAYOUT_MANAGER_ICON
};


enum
{
	GL_LAYOUT_MANAGER_START,
	GL_LAYOUT_MANAGER_LAST_SIGNAL
};


//static guint gl_layout_manager_signals[GL_LAYOUT_MANAGER_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlLayoutManager, gl_layout_manager, GTK_TYPE_BOX);

static gboolean
layout_manager_check_layout ( GlLayoutManager *lmanager, GlLayout *activate )
{
	GList    *layouts = gtk_container_get_children(GTK_CONTAINER(lmanager->priv->layouts_stack));
	if(layouts)
	{
		GList *l = g_list_find(layouts,(gconstpointer)activate);
		gboolean is_found = ( l!=NULL);
		g_list_free(layouts);
		return is_found;
	}
	return FALSE;
}

static gint
compare_laout_id  (gconstpointer  a, gconstpointer  b)
{
	return g_strcmp0(gl_layout_get_id(GL_LAYOUT(a)),(const gchar *)b);
}

static GlLayout*
gl_layout_manager_find_layout ( GlLayoutManager *lmanager , const gchar *id)
{
	GlLayout *ret = NULL;
	GList    *layouts = gtk_container_get_children(GTK_CONTAINER(lmanager->priv->layouts_stack));
	if(layouts)
	{
		GList    *l       = g_list_find_custom(layouts,id,compare_laout_id);
		if(l )ret = GL_LAYOUT(l->data);
		g_list_free(layouts);
	}
	return ret;
}

static GlLayout*
gl_layout_manager_find_root_layout ( GlLayoutManager *lmanager)
{
	GlLayout *ret = NULL;
	GList    *layouts = gtk_container_get_children(GTK_CONTAINER(lmanager->priv->layouts_stack));
	if(layouts)
	{
		GList    *l       = NULL;
		for(l=layouts;l!=NULL;l=l->next)
		{
			if(gl_layout_get_parent(GL_LAYOUT(l->data)) == NULL)
			{
				ret = GL_LAYOUT(l->data);
			}
		}
	}
	return ret;
}

static void
layout_manager_realize_head ( GlLayoutManager *lmanager ,GlLayout *activate )
{
	gtk_label_set_text(lmanager->priv->layout_name,gl_layout_get_name(activate));
	GlLayout *parent = gl_layout_get_parent(activate);
	if(parent)
	{
		gtk_label_set_text(lmanager->priv->parent_name,gl_layout_get_name(parent));
		gtk_widget_show(GTK_WIDGET(lmanager->priv->back_button));
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(lmanager->priv->back_button));
	}
}


static gboolean
layout_manager_is_parent ( GlLayout *current, GlLayout *next )
{
	if((gpointer)current == (gpointer)gl_layout_get_parent(next))
		return TRUE;
	return FALSE;
}


static gboolean
layout_manager_is_child ( GlLayout *current, GlLayout *next )
{
	GList *l = NULL;
	for(l=gl_layout_get_children(next);l!=NULL;l=l->next)
	{
		if((gpointer)current == (gpointer)l->data)
			return TRUE;
	}
	return FALSE;
}


static void
layout_manager_realize ( GlLayoutManager *lmanager ,GlLayout *activate)
{
	g_return_if_fail(layout_manager_check_layout ( lmanager, activate));
	GlLayout *last = GL_LAYOUT(gtk_stack_get_visible_child(lmanager->priv->layouts_stack));
	if(last&& ((gpointer)activate!=(gpointer)last))
	{
		if(layout_manager_is_parent(last,activate))
		{
			gl_layout_change_state(last,GL_LAYOUT_DEACTIVATE_RIGHT);
			gl_layout_change_state(activate,GL_LAYOUT_ACTIVATE_RIGHT);
		}
		else if(layout_manager_is_child(last,activate))
		{
			gl_layout_change_state(last,GL_LAYOUT_DEACTIVATE_LEFT);
			gl_layout_change_state(activate,GL_LAYOUT_ACTIVATE_LEFT);
		}
		gl_layout_deactivate(GL_LAYOUT(last));
	}
	else
	{
		gl_layout_change_state(activate,GL_LAYOUT_ACTIVATE_UP);
	}
	layout_manager_realize_head(lmanager,activate);
	gtk_stack_set_visible_child(lmanager->priv->layouts_stack,GTK_WIDGET(activate));
}

static void
back_button_clicked_cb  ( GlLayoutManager *lmanager , GtkButton *button )
{
	//g_signal_emit(action,gl_layout_manager_signals[GL_LAYOUT_MANAGER_START],0);
	GlLayout *layout = GL_LAYOUT(gtk_stack_get_visible_child(lmanager->priv->layouts_stack));
	if(layout)
	{
		GlLayout *parent = gl_layout_get_parent(layout);
		if(parent)
		{
			gl_layout_activate(parent);
		}
	}
}

static void
show_desktop_callback(LgdmUi *desktop , GlLayoutManager *lmanager )
{
	GlLayout *layout = gl_layout_manager_get_visible_layout();
	if(gl_layout_get_activate(layout))
	{
		gl_layout_change_state(layout,GL_LAYOUT_DEACTIVATE_DOWN);
		gl_layout_deactivate(layout);
	}
}

static void
gl_layout_manager_init(GlLayoutManager *lmanager)
{
	g_return_if_fail (lmanager != NULL);
	g_return_if_fail (GL_IS_LAYOUT_MANAGER(lmanager));
	lmanager->priv = gl_layout_manager_get_instance_private (lmanager);
	gtk_widget_init_template (GTK_WIDGET (lmanager));
	lmanager->priv->layouts_stack = GTK_STACK(gtk_stack_new());
	gtk_stack_set_transition_duration(lmanager->priv->layouts_stack,200);
	gtk_stack_set_transition_type(lmanager->priv->layouts_stack,GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
	gtk_box_pack_start(GTK_BOX(lmanager->priv->stack_halter),GTK_WIDGET(lmanager->priv->layouts_stack),TRUE,TRUE,1);
	gtk_widget_show(GTK_WIDGET(lmanager->priv->layouts_stack));
//	g_signal_connect(lmanager->priv->status_settings,"changed",G_CALLBACK(gl_layout_manager_status_settings_changed),lmanager);
//	g_signal_connect(lmanager->priv->sidebar_settings,"changed",G_CALLBACK(gl_layout_manager_sidebar_settings_changed),lmanager);
}



static void
gl_layout_manager_finalize (GObject *object)
{
	//GlLayoutManager* lmanager = GL_LAYOUT_MANAGER(object);
	G_OBJECT_CLASS (gl_layout_manager_parent_class)->finalize(object);
}





static void
gl_layout_manager_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_LAYOUT_MANAGER(object));
	GlLayoutManager* manager = GL_LAYOUT_MANAGER(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_layout_manager_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_LAYOUT_MANAGER(object));
	GlLayoutManager* manager = GL_LAYOUT_MANAGER(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_layout_manager_class_init(GlLayoutManagerClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_layout_manager_finalize;
	object_class -> set_property           =  gl_layout_manager_set_property;
	object_class -> get_property           =  gl_layout_manager_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/layout/layout-manager.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlLayoutManager, back_button);
	gtk_widget_class_bind_template_child_private (widget_class, GlLayoutManager, layout_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlLayoutManager, parent_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlLayoutManager, stack_halter);


	//gtk_widget_class_bind_template_child_private (widget_class, GlLayoutManager, layouts_stack);
	gtk_widget_class_bind_template_callback (widget_class, back_button_clicked_cb);

}

//FIXME : plug in info window (last plugin ) Start last plugin from list..

GlLayoutManager*
gl_layout_manager_new ( )
{
	GlLayoutManager *layout_manager;
	layout_manager   = GL_LAYOUT_MANAGER(g_object_new( GL_TYPE_LAYOUT_MANAGER,NULL));
	return     layout_manager;
}


GlLayoutManager*
gl_layout_manager_get_default ( )
{
	return gl_application_default_layout_manager();
}

static void
gl_layout_manager_activated_layout_callback ( GlLayout *layout  , GlLayoutManager *manager )
{
	layout_manager_realize(manager,layout);
}

gboolean
gl_layout_manager_default_activate_layout ( GlLayout *layout )
{
	g_return_val_if_fail(gl_layout_manager_get_default()!=NULL,FALSE);
	g_return_val_if_fail (GL_IS_LAYOUT_MANAGER(gl_layout_manager_get_default()),FALSE);
	gl_layout_activate(layout);
	return TRUE;
}


gboolean
gl_layout_manager_default_activate_named          ( const gchar* id  )
{
	GlLayout *layout = gl_layout_manager_find_layout(gl_layout_manager_get_default(),id);
	if(layout)
	{
		gl_layout_activate(layout);
		return TRUE;
	}
	return FALSE;
}

void
gl_layout_manager_default_change_level            ( guint level )
{
	GlLayoutManager *lmanager = gl_application_default_layout_manager();
	GlLayout *layout = GL_LAYOUT(gtk_stack_get_visible_child(lmanager->priv->layouts_stack));
	if(layout && level< gl_layout_get_close_level(layout))
	{
		GlLayout *last = layout;
		GlLayout *parent = gl_layout_get_parent(layout);
		while(parent)
		{
			last = parent;
			if(level>= gl_layout_get_close_level(layout))
				break;
			parent = gl_layout_get_parent(layout);
		}
		if(last)
		{
			gl_layout_activate(last);
		}
	}
}


gboolean
gl_layout_manager_default_activate_root           (  )
{
	g_return_val_if_fail(gl_layout_manager_get_default()!=NULL,FALSE);
	g_return_val_if_fail (GL_IS_LAYOUT_MANAGER(gl_layout_manager_get_default()),FALSE);
	GlLayout *layout = gl_layout_manager_find_root_layout(gl_layout_manager_get_default());
	if(layout)
	{
		gl_layout_activate(layout);
		return TRUE;
	}
	return FALSE;
}


gboolean
gl_layout_manager_default_add_layout                     ( GlLayout *layout )
{
	g_return_val_if_fail (gl_layout_manager_get_default()!=NULL,FALSE);
	g_return_val_if_fail (layout!=NULL,FALSE);
	g_return_val_if_fail(gtk_widget_get_parent(GTK_WIDGET(layout))==NULL,FALSE);
	GList *childs = gtk_container_get_children(GTK_CONTAINER(gl_layout_manager_get_default()->priv->layouts_stack));
	//gl_layout_deactivate(layout);
	gtk_stack_add_named(gl_layout_manager_get_default()->priv->layouts_stack,GTK_WIDGET(layout),gl_layout_get_id(layout));
	g_signal_connect(layout,"layout-activated",G_CALLBACK(gl_layout_manager_activated_layout_callback),gl_layout_manager_get_default());
	if(childs == NULL || g_list_length(childs)<1 )
	{
		gl_layout_activate(layout);
	}
	if(childs) g_list_free(childs);
	return TRUE;
}


GlLayout*
gl_layout_manager_get_visible_layout                    ( )
{
	g_return_val_if_fail (gl_layout_manager_get_default()!=NULL,NULL);
	GlLayout *visiable = GL_LAYOUT(gtk_stack_get_visible_child(gl_application_default_layout_manager()->priv->layouts_stack));
	return visiable;
}
