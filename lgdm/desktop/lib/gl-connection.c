/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-item-manager.c
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 * 
 * gl-item-manager.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-item-manager.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-connection.h"
#include "gl-indicate.h"
#include "gl-xkbd.h"
#include "gl-widget-option.h"
#include "gl-tree-data.h"
#include <mkt-atom.h>
#include <mkt-collector.h>
#include <mkt-value.h>
#include <market-translation.h>
#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "../lgdm-status.h"

//#include "mkt-value.h"




struct _GlBindingPrivate
{
	GList         *wbind;
	GValue        *temp;
	GValue        *in;
	GValue        *out;
	GtkTreeIter    save_iter;
	gboolean       save_off;

	gchar         *name;
	gboolean       lock_signal;
	gchar         *connec_name;
	gchar         *trans_name;
	guchar         type ;

	guint          flag;
	GlConnection  *connection;
	GlTranslation *translation;

	GSource             *source_idle;
	GSource             *incoming_source;
	GMainContext        *context;
//	GMutex               lock;
};

enum
{
	PROP_BINDING_0,
	PROP_BINDING_NAME,
	PROP_BINDING_CONNECTION_ID,
	PROP_BINDING_TRANSLATION_ID
};




#define GL_BINDING_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_BINDING, GlBindingPrivate))

G_DEFINE_TYPE (GlBinding, gl_binding, G_TYPE_OBJECT);


static gboolean           gl_binding_gobject_realize      ( GlBinding *binding , GObject *object );

static gboolean           gl_binding_incoming_item_real   ( GlBinding *binding );


enum
{
	GL_BINDING_DESTROY,
	GL_BINDING_REALIZE,
	GL_BINDING_INCOMING_ITEM,
	GL_BINDING_LAST_SIGNAL
};

static guint gl_binding_signals[GL_BINDING_LAST_SIGNAL] = { 0 };

/*
 *
 * return newly allocate string
 */

void
gl_binding_transmit_adrressed_value ( GlBinding *binding , GValue *value )
{
	// FIX : set string value .. zu kurze lebenszeit.
	g_return_if_fail(binding!=NULL);
	g_return_if_fail (GL_IS_BINDING (binding));
	gl_connection_change_value       ( binding->priv->connection ,binding );
}

void
gl_binding_transmit_value_name ( const gchar *id ,  GValue *value )
{
	g_return_if_fail(id!=NULL);
	GlBinding *binding = gl_connection_get_binding(id);
	if(binding!=NULL )
		gl_binding_transmit_adrressed_value(binding,value);
}

static void
gl_binding_transmit_object ( GlBinding *binding , GObject *obj )
{
	//MUTEX:g_mutex_lock(&binding->priv->lock);
	char     *sWid    = NULL;
	guint     uWid = 0;
	gdouble   fWid = 0.0;
	gchar    *dupvalue = NULL;
	if (GTK_IS_ENTRY(obj))
	{
		sWid = (char *)gtk_entry_get_text(GTK_ENTRY(obj));
		dupvalue = g_strdup(sWid);
	}
	else if (GTK_IS_TOGGLE_BUTTON(obj))
	{
		uWid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(obj)) ? 1 : 0;
		fWid = (gdouble)uWid;
		dupvalue = g_strdup_printf("%d",uWid);
	}
	else if (GTK_IS_COMBO_BOX(obj))
	{
		uWid = gtk_combo_box_get_active(GTK_COMBO_BOX(obj));
		fWid = (gdouble)uWid;
		dupvalue = g_strdup_printf("%d",uWid);
	}
	else if (GTK_IS_SCALE_BUTTON(obj))
	{
		fWid = gtk_scale_button_get_value(GTK_SCALE_BUTTON(obj));
		dupvalue = g_strdup_printf("%f",fWid);
		uWid = fWid;
	}
	else if (GTK_IS_RANGE(obj))
	{
		fWid = gtk_range_get_value(GTK_RANGE(obj));
		dupvalue = g_strdup_printf("%f",fWid);
		uWid = fWid;
	}
	else if(GL_IS_TREE_DATA(obj))
	{
		sWid = (char *)gl_tree_data_get_text(GL_TREE_DATA(obj));
		dupvalue = g_strdup(sWid);
	}
	else
	{
		fprintf( stderr, "error: gui-process cannot transmit item %s of unknown gtk widget type %u\n", gl_binding_get_name(binding), GTK_WIDGET_TYPE(obj) );
	}
	if(dupvalue)g_free(dupvalue);

}



static void
gl_binding_transmit_value(GlBinding *binding )
{
	//MUTEX:g_mutex_lock(&binding->priv->lock);
	//TEST:g_debug("gl_binding_transmit_value");
	//MUTEX:g_mutex_unlock(&binding->priv->lock);
}


/*static void
gl_binding_scale_button_value_changed_intern (GtkScaleButton *button, gdouble value , GlBinding *binding )
{
	g_return_if_fail(button != NULL);
	g_return_if_fail(GTK_IS_SCALE_BUTTON(button));
	gl_widget_option_set_scale_button(button,value);
}*/


static gboolean
gl_binding_button_toggled(GtkWidget *widget,GlBinding *binding)
{
	//TEST:g_debug("gl_signal_radio_button_toggled\n");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	if(binding->priv->lock_signal) return TRUE;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
	return TRUE;
}

static gboolean
gl_binding_spin_value_change(GtkWidget *widget,GlBinding *binding)
{
	//TEST:g_debug("gl_signal_spin_value_change\n");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	if(binding->priv->lock_signal) return TRUE;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
	return TRUE;
}
static gboolean
gl_binding_entry_value_change(GtkWidget *widget,GlBinding *binding)
{
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	if(binding->priv->lock_signal) return TRUE;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
	return TRUE;
}

static gboolean
gl_binding_entry_focus_in (GtkWidget *widget,GdkEvent *event,GlBinding *binding)
{
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	//TEST:g_debug("gl_binding_entry_focus_in %s",gl_widget_option_get_name(G_OBJECT(widget)));
	gl_xkbd_need_keyboard(widget,widget,GL_XKBD_TYPE_COMPACT);
	return FALSE;
}
static gboolean
gl_binding_spin_focus_in (GtkWidget *widget,GdkEvent *event,GlBinding *binding)
{
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	//TEST:g_debug("gl_binding_spin_focus_in %s",gl_widget_option_get_name(G_OBJECT(widget)));
	gint type = GL_XKBD_TYPE_KEYPAD;
	gl_xkbd_need_keyboard(widget,widget,type);
	return FALSE;
}

static gboolean
gl_binding_spin_focus_out (GtkWidget *widget,GdkEvent *event,GlBinding *binding)
{
	//TEST:g_debug("gl_signal_entry_focus_out\n");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	return FALSE;
}

void
gl_binding_tree_data_focus_in (GlTreeData *data , GtkEntry *entry ,GlBinding *binding)
{
	g_return_if_fail(binding != NULL);
	g_return_if_fail(GL_IS_BINDING(binding));
	if(GTK_IS_ENTRY(entry))
	{
		gl_xkbd_need_keyboard(gl_tree_data_get_tree(data),GTK_WIDGET(entry),GL_XKBD_TYPE_COMPACT);
	}
	else if(GTK_IS_SPIN_BUTTON(entry))
	{
		gl_xkbd_need_keyboard(gl_tree_data_get_tree(data),GTK_WIDGET(entry),GL_XKBD_TYPE_KEYPAD);
	}
}

void
gl_binding_tree_data_value_change(GlTreeData *data , GlBinding *binding)
{
	//TEST:g_debug("gl_signal_tree_data_value_change\n");
	g_return_if_fail(binding != NULL);
	g_return_if_fail(GL_IS_BINDING(binding));
	if(binding->priv->lock_signal) return ;
	gl_binding_transmit_object(binding,G_OBJECT(data));
	gl_connection_change_value(binding->priv->connection,binding);
}

void
gl_binding_tree_data_value_change_nosave(GlTreeData *data , GlBinding *binding)
{
	//TEST:g_debug("gl_signal_tree_data_value_change\n");
	g_return_if_fail(binding != NULL);
	g_return_if_fail(GL_IS_BINDING(binding));
	if(binding->priv->lock_signal) return ;
	gl_binding_transmit_object(binding,G_OBJECT(data));
	gl_connection_change_value(binding->priv->connection,binding);
}


static gboolean
gl_binding_entry_focus_out (GtkWidget *widget,GdkEvent *event,GlBinding *binding)
{
	//TEST:g_debug("gl_signal_entry_focus_out\n");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	return FALSE;
}


/*
static gboolean
gl_binding_check_toggled(GtkWidget *widget,GlBinding *binding)
{
	g_return_val_if_fail(binding != NULL,TRUE);
	g_return_val_if_fail(GL_IS_BINDING(binding),TRUE);
	if(binding->priv->lock_signal) return TRUE;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
	return TRUE;
}*/

static gboolean
gl_binding_combobox_changed (GtkWidget *widget , GlBinding *binding )
{
	//TEST:g_debug("gl_signal_combobox_changed\n");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	if(binding->priv->lock_signal) return TRUE;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
	return FALSE;
}

static void
gl_binding_range_changed (GtkRange *widget , GlBinding *binding )
{
	g_return_if_fail(binding != NULL);
	g_return_if_fail(GL_IS_BINDING(binding));
	if(binding->priv->lock_signal) return ;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
}

/*
static void
gl_binding_scale_button_changed (GtkScaleButton *button, gdouble value , GlBinding *binding )
{
	g_return_if_fail(binding != NULL);
	g_return_if_fail(GL_IS_BINDING(binding));
	if(binding->priv->lock_signal) return ;
	gl_binding_transmit_object(binding,G_OBJECT(button));
	gl_connection_change_value(binding->priv->connection,binding);
}
*/

static gboolean
gl_binding_button_clicked ( GtkWidget *widget , GlBinding *binding)
{
	//TEST:g_debug("gl_signal_button_clicked\n");
	g_return_val_if_fail(binding != NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	if(binding->priv->lock_signal) return TRUE;
	gl_binding_transmit_object(binding,G_OBJECT(widget));
	gl_connection_change_value(binding->priv->connection,binding);
	return TRUE;
}

gulong
gl_binding_signal_connect ( GlBinding *binding,GObject *widget)
{
	g_return_val_if_fail(widget!=NULL,0);
	g_return_val_if_fail(binding!=NULL,0);
	gulong ret = 0;
	if(GTK_IS_SPIN_BUTTON (widget))
	{
		ret = g_signal_connect(widget,"value-changed",G_CALLBACK(gl_binding_spin_value_change),binding );
		g_signal_connect( widget ,"focus_in_event",G_CALLBACK(gl_binding_spin_focus_in),binding);
		g_signal_connect( widget ,"focus_out_event",G_CALLBACK(gl_binding_spin_focus_out),binding );
	}
	else if(GTK_IS_CHECK_BUTTON(widget))
	{
		ret++;
		ret = g_signal_connect( widget ,"toggled", G_CALLBACK(gl_binding_button_toggled),binding );
	}
	else if(GTK_IS_COMBO_BOX(widget))
	{
		ret = g_signal_connect( widget,"changed",G_CALLBACK(gl_binding_combobox_changed),binding);
	}
	else if(GTK_IS_RANGE(widget))
	{
		ret = g_signal_connect( widget,"value-changed",G_CALLBACK(gl_binding_range_changed),binding);
	}
	else if(GTK_IS_ENTRY(widget))
	{
		ret = g_signal_connect( widget ,"changed" , G_CALLBACK(gl_binding_entry_value_change),binding );
		g_signal_connect( G_OBJECT( widget ),"focus_in_event" , G_CALLBACK(gl_binding_entry_focus_in),binding);
		g_signal_connect( G_OBJECT( widget ),"focus_out_event" , G_CALLBACK(gl_binding_entry_focus_out),binding );
	}
	else if(GTK_IS_TOGGLE_BUTTON(widget))
	{
		ret = g_signal_connect( widget,"toggled", G_CALLBACK(gl_binding_button_toggled),binding);
	}
	else if( GTK_IS_BUTTON ( widget ))
	{
		ret = g_signal_connect(widget,"clicked", G_CALLBACK(gl_binding_button_clicked), binding);
	}
	else if( GL_IS_TREE_DATA ( widget ))
	{
		ret = g_signal_connect( widget ,"tree-data-focus-in", G_CALLBACK(gl_binding_tree_data_focus_in) , binding);
		ret = g_signal_connect( widget ,"tree-data-changed",  G_CALLBACK(gl_binding_tree_data_value_change) , binding);
	}
	else if( GTK_IS_LABEL ( widget ))
	{
		//g_warning("GtkLabel widget have not signals");
	}
	else
	{
		g_warning("widget %s have incomparable type signal not init",gl_widget_option_get_name(G_OBJECT(widget)));
	}
	return ret;
}

static void
gl_binding_init (GlBinding *object)
{
	object->priv = GL_BINDING_PRIVATE(object);
	object->priv->context          = g_main_context_default ();
	object->priv->connec_name      = g_strdup("com.lar.GlConnection.market-main");
	object->priv->name             = g_strdup("noname");
	object->priv->in               = NULL;
	object->priv->out              = NULL;
	object->priv->wbind            = NULL;
	object->priv->translation      = NULL;
	object->priv->connection       = NULL;
	object->priv->flag   = GL_CONNECTION_WIDGET_INPUT | GL_CONNECTION_WIDGET_OUTPUT;
}

static void
gl_binding_dispose ( GObject *object )
{

	g_signal_emit(object,gl_binding_signals[GL_BINDING_DESTROY],0);
	G_OBJECT_CLASS (gl_binding_parent_class)->dispose (object);
}

static void
gl_binding_finalize (GObject *object)
{
	GlBinding *binding = GL_BINDING(object);

	//MUTEX:g_mutex_lock(&binding->priv->lock);
	g_list_free(binding->priv->wbind);
	if(binding->priv->connec_name != NULL) g_free(binding->priv->connec_name);
	//MUTEX:g_mutex_unlock(&binding->priv->lock);
	//MUTEX:g_mutex_clear (&binding->priv->lock);
	G_OBJECT_CLASS (gl_binding_parent_class)->finalize (object);
}

static void
gl_binding_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_BINDING (object));
	GlBinding *binding = GL_BINDING(object);
	switch (prop_id)
	{
	case PROP_BINDING_NAME:
		if(binding->priv->name)g_free(binding->priv->name);
		binding->priv->name  = g_value_dup_string(value);
		break;
	case PROP_BINDING_CONNECTION_ID:
		if(binding->priv->connec_name)g_free(binding->priv->connec_name);
		binding->priv->connec_name  = g_value_dup_string(value);
		binding->priv->connection = GL_CONNECTION(mkt_collector_get_atom_static(binding->priv->connec_name));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_binding_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_BINDING (object));
	GlBinding *binding = GL_BINDING(object);
	switch (prop_id)
	{
	case PROP_BINDING_NAME:
		g_value_set_string(value,binding->priv->name);
		break;
	case PROP_BINDING_CONNECTION_ID:
		g_value_set_string(value,binding->priv->connec_name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



static void
gl_binding_class_init (GlBindingClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	//g_debug( "GL_Connection : gl_Connection_class_init start\n");
	g_type_class_add_private (klass, sizeof (GlBindingPrivate));
	object_class->finalize        = gl_binding_finalize;
	object_class->dispose         = gl_binding_dispose;
	object_class->set_property    = gl_binding_set_property;
	object_class->get_property    = gl_binding_get_property;
	klass->realize                = gl_binding_gobject_realize;
	klass->incoming_item          = gl_binding_incoming_item_real;

	GParamSpec *pspec;
	pspec = g_param_spec_string ("binding-name",
			"Connection id",
			"Set|Get binding id name",
			"noname",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_BINDING_NAME,pspec);
	pspec = g_param_spec_string ("connection-id",
			"Connection id",
			"Set|Get connection id name",
			"com.lar.GlConnection.market-main",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_BINDING_CONNECTION_ID,pspec);


	gl_binding_signals[GL_BINDING_REALIZE] =
				g_signal_new ("realize-object",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET (GlBindingClass, realize),
						NULL, NULL,
						g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1 , G_TYPE_OBJECT);
	gl_binding_signals[GL_BINDING_INCOMING_ITEM] =
			g_signal_new ("incoming-item",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlBindingClass, incoming_item),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0 );
	gl_binding_signals[GL_BINDING_DESTROY] =
				g_signal_new ("destroy-binding",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET (GlBindingClass, incoming_item),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0 );
}


static void
gl_binding_gtk_widget_destroy ( GtkWidget *object , GlBinding *binding)
{
	g_return_if_fail(binding!=NULL);
	g_return_if_fail(object!=NULL);
	g_return_if_fail(GL_IS_BINDING(binding));

	//MUTEX:g_mutex_lock(&binding->priv->lock);
	//TEST:g_debug("Destroy %s widget type %s ",gl_widget_option_get_name(G_OBJECT(object)),G_OBJECT_TYPE_NAME(G_OBJECT(object)));
	binding->priv->wbind = g_list_remove(binding->priv->wbind,object);
}

static void
gl_binding_g_object_destroy(GtkObject *object, GlBinding *binding)
{
	g_return_if_fail(binding!=NULL);
	g_return_if_fail(object!=NULL);
	g_return_if_fail(GL_IS_BINDING(binding));

	//MUTEX:g_mutex_lock(&binding->priv->lock);
	//TEST:	g_debug("Destroy %s object type %s ",gl_widget_option_get_name(G_OBJECT(object)),G_OBJECT_TYPE_NAME(G_OBJECT(object)));
	binding->priv->wbind = g_list_remove(binding->priv->wbind,object);
	//MUTEX:g_mutex_unlock(&binding->priv->lock);
}



gboolean
gl_binding_add_gobject ( GlBinding *binding , GObject *object )
{
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(object!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	GList *lo = g_list_find(binding->priv->wbind,object);
	if(lo==NULL)
	{
		binding->priv->wbind = g_list_append(binding->priv->wbind,object);

		if(GL_IS_TREE_DATA(object))
		{
			//TEST:g_debug("Connect Tree data object ... ");
			g_signal_connect(object,"tree-data-destroy",G_CALLBACK(gl_binding_g_object_destroy),binding);
		}
		else if(GTK_IS_WIDGET(object))
		{
			g_signal_connect(object,"destroy",G_CALLBACK(gl_binding_gtk_widget_destroy),binding);
		}
		gl_binding_signal_connect(binding,object);
		g_signal_emit(binding,gl_binding_signals[GL_BINDING_REALIZE],0, object);
	}
	return TRUE;
}

gboolean
gl_binding_remove_gtk_object_full(GlBinding *binding , GtkObject *object)
{
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(object!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	binding->priv->wbind = g_list_remove(binding->priv->wbind,object);
	return FALSE;
}

gboolean
gl_binding_remove_gtk_object_( const gchar *binding_id,GtkObject *object)
{
	g_return_val_if_fail(binding_id!=NULL,FALSE);
	return gl_binding_remove_gtk_object_full(GL_BINDING(gl_connection_get_binding(binding_id)),object);
}


gboolean
gl_binding_translate_widget ( GlBinding *binding , GObject *widget)
{
	g_return_val_if_fail ( widget != NULL,FALSE);
	if(binding->priv->translation==NULL) return FALSE;
	gchar  *value   = NULL;
	gchar  *id      = (gchar*) gl_binding_get_name(binding);
	if(!gl_widget_option_is_translate(G_OBJECT(widget))) return FALSE;
	g_debug("TranslateWidget ... test .. type = %s",G_OBJECT_TYPE_NAME(widget));
	value = (gchar*) market_translation_get(id);
	if (value != NULL)
	{
		gl_widget_option_set_translate(G_OBJECT(widget),value);
		return TRUE;
	}
	//g_debug ("gl_plugin_translate_widget end ");
	return FALSE;
}


gboolean
gl_binding_gobject_realize   (GlBinding *binding , GObject *object )
{
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(object!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	g_return_val_if_fail(G_IS_OBJECT(object),FALSE);
	//Realize binding internal value in widget.
	return TRUE;
}

gboolean
gl_binding_translate(GlBinding *binding )
{
	//MUTEX:g_mutex_lock(&binding->priv->lock);
	GList *lw = NULL;
	for(lw=binding->priv->wbind;lw!=NULL;lw=lw->next)
	{
		if(lw->data && G_IS_OBJECT(lw->data))
			gl_binding_translate_widget(binding ,G_OBJECT(lw->data));
	}
	// Realize all translate
	//MUTEX:g_mutex_unlock(&binding->priv->lock);
	return TRUE;
}

gboolean
gl_binding_realize(GlBinding *binding )
{
	//MUTEX:g_mutex_lock(&binding->priv->lock);
	GList *lw = NULL;
	for(lw=binding->priv->wbind;lw!=NULL;lw=lw->next)
	{
		if(lw->data && G_IS_OBJECT(lw->data))
		{
			//TEST:g_debug("Object %s type %s realize",gl_widget_option_get_name(G_OBJECT(lw->data)),G_OBJECT_TYPE_NAME(G_OBJECT(lw->data)));
			gl_binding_gobject_realize(binding ,G_OBJECT(lw->data));
		}
	}
	// Realize all translate
	//MUTEX:g_mutex_unlock(&binding->priv->lock);
	return TRUE;
}


gboolean
gl_binding_incoming_item_real ( GlBinding *binding )
{

	//TEST:g_debug("Binding incoming item real ");

	return TRUE;
}

GlBinding*
gl_binding_new ( const gchar *name )
{
	GlBinding* binding = GL_BINDING(g_object_new(GL_TYPE_BINDING,"binding-name",name,NULL));
	//TEST: g_debug("New binding Atom %s",gl_binding_get_name(binding));
	return binding;
}
void
gl_binding_destroy (GlBinding *binding )
{
	g_signal_emit(binding,gl_binding_signals[GL_BINDING_DESTROY],0);
}

const gchar*
gl_binding_get_name ( GlBinding *binding)
{
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	return binding->priv->name;
}


GlBinding*
gl_binding_insert_object ( GObject *object )
{
	g_return_val_if_fail(object!=NULL,NULL);
	g_return_val_if_fail(G_IS_OBJECT(object),NULL);
	const gchar *name = gl_widget_option_get_name(object);
	g_return_val_if_fail(name!=NULL,NULL);
	gchar **parts = g_strsplit_set(name,":",3);
	gchar *id = NULL;
	if(parts)
	{
		switch (g_strv_length(parts))
		{
		case 1:id = g_strdup(parts[0]);
		break;
		case 2:id = g_strdup(parts[0]);
		break;
		case 3:id = g_strdup(parts[0]);
		break;
		default:
			g_warning("binding widget %s have name wrong format ",name);
			break;
		}
		g_strfreev(parts);
	}
	if( id != NULL )
	{
		//TEST:		g_debug("Add Gtk Object %s",id);
		GlBinding *binding = gl_connection_get_binding(id);
		if( binding!=NULL )
		{
			//TEST:
			g_debug("Add Gtk Object type(%s) %s done",G_OBJECT_TYPE_NAME(object),id );
			gl_binding_translate_widget ( GL_BINDING(binding) ,G_OBJECT(object) );
			gl_binding_add_gobject ( GL_BINDING(binding),G_OBJECT(object));
			return GL_BINDING(binding);
		}
		g_free(id);
	}
	return NULL;
}

GlBinding*
gl_binding_insert_object_full           ( GObject *object , guint flag )
{
	GlBinding *binding = gl_binding_insert_object(object);
	g_return_val_if_fail(binding!=NULL,NULL);
	g_return_val_if_fail(GL_IS_BINDING(binding),NULL);
	gl_binding_set_flag(binding,flag);
	return binding;
}


GlBinding*
gl_binding_insert_widget ( GtkWidget *widget )
{
	g_return_val_if_fail(widget!=NULL,NULL);
	return gl_binding_insert_object(G_OBJECT(widget));

}

GlBinding*
gl_binding_insert_widget_full       ( GtkWidget *widget , guint flag )
{
	GlBinding *binding = gl_binding_insert_widget(widget);
	g_return_val_if_fail(binding!=NULL,NULL);
	g_return_val_if_fail(GL_IS_BINDING(binding),NULL);
	gl_binding_set_flag(binding,flag);
	return binding;
}

gboolean
gl_binding_translate_signal(GlTranslation *translation,GlBinding *binding )
{
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	g_return_val_if_fail(translation!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_TRANSLATION(translation),FALSE);
	//g_debug("Translate %s binding",gl_binding_get_name(binding));
	return gl_binding_translate(binding);
}

gboolean
gl_binding_add_translation  ( GlBinding *binding ,GlTranslation *translation )
{
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	g_return_val_if_fail(translation!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_TRANSLATION(translation),FALSE);
	binding->priv->translation = translation;
	g_signal_connect(translation,"translate",G_CALLBACK(gl_binding_translate_signal),binding);
	return TRUE;
}


void
gl_binding_set_flag   ( GlBinding *binding , guint flag )
{
	g_return_if_fail(binding!=NULL);
	g_return_if_fail(GL_IS_BINDING(binding));
	binding -> priv->flag = flag;
}


struct _GlConnectionPrivate
{
	gchar       *rxpath;
    gchar       *txpath;
	GHashTable  *bindings;
	GList       *save_widget;
	GtkWidget   *save_info;
	GlIndicate  *save_items_indicate;
	GThread     *txthread;

	GList       *item_commings;
	guint        update_timeout;

//	GMutex       lock;

//	GMutex      *connection_tx;
	gboolean     block_signal;

};


static GlConnection *__market_connection = NULL;

#define GL_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_CONNECTION, GlConnectionPrivate))
enum
{
	PROP_0,
	PROP_RXFILEPATH,
	PROP_TXFILEPATH,
};


G_DEFINE_TYPE (GlConnection, gl_connection, MKT_TYPE_ATOM);

enum
{
	GL_CONNECTION_MUST_SAVED,
	GL_CONNECTION_NEED_KEYBOARD,
	GL_CONNECTION_ADD_INDICATE,
	LAST_SIGNAL
};

static guint gl_connection_signals[LAST_SIGNAL] = { 0 };


static void        gl_connection_save_all_changed      ( GlConnection *connection , gboolean save );


static void
gl_connection_save_all_changed_signal  ( GlIndicate *indicate , GlConnection *connection )
{
	g_return_if_fail(GL_IS_CONNECTION(connection));
	gl_connection_save_all_changed(connection,TRUE);
	gl_indicate_stop(connection->priv->save_items_indicate);
}

static void
gl_connection_default_all_changed_signal  ( GlIndicate *indicate , GlConnection *connection )
{
	g_return_if_fail(GL_IS_CONNECTION(connection));
	gl_connection_save_all_changed(connection,FALSE);
	gl_indicate_stop(connection->priv->save_items_indicate);
}

void
gl_connection_cell_toggled_callback (GtkCellRendererToggle *cell, gchar *path_string, gpointer  user_data)
{
	//	g_debug("gl_data_base_cell_toggled_callback");
	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GL_IS_CONNECTION(user_data));
	GlConnection *connection = GL_CONNECTION(user_data);
	g_return_if_fail(connection->priv->save_info!=NULL);
	GtkTreeModel *model  = gtk_tree_view_get_model(GTK_TREE_VIEW(connection->priv->save_info));
	g_return_if_fail(model!=NULL);
	//TEST:g_debug("Test .... 0.1");
	GtkTreeIter obj_iter;
	gtk_tree_model_get_iter_from_string(model,&obj_iter,path_string);
	gchar *binding_id=NULL;
	gboolean set=FALSE;
	gtk_tree_model_get (model,&obj_iter,0,&binding_id,3,&set,-1);
	if(binding_id)
	{
		GlBinding *binding = gl_connection_get_binding(binding_id);
		if(binding)
		{
			//TEST:g_debug("test ... off=%d",set);
			binding->priv->save_off = set;
			gtk_tree_store_set(GTK_TREE_STORE (model),&obj_iter,3,!set,-1);
		}
		g_free(binding_id);
	}
}


static void
gl_connection_create_save_info ( GlConnection *connection)
{
	g_return_if_fail(connection->priv->save_info!=NULL);
	g_return_if_fail(GTK_IS_TREE_VIEW(connection->priv->save_info));

	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,"");
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(connection->priv->save_info),col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,"");
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(connection->priv->save_info),col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,"");
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",2);
	gtk_tree_view_append_column(GTK_TREE_VIEW(connection->priv->save_info),col);

	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(renderer,"toggled",G_CALLBACK(gl_connection_cell_toggled_callback),connection);
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("  ",
			renderer,
			"active", 3,
			NULL );
	gtk_tree_view_column_set_fixed_width ( GTK_TREE_VIEW_COLUMN (column), 30 );
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN (column),
			GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment(GTK_TREE_VIEW_COLUMN (column),0.1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(connection->priv->save_info),column);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,"");
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",4);
	gtk_tree_view_append_column(GTK_TREE_VIEW(connection->priv->save_info),col);




	GtkTreeStore       *treestore;
	treestore = gtk_tree_store_new( 6,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING ,G_TYPE_BOOLEAN ,G_TYPE_STRING,G_TYPE_ULONG);
	gtk_tree_view_set_model(GTK_TREE_VIEW(connection->priv->save_info),GTK_TREE_MODEL(treestore));
	g_object_unref( treestore );
}

static void
gl_connection_clean_save_info ( GlConnection *connection)
{
	GList *list;
	GList *column = gtk_tree_view_get_columns(GTK_TREE_VIEW(connection->priv->save_info));
	list = column;
	while(list!= NULL)
	{
		gtk_tree_view_remove_column(GTK_TREE_VIEW(connection->priv->save_info),GTK_TREE_VIEW_COLUMN( list->data));
		list = list->next;
	}
	g_list_free(column);
	gl_connection_create_save_info(connection);
}
static void
gl_connection_new_save_info ( GlConnection *connection)
{
	if(connection->priv->save_info) g_object_unref(connection->priv->save_info);
	connection->priv->save_info = gtk_tree_view_new();
	gl_connection_create_save_info(connection);
}


void
gl_connection_create_indicate (GlConnection *connection)
{
	//TEST:g_printf("gl_connection_create_indicate\n");
	if(connection -> priv->save_items_indicate)
	{
		g_object_unref(connection -> priv->save_items_indicate);
	}
	connection -> priv-> save_items_indicate = gl_indicate_new("com.lar.GlIndicate.SaveParameter",GL_INDICATE_ASK_CONFIRM);
	gchar *icons = "/lar/gui/document-save.png,/lar/gui/document-save-as.png";
	gl_indicate_set_indicate_profile(connection -> priv-> save_items_indicate,icons);
	GtkWidget *label = gtk_label_new(_TR_("TRANSLATE_static_SaveParameter","save changed parameter"));
 	gl_widget_option_set_name(G_OBJECT(label),"TRANSLATE_com.lar.GlIndicate.SaveParameter_IndicateDescription");
	gl_binding_insert_widget(label);
	gtk_misc_set_alignment(GTK_MISC(label),0.01,0.5);
	gtk_widget_modify_font(GTK_WIDGET(label),pango_font_description_from_string("Oreal 8"));
	gl_connection_new_save_info(connection);
	gtk_widget_show(label);
	gtk_widget_show(GTK_WIDGET(connection->priv->save_info));
	gl_indicate_set_indicate_box(connection -> priv-> save_items_indicate,label);
	gl_indicate_set_indicate_window(connection -> priv-> save_items_indicate,GTK_WIDGET(connection->priv->save_info));
	g_signal_connect(connection -> priv-> save_items_indicate,"click_yes",G_CALLBACK(gl_connection_save_all_changed_signal),connection);
	g_signal_connect(connection -> priv-> save_items_indicate,"click_no",G_CALLBACK(gl_connection_default_all_changed_signal),connection);
}

static void
gl_connection_init (GlConnection *object)
{
	/* TODO: Add initialization code here */
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_CONNECTION,GlConnectionPrivate);

	object->priv->save_info            = NULL;
	object->priv->save_items_indicate  = NULL;
	object->priv-> bindings            = g_hash_table_new(g_str_hash,g_str_equal);
	object->priv->save_widget          = NULL;
	object->priv->item_commings        = NULL;
	object->priv->rxpath               = NULL;
	object->priv->txpath               = NULL;
	object->priv->block_signal         = FALSE;
}

static void
gl_connection_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlConnection *connection  = GL_CONNECTION(object);
	g_hash_table_destroy(connection -> priv-> bindings);
	//MUTEX:g_mutex_clear (&connection->priv->lock);
	G_OBJECT_CLASS (gl_connection_parent_class)->finalize (object);
}

static void
gl_connection_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_CONNECTION (object));
	//GlConnection *manager = GL_CONNECTION(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_connection_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_CONNECTION (object));
	//GlConnection *manager = GL_CONNECTION(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_connection_class_init (GlConnectionClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	//g_debug( "GL_Connection : gl_Connection_class_init start\n");
	g_type_class_add_private (klass, sizeof (GlConnectionPrivate));

	object_class->finalize        = gl_connection_finalize;
	object_class->set_property    = gl_connection_set_property;
	object_class->get_property    = gl_connection_get_property;

	klass->need_keyboard          = NULL;
	klass->must_saved             = NULL;


	/*GParamSpec *pspec;
	pspec = g_param_spec_string ("rxpath",
			"Items Rx  file path",
			"Set|Get Rx file path",
			"/lar/var/gui-fifo.set",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_RXFILEPATH,pspec);
	pspec = g_param_spec_string ("txpath",
			"Items Tx file path",
			"Set|Get Tx file path",
			"/lar/var/gui-fifo.get",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_TXFILEPATH,pspec);*/

	gl_connection_signals[GL_CONNECTION_MUST_SAVED] =
			g_signal_new ("must_saved",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlConnectionClass, must_saved),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_connection_signals[GL_CONNECTION_NEED_KEYBOARD] =
			g_signal_new ("need_keyboard",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlConnectionClass, need_keyboard),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


GlConnection*
gl_connection_new  (const gchar *id,const gchar *rxPaht ,const  gchar *txPath)
{
	if(__market_connection== NULL )
	{
		__market_connection = GL_CONNECTION(mkt_atom_object_new(GL_TYPE_CONNECTION,MKT_ATOM_PN_ID,id,"rxpath",rxPaht,"txpath",txPath,NULL));
	}
	return __market_connection;
}


GlConnection*
gl_connection_market_get   ( )
{
	if(__market_connection == NULL )
	{
		__market_connection = gl_connection_new ( "com.lar.GlConnection.market-main","/lar/var/gui-fifo.get","/lar/var/gui-fifo.set" );
	}
	return __market_connection;
}

GlBinding*
gl_connection_get_binding            ( const gchar *name )
{
	GlConnection *conn =  gl_connection_market_get();
	g_return_val_if_fail(conn!=NULL,NULL);
	g_return_val_if_fail(name!=NULL,NULL);
	g_return_val_if_fail(GL_IS_CONNECTION(conn),NULL);
	return GL_BINDING(g_hash_table_lookup(conn->priv->bindings,name));
}

void
gl_connection_binding_will_destroy ( GlBinding *binding , GlConnection *conn  )
{
	g_return_if_fail(binding!=NULL);
	g_return_if_fail(conn!=NULL);
	g_hash_table_remove(conn->priv->bindings,binding->priv->name);
}

gboolean
gl_connection_add_binding            ( GlBinding *binding )
{
	GlConnection *conn =  gl_connection_market_get();
	g_return_val_if_fail(conn!=NULL,FALSE);
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_CONNECTION(conn),FALSE);
	if(!gl_connection_get_binding(binding->priv->name))
	{
		g_hash_table_insert(conn->priv->bindings,binding->priv->name,binding);
		g_signal_connect( binding , "destroy-binding" , G_CALLBACK(gl_connection_binding_will_destroy),conn );
	}
	return TRUE;
}



gboolean
gl_connection_connect_binding_signal ( const gchar *name , GCallback callback , gpointer user_data )
{
	GlConnection *conn =  gl_connection_market_get();
	g_return_val_if_fail(conn!=NULL,FALSE);
	g_return_val_if_fail(name!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_CONNECTION(conn),FALSE);
	//TEST:g_debug("CONNECTION find binding %s",name);
	GlBinding *binding = gl_connection_get_binding(name);
	g_return_val_if_fail(binding!=NULL,FALSE);
	g_signal_handlers_disconnect_by_func(binding,callback,user_data);
	g_signal_connect(binding,"incoming-item",callback,user_data);
	return TRUE;
}


gboolean
gl_connection_change_value           ( GlConnection *connection , GlBinding *binding )
{
	//MUTEX:g_mutex_lock(&connection->priv->lock);
	if(binding->priv->out )
	{
		if( !( binding->priv->flag&GL_CONNECTION_WIDGET_NOSAVE ) )
		{
			GtkTreeModel *model  = gtk_tree_view_get_model(GTK_TREE_VIEW(connection->priv->save_info));
			if(NULL == g_list_find(connection->priv->save_widget,binding))
			{
				connection->priv->save_widget   = g_list_append( connection->priv->save_widget , binding );
				if( connection->priv->save_info != NULL )
				{
					binding->priv->save_off = FALSE;
					gtk_tree_store_append(GTK_TREE_STORE(model),&binding->priv->save_iter,NULL);
					gchar *old = "old"; //gl_binding_item_value_to_str(binding->priv->in);
					gchar *new = "new"; //gl_binding_item_value_to_str(&binding->priv->temp);
					gtk_tree_store_set(GTK_TREE_STORE(model),&binding->priv->save_iter,0,gl_binding_get_name(binding),1,old,2,new,3,!binding->priv->save_off,-1);
					g_free(old);
					g_free(new);
				}

				g_signal_emit(connection,gl_connection_signals[GL_CONNECTION_MUST_SAVED],0);
				gl_indicate_start(connection->priv->save_items_indicate);
			}
			else if(gtk_tree_store_iter_is_valid(GTK_TREE_STORE(model),&binding->priv->save_iter))
			{
				binding->priv->save_off = FALSE;
				gchar *old = "old"; //gl_binding_item_value_to_str(binding->priv->in);
				gchar *new = "new"; //gl_binding_item_value_to_str(&binding->priv->temp);
				gtk_tree_store_set(GTK_TREE_STORE(model),&binding->priv->save_iter,1,old,2,new,3,!binding->priv->save_off,-1);
				g_free(old);
				g_free(new);

			}
		}
		else
		{
			gl_binding_transmit_value(binding);
		}
	}
	//MUTEX:g_mutex_unlock(&connection->priv->lock);
	return TRUE;
}

gboolean
gl_connection_saved_value           ( GlConnection *connection , const GValue *old_value, MktItem *item  )
{
	g_debug("Test ......................gl_connection_saved_value");
	GtkTreeModel *model  = gtk_tree_view_get_model(GTK_TREE_VIEW(connection->priv->save_info));
	GtkTreeIter iter;
	gtk_tree_store_append(GTK_TREE_STORE(model),&iter,NULL);
	gchar *old_value_str = mkt_value_stringify(old_value);
	const gchar *new_value_str = mkt_item_get_value(item);
	const gchar *desc = mkt_param_description(MKT_PARAM(item));
	gulong iD = mkt_item_get_object_id(item);
	gboolean activated  = mkt_param_activated(MKT_PARAM(item));
	gtk_tree_store_set(GTK_TREE_STORE(model),&iter,0,desc,1,old_value_str,2,new_value_str,3,activated,5,iD,-1);
	g_signal_emit(connection,gl_connection_signals[GL_CONNECTION_MUST_SAVED],0);
	gl_indicate_start(connection->priv->save_items_indicate);
	return TRUE;
}

gboolean
gl_connection_is_signal_block        ( GlConnection *connection )
{
	g_return_val_if_fail(GL_IS_CONNECTION(connection),FALSE);
	return connection->priv->block_signal;
}

void
gl_connection_set_signal_block        ( GlConnection *connection , gboolean block )
{
	g_return_if_fail(GL_IS_CONNECTION(connection));
	connection->priv->block_signal = block;
}

/*void
gl_connection_item_processed   ( GlConnection *connection , mktItem_t *item )
{
	g_return_if_fail(GL_IS_CONNECTION(connection));
	g_return_if_fail( item != NULL );
	if( item->reserved20 && GUI_ITEMS_CONNECTED )
	{
		//TEST:g_debug ( "Connection item %s processed \n",item->name);
		GList *il = ( GList* )item->resPointer;
		while(il)
		{
			if(il->data)
			{
				if(GTK_IS_WIDGET(il->data))
				{
					connection->priv->block_signal = TRUE;
					gl_binding_item_to_widget(G_OBJECT(il->data),item);
					connection->priv->block_signal = FALSE;
				}
				else if(GL_IS_TREE_DATA(il->data))
				{
					connection->priv->block_signal = TRUE;
					gl_binding_item_to_widget(G_OBJECT(il->data),item);
					connection->priv->block_signal = FALSE;
				}
				else if(GL_IS_NOTIFY (il->data )) gl_notify_run(GL_NOTIFY(il->data));
			}
			il = il->next;
		}
		item->isChanged = 0;
	}
}*/


#if !defined(G_OS_WIN32) && !defined(G_WITH_CYGWIN)

// TODO : Unic socket open
/*static int gl_connection_init_fifo(const gchar *path)
{
	struct stat st;
	int socket;

	if (lstat (path, &st) == 0)
	{
		if ((st.st_mode & S_IFMT) == S_IFREG)
		{
			errno = EEXIST;
			perror("lstat");
			exit (1);
		}
	}
	unlink (path);
	if (mkfifo (path, 0666) == -1)
	{
		perror("mkfifo");
		exit (1);
	}

	socket = open (path, O_RDWR | O_NONBLOCK );

	if (socket == -1) {
		perror("open");
		exit (1);
	}
	//TEST:g_debug("Now, pipe some URL's into > %s\n", path);
	return socket
}*/



static gboolean
gl_connection_open_unix  ( GlConnection *connection )
{
	g_return_val_if_fail(GL_IS_CONNECTION(connection),FALSE);
	//GError *error = NULL;
	gboolean ret = TRUE;

	return ret;
}
#endif

gboolean   gl_connection_open  ( GlConnection *connection )
{
	g_return_val_if_fail(GL_IS_CONNECTION( connection ),FALSE);
#if !defined(G_OS_WIN32) && !defined(G_WITH_CYGWIN)
	return  gl_connection_open_unix  ( connection );
#else
	return  gl_connection_open_win   ( connection );
#endif
}

#include <string.h>



G_LOCK_DEFINE (rxfifo_connection);



gboolean  gl_connection_run ( GlConnection *connection )
{
	g_return_val_if_fail(GL_IS_CONNECTION(connection),FALSE);
	//GError *err = NULL;
	// | G_IO_ERR | G_IO_HUP
	//g_io_add_watch(connection->priv->rxChannel,G_IO_IN | G_IO_PRI ,(GIOFunc) gl_connection_rx_cb , (gpointer) connection);
	//g_io_add_watch(manager->priv->txChannel,G_IO_OUT | G_IO_ERR | G_IO_HUP ,(GIOFunc) gl_Connection_tx_cb, (gpointer) manager);

	/*g_thread_init(NULL);
	if( (connection->priv->txthread = g_thread_create_full((GThreadFunc)gl_connection_ipc_cb_thread,(gpointer) connection,0, TRUE,FALSE,G_THREAD_PRIORITY_NORMAL, &err)) == NULL)
	{
		g_critical("Iten manager tX thread create failed: %s\n", err->message );
		g_error_free ( err ) ;
	}
	else
	{
		g_debug("Item manager rX thread created and started\n");
	}*/
	//connection->priv->update_timeout  = gtk_timeout_add(30,gl_connection_update_comming_items ,connection);
	//connection->priv->update_timeout  = gtk_timeout_add(900,gl_connection_update_comming_items ,connection);

	return TRUE;
}


void
gl_connection_save_all_changed (GlConnection *connection , gboolean save)
{
	GList *curr = connection->priv->save_widget;
	GSList *items = mkt_model_select(MKT_TYPE_ITEM_OBJECT,"select * from $tablename");
	GSList *l = NULL;
	for(l=items;l!=NULL;l=l->next)
	{
		if(mkt_param_activated(MKT_PARAM(l->data)))
			mkt_item_object_save(MKT_ITEM(l->data));
	}
	if(items)mkt_slist_free_full(items,g_object_unref);

	while(curr)
	{
		if(curr->data)
		{
			if(GL_IS_BINDING(curr->data))
			{

				if(save && !GL_BINDING(curr->data)->priv->save_off)
				{
					gl_binding_transmit_value(GL_BINDING(curr->data));
				}
				else
				{
					gl_binding_realize(GL_BINDING(curr->data));
				}
			}
		}
		curr = curr->next;
	}
	gl_connection_clean_save_info(connection);
	if(connection->priv->save_widget)g_list_free(connection->priv->save_widget);connection->priv->save_widget = NULL;
}
