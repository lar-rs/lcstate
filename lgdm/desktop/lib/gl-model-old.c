/*
 * gl-model.c
 *
 *  Created on: 25.09.2013
 *      Author: sascha
 */


#include "gl-widget-option.h"
#include "gl-xkbd.h"
#include <ultra-axis.h>

#include <market-translation.h>
#include <mkt-value.h>
#include <mkt-item-object.h>
#include <glib-object.h>

#include <string.h>
#include "../gl-model.h"


/*
static GType
gl_model_get_modelwtype ( GtkWidget *widget )
{
	GParamSpec *pspec = g_object_class_find_property(G_OBEJCT(model_container->priv->model),gl_widget_option_get_name(G_OBJECT(widget)));
	if(pspec == NULL ) return G_TYPE_NONE;
	else return  pspec->value_type;

}
*/



gboolean
property_cmp0(const gchar *wname , const gchar *property )
{
	size_t l = strcspn(wname,":");
	return 0==strncmp(wname,property,l);
}

static  GtkWidget*
gl_model_find_recursive (  GtkWidget *widget , const gchar *name  )
{
	const gchar *wname = gl_widget_option_get_name(G_OBJECT(widget));
	GtkWidget *wid = NULL;
	//g_debug("test ... %s - %s",wname,name);
	if( wname != NULL && property_cmp0(wname,name))
	{
		return widget;
	}
	if(GTK_IS_CONTAINER(widget))
	{
		GList *lw = gtk_container_get_children(GTK_CONTAINER(widget));
		GList *l = NULL;
		for(l=lw;l!=NULL;l=l->next)
		{
			wid = gl_model_find_recursive(GTK_WIDGET(l->data),name);
			if(wid){g_list_free(lw); return wid;}
		}
		g_list_free(lw);
	}
	return NULL;
}

gchar *
gl_model_get_property (GtkWidget *widget  )
{
	g_return_val_if_fail(widget != NULL ,NULL);
	g_return_val_if_fail(GTK_IS_WIDGET(widget),NULL);
	const gchar *wid = gl_widget_option_get_name(G_OBJECT(widget));
	if(wid == NULL )
	{
		g_warning ("Widget invalid identifications name : ( wid == NULL)");
		return NULL;
	}
	gchar *p = ( gchar* ) wid ;
	gchar *t = p;
	for(;t!= NULL && *t!='\0';)
	{
		p = t;
		t = strchr(t, ':');
		if(t)t++;
	}
	if(p== NULL ) return NULL;
	//	g_debug("prop = %s",p);
	gchar *prop = g_strdup(p);
	return prop;
}

GValue*
gl_model_get_widget_value (GtkWidget *widget , GType vorgabe )
{
	g_return_val_if_fail( widget != NULL ,FALSE);
	g_return_val_if_fail( GTK_IS_WIDGET(widget),FALSE);
	GValue *value = NULL;
	if (GTK_IS_ENTRY(widget))
	{
		value = mkt_value_new(vorgabe);
		if(!mkt_set_gvalue_from_string( value,gtk_entry_get_text(GTK_ENTRY(widget))) )
		{
		   mkt_value_free(value);
		   value = NULL;
		}
	}
	else if (GTK_IS_TOGGLE_BUTTON(widget))
	{

		value = mkt_value_new(G_TYPE_BOOLEAN);
		g_value_set_boolean(value,gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	}
	else if (GTK_IS_COMBO_BOX(widget))
	{
		value = gl_model_get_combobox_value(widget,1);
	}
	else if (GTK_IS_SCALE_BUTTON(widget))
	{
		value = mkt_value_new(G_TYPE_DOUBLE);
		g_value_set_double(value,gtk_scale_button_get_value(GTK_SCALE_BUTTON(widget)));

	}
	else if (GTK_IS_RANGE(widget))
	{
		value = mkt_value_new(G_TYPE_DOUBLE);
		g_value_set_double(value,gtk_range_get_value(GTK_RANGE(widget)));
	}
	else
	{
		g_critical (  "error: gui-process cannot transmit model of unknown gtk widget type ( id: %s ) \n",gl_widget_option_get_name(G_OBJECT(widget)));
	}
	return value;
}


static MktParameter *
gl_model_get_widget_parameter (MktModel *model , GtkWidget *widget )
{
	MktParameter *param = NULL;
	gchar *pname = gl_model_get_property(widget);
	if(pname)
	{
		GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(model),pname);
		if(pspec)
		{
			param = mkt_parameter_new(G_OBJECT(model),pspec->name,pspec->value_type,0);
			if(param)
			{
				GValue *value = gl_model_get_widget_value( widget , pspec->value_type);
				if(value != NULL )
				{
					if(g_value_type_compatible (param->value->g_type ,value->g_type))
					{
						g_value_copy(value,param->value);
					}
					else if(g_value_type_transformable(param->value->g_type ,value->g_type))
					{
						g_value_transform(value,param->value);
					}
					else
					{
						g_critical ( "Model %s property %s have not compatible types ( Widget : %s need type %s)",G_OBJECT_TYPE_NAME(G_OBJECT(model)),param->name,gl_widget_option_get_name(G_OBJECT(widget)),g_type_name(value->g_type));
					}
				}

			}
		}
	}
	return param;
}


static void
gl_model_transmit_object ( GtkWidget *widget )
{
	//MUTEX:g_mutex_lock(&binding->priv->lock);
	const gchar *wid = gl_widget_option_get_name(G_OBJECT(widget));
	if(wid == NULL )
	{
		g_warning ("Widget invalid identifications name : ( wid == NULL)");
		return;
	}
	MktModel *model = mkt_model_select_parser(wid);
	if(model)
	{

		MktParameter *param = gl_model_get_widget_parameter (model ,widget );

		if(param )
		{
			/*MktItem *item  =   mkt_item_new_object  (model,param->name ,param->value);

			GValue *old = mkt_value_new(param->value->g_type);
			g_object_get_property(G_OBJECT(model),param->name,old);
			//gl_connection_saved_value            (gl_connection_market_get() , old,item  );
			mkt_value_free(old);
			g_object_unref(item);*/
			g_object_set_property(G_OBJECT(model),param->name,param->value);
			mkt_parameter_free(param);
		}
		g_object_unref(model);
	}
	else
	{
		g_warning("Widget model not found .. ");
	}
}

static gboolean
gl_model_button_toggled(GtkWidget *widget,gpointer data  )
{
	//TEST:g_debug("gl_signal_radio_button_toggled\n");

	gl_model_transmit_object(widget);
	return TRUE;
}

static gboolean
gl_model_spin_value_change(GtkWidget *widget,gpointer data )
{
	//TEST:	g_debug("gl_signal_spin_value_change\n");
	gl_model_transmit_object(widget);
	return TRUE;
}
static gboolean
gl_model_entry_value_change(GtkWidget *widget,gpointer data )
{
	//g_debug("gl_model_entry_value_change\n");
	gl_model_transmit_object(widget);
	return TRUE;
}

static gboolean
gl_model_entry_focus_in (GtkWidget *widget,GdkEvent *event,gpointer data  )
{
	/*
	//TEST:g_debug("gl_binding_entry_focus_in %s",gl_widget_option_get_name(G_OBJECT(widget)));
	if( G_TYPE_STRING == gl_model_get_modelwtype( model ,widget ) )
		gl_xkbd_need_keyboard(widget,widget,GL_XKBD_TYPE_COMPACT);
	else
		gl_xkbd_need_keyboard(widget,widget,GL_XKBD_TYPE_KEYPAD);
	*/
	gint type = GL_XKBD_TYPE_COMPACT;
	gl_xkbd_need_keyboard(widget,widget,type);
	return FALSE;
}
static gboolean
gl_model_spin_focus_in (GtkWidget *widget,GdkEvent *event,gpointer data )
{
	//TEST:g_debug("gl_binding_spin_focus_in %s",gl_widget_option_get_name(G_OBJECT(widget)));
	gint type = GL_XKBD_TYPE_KEYPAD;
	gl_xkbd_need_keyboard(widget,widget,type);
	return FALSE;
}

static gboolean
gl_model_spin_focus_out (GtkWidget *widget,GdkEvent *event,gpointer data )
{
	//TEST:g_debug("gl_signal_entry_focus_out\n");
	return FALSE;
}


static gboolean
gl_model_entry_focus_out (GtkWidget *widget,GdkEvent *event,gpointer data  )
{
	//TEST:g_debug("gl_signal_entry_focus_out\n");
	return FALSE;
}

static gboolean
gl_model_check_toggled(GtkWidget *widget,gpointer data  )
{
	gl_model_transmit_object(widget);
	return TRUE;
}

static gboolean
gl_model_combobox_changed (GtkWidget *widget ,gpointer data  )
{
	//TEST:g_debug("gl_signal_combobox_changed\n");
	gl_model_transmit_object(widget);
	return FALSE;
}

static void
gl_model_range_changed (GtkRange *widget,gpointer data   )
{
	gl_model_transmit_object(GTK_WIDGET(widget));
}



void
gl_model_incomming_data ( MktWindow *win , GtkWidget *widget ,  MktModel *model )
{
	g_return_if_fail(widget!= NULL);
	g_return_if_fail(model!= NULL);
	gchar *pname = gl_model_get_property(widget);

	if(pname == NULL )
	{
		return;
	}

	//


	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(model),pname);
	//g_debug("Find property %s -> %s for %s",pname,pspec->name,G_OBJECT_TYPE_NAME(model));
	g_free(pname);
	if(pspec == NULL )
	{
		return ;
	}

	GValue *value     = mkt_value_new(pspec->value_type);
	g_object_get_property(G_OBJECT(model),pspec->name,value);
	if (GTK_IS_SPIN_BUTTON(widget))
	{
		if(!GTK_WIDGET_HAS_FOCUS(widget))
		{
			GValue *real = mkt_value_new(G_TYPE_DOUBLE);
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_spin_value_change),win);
			if(g_value_transform(value,real))
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),g_value_get_double(real));
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_spin_value_change),win);
			mkt_value_free(real);
		}
	}
	else if (GTK_IS_TEXT_VIEW(widget))
	{

	}
	else if (GTK_IS_ENTRY(widget))
	{
		gboolean editable = TRUE;
		g_object_get(widget,"editable",&editable,NULL);
		if(G_VALUE_TYPE(value)==G_TYPE_DOUBLE)
		{
			if(editable)g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_entry_value_change),win);
			gtk_entry_set_text(GTK_ENTRY(widget),mkt_value_stringify_static(value));
			if(editable)g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_entry_value_change),win);
		}
		else
		{
			if(editable)g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_entry_value_change),win);
			gtk_entry_set_text( GTK_ENTRY(widget), mkt_value_stringify_static(value) );
			if(editable)g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_entry_value_change),win);
		}
	}
/*	else if(GL_IS_TREE_DATA(widget))
	{
		//TEST:g_debug("TREE_DATA set value %s",sWid);
		GValue *str = mkt_value_new(G_TYPE_STRING);const gchar *wid = gl_widget_option_get_name(G_OBJECT(widget));
	if(wid == NULL )
	{
		g_warning ("Widget invalid identifications name : ( wid == NULL)");
		return;
	}
	MktModel *model =mkt_model_select_parser(wid);
	if(model)
	{

		MktParameter *param = gl_model_get_widget_parameter (model ,widget );
		if(param )
		{
			g_object_set_property(G_OBJECT(model),param->name,param->value);
			mkt_parameter_free(param);
		}
		g_object_unref(model);
	}
	else
	{
		g_warning("Widget model not found .. ");
	}
		if(g_value_transform(value,str))gl_tree_data_set_text(GL_TREE_DATA(widget),g_value_get_string(str) );
		mkt_value_free(str);
	}*/
	else if (GTK_IS_RANGE(widget))
	{
		if(!GTK_WIDGET_HAS_FOCUS(widget))
		{
			GValue *real = mkt_value_new(G_TYPE_DOUBLE);
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_range_changed),win);
			if(g_value_transform(value,real))gtk_range_set_value(GTK_RANGE(widget),g_value_get_double(real));
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_range_changed),win);
			mkt_value_free(real);
		}
	}
	/*else if (GTK_IS_SCALE_BUTTON(widget))
	{
		if(!GTK_WIDGET_HAS_FOCUS(widget))
		{
			GValue *real = mkt_value_new(G_TYPE_DOUBLE);
			gtk_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_scale_button_changed),win);
			if(g_value_transform(value,real))gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget),g_value_get_double(real));
			gtk_signal_handler_unblock_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_scale_button_changed),win);
			mkt_value_free(real);
		}
	}*/
	else if ((GTK_IS_TOGGLE_BUTTON(widget)))
	{

		if(!GTK_WIDGET_HAS_FOCUS(GTK_WIDGET(widget)))
		{
			GValue *bool = mkt_value_new(G_TYPE_BOOLEAN);
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_button_toggled),win);
			if(g_value_transform(value,bool))gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget), g_value_get_boolean(bool) );
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_button_toggled),win);
			mkt_value_free(bool);

		}
	}
	else if (GTK_IS_RADIO_BUTTON(widget))	// will never be executed, used TOGGLE_BUTTON
	{
		if(!GTK_WIDGET_HAS_FOCUS(widget))
		{
			GValue *bool = mkt_value_new(G_TYPE_BOOLEAN);
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_button_toggled),win);
			if(g_value_transform(value,bool))gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget),  g_value_get_boolean(bool) );
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_button_toggled),win);
			mkt_value_free(bool);
		}
	}
	else if (GTK_IS_COMBO_BOX(widget))
	{
		if(!GTK_WIDGET_HAS_FOCUS(widget))
		{
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_combobox_changed),win);
			gl_model_set_combobox_value(widget,1,value);
			g_signal_handler_block_by_func(GTK_OBJECT(widget),G_CALLBACK(gl_model_combobox_changed),win);
		}
	}
	else if (GTK_IS_LABEL(widget))
	{
		if(G_VALUE_TYPE(value) == G_TYPE_DOUBLE)
		{
			gtk_label_set_text(GTK_LABEL(widget),mkt_value_stringify_static(value));
		}
		else if(G_VALUE_TYPE(value) == G_TYPE_BOOLEAN)
		{
			if(g_value_get_boolean(value))
				gtk_label_set_text(GTK_LABEL(widget), _TR_("TRANSLATE_static_Item_BooleanOn","on") );
			else
				gtk_label_set_text(GTK_LABEL(widget), _TR_("TRANSLATE_static_Item_BooleanOff","off") );
		}
		else
		{
			GValue *str = mkt_value_new(G_TYPE_STRING);
			if(g_value_transform(value,str))gtk_label_set_text(GTK_LABEL(widget),g_value_get_string(str) );
			mkt_value_free(str);
		}

		/*if(item->type == MKT_ITEM_TYPE_bool32)
		{
			if(item->value.bool32)
				gtk_label_set_text(GTK_LABEL(widget), _TR_("TRANSLATE_static_Item_BooleanOn","on") );
			else
				gtk_label_set_text(GTK_LABEL(widget), _TR_("TRANSLATE_static_Item_BooleanOff","off") );
		}*/

	}
	else if (GTK_IS_PROGRESS_BAR(widget))	// @TODO: Think about GtkProgressBar
	{
		GValue *real = mkt_value_new(G_TYPE_DOUBLE);
		if(g_value_transform(value,real))gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR(widget), g_value_get_double(real) );
		mkt_value_free(real);



	}
	else if (GTK_IS_CONTAINER(widget))
	{

		GValue *bool = mkt_value_new(G_TYPE_BOOLEAN);
		if(g_value_transform(value,bool))gtk_widget_set_sensitive(GTK_WIDGET(widget),g_value_get_boolean(bool));
		mkt_value_free(bool);
	}
	else
	{
		g_warning ("Widget %s invalid type %s",gl_widget_option_get_name(G_OBJECT(widget)),G_OBJECT_TYPE_NAME(widget));
		//gtk_widget_hide( GTK_WIDGET(widget) );	// the hard way of user notification ;-)
		//g_warning("Binding %s set item to gobject fail :  unknown object %s ",binding->priv->name,gl_widget_option_get_name(widget));
	}
	if(value ) mkt_value_free(value);
}

gboolean
gl_model_register_signal ( MktWindow *window , GtkWidget *widget )
{
	g_return_val_if_fail(widget!=NULL,0);
	gulong ret = 0;

	if(GTK_IS_SPIN_BUTTON (widget))
	{
		ret = g_signal_connect(widget,"value-changed",G_CALLBACK(gl_model_spin_value_change),window );
		g_signal_connect( widget ,"focus_in_event",G_CALLBACK(gl_model_spin_focus_in),window);
		g_signal_connect( widget ,"focus_out_event",G_CALLBACK(gl_model_spin_focus_out),window );
	}
	else if(GTK_IS_CHECK_BUTTON(widget))
	{
		ret++;
		ret = g_signal_connect( widget ,"toggled", G_CALLBACK(gl_model_button_toggled),window );
	}
	else if(GTK_IS_COMBO_BOX(widget))
	{
		ret = g_signal_connect( widget,"changed",G_CALLBACK(gl_model_combobox_changed),window );
	}
	else if(GTK_IS_RANGE(widget))
	{
		ret = g_signal_connect( widget,"value-changed",G_CALLBACK(gl_model_range_changed),window );
	}
	else if(GTK_IS_ENTRY(widget))
	{
		gboolean editable = TRUE;
		g_object_get(widget,"editable",&editable,NULL);
		if(editable)
		{
			ret = g_signal_connect( widget ,"changed" , G_CALLBACK(gl_model_entry_value_change),window );
			g_signal_connect( G_OBJECT( widget ),"focus_in_event" , G_CALLBACK(gl_model_entry_focus_in),window);
			g_signal_connect( G_OBJECT( widget ),"focus_out_event" , G_CALLBACK(gl_model_entry_focus_out),window );
		}
	}
	else if(GTK_IS_TOGGLE_BUTTON(widget))
	{
		ret = g_signal_connect( widget,"toggled", G_CALLBACK(gl_model_button_toggled),window);
	}
	/*else if( GL_IS_TREE_DATA ( widget ))
	{
		ret = g_signal_connect( widget ,"tree-data-focus-in", G_CALLBACK(gl_model_tree_data_focus_in) , window);
		ret = g_signal_connect( widget ,"tree-data-changed",  G_CALLBACK(gl_model_tree_data_value_change) , window);
	}*/
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

MktModel*
gl_model_select ( GtkWidget *widget )
{
	const gchar *wid = gl_widget_option_get_name(G_OBJECT(widget));
	if(wid == NULL )
	{
		g_warning ("Widget invalid identifications name : ( wid == NULL)");
		return NULL;
	}
	MktModel *model =mkt_model_select_parser(wid);
	return model;
}

void
gl_model_register(MktWindow *window , GtkWidget *widget)
{
	MktModel *model =gl_model_select(widget);
	if(model)
	{
		gl_model_register_signal(window,widget);
		gl_model_incomming_data(window,widget,model);
		g_object_unref(model);

	}
	else
	{
		g_warning("Widget model not found .. ");
	}

}



GType
gl_model_interface_type ( const gchar *wid )
{
	g_return_val_if_fail (wid != NULL , G_TYPE_NONE  );
	gchar **parts = g_strsplit_set(wid,":",-1);
	GType iface =G_TYPE_NONE;
	if( parts == NULL || g_strv_length(parts) < 2 )
	{
		g_warning ( "Interface type widget SQL %s failed" ,wid);
		iface = G_TYPE_NONE;
	}
	else
	{
		iface = g_type_from_name(parts[0]);
		if(iface < G_TYPE_INTERFACE )
		{
			g_warning ("Parse SQL %s interface (%d:%s) fail",wid,iface,g_type_name(iface));
			iface = G_TYPE_NONE;
		}
	}
	if(parts)g_strfreev(parts);
	return iface;
}
gboolean
gl_model_to_widget_container         ( MktWindow *win , MktModel *model , GtkWidget *widget )
{
	g_return_val_if_fail(model!=NULL,FALSE);
	g_return_val_if_fail(widget!=NULL,FALSE);
	guint npar = 0;
	GParamSpec **pspec = g_object_class_list_properties(G_OBJECT_GET_CLASS(model),&npar);
	gint i = 0;
	for(i=0;i<npar;i++)
	{
		//g_debug("FIND: %s",pspec[i]->name);
		GtkWidget *wid  = gl_model_find_recursive(widget,pspec[i]->name);
		if(wid)
		{
			gl_model_incomming_data(win,wid,model);
		}
	}
	g_free(pspec);
	return TRUE;
}



gboolean
gl_model_set_combobox_model_uint          ( GtkWidget *widget , guint value )
{
	g_return_val_if_fail( widget != NULL , FALSE);
	g_return_val_if_fail( GTK_IS_COMBO_BOX(widget), FALSE);
	GtkTreeModel *treemodel = gtk_combo_box_get_model ( GTK_COMBO_BOX(widget));
	if(gtk_tree_model_get_n_columns(treemodel)<=1)
	{
		return FALSE;
	}
	if(gtk_tree_model_get_column_type(treemodel,1)!= G_TYPE_UINT)
	{
		return FALSE;
	}
	GtkTreeIter iter ;
	gint counter = 0;
	gboolean valid = gtk_tree_model_get_iter_first ( treemodel,&iter );
	while(valid)
	{
		guint val = -1;
		gtk_tree_model_get(treemodel,&iter,1,&val,-1);
		if(value == val)
		{
			gtk_combo_box_set_active ( GTK_COMBO_BOX(widget),counter );
			return TRUE;
		}
		valid =  gtk_tree_model_iter_next ( treemodel,&iter );
		counter++;
	}
	return FALSE;
}

gboolean
gl_model_is_combobox_uint_model           ( GtkWidget *widget )
{
	g_return_val_if_fail( widget != NULL , FALSE);
	g_return_val_if_fail( GTK_IS_COMBO_BOX(widget), FALSE);
	GtkTreeModel *treemodel = gtk_combo_box_get_model ( GTK_COMBO_BOX(widget));
	if(gtk_tree_model_get_n_columns(treemodel)<=1)
	{
		return FALSE;
	}
	if(gtk_tree_model_get_column_type(treemodel,1)!= G_TYPE_UINT)
	{
		return FALSE;
	}
	return TRUE;
}

guint
gl_model_get_combobox_uint           ( GtkWidget *widget )
{
	g_return_val_if_fail( widget != NULL , 0);
	g_return_val_if_fail( GTK_IS_COMBO_BOX(widget), 0);
	g_return_val_if_fail(gl_model_is_combobox_uint_model(widget),0);
	GtkTreeModel *treemodel = gtk_combo_box_get_model ( GTK_COMBO_BOX(widget));

	GtkTreeIter iter ;
	gint activate = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	if(activate < 0 )activate = 0;
	gint counter = 0;
	gboolean valid = gtk_tree_model_get_iter_first ( treemodel,&iter );
	while(valid)
	{
		if(activate == counter)
		{
			guint val = -1;
			gtk_tree_model_get(treemodel,&iter,1,&val,-1);
			return val;
		}
		valid =  gtk_tree_model_iter_next ( treemodel,&iter );
		counter++;
	}
	return FALSE;
}

gboolean
gl_model_set_combobox_value          ( GtkWidget *widget , gint column , const GValue *in_value )
{
	g_return_val_if_fail( in_value != NULL ,FALSE);
	g_return_val_if_fail( widget != NULL ,FALSE);
	g_return_val_if_fail( GTK_IS_COMBO_BOX(widget),FALSE);
	GtkTreeModel *treemodel = gtk_combo_box_get_model ( GTK_COMBO_BOX(widget));
	if(gtk_tree_model_get_n_columns(treemodel)<=column)
	{
		g_critical ( "GtkCombobox max_col <= %d",column);
		return FALSE;
	}
	GValue *value = mkt_value_new(G_TYPE_INVALID);
	GtkTreeIter iter ;
	gint activate = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	if(activate < 0 )activate = 0;
	gint counter = 0;
	gint active = 0;
	gboolean valid = gtk_tree_model_get_iter_first ( treemodel,&iter );
	while(valid)
	{

		gtk_tree_model_get_value(treemodel,&iter,1,value);
		if(mkt_value_g_value_equal(value,in_value))
		{
			active = counter;
			break;
		}
		else
		{
			g_value_unset(value);
		}
		valid =  gtk_tree_model_iter_next ( treemodel,&iter );
		counter++;
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),active);
	mkt_value_free(value);
	return TRUE;
}

GValue*
gl_model_get_combobox_value           ( GtkWidget *widget , gint column )
{
	g_return_val_if_fail( widget != NULL ,NULL);
	g_return_val_if_fail( GTK_IS_COMBO_BOX(widget),NULL);
	GtkTreeModel *treemodel = gtk_combo_box_get_model ( GTK_COMBO_BOX(widget));
	if(gtk_tree_model_get_n_columns(treemodel)<=column)
	{
		g_warning ( "GtkCombobox max_col <= %d",column);
		GValue *value = mkt_value_new(G_TYPE_INT);
		g_value_set_int(value,gtk_combo_box_get_active(GTK_COMBO_BOX(widget)));
		return value;
	}
	GValue *value = mkt_value_new(G_TYPE_INVALID);
	GtkTreeIter iter ;
	gint activate = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	if(activate < 0 )activate = 0;
	gint counter = 0;
	gboolean valid = gtk_tree_model_get_iter_first ( treemodel,&iter );
	while(valid)
	{
		if(activate == counter)
		{
			gtk_tree_model_get_value(treemodel,&iter,1,value);
			return value;
		}
		valid =  gtk_tree_model_iter_next ( treemodel,&iter );
		counter++;
	}
	return NULL;
}

void
gl_model_create_param_model_uint_list_store ( GtkComboBox *combobox , const gchar *model_type)
{
	g_return_if_fail(combobox!=NULL);
	GType type = g_type_from_name(model_type);
	if(type <= G_TYPE_NONE) return ;
	GtkListStore       *store;
	store = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_UINT);
	GtkTreeIter iter ;
	GSList *list = mkt_model_select (type,"select * from $tablename");
	GSList *l=NULL;
	for(l=list;l!=NULL;l=l->next)
	{
		const gchar *desc = "leer";
		if(MKT_IS_PARAM(l->data))
			desc=mkt_param_description(MKT_PARAM(l->data));
		g_debug("APPEND .. %s model to list store ",desc);
		gtk_list_store_append(GTK_LIST_STORE(store),&iter);
		gtk_list_store_set(GTK_LIST_STORE(store),&iter,0,desc,1,mkt_model_ref_link(MKT_IMODEL(l->data)),-1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox),GTK_TREE_MODEL(store));
	g_object_unref(store);
	mkt_slist_free_full(list,g_object_unref);

}

