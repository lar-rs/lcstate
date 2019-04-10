/**
 * @ingroup GlDateCompleteDialog
 * @{
 * @file  gl-date-complete-dialog.c	object file
 *
 *  Copyright (C) LAR 2015
 *
 * @author G.Stützer <gstuetzer@lar.com>
 *
 */

#include "gl-date-complete-dialog.h"
#include "gl-date-dialog.h"
#include <market-time.h>

#include "../config.h"
#include <glib/gi18n-lib.h>


struct _GlDateCompleteDialogPrivate
{
	GtkRevealer* date_revealer;
	GtkBox*      date_box;

	GtkLabel    *cancel_name;
	GtkLabel    *set_label;

	GlDateDialog* date;

	gdouble total_sec;
};


enum {
	PROP_NULL,
	PROP_NAME
};

enum
{
	SIGNAL_CANCEL,
	SIGNAL_OK,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = {0};


G_DEFINE_TYPE_WITH_PRIVATE (GlDateCompleteDialog, gl_date_complete_dialog, GTK_TYPE_WINDOW);



static void date_complete_dialog_cancel_clicked_cb (GlDateCompleteDialog* dialog, GtkButton* button)
{
	gtk_widget_hide (GTK_WIDGET (dialog));
	g_signal_emit   (dialog, signals[SIGNAL_CANCEL], 0);
}

static void date_complete_dialog_set_clicked_cb (GlDateCompleteDialog* dialog, GtkButton* button)
{
	gtk_widget_hide (GTK_WIDGET (dialog));
	g_signal_emit   (dialog, signals[SIGNAL_OK], 0);
}


static void date_complete_dialog_start_visible (GObject* object, GParamSpec* pspec, GlDateCompleteDialog* dialog)
{
	gint x, y;

//	if (gtk_widget_is_visible (GTK_WIDGET (dialog))) {
//	}

	gtk_window_get_position (GTK_WINDOW(dialog), &x, &y);
	gtk_window_move         (GTK_WINDOW(dialog),  x, 40);
}



static void gl_date_complete_dialog_init (GlDateCompleteDialog* gl_date_complete_dialog)
{
	g_return_if_fail (gl_date_complete_dialog);
	g_return_if_fail (GL_IS_DATE_COMPLETE_DIALOG(gl_date_complete_dialog));

	gl_date_complete_dialog -> priv = gl_date_complete_dialog_get_instance_private (gl_date_complete_dialog);

	gtk_widget_init_template (GTK_WIDGET (gl_date_complete_dialog));
}



static void gl_date_complete_dialog_finalize (GObject* object)
{
//	GlDateCompleteDialog* gl_date_complete_dialog = GL_DATE_COMPLETE_DIALOG(object);
	G_OBJECT_CLASS (gl_date_complete_dialog_parent_class)->finalize(object);
}



static void gl_date_complete_dialog_set_property (GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
	GlDateCompleteDialog* dialog;

	g_return_if_fail (GL_IS_DATE_COMPLETE_DIALOG (object));

	dialog = GL_DATE_COMPLETE_DIALOG(object);

	switch (prop_id) {
		case PROP_NAME:
			if (dialog -> priv -> date)
				g_object_set (dialog->priv->date, "value-name", g_value_get_string(value), NULL);

			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}

}



static void gl_date_complete_dialog_get_property (GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
	GlDateCompleteDialog* dialog;

	g_return_if_fail (GL_IS_DATE_COMPLETE_DIALOG (object));

	dialog = GL_DATE_COMPLETE_DIALOG (object);

	switch (prop_id) {
		case PROP_NAME:
			if (dialog -> priv -> date) {
				gchar* string;
				g_object_get       (dialog->priv->date, "value-name", &string, NULL);
				g_value_set_string (value,              string);
			}
			else
				g_value_set_string (value, "--");

			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}



static void gl_date_complete_dialog_constructed (GObject* object)
{

	GlDateCompleteDialog* dialog = GL_DATE_COMPLETE_DIALOG (object);

	gtk_widget_show (GTK_WIDGET (dialog -> priv -> date_revealer));

	dialog -> priv -> date = GL_DATE_DIALOG (g_object_new (GL_TYPE_DATE_DIALOG, "value-name", _("date and time"), "activate-seconds", FALSE, NULL));

	gl_date_dialog_set_total_sec (dialog->priv->date, market_db_time_now());

	gtk_box_pack_start  (dialog->priv->date_box, GTK_WIDGET(dialog->priv->date), TRUE, TRUE, 2);
	gtk_widget_show_all (GTK_WIDGET (dialog -> priv -> date));

	g_signal_connect (dialog, "notify::visible", G_CALLBACK(date_complete_dialog_start_visible), dialog);
	gtk_label_set_text(dialog->priv->set_label,_("OK"));
	gtk_label_set_text(dialog->priv->cancel_name,_("CANCEL"));

	if (G_OBJECT_CLASS (gl_date_complete_dialog_parent_class) -> constructed)
		G_OBJECT_CLASS (gl_date_complete_dialog_parent_class) -> constructed (object);
}



static void gl_date_complete_dialog_class_init (GlDateCompleteDialogClass* klass)
{
	GObjectClass*   object_class = G_OBJECT_CLASS   (klass);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

	object_class -> finalize      = gl_date_complete_dialog_finalize;
	object_class -> get_property  = gl_date_complete_dialog_get_property;
	object_class -> set_property  = gl_date_complete_dialog_set_property;
	object_class -> constructed   = gl_date_complete_dialog_constructed;

	gtk_widget_class_set_template_from_resource (widget_class, "/lgdm/ui/dialog/date-complete-dialog.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GlDateCompleteDialog, date_revealer);  // Zum sichtbar machen
	gtk_widget_class_bind_template_child_private (widget_class, GlDateCompleteDialog, date_box);       // Einhängen des GL_TYPE_DATE_DIALOG Objektes
	gtk_widget_class_bind_template_child_private (widget_class, GlDateCompleteDialog, cancel_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateCompleteDialog, set_label);


	gtk_widget_class_bind_template_callback (widget_class, date_complete_dialog_cancel_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, date_complete_dialog_set_clicked_cb);

	g_object_class_install_property (
		object_class,
		PROP_NAME,
		g_param_spec_string ("value-name", _("Parameter name"), _("Parameter name"), "No name", G_PARAM_WRITABLE)
	);

	signals [SIGNAL_CANCEL] = g_signal_new ("cancel-pressed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	signals [SIGNAL_OK]     = g_signal_new ("ok-pressed",     G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

gdouble gl_date_complete_dialog_get_time (GlDateCompleteDialog* dialog)
{
	if (! dialog -> priv -> date)
		return 0;

	return gl_date_dialog_get_total_sec (dialog -> priv -> date);
}

void gl_date_complete_dialog_set_time (GlDateCompleteDialog* dialog, gdouble total_sec)
{
	if (dialog -> priv -> date)
		gl_date_dialog_set_total_sec (dialog->priv->date, total_sec);
}


/** @} */
