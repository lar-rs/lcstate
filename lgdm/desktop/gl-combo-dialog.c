/*
 * @ingroup GlComboDialog
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

#include "gl-combo-dialog.h"
#include <mktlib.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

// static GlComboDialog *__gui_process_desktop = NULL;

struct _GlComboDialogPrivate {
    GtkLabel *     value_name;
    GtkLabel *     cancel_name;
    GtkListBox *   listbox_values;
    GtkListBoxRow *cancel_row;
    gchar *        value;
    gchar *        name;
    guint          index;
    gboolean       wait_set_walue;
    guint          move_tag;
    gboolean       block_activate;
};

enum {
    GL_COMBO_DIALOG_PROP_NULL,
    GL_COMBO_DIALOG_PROP_NAME,
    GL_COMBO_DIALOG_PROP_VALUE_STRING,
    GL_COMBO_DIALOG_PROP_VALUE_NAME,
};

enum { GL_COMBO_DIALOG_LAST_SIGNAL };

// static guint gl_combo_dialog_signals[GL_COMBO_DIALOG_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlComboDialog, gl_combo_dialog, GTK_TYPE_WINDOW);

/*
static void
combo_dialog_main_row_activated ( GlComboDialog *dialog, GtkButton *button )
{
        g_debug("combo_dialog_cancel_clicked_cb");
        if(dialog->priv->value!=NULL)gtk_entry_set_text(dialog->priv->value_entry,dialog->priv->value);
        gtk_widget_hide(GTK_WIDGET(dialog));
}
*/

static void gl_combo_dialog_row_activate_signal(GlComboRow *row, GlComboDialog *dialog) {
    if (!dialog->priv->wait_set_walue) {
        GList *row_list = gtk_container_get_children(GTK_CONTAINER(dialog->priv->listbox_values));
        GList *element  = NULL;
        guint  idx      = 0;

        for (element = row_list; element; element = element->next) {
            if (GL_IS_COMBO_ROW(element->data)) {
                idx++;

                if (row == GL_COMBO_ROW(element->data)) {
                    dialog->priv->index          = idx;
                    dialog->priv->wait_set_walue = TRUE;
                    gl_combo_row_activate(GL_COMBO_ROW(element->data));
                    dialog->priv->wait_set_walue = FALSE;

                    break;
                }
            }
        }

        if (row_list) g_list_free(row_list);

        g_object_set(dialog, "combo-value", gl_combo_row_get_value(GL_COMBO_ROW(row)), "value-name", gl_combo_row_get_name(GL_COMBO_ROW(row)), NULL);
        gtk_widget_hide(GTK_WIDGET(dialog));
    }
}

static void gl_combo_change_active_row(GlComboDialog *combo_dialog) {
    // if(gtk_widget_is_visible(GTK_WIDGET(combo_dialog)))
    //{
    // g_debug("CHECK Combobox value : %s",combo_dialog->priv->value);
    GList *row_list = gtk_container_get_children(GTK_CONTAINER(combo_dialog->priv->listbox_values));
    GList *l        = NULL;
    guint  idx      = 0;

    for (l = row_list; l != NULL; l = l->next) {
        if (GL_IS_COMBO_ROW(l->data)) {

            if (0 == g_strcmp0(combo_dialog->priv->value, gl_combo_row_get_value(GL_COMBO_ROW(l->data)))) {
                combo_dialog->priv->index          = idx;
                combo_dialog->priv->wait_set_walue = TRUE;
                gl_combo_row_activate(GL_COMBO_ROW(l->data));
                g_object_set(combo_dialog, "value-name", gl_combo_row_get_name(GL_COMBO_ROW(l->data)), NULL);
                combo_dialog->priv->wait_set_walue = FALSE;
            }

            idx++;
        }
    }
    if (row_list) g_list_free(row_list);
    //}
}

static gboolean combo_dialog_move_callback(gpointer user_data) {
    GlComboDialog *combo_dialog = GL_COMBO_DIALOG(user_data);
    gtk_window_move(GTK_WINDOW(combo_dialog), 5, 5);
    combo_dialog->priv->move_tag = 0;
    return FALSE;
}
static void combo_dialog_start_visible(GObject *object, GParamSpec *pspec, GlComboDialog *combo_dialog) {

    if (gtk_widget_is_visible(GTK_WIDGET(combo_dialog))) {
        gl_combo_change_active_row(combo_dialog);
    }
    if (combo_dialog->priv->move_tag == 0) g_timeout_add(20, combo_dialog_move_callback, combo_dialog);
}

static void combo_dialog_values_row_activated(GlComboDialog *combo_dialog, GtkListBoxRow *row, GtkListBox *list) {
    if (GL_IS_COMBO_ROW(row)) {
        gl_combo_row_activate(GL_COMBO_ROW(row));
    }
}

static void combo_dialog_main_row_activated(GlComboDialog *combo_dialog, GtkListBoxRow *row, GtkListBox *list) {
    if (row == combo_dialog->priv->cancel_row) gtk_widget_hide(GTK_WIDGET(combo_dialog));
}

static void gl_combo_dialog_init(GlComboDialog *gl_combo_dialog) {
    g_return_if_fail(gl_combo_dialog != NULL);
    g_return_if_fail(GL_IS_COMBO_DIALOG(gl_combo_dialog));
    gl_combo_dialog->priv           = gl_combo_dialog_get_instance_private(gl_combo_dialog);
    gl_combo_dialog->priv->move_tag = 0;
    gtk_widget_init_template(GTK_WIDGET(gl_combo_dialog));
    g_signal_connect(gl_combo_dialog, "notify::visible", G_CALLBACK(combo_dialog_start_visible), gl_combo_dialog);
    gl_combo_dialog->priv->wait_set_walue = FALSE;
    gtk_label_set_text(gl_combo_dialog->priv->cancel_name, _("CANCEL"));
}

static void gl_combo_dialog_finalize(GObject *object) {
    GlComboDialog *gl_combo_dialog = GL_COMBO_DIALOG(object);
    if (gl_combo_dialog->priv->value) g_free(gl_combo_dialog->priv->value);
    if (gl_combo_dialog->priv->name) g_free(gl_combo_dialog->priv->name);
    G_OBJECT_CLASS(gl_combo_dialog_parent_class)->finalize(object);
}

static void gl_combo_dialog_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_COMBO_DIALOG(object));
    GlComboDialog *gl_combo_dialog = GL_COMBO_DIALOG(object);
    switch (prop_id) {
    case GL_COMBO_DIALOG_PROP_NAME:
        gtk_label_set_text(gl_combo_dialog->priv->value_name, g_value_get_string(value));
        break;
    case GL_COMBO_DIALOG_PROP_VALUE_STRING:
        if (gl_combo_dialog->priv->value) g_free(gl_combo_dialog->priv->value);
        gl_combo_dialog->priv->value = g_value_dup_string(value);
        gl_combo_change_active_row(gl_combo_dialog);
        break;
    case GL_COMBO_DIALOG_PROP_VALUE_NAME:
        if (gl_combo_dialog->priv->name) g_free(gl_combo_dialog->priv->name);
        gl_combo_dialog->priv->name = g_value_dup_string(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_combo_dialog_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_COMBO_DIALOG(object));
    GlComboDialog *gl_combo_dialog = GL_COMBO_DIALOG(object);
    switch (prop_id) {
    case GL_COMBO_DIALOG_PROP_NAME:
        g_value_set_string(value, gtk_label_get_text(gl_combo_dialog->priv->value_name));
        break;
    case GL_COMBO_DIALOG_PROP_VALUE_STRING:
        g_value_set_string(value, gl_combo_dialog->priv->value);
        break;
    case GL_COMBO_DIALOG_PROP_VALUE_NAME:
        g_value_set_string(value, gl_combo_dialog->priv->name);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_combo_dialog_class_init(GlComboDialogClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->finalize       = gl_combo_dialog_finalize;
    object_class->set_property   = gl_combo_dialog_set_property;
    object_class->get_property   = gl_combo_dialog_get_property;

    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/ui/dialog/combo-dialog.ui");
    gtk_widget_class_bind_template_child_private(widget_class, GlComboDialog, cancel_row);
    gtk_widget_class_bind_template_child_private(widget_class, GlComboDialog, value_name);
    gtk_widget_class_bind_template_child_private(widget_class, GlComboDialog, cancel_name);
    gtk_widget_class_bind_template_child_private(widget_class, GlComboDialog, listbox_values);

    gtk_widget_class_bind_template_callback(widget_class, combo_dialog_main_row_activated);
    gtk_widget_class_bind_template_callback(widget_class, combo_dialog_values_row_activated);

    g_object_class_install_property(object_class, GL_COMBO_DIALOG_PROP_NAME,
                                    g_param_spec_string("combo-name", "Parameter name ", "Parameter name ", "No name", G_PARAM_WRITABLE | G_PARAM_READABLE));

    g_object_class_install_property(object_class, GL_COMBO_DIALOG_PROP_VALUE_STRING,
                                    g_param_spec_string("combo-value", "Parameter value", "Parameter value", "--", G_PARAM_WRITABLE | G_PARAM_READABLE));
    g_object_class_install_property(object_class, GL_COMBO_DIALOG_PROP_VALUE_NAME,
                                    g_param_spec_string("value-name", "Parameter value", "Parameter value", "--", G_PARAM_WRITABLE | G_PARAM_READABLE));
}

void gl_combo_dialog_add_row(GlComboDialog *dialog, GlComboRow *row) {
    g_return_if_fail(dialog != NULL);
    g_return_if_fail(GL_IS_COMBO_DIALOG(dialog));
    g_return_if_fail(row != NULL);
    g_return_if_fail(GL_IS_COMBO_ROW(row));
    GList *children = gtk_container_get_children(GTK_CONTAINER(dialog->priv->listbox_values));
    if (children && g_list_length(children)) {
        gtk_container_add(GTK_CONTAINER(dialog->priv->listbox_values), GTK_WIDGET(row));
        GtkListBoxRow *trow = gtk_list_box_get_row_at_index(dialog->priv->listbox_values, 0);
        if (trow) gl_combo_row_join_group(row, GL_COMBO_ROW(trow));
    } else {
        gtk_container_add(GTK_CONTAINER(dialog->priv->listbox_values), GTK_WIDGET(row));
    }
    g_signal_connect(row, "row-activated", G_CALLBACK(gl_combo_dialog_row_activate_signal), dialog);
}

const gchar *gl_combo_dialog_get_value(GlComboDialog *dialog) {
    g_return_val_if_fail(dialog != NULL, NULL);
    g_return_val_if_fail(GL_IS_COMBO_DIALOG(dialog), NULL);
    return dialog->priv->value;
}

guint gl_combo_dialog_get_index(GlComboDialog *dialog) {
    g_return_val_if_fail(dialog, 0);
    g_return_val_if_fail(GL_IS_COMBO_DIALOG(dialog), 0);

    return dialog->priv->index;
}

GlComboRow *gl_combo_row_get_activated(GlComboDialog *dialog) {
    GlComboRow *row      = NULL;
    GList *     row_list = gtk_container_get_children(GTK_CONTAINER(dialog->priv->listbox_values));
    GList *     l        = NULL;
    for (l = row_list; l != NULL; l = l->next) {
        if (GL_IS_COMBO_ROW(l->data)) {
            if (0 == g_strcmp0(dialog->priv->value, gl_combo_row_get_value(GL_COMBO_ROW(l->data)))) row = GL_COMBO_ROW(l->data);
        }
    }
    if (row_list) g_list_free(row_list);
    return row;
}

/** @} */
