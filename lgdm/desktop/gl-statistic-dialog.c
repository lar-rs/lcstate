/*
 * @ingroup GlStatisticDialog
 * @{
 * @file  gl-statistic-dialog.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-statistic-dialog.h"

#include "../config.h"
#include <glib/gi18n-lib.h>



//static GlStatisticDialog *__gui_process_desktop = NULL;

struct _GlStatisticDialogPrivate
{
	guint    replicates;
	guint    outliers;
	gdouble  max_cv;
	guint    temp_replicates;
	guint    temp_outliers;
	gdouble  temp_max_cv;

	GtkSpinButton *replicate;
	GtkSpinButton *outlier;
	GtkSpinButton *cv_max;

};


enum {
	GL_STATISTIC_DIALOG_PROP_NULL,
	GL_STATISTIC_DIALOG_PROP_REPLICATE,
	GL_STATISTIC_DIALOG_PROP_OUTLIER,
	GL_STATISTIC_DIALOG_PROP_MAX_CV,
};


enum
{
	GL_STATISTIC_DIALOG_LAST_SIGNAL
};


//static guint gl_statistic_dialog_signals[GL_STATISTIC_DIALOG_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlStatisticDialog, gl_statistic_dialog, GTK_TYPE_WINDOW);


static void
statistic_dialog_set_clicked_cb ( GlStatisticDialog *dialog, GtkButton *button )
{
	gtk_widget_hide (GTK_WIDGET(dialog));
	g_object_set(dialog,"replicates",dialog->priv->temp_replicates,"outliers",dialog->priv->temp_outliers,"max-cv",dialog->priv->temp_max_cv,NULL);
}

static void
statistic_dialog_cancel_clicked_cb ( GlStatisticDialog *dialog, GtkButton *button )
{
	gtk_spin_button_set_value(dialog->priv->replicate,(gdouble)dialog->priv->replicates);
	gtk_spin_button_set_value(dialog->priv->outlier,(gdouble)dialog->priv->outliers);
	gtk_spin_button_set_value(dialog->priv->cv_max,dialog->priv->max_cv);
	gtk_widget_hide (GTK_WIDGET(dialog));
}


static void
statistic_replicate_value_changed (GlStatisticDialog *statistic , GtkSpinButton *spin_button )
{
	statistic->priv->temp_replicates = (guint) gtk_spin_button_get_value(spin_button);
}

static void
statistic_outliers_value_changed (GlStatisticDialog *statistic , GtkSpinButton *spin_button )
{
	statistic->priv->temp_outliers = (guint) gtk_spin_button_get_value(spin_button);
}


static void
statistic_cv_max_value_changed (GlStatisticDialog *statistic , GtkSpinButton *spin_button )
{
	statistic->priv->temp_max_cv = gtk_spin_button_get_value(spin_button);
}

static void
dialog_start_visible (GObject *object ,GParamSpec *pspec , GlStatisticDialog *dialog)
{
	if(gtk_widget_is_visible(GTK_WIDGET(dialog)))
	{
		gtk_spin_button_set_value(dialog->priv->replicate,(gdouble)dialog->priv->replicates);
		gtk_spin_button_set_value(dialog->priv->outlier,(gdouble)dialog->priv->outliers);
		gtk_spin_button_set_value(dialog->priv->cv_max,(gdouble)dialog->priv->max_cv);
	}
}

static void
gl_statistic_dialog_init(GlStatisticDialog *gl_statistic_dialog)
{
	g_return_if_fail (gl_statistic_dialog != NULL);
	g_return_if_fail (GL_IS_STATISTIC_DIALOG(gl_statistic_dialog));
	gl_statistic_dialog->priv = gl_statistic_dialog_get_instance_private (gl_statistic_dialog);
	gtk_widget_init_template (GTK_WIDGET (gl_statistic_dialog));
	gl_statistic_dialog->priv->replicates = 0;
	gl_statistic_dialog->priv->outliers   = 0;
	gl_statistic_dialog->priv->max_cv     = 0;
	g_signal_connect(gl_statistic_dialog,"notify::visible",G_CALLBACK(dialog_start_visible),gl_statistic_dialog);

}



static void
gl_statistic_dialog_finalize (GObject *object)
{
	//GlStatisticDialog* gl_statistic_dialog = GL_STATISTIC_DIALOG(object);
	G_OBJECT_CLASS (gl_statistic_dialog_parent_class)->finalize(object);
}



static void
gl_statistic_dialog_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_STATISTIC_DIALOG(object));
	GlStatisticDialog* gl_statistic_dialog = GL_STATISTIC_DIALOG(object);
	switch (prop_id)
	{
	case GL_STATISTIC_DIALOG_PROP_REPLICATE:
		gl_statistic_dialog->priv->replicates=g_value_get_uint(value);
		break;
	case GL_STATISTIC_DIALOG_PROP_OUTLIER:
		gl_statistic_dialog->priv->outliers=g_value_get_uint(value);
		break;
	case GL_STATISTIC_DIALOG_PROP_MAX_CV:
		gl_statistic_dialog->priv->max_cv=g_value_get_double(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_statistic_dialog_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_STATISTIC_DIALOG(object));
	GlStatisticDialog* gl_statistic_dialog = GL_STATISTIC_DIALOG(object);
	switch (prop_id)
	{
	case GL_STATISTIC_DIALOG_PROP_REPLICATE:
		g_value_set_uint(value,gl_statistic_dialog->priv->replicates);
		break;
	case GL_STATISTIC_DIALOG_PROP_OUTLIER:
		g_value_set_uint(value,gl_statistic_dialog->priv->outliers);
		break;
	case GL_STATISTIC_DIALOG_PROP_MAX_CV:
		g_value_set_double(value,gl_statistic_dialog->priv->max_cv);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_statistic_dialog_class_init(GlStatisticDialogClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_statistic_dialog_finalize;
	object_class -> set_property           =  gl_statistic_dialog_set_property;
	object_class -> get_property           =  gl_statistic_dialog_get_property;
	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/dialog/statistic-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlStatisticDialog, replicate);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatisticDialog, outlier);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatisticDialog, cv_max);

	gtk_widget_class_bind_template_callback (widget_class, statistic_dialog_set_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, statistic_dialog_cancel_clicked_cb);

	gtk_widget_class_bind_template_callback (widget_class, statistic_replicate_value_changed);
	gtk_widget_class_bind_template_callback (widget_class, statistic_outliers_value_changed);
	gtk_widget_class_bind_template_callback (widget_class, statistic_cv_max_value_changed);


	g_object_class_install_property (object_class,GL_STATISTIC_DIALOG_PROP_REPLICATE,
			g_param_spec_uint  ("replicates",
					"Statistic settings replicate",
					"Statistic settings replicate",
					1,100,1,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_STATISTIC_DIALOG_PROP_OUTLIER,
			g_param_spec_uint  ("outliers",
					"Statistic settings outliers",
					"Statistic settings outliers",
					0,30,0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_STATISTIC_DIALOG_PROP_MAX_CV,
			g_param_spec_double  ("max-cv",
					"Statistic settings max-cv",
					"Statistic settings max-cv",
					0.0,100.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

}

/** @} */
