/*
 * @ingroup GlSpinnerDialog
 * @{
 * @file  gl-spinner-dialog.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-spinner-dialog.h"

#include "../config.h"
#include <glib/gi18n-lib.h>



//static GlSpinnerDialog *__gui_process_desktop = NULL;

struct _GlSpinnerDialogPrivate
{
	GtkSpinButton     *value_spinn;
	GtkLabel          *value_name;
	GtkLabel          *cancel_name;
	GtkLabel          *set_label;
	gdouble            result;
	gdouble            min;
	gdouble            max;
	guint              move_tag;
	guint              enter_tag;

};


enum {
	GL_SPINNER_DIALOG_PROP_NULL,
	GL_SPIN_BUTTON_PROP_RESULT,
	GL_SPIN_BUTTON_PROP_NAME,
	GL_SPIN_BUTTON_PROP_MIN,
	GL_SPIN_BUTTON_PROP_MAX,
};


enum
{
	GL_SPINNER_DIALOG_LAST_SIGNAL
};


//static guint gl_spinner_dialog_signals[GL_SPINNER_DIALOG_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlSpinnerDialog, gl_spinner_dialog, GTK_TYPE_WINDOW);





static void
spinner_dialog_set_clicked_cb ( GlSpinnerDialog *spinn_button, GtkButton *button )
{
	gdouble value = gtk_spin_button_get_value(spinn_button->priv->value_spinn);
	g_object_set(spinn_button,"spiner-result",value,NULL);
	gtk_widget_hide(GTK_WIDGET(spinn_button));
//	GtkTextBuffer *buffer = gtk_text_view_get_buffer(spinn_button->priv->value_view);

}



static void
spinner_dialog_cancel_clicked_cb ( GlSpinnerDialog *spinn_button, GtkButton *button )
{
	gtk_spin_button_set_value(spinn_button->priv->value_spinn,spinn_button->priv->result);
	gtk_widget_hide(GTK_WIDGET(spinn_button));
}



static gboolean
spinner_dialog_move_callback ( gpointer user_data )
{
	GlSpinnerDialog* dialog = GL_SPINNER_DIALOG(user_data);
	gtk_window_move(GTK_WINDOW(dialog),1,1);
	dialog->priv->move_tag = 0;
	return FALSE;
}


static void
spinner_dialog_start_visible (GObject *object ,GParamSpec *pspec , GlSpinnerDialog *dialog)
{

	if(gtk_widget_is_visible(GTK_WIDGET(dialog)))
	{
		if(dialog->priv->value_spinn!=NULL)gtk_spin_button_set_value(dialog->priv->value_spinn,dialog->priv->result);
		gtk_window_set_focus(GTK_WINDOW(dialog),GTK_WIDGET(dialog->priv->value_spinn));

	}
	if(dialog->priv->move_tag == 0)
		g_timeout_add(20,spinner_dialog_move_callback,dialog);
}

// Test signals ..


static void
gl_spinner_dialog_init(GlSpinnerDialog *gl_spinner_dialog)
{
	g_return_if_fail (gl_spinner_dialog != NULL);
	g_return_if_fail (GL_IS_SPINNER_DIALOG(gl_spinner_dialog));
	gl_spinner_dialog->priv = gl_spinner_dialog_get_instance_private (gl_spinner_dialog);
	gtk_widget_init_template (GTK_WIDGET (gl_spinner_dialog));
	gl_spinner_dialog->priv->move_tag = 0;
	g_signal_connect(gl_spinner_dialog,"notify::visible",G_CALLBACK(spinner_dialog_start_visible),gl_spinner_dialog);
	gtk_label_set_text(gl_spinner_dialog->priv->cancel_name,_("CANCEL"));
	gtk_label_set_text(gl_spinner_dialog->priv->set_label,_("OK"));


}





static void
gl_spinner_dialog_finalize (GObject *object)
{
	//GlSpinnerDialog* gl_spinner_dialog = GL_SPINNER_DIALOG(object);
	G_OBJECT_CLASS (gl_spinner_dialog_parent_class)->finalize(object);
}

static gboolean
spinner_enter_callback ( gpointer user_data )
{
	GlSpinnerDialog* dialog = GL_SPINNER_DIALOG(user_data);
	gdouble value = gtk_spin_button_get_value(dialog->priv->value_spinn);
	g_object_set(dialog,"spiner-result",value,NULL);
	gtk_widget_hide(GTK_WIDGET(dialog));
	dialog->priv->enter_tag = 0;
	return FALSE;
}

static gboolean
spinner_dialog_key_press_event_cb (GlSpinnerDialog* dialog,GdkEventKey *event , GtkWidget *widget)
{
	if(event->keyval == GDK_KEY_Return)
	{
		if(dialog->priv->enter_tag == 0)
			dialog->priv->enter_tag =g_timeout_add(20,spinner_enter_callback,dialog);
	}
	return FALSE;
}

static void
gl_spinner_dialog_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SPINNER_DIALOG(object));
	GlSpinnerDialog* gl_spinner_dialog = GL_SPINNER_DIALOG(object);
	switch (prop_id)
	{
	case GL_SPIN_BUTTON_PROP_RESULT:
		gl_spinner_dialog->priv->result = g_value_get_double(value);
		break;
	case GL_SPIN_BUTTON_PROP_MIN:
		gl_spinner_dialog->priv->min = g_value_get_double(value);
		gtk_spin_button_set_range(gl_spinner_dialog->priv->value_spinn,gl_spinner_dialog->priv->min,gl_spinner_dialog->priv->max);
		break;
	case GL_SPIN_BUTTON_PROP_MAX:
		gl_spinner_dialog->priv->max = g_value_get_double(value);
		gtk_spin_button_set_range(gl_spinner_dialog->priv->value_spinn,gl_spinner_dialog->priv->min,gl_spinner_dialog->priv->max);
		break;
	case GL_SPIN_BUTTON_PROP_NAME:
		gtk_label_set_text(gl_spinner_dialog->priv->value_name,g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_spinner_dialog_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SPINNER_DIALOG(object));
	GlSpinnerDialog* gl_spinner_dialog = GL_SPINNER_DIALOG(object);
	switch (prop_id)
	{
	case GL_SPIN_BUTTON_PROP_RESULT:
		g_value_set_double(value,gl_spinner_dialog->priv->result);
		break;
	case GL_SPIN_BUTTON_PROP_MIN:
		g_value_set_double(value,gl_spinner_dialog->priv->min);
		break;
	case GL_SPIN_BUTTON_PROP_MAX:
		g_value_set_double(value,gl_spinner_dialog->priv->max);
		break;
	case GL_SPIN_BUTTON_PROP_NAME:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_spinner_dialog_class_init(GlSpinnerDialogClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_spinner_dialog_finalize;
	object_class -> set_property           =  gl_spinner_dialog_set_property;
	object_class -> get_property           =  gl_spinner_dialog_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/dialog/spinner-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlSpinnerDialog, value_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlSpinnerDialog, value_spinn);
	gtk_widget_class_bind_template_child_private (widget_class, GlSpinnerDialog, cancel_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlSpinnerDialog, set_label);

	gtk_widget_class_bind_template_callback (widget_class, spinner_dialog_set_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, spinner_dialog_cancel_clicked_cb);

	gtk_widget_class_bind_template_callback (widget_class, spinner_dialog_key_press_event_cb);

	g_object_class_install_property (object_class,GL_SPIN_BUTTON_PROP_NAME,
			g_param_spec_string ("spiner-name",
					"Spinner dialog result value ",
					"Draw mantissa counter",
					"No Name",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));


	g_object_class_install_property (object_class,GL_SPIN_BUTTON_PROP_RESULT,
			g_param_spec_double ("spiner-result",
					"Spinner dialog result value ",
					"Draw mantissa counter",
					-G_MAXDOUBLE,G_MAXDOUBLE,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_SPIN_BUTTON_PROP_MIN,
			g_param_spec_double ("spiner-min",
					"Spinner dialog result value ",
					"Draw mantissa counter",
					-G_MAXDOUBLE,G_MAXDOUBLE,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_SPIN_BUTTON_PROP_MAX,
			g_param_spec_double ("spiner-max",
					"Spinner dialog result value ",
					"Draw mantissa counter",
					-G_MAXDOUBLE,G_MAXDOUBLE,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));


	//gtk_spin_button_set_increments()
}


void
gl_spinner_dialog_set_digits ( GlSpinnerDialog* spinner_dialog , guint digits )
{
	g_return_if_fail(spinner_dialog!=NULL);
	g_return_if_fail(GL_IS_SPINNER_DIALOG(spinner_dialog));
	gtk_spin_button_set_digits(spinner_dialog->priv->value_spinn,digits);
}

void
gl_spinner_dialog_set_increments ( GlSpinnerDialog* spinner_dialog , gdouble step,  gdouble page )
{
	g_return_if_fail(spinner_dialog!=NULL);
	g_return_if_fail(GL_IS_SPINNER_DIALOG(spinner_dialog));
	gtk_spin_button_set_increments(spinner_dialog->priv->value_spinn,step,page);
}
void
gl_spinner_dialog_set_range ( GlSpinnerDialog* spinner_dialog , gdouble min,  gdouble max )
{
	g_return_if_fail(spinner_dialog!=NULL);
	g_return_if_fail(GL_IS_SPINNER_DIALOG(spinner_dialog));
	g_object_set(spinner_dialog,"spiner-min",min,"spiner-max",max,NULL);

}

void
gl_spinner_dialog_set_numeric ( GlSpinnerDialog* spinner_dialog , gboolean numeric )
{
	g_return_if_fail(spinner_dialog!=NULL);
	g_return_if_fail(GL_IS_SPINNER_DIALOG(spinner_dialog));
	gtk_spin_button_set_numeric(spinner_dialog->priv->value_spinn,numeric);
}

GtkAdjustment*
gl_spinner_dialog_get_adjustment          ( GlSpinnerDialog* spinner_dialog )
{
	g_return_val_if_fail(spinner_dialog!=NULL,NULL);
	g_return_val_if_fail(GL_IS_SPINNER_DIALOG(spinner_dialog),NULL);
	return gtk_spin_button_get_adjustment(spinner_dialog->priv->value_spinn);
}


//void
//gl_spin_dialog_set_incremets


/** @} */
