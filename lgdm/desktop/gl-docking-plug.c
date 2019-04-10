/*
 * @ingroup GlDockingPlug
 * @{
 * @file  gl-dock-action.c	LGDM dock application dock
 * @brief LGDM dock application dock.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-docking-plug.h"
#include "gl-application-object.h"
#include "lgdm-desktop.h"
#include "lgdm-app-generated-code.h"
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <mkt-utils.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

enum {
  GL_DOCKING_PLUG_NONE,
  GL_DOCKING_PLUG_LDM_APPLICATION,
  GL_APP_GSETTINGS_APPLICATION
};

// static GlDockingPlug *__gui_process_dock = NULL;

struct _GlDockingPlugPrivate {
  gboolean is_run;
  GtkWidget *spinner;
  GtkWidget *waite_box;
  GtkSocket *socket;
  gchar *exec;
  GSubprocess *process;
  GDataInputStream *data_input;
  GCancellable *input_cancel;
  GCancellable *output_cancel;

  gchar read_buffer[4096];
};

enum {
  GL_DOCKING_PLUG_PROP_NULL,
  GL_DOCKING_PLUG_EXEC,
  GL_DOCKING_PLUG_RUNNED,

};

enum { GL_DOCKING_PLUG_LAST_SIGNAL };

// static guint gl_docking_plug_signals[GL_DOCKING_PLUG_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlDockingPlug, gl_docking_plug, GTK_TYPE_BOX);

// GL Application dbus client object manager functions -----------------------

static void gl_docking_plug_init(GlDockingPlug *dock) {
  g_return_if_fail(dock != NULL);
  g_return_if_fail(GL_IS_DOCKING_PLUG(dock));
  dock->priv = gl_docking_plug_get_instance_private(dock);
  dock->priv->is_run = FALSE;
  dock->priv->socket = NULL;
  dock->priv->process = NULL;
  dock->priv->data_input = NULL;
  dock->priv->input_cancel = g_cancellable_new();
  dock->priv->output_cancel = g_cancellable_new();

  //  Desktop window -----------------------------------------------------------
}

static void gl_docking_plug_finalize(GObject *object) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(object);
  g_cancellable_cancel(dock->priv->input_cancel);
  g_cancellable_cancel(dock->priv->output_cancel);
  if (dock->priv->process) {
    if (!g_subprocess_get_if_exited(dock->priv->process))
      g_subprocess_get_term_sig(dock->priv->process);
    g_object_unref(dock->priv->process);
  }
  if (dock->priv->exec)
    g_free(dock->priv->exec);

  G_OBJECT_CLASS(gl_docking_plug_parent_class)->finalize(object);
}

static gboolean docking_plug_plugin_add(gpointer user_data) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(user_data);
  gtk_widget_show(GTK_WIDGET(dock->priv->socket));
  gtk_widget_hide(GTK_WIDGET(dock->priv->waite_box));
  return FALSE;
}

static gboolean gl_docking_plug_plugin_added(GtkSocket *socket_,
                                             gpointer user_data) {
  // GlDockingPlug *dock  = GL_DOCKING_PLUG ( user_data);
  g_timeout_add(999, docking_plug_plugin_add, user_data);
  return TRUE;
}

static gboolean gl_docking_plug_plugin_removed(GtkSocket *socket_,
                                               gpointer user_data) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(user_data);
  gtk_widget_show(GTK_WIDGET(dock->priv->waite_box));
  gtk_spinner_start(GTK_SPINNER(dock->priv->spinner));
  gtk_widget_hide(GTK_WIDGET(dock->priv->socket));
  return TRUE;
}

/*
static void
gl_docking_plug_launched_callback (GAppLaunchContext *context,
               GAppInfo          *info,
               GVariant          *platform_data,
                           GlDockingPlug     *dock)
{


}

static void
gl_docking_plug_launch_failed_callback (GAppLaunchContext *context,
               gchar             *startup_notify_id,
               gpointer           user_data)
{
        g_debug("LAUNCH FAILED");
}
*/

static void read_done_cb(GObject *source_object, GAsyncResult *res,
                         gpointer user_data) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(user_data);
  GDataInputStream *stream = G_DATA_INPUT_STREAM(source_object);
  // if(g_cancellable_is_cancelled(dock->priv->input_cancel)) return ;
  gsize length = 0;
  // GError *error  = NULL;
  gchar *data_string =
      g_data_input_stream_read_line_finish(stream, res, &length, NULL);
  if (data_string) {
    // g_print("TEST:%s|", data_string);
    gchar *pos = g_strrstr_len("PLUGID=", 4095, dock->priv->read_buffer);
    if (pos) {
      g_print("FIND PLUGID; %s", pos);
    }
  }
  g_data_input_stream_read_line_async(
      stream, G_PRIORITY_DEFAULT, dock->priv->input_cancel, read_done_cb, dock);
}

/*
static void read_buffer_done_cb(GObject *source_object, GAsyncResult *res,
                                gpointer user_data) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(user_data);
  GInputStream *stream = G_INPUT_STREAM(source_object);
  // if(g_cancellable_is_cancelled(dock->priv->input_cancel)) return ;
  // GError *error  = NULL;

  g_input_stream_read_finish(stream, res, NULL);
  g_print("TEST:%s|", dock->priv->read_buffer);
  g_input_stream_read_async(
      stream, dock->priv->read_buffer, sizeof dock->priv->read_buffer,
      G_PRIORITY_DEFAULT, dock->priv->input_cancel, read_buffer_done_cb, dock);
}
*/

static void gl_docking_plug_waite_process(GObject *source_object,
                                          GAsyncResult *res,
                                          gpointer user_data) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(user_data);
  g_cancellable_cancel(dock->priv->input_cancel);
  g_cancellable_cancel(dock->priv->output_cancel);
  g_object_unref(source_object);

  dock->priv->process = NULL;
}

static void gl_docking_plug_constructed(GObject *object) {
  GlDockingPlug *dock = GL_DOCKING_PLUG(object);
  dock->priv->waite_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(dock->priv->waite_box), label, TRUE, TRUE, 0);
  gtk_widget_show(label);
  dock->priv->spinner = gtk_spinner_new();
  gtk_widget_set_size_request(dock->priv->spinner, 100, 100);
  gtk_box_pack_start(GTK_BOX(dock->priv->waite_box), dock->priv->spinner, TRUE,
                     TRUE, 0);
  gtk_widget_show(dock->priv->spinner);
  label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(dock->priv->waite_box), label, TRUE, TRUE, 0);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(dock), dock->priv->waite_box, TRUE, TRUE, 0);
  dock->priv->socket = GTK_SOCKET(gtk_socket_new());
  gtk_box_pack_start(GTK_BOX(dock), GTK_WIDGET(dock->priv->socket), TRUE, TRUE,
                     0);
  gtk_widget_set_no_show_all(GTK_WIDGET(dock->priv->waite_box), TRUE);
  gtk_widget_set_no_show_all(GTK_WIDGET(dock->priv->socket), TRUE);
  gtk_widget_show(dock->priv->waite_box);
  g_object_ref(dock->priv->socket);
  gtk_spinner_start(GTK_SPINNER(dock->priv->spinner));

  g_signal_connect(dock->priv->socket, "plug-added",
                   G_CALLBACK(gl_docking_plug_plugin_added), dock);
  g_signal_connect(dock->priv->socket, "plug-removed",
                   G_CALLBACK(gl_docking_plug_plugin_removed), dock);
  dock->priv->process = g_subprocess_new(G_SUBPROCESS_FLAGS_STDIN_PIPE |
                                             G_SUBPROCESS_FLAGS_STDOUT_PIPE,
                                         NULL, dock->priv->exec, NULL);
  dock->priv->data_input = g_data_input_stream_new(
      g_subprocess_get_stdout_pipe(dock->priv->process));

  // g_input_stream_read_async(out_stream,dock->priv->read_buffer,sizeof
  // dock->priv->read_buffer,G_PRIORITY_DEFAULT,NULL,read_buffer_done_cb,dock);
  g_subprocess_wait_async(dock->priv->process, NULL,
                          gl_docking_plug_waite_process, dock);
  g_data_input_stream_read_line_async(
      dock->priv->data_input, G_PRIORITY_DEFAULT, dock->priv->input_cancel,
      read_done_cb, dock);
}

static void gl_docking_plug_set_property(GObject *object, guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  ////TEST:
  g_return_if_fail(GL_IS_DOCKING_PLUG(object));
  GlDockingPlug *dock = GL_DOCKING_PLUG(object);
  switch (prop_id) {
  case GL_DOCKING_PLUG_EXEC:
    if (dock->priv->exec)
      g_free(dock->priv->exec);
    dock->priv->exec = g_value_dup_string(value);
    break;
  case GL_DOCKING_PLUG_RUNNED:
    dock->priv->is_run = g_value_get_boolean(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gl_docking_plug_get_property(GObject *object, guint prop_id,
                                         GValue *value, GParamSpec *pspec) {
  ////TEST:g_debug("Get (GL_MANAGER) property \n");
  g_return_if_fail(GL_IS_DOCKING_PLUG(object));
  GlDockingPlug *dock = GL_DOCKING_PLUG(object);
  switch (prop_id) {
  case GL_DOCKING_PLUG_EXEC:
    g_value_set_string(value, dock->priv->exec);
    break;
  case GL_DOCKING_PLUG_RUNNED:
    g_value_set_boolean(value, dock->priv->is_run);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gl_docking_plug_class_init(GlDockingPlugClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = gl_docking_plug_finalize;
  object_class->constructed = gl_docking_plug_constructed;
  object_class->set_property = gl_docking_plug_set_property;
  object_class->get_property = gl_docking_plug_get_property;

  g_object_class_install_property(
      object_class, GL_DOCKING_PLUG_RUNNED,
      g_param_spec_boolean("runned", "Desktop action button name",
                           "Desktop action button name", FALSE,
                           G_PARAM_WRITABLE | G_PARAM_READABLE));

  g_object_class_install_property(
      object_class, GL_DOCKING_PLUG_EXEC,
      g_param_spec_string("app-exec", "Desktop application file ",
                          "Desktop application file ", "exec",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void write_cb(GObject *source_object, GAsyncResult *result,
                     gpointer user_data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source_object);
  GError *error = NULL;
  /* Finish the write. */
  g_output_stream_write_finish(stream, result, &error);
  if (error != NULL) {
    g_warning("Error: %s", error->message);
    g_error_free(error);
  }
  g_free(user_data);
}

void gl_docking_plug_write_pipe(GlDockingPlug *docking, const gchar *format,
                                ...) {
  g_return_if_fail(docking != NULL);
  g_return_if_fail(GL_IS_DOCKING_PLUG(docking));
  va_list args;
  gchar *buffer;
  va_start(args, format);
  buffer = g_strdup_vprintf(format, args);
  va_end(args);
  if (docking->priv->process) {
    GOutputStream *out_stream =
        g_subprocess_get_stdin_pipe(docking->priv->process);
    g_output_stream_write_async(out_stream, buffer, sizeof buffer,
                                G_PRIORITY_DEFAULT,
                                docking->priv->output_cancel, write_cb, buffer);
  }
}

/** @} */
