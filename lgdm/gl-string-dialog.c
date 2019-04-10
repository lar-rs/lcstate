/*
 * @ingroup GlStringDialog
 * @{
 * @file  gl-string-dialog.c	generated object file
 * @brief generated object file
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-string-dialog.h"

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>



//static GlStringDialog *__gui_process_desktop = NULL;

struct _GlStringDialogPrivate
{
	GtkEntry          *value_entry;
	GtkLabel          *value_name;
	GtkLabel          *cancel_name;
	GtkLabel          *set_label;
	gchar             *value;
	guint              move_tag;
};


enum {
	GL_STRING_DIALOG_PROP_NULL,
	GL_STRING_DIALOG_PROP_NAME,
	GL_STRING_DIALOG_PROP_VALUE,
};


enum
{
	GL_STRING_OK_SIGNAL,
	GL_STRING_DIALOG_LAST_SIGNAL
};


static guint gl_string_dialog_signals[GL_STRING_DIALOG_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlStringDialog, gl_string_dialog, GTK_TYPE_WINDOW);



static void
string_dialog_cancel_clicked_cb ( GlStringDialog *dialog, GtkButton *button )
{
	if(dialog->priv->value!=NULL)gtk_entry_set_text(dialog->priv->value_entry,dialog->priv->value);
	gtk_widget_hide(GTK_WIDGET(dialog));

}

static void
string_dialog_set_clicked_cb ( GlStringDialog *dialog, GtkButton *button )
{
	g_object_set(dialog,"string-value",gtk_entry_get_text(dialog->priv->value_entry),NULL);
	gtk_widget_hide(GTK_WIDGET(dialog));
	g_signal_emit(dialog,gl_string_dialog_signals[GL_STRING_OK_SIGNAL],0,dialog->priv->value);

}



static gboolean
string_dialog_move_callback ( gpointer user_data )
{
	GlStringDialog* dialog = GL_STRING_DIALOG(user_data);
	gtk_window_move(GTK_WINDOW(dialog),1,1);
	dialog->priv->move_tag = 0;
	return FALSE;
}


static void
string_dialog_start_visible (GObject *object ,GParamSpec *pspec , GlStringDialog *dialog)
{
	if(gtk_widget_is_visible(GTK_WIDGET(dialog)))
	{
		if(dialog->priv->value!=NULL)gtk_entry_set_text(dialog->priv->value_entry,dialog->priv->value);
		gtk_window_set_focus(GTK_WINDOW(dialog),GTK_WIDGET(dialog->priv->value_entry));
	}
	if(dialog->priv->move_tag == 0)
		g_timeout_add(20,string_dialog_move_callback,dialog);
}



static void
gl_string_dialog_init(GlStringDialog *gl_string_dialog)
{
	g_return_if_fail (gl_string_dialog != NULL);
	g_return_if_fail (GL_IS_STRING_DIALOG(gl_string_dialog));
	gl_string_dialog->priv = gl_string_dialog_get_instance_private(gl_string_dialog);
	gl_string_dialog->priv->move_tag = 0;
	gtk_widget_init_template (GTK_WIDGET (gl_string_dialog));
	g_signal_connect(gl_string_dialog,"notify::visible",G_CALLBACK(string_dialog_start_visible),gl_string_dialog);
	gtk_label_set_text(gl_string_dialog->priv->cancel_name,_("CANCEL"));
	gtk_label_set_text(gl_string_dialog->priv->set_label,_("OK"));


}




static gboolean
string_dialog_key_press_event_cb (GlStringDialog* dialog,GdkEventKey *event , GtkWidget *widget)
{
	if(event->keyval == GDK_KEY_Return)
	{
		g_object_set(dialog,"string-value",gtk_entry_get_text(dialog->priv->value_entry),NULL);
		gtk_widget_hide(GTK_WIDGET(dialog));
		return TRUE;
	}
	return FALSE;
}

static void
gl_string_dialog_finalize (GObject *object)
{
	GlStringDialog* gl_string_dialog = GL_STRING_DIALOG(object);
	if(gl_string_dialog->priv->value)g_free(gl_string_dialog->priv->value);
	G_OBJECT_CLASS (gl_string_dialog_parent_class)->finalize(object);
}





static void
gl_string_dialog_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_STRING_DIALOG(object));
	GlStringDialog* gl_string_dialog = GL_STRING_DIALOG(object);
	switch (prop_id)
	{
	case GL_STRING_DIALOG_PROP_NAME:
		if(gl_string_dialog->priv->value_name)gtk_label_set_text(gl_string_dialog->priv->value_name,g_value_get_string(value));
		break;
	case GL_STRING_DIALOG_PROP_VALUE:
		if(gl_string_dialog->priv->value)g_free(gl_string_dialog->priv->value);
		gl_string_dialog->priv->value = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_string_dialog_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_STRING_DIALOG(object));
	GlStringDialog* gl_string_dialog = GL_STRING_DIALOG(object);
	switch (prop_id)
	{
	case GL_STRING_DIALOG_PROP_NAME:
		if(gl_string_dialog->priv->value_name)g_value_set_string(value,gtk_label_get_text(gl_string_dialog->priv->value_name));
		break;
	case GL_STRING_DIALOG_PROP_VALUE:
		g_value_set_string(value,gl_string_dialog->priv->value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_string_dialog_class_init(GlStringDialogClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_string_dialog_finalize;
	object_class -> set_property           =  gl_string_dialog_set_property;
	object_class -> get_property           =  gl_string_dialog_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/dialog/string-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlStringDialog, value_entry);
	gtk_widget_class_bind_template_child_private (widget_class, GlStringDialog, value_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlStringDialog, cancel_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlStringDialog, set_label);
	gtk_widget_class_bind_template_callback (widget_class, string_dialog_cancel_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, string_dialog_set_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, string_dialog_key_press_event_cb);


	g_object_class_install_property (object_class,GL_STRING_DIALOG_PROP_NAME,
			g_param_spec_string ("string-name",
					"Parameter name ",
					"Parameter name ",
					"No name",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

	g_object_class_install_property (object_class,GL_STRING_DIALOG_PROP_VALUE,
			g_param_spec_string ("string-value",
					"Parameter value",
					"Parameter value",
					"No name",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

	gl_string_dialog_signals[GL_STRING_OK_SIGNAL] =
				g_signal_new ("dialog-ok",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						0,
						NULL, NULL,
						g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
}


void
gl_string_dialog_change_input_purpose( GlStringDialog *gl_string_dialog, GtkInputPurpose purpose )
{
	g_return_if_fail(gl_string_dialog!=NULL);
	g_return_if_fail(GL_IS_STRING_DIALOG(gl_string_dialog));
	g_return_if_fail(gl_string_dialog->priv->value_entry!=NULL);
	gtk_entry_set_input_purpose(gl_string_dialog->priv->value_entry,purpose);
	if(GTK_INPUT_PURPOSE_PASSWORD == purpose)
	{
		gtk_entry_set_visibility(gl_string_dialog->priv->value_entry,FALSE);
	}
}

const gchar*
gl_string_dialog_get_value               ( GlStringDialog *gl_string_dialog )
{
	g_return_val_if_fail(gl_string_dialog!=NULL,NULL);
	g_return_val_if_fail(GL_IS_STRING_DIALOG(gl_string_dialog),NULL);
	return gl_string_dialog->priv->value;
}

void
gl_string_dialog_clean                   ( GlStringDialog *gl_string_dialog )
{
	g_return_if_fail(gl_string_dialog!=NULL);
	g_return_if_fail(GL_IS_STRING_DIALOG(gl_string_dialog));
	if(gl_string_dialog->priv->value)g_free(gl_string_dialog->priv->value);
	gl_string_dialog->priv->value = g_strdup("");
}


/** @} */
