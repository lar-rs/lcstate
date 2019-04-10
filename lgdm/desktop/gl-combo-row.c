/*
 * @ingroup GlComboRow
 * @{
 * @file  gl-combo-row.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-combo-row.h"
#include <mkt-value.h>

#include "../config.h"
#include <glib/gi18n-lib.h>



//static GlComboRow *__gui_process_desktop = NULL;

struct _GlComboRowPrivate
{
	GtkLabel           *name;
	GtkLabel           *desc;
	GtkLabel           *value;
	GtkBox             *user_content_area;
	GtkRadioButton     *rbutton;
	GtkWidget          *content;
};


enum {
	GL_COMBO_ROW_PROP_NULL,
	GL_COMBO_ROW_PROP_NAME,
	GL_COMBO_ROW_PROP_DISKRIPTION,
	GL_COMBO_ROW_PROP_DISKRIPTION_VISABLE,
	GL_COMBO_ROW_PROP_VALUE,
	GL_COMBO_ROW_PROP_VALUE_VISABLE,
};


enum
{
	GL_COMBO_ROW_ACTIVATED,
	GL_COMBO_ROW_LAST_SIGNAL
};


static guint gl_combo_row_signals[GL_COMBO_ROW_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlComboRow, gl_combo_row, GTK_TYPE_LIST_BOX_ROW);



static void
combo_row_activated_cb ( GlComboRow *combo_row , GtkListBoxRow *row )
{
}

static void
rbutton_toggled_cb ( GlComboRow *gl_combo_row , GtkToggleButton *button )
{
	if(gtk_toggle_button_get_active(button))
		g_signal_emit(gl_combo_row,gl_combo_row_signals[GL_COMBO_ROW_ACTIVATED],0);
}

static void
gl_combo_row_init(GlComboRow *gl_combo_row)
{
	g_return_if_fail (gl_combo_row != NULL);
	g_return_if_fail (GL_IS_COMBO_ROW(gl_combo_row));
	gl_combo_row->priv = gl_combo_row_get_instance_private (gl_combo_row);
	gtk_widget_init_template (GTK_WIDGET (gl_combo_row));
}



static void
gl_combo_row_finalize (GObject *object)
{
	//GlComboRow* gl_combo_row = GL_COMBO_ROW(object);
	G_OBJECT_CLASS (gl_combo_row_parent_class)->finalize(object);
}

static void
gl_combo_row_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_COMBO_ROW(object));
	GlComboRow* gl_combo_row = GL_COMBO_ROW(object);
	switch (prop_id)
	{
	case GL_COMBO_ROW_PROP_NAME:
		gtk_label_set_text(gl_combo_row->priv->name,g_value_get_string(value));
		break;
	case GL_COMBO_ROW_PROP_DISKRIPTION:
		gtk_label_set_text(gl_combo_row->priv->desc,g_value_get_string(value));
		break;
	case GL_COMBO_ROW_PROP_VALUE:
		gtk_label_set_text(gl_combo_row->priv->value,g_value_get_string(value));
		break;
	case GL_COMBO_ROW_PROP_DISKRIPTION_VISABLE:
		gtk_widget_set_visible(GTK_WIDGET(gl_combo_row->priv->desc),g_value_get_boolean(value));
		break;
	case GL_COMBO_ROW_PROP_VALUE_VISABLE:
		gtk_widget_set_visible(GTK_WIDGET(gl_combo_row->priv->value),g_value_get_boolean(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_combo_row_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_COMBO_ROW(object));
	GlComboRow* gl_combo_row = GL_COMBO_ROW(object);
	switch (prop_id)
	{
	case GL_COMBO_ROW_PROP_NAME:
		g_value_set_string(value,gtk_label_get_text(gl_combo_row->priv->name));
		break;
	case GL_COMBO_ROW_PROP_DISKRIPTION:
		g_value_set_string(value,gtk_label_get_text(gl_combo_row->priv->desc));
		break;
	case GL_COMBO_ROW_PROP_VALUE:
		g_value_set_string(value,gtk_label_get_text(gl_combo_row->priv->value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_combo_row_class_init(GlComboRowClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_combo_row_finalize;
	object_class -> set_property           =  gl_combo_row_set_property;
	object_class -> get_property           =  gl_combo_row_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/dialog/combo-row.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlComboRow,rbutton);
	gtk_widget_class_bind_template_child_private (widget_class, GlComboRow,name);
	gtk_widget_class_bind_template_child_private (widget_class, GlComboRow,desc);
	gtk_widget_class_bind_template_child_private (widget_class, GlComboRow,value);
	gtk_widget_class_bind_template_child_private (widget_class, GlComboRow,user_content_area);

	gtk_widget_class_bind_template_callback (widget_class, rbutton_toggled_cb);
	gtk_widget_class_bind_template_callback (widget_class, combo_row_activated_cb);




	g_object_class_install_property (object_class,GL_COMBO_ROW_PROP_NAME,
			g_param_spec_string ("row-name",
					"Row name ",
					"Row name ",
					"No name",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_COMBO_ROW_PROP_DISKRIPTION,
			g_param_spec_string ("row-description",
					"Row description ",
					"Row discription ",
					"No name",
					G_PARAM_WRITABLE | G_PARAM_READABLE  | G_PARAM_CONSTRUCT_ONLY  ));

	g_object_class_install_property (object_class,GL_COMBO_ROW_PROP_VALUE,
			g_param_spec_string ("row-value",
					"Row value ",
					"Row value ",
					"No name",
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));

	g_object_class_install_property (object_class,GL_COMBO_ROW_PROP_DISKRIPTION_VISABLE,
			g_param_spec_boolean ("row-description-visable",
					"Row description visable",
					"Row discription visable",
					FALSE,
					G_PARAM_WRITABLE  | G_PARAM_CONSTRUCT_ONLY   ));
	g_object_class_install_property (object_class,GL_COMBO_ROW_PROP_VALUE_VISABLE,
			g_param_spec_boolean ("row-value-visable",
					"Row value visable",
					"Row value visable",
					FALSE,
					G_PARAM_WRITABLE  | G_PARAM_CONSTRUCT_ONLY  ));

	gl_combo_row_signals[GL_COMBO_ROW_ACTIVATED] =
				g_signal_new ("row-activated",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						0,
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

void
gl_combo_row_join_group ( GlComboRow *row , GlComboRow *source )
{
	g_return_if_fail(row!=NULL);
	g_return_if_fail(GL_IS_COMBO_ROW(row));
	g_return_if_fail(row!=NULL);
	g_return_if_fail(GL_IS_COMBO_ROW(row));
	gtk_radio_button_join_group(row->priv->rbutton,source->priv->rbutton);
}

void
gl_combo_row_activate                ( GlComboRow *row )
{
	g_return_if_fail(row!=NULL);
	g_return_if_fail(GL_IS_COMBO_ROW(row));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(row->priv->rbutton)))
		g_signal_emit(row,gl_combo_row_signals[GL_COMBO_ROW_ACTIVATED],0);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(row->priv->rbutton),TRUE);
}

void
gl_combo_row_set_name                ( GlComboRow *row , const gchar *name )
{
	g_return_if_fail(row!=NULL);
	g_return_if_fail(GL_IS_COMBO_ROW(row));
	g_object_set(row,"row-name",name,NULL);
}

const gchar*
gl_combo_row_get_value               ( GlComboRow *row )
{
	g_return_val_if_fail(row!=NULL,NULL);
	g_return_val_if_fail(GL_IS_COMBO_ROW(row),NULL);
	return gtk_label_get_text(row->priv->value);
}

const gchar*
gl_combo_row_get_name                ( GlComboRow *row )
{
	g_return_val_if_fail(row!=NULL,NULL);
	g_return_val_if_fail(GL_IS_COMBO_ROW(row),NULL);
	return gtk_label_get_text(row->priv->name);
}

void
gl_combo_row_pack_content            ( GlComboRow *row, GtkWidget *widget, gboolean expand, gboolean fill, guint padding )
{
	g_return_if_fail(row!=NULL);
	g_return_if_fail(GL_IS_COMBO_ROW(row));
	gtk_box_pack_start(row->priv->user_content_area,widget,expand,fill,padding);
}

GtkWidget*
gl_combo_row_get_content             ( GlComboRow *row )
{
	g_return_val_if_fail(row!=NULL,NULL);
	g_return_val_if_fail(GL_IS_COMBO_ROW(row),NULL);
	return GTK_WIDGET(row->priv->user_content_area);
}



/** @} */
