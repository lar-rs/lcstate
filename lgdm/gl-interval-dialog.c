/*
 * @ingroup GlIntervalDialog
 * @{
 * @file  gl-interval-dialog.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-interval-dialog.h"
#include "gl-date-dialog.h"
#include <mktlib.h>



#include "../config.h"
#include <glib/gi18n-lib.h>




//static GlIntervalDialog *__gui_process_desktop = NULL;

struct _GlIntervalDialogPrivate
{
	GtkLabel          *value_name;

	GtkRevealer       *date_interval;
	GtkBox            *date_interval_box;
	GtkLabel          *cancel_name;

	GtkWidget         *start_date;
	GtkWidget         *end_date;
	gdouble            start_sec;
	gdouble            end_sec;
	guint              move_tag;
};


enum {
	GL_INTERVAL_DIALOG_PROP_NULL,
	GL_INTERVAL_DIALOG_PROP_NAME,
	GL_INTERVAL_DIALOG_PROP_INTERVAL_SELECTED,
	GL_INTERVAL_DIALOG_PROP_START_SEC,
	GL_INTERVAL_DIALOG_PROP_END_SEC,
	GL_INTERVAL_DIALOG_PROP_LAST_ACTIVATED,
};


enum
{
	GL_INTERVAL_DIALOG_LAST_SIGNAL
};


//static guint gl_interval_dialog_signals[GL_INTERVAL_DIALOG_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlIntervalDialog, gl_interval_dialog, GTK_TYPE_WINDOW);



static void
cancel_clicked_cb ( GlIntervalDialog *dialog, GtkButton *button )
{
	//g_object_set(dialog,"interval-value",FALSE,NULL);
	gtk_widget_hide(GTK_WIDGET(dialog));
}

static void
set_clicked_cb ( GlIntervalDialog *dialog, GtkButton *button )
{
	gdouble start_sec  = gl_date_dialog_get_total_sec(GL_DATE_DIALOG(dialog->priv->start_date));
	gdouble end_sec   = gl_date_dialog_get_total_sec(GL_DATE_DIALOG(dialog->priv->end_date));
	g_object_set(dialog,"interval-start",start_sec,"interval-end",end_sec,NULL);
	gtk_widget_hide(GTK_WIDGET(dialog));
}



static gboolean
interval_dialog_move_callback ( gpointer user_data )
{
	GlIntervalDialog* dialog = GL_INTERVAL_DIALOG(user_data);
	gtk_window_move(GTK_WINDOW(dialog),1,1);
	dialog->priv->move_tag = 0;
	return FALSE;
}

static void
interval_dialog_start_visible (GObject *object ,GParamSpec *pspec , GlIntervalDialog *dialog)
{
	gl_date_dialog_set_total_sec(GL_DATE_DIALOG(dialog->priv->start_date ),(dialog->priv->start_sec));
	gl_date_dialog_set_total_sec(GL_DATE_DIALOG(dialog->priv->end_date ),(dialog->priv->end_sec));
	if(dialog->priv->move_tag == 0)
		g_timeout_add(20,interval_dialog_move_callback,dialog);
}



static void
gl_interval_dialog_init(GlIntervalDialog *gl_interval_dialog)
{
	g_return_if_fail (gl_interval_dialog != NULL);
	g_return_if_fail (GL_IS_INTERVAL_DIALOG(gl_interval_dialog));
	gl_interval_dialog->priv = gl_interval_dialog_get_instance_private(gl_interval_dialog);
	gtk_widget_init_template (GTK_WIDGET (gl_interval_dialog));
	gtk_label_set_text(gl_interval_dialog->priv->cancel_name,_("CANCEL"));

}



static void
gl_interval_dialog_constructed (GObject *object)
{
	GlIntervalDialog* dialog = GL_INTERVAL_DIALOG(object);
	gtk_widget_show(GTK_WIDGET(dialog->priv->date_interval));
	dialog->priv->start_sec = market_db_time_now()-14400.0;
	dialog->priv->end_sec = market_db_time_now();
	dialog->priv->start_date = GTK_WIDGET(g_object_new(GL_TYPE_DATE_DIALOG,"value-name",_("Start date"),"activate-seconds",FALSE,"total-seconds",(dialog->priv->start_sec),NULL));
	gl_date_dialog_set_total_sec(GL_DATE_DIALOG(dialog->priv->start_date ),(dialog->priv->start_sec));
	dialog->priv->end_date = GTK_WIDGET(g_object_new(GL_TYPE_DATE_DIALOG,"value-name",_("End date"),"activate-seconds",FALSE,"total-seconds",dialog->priv->end_sec,NULL));
	gl_date_dialog_set_total_sec(GL_DATE_DIALOG(dialog->priv->end_date ),(dialog->priv->end_sec));
	gtk_box_pack_start(dialog->priv->date_interval_box,dialog->priv->start_date,TRUE,TRUE,2);
	gtk_widget_show_all(dialog->priv->start_date);
	gtk_box_pack_start(dialog->priv->date_interval_box,dialog->priv->end_date,TRUE,TRUE,2);
	gtk_widget_show_all(dialog->priv->end_date);
	g_signal_connect(dialog,"notify::visible",G_CALLBACK(interval_dialog_start_visible),dialog);
	G_OBJECT_CLASS (gl_interval_dialog_parent_class)->constructed(object);
}

static void
gl_interval_dialog_finalize (GObject *object)
{
	//GlIntervalDialog* gl_interval_dialog = GL_INTERVAL_DIALOG(object);
	G_OBJECT_CLASS (gl_interval_dialog_parent_class)->finalize(object);
}





static void
gl_interval_dialog_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_INTERVAL_DIALOG(object));
	GlIntervalDialog* gl_interval_dialog = GL_INTERVAL_DIALOG(object);
	switch (prop_id)
	{
	case GL_INTERVAL_DIALOG_PROP_NAME:
		if(gl_interval_dialog->priv->value_name)gtk_label_set_text(gl_interval_dialog->priv->value_name,g_value_get_string(value));
		break;
	case GL_INTERVAL_DIALOG_PROP_START_SEC:
		gl_interval_dialog->priv->start_sec = g_value_get_double(value);
		break;
	case GL_INTERVAL_DIALOG_PROP_END_SEC:
		gl_interval_dialog->priv->end_sec = g_value_get_double(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_interval_dialog_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_INTERVAL_DIALOG(object));
	GlIntervalDialog* gl_interval_dialog = GL_INTERVAL_DIALOG(object);
	switch (prop_id)
	{
	case GL_INTERVAL_DIALOG_PROP_START_SEC:
		g_value_set_double(value,gl_interval_dialog->priv->start_sec);
		break;
	case GL_INTERVAL_DIALOG_PROP_END_SEC:
		g_value_set_double(value,gl_interval_dialog->priv->end_sec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_interval_dialog_class_init(GlIntervalDialogClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_interval_dialog_finalize;
	object_class -> set_property           =  gl_interval_dialog_set_property;
	object_class -> get_property           =  gl_interval_dialog_get_property;
	object_class -> constructed            =  gl_interval_dialog_constructed;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/dialog/interval-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlIntervalDialog, value_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlIntervalDialog, cancel_name);

	gtk_widget_class_bind_template_child_private (widget_class, GlIntervalDialog, date_interval);
	gtk_widget_class_bind_template_child_private (widget_class, GlIntervalDialog, date_interval_box);




	gtk_widget_class_bind_template_callback (widget_class, cancel_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, set_clicked_cb);

	g_object_class_install_property (object_class,GL_INTERVAL_DIALOG_PROP_NAME,
			g_param_spec_string ("interval-name",
					"Parameter name ",
					"Parameter name ",
					"No name",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

		g_object_class_install_property (object_class,GL_INTERVAL_DIALOG_PROP_START_SEC,
			g_param_spec_double("interval-start",
					"Parameter value",
					"Parameter value",
					0.0,G_MAXDOUBLE,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_INTERVAL_DIALOG_PROP_END_SEC,
			g_param_spec_double("interval-end",
					"Parameter value",
					"Parameter value",
					0.0,G_MAXDOUBLE,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

}


gdouble
gl_interval_get_start_time                 ( GlIntervalDialog *dialog )
{
	g_return_val_if_fail(dialog != NULL,0.0);
	g_return_val_if_fail(GL_IS_INTERVAL_DIALOG(dialog),0.0);
	return dialog->priv->start_sec;
}

gdouble
gl_interval_get_stop_time                  ( GlIntervalDialog *dialog )
{
	g_return_val_if_fail(dialog != NULL,0.0);
	g_return_val_if_fail(GL_IS_INTERVAL_DIALOG(dialog),0.0);
	return dialog->priv->end_sec;

}


/** @} */
