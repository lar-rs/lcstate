/*
 * @ingroup GlBooleanDialog
 * @{
 * @file  gl-boolean-dialog.c	generated object file
 * @brief generated object file
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-boolean-dialog.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

// static GlBooleanDialog *__gui_process_desktop = NULL;

struct _GlBooleanDialogPrivate {
    GtkLabel *user_info;
    GtkLabel *value_name;
    GtkLabel *cancel_name;
    gchar *   value;
};

enum { GL_BOOLEAN_DIALOG_PROP_NULL, GL_BOOLEAN_DIALOG_PROP_NAME, GL_BOOLEAN_DIALOG_PROP_VALUE, GL_BOOLEAN_DIALOG_PROP_TEXT };

enum { GL_BOOLEAN_DIALOG_LAST_SIGNAL };

// static guint gl_boolean_dialog_signals[GL_BOOLEAN_DIALOG_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlBooleanDialog, gl_boolean_dialog, GTK_TYPE_WINDOW);

static void boolean_dialog_cancel_clicked_cb(GlBooleanDialog *dialog, GtkButton *button) {
    g_object_set(dialog, "boolean-value", FALSE, NULL);
    gtk_widget_hide(GTK_WIDGET(dialog));
}

static void boolean_dialog_set_clicked_cb(GlBooleanDialog *dialog, GtkButton *button) {
    g_object_set(dialog, "boolean-value", TRUE, NULL);
    gtk_widget_hide(GTK_WIDGET(dialog));
}

static void boolean_dialog_start_visible(GObject *object, GParamSpec *pspec, GlBooleanDialog *dialog) {
    gint x, y;
    gtk_window_get_position(GTK_WINDOW(dialog), &x, &y);
    gtk_window_move(GTK_WINDOW(dialog), x, 40);
}

static void gl_boolean_dialog_init(GlBooleanDialog *gl_boolean_dialog) {
    g_return_if_fail(gl_boolean_dialog != NULL);
    g_return_if_fail(GL_IS_BOOLEAN_DIALOG(gl_boolean_dialog));
    gl_boolean_dialog->priv = gl_boolean_dialog_get_instance_private(gl_boolean_dialog);
    gtk_widget_init_template(GTK_WIDGET(gl_boolean_dialog));
    g_signal_connect(gl_boolean_dialog, "notify::visible", G_CALLBACK(boolean_dialog_start_visible), gl_boolean_dialog);
    gtk_label_set_text(gl_boolean_dialog->priv->cancel_name, _("CANCEL"));
}

static void gl_boolean_dialog_finalize(GObject *object) {
    GlBooleanDialog *gl_boolean_dialog = GL_BOOLEAN_DIALOG(object);
    if (gl_boolean_dialog->priv->value) g_free(gl_boolean_dialog->priv->value);
    G_OBJECT_CLASS(gl_boolean_dialog_parent_class)->finalize(object);
}

static void gl_boolean_dialog_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_BOOLEAN_DIALOG(object));
    GlBooleanDialog *gl_boolean_dialog = GL_BOOLEAN_DIALOG(object);
    switch (prop_id) {
    case GL_BOOLEAN_DIALOG_PROP_NAME:
        if (gl_boolean_dialog->priv->value_name) gtk_label_set_text(gl_boolean_dialog->priv->value_name, g_value_get_string(value));
        break;
    case GL_BOOLEAN_DIALOG_PROP_VALUE:
        if (gl_boolean_dialog->priv->value) g_free(gl_boolean_dialog->priv->value);
        gl_boolean_dialog->priv->value = g_value_dup_string(value);
        break;
    case GL_BOOLEAN_DIALOG_PROP_TEXT:
        if (gl_boolean_dialog->priv->user_info) gtk_label_set_text(gl_boolean_dialog->priv->user_info, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_boolean_dialog_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_BOOLEAN_DIALOG(object));
    GlBooleanDialog *gl_boolean_dialog = GL_BOOLEAN_DIALOG(object);
    switch (prop_id) {
    case GL_BOOLEAN_DIALOG_PROP_NAME:
        if (gl_boolean_dialog->priv->value_name) g_value_set_string(value, gtk_label_get_text(gl_boolean_dialog->priv->value_name));
        break;
    case GL_BOOLEAN_DIALOG_PROP_VALUE:
        g_value_set_boolean(value, gl_boolean_dialog->priv->value);
        break;
    case GL_BOOLEAN_DIALOG_PROP_TEXT:
        if (gl_boolean_dialog->priv->user_info) gtk_label_get_text(gl_boolean_dialog->priv->user_info);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_boolean_dialog_class_init(GlBooleanDialogClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->finalize       = gl_boolean_dialog_finalize;
    object_class->set_property   = gl_boolean_dialog_set_property;
    object_class->get_property   = gl_boolean_dialog_get_property;

    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/ui/dialog/boolean-dialog.ui");
    gtk_widget_class_bind_template_child_private(widget_class, GlBooleanDialog, user_info);
    gtk_widget_class_bind_template_child_private(widget_class, GlBooleanDialog, value_name);
    gtk_widget_class_bind_template_child_private(widget_class, GlBooleanDialog, cancel_name);

    gtk_widget_class_bind_template_callback(widget_class, boolean_dialog_cancel_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, boolean_dialog_set_clicked_cb);

    g_object_class_install_property(object_class, GL_BOOLEAN_DIALOG_PROP_NAME,
                                    g_param_spec_boolean("boolean-name", "Parameter name ", "Parameter name ", "No name", G_PARAM_WRITABLE | G_PARAM_READABLE));

    g_object_class_install_property(object_class, GL_BOOLEAN_DIALOG_PROP_VALUE,
                                    g_param_spec_boolean("boolean-value", "Parameter value", "Parameter value", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE));

    g_object_class_install_property(
        object_class, GL_BOOLEAN_DIALOG_PROP_TEXT,
        g_param_spec_string("boolean-info", "Parameter value", "Parameter value", "---", GTK_INPUT_PURPOSE_FREE_FORM, G_PARAM_WRITABLE | G_PARAM_READABLE));
}

void gl_boolean_dialog_change_user_info(GlBooleanDialog *gl_boolean_dialog, const gchar *text) {
    g_return_if_fail(gl_boolean_dialog != NULL);
    g_return_if_fail(GL_IS_BOOLEAN_DIALOG(gl_boolean_dialog));
    g_return_if_fail(gl_boolean_dialog->priv->user_info != NULL);
    gtk_label_set_text(gl_boolean_dialog->priv->user_info, text);
}

/** @} */
