/*
 * @ingroup GlAppInfo
 * @{
 * @file  gl-info-action.c	LGDM info application info
 * @brief LGDM info application info.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-app-info.h"
#include "gl-application-object.h"
#include "gl-desktop.h"
#include "lgdm-app-generated-code.h"
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <mkt-utils.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

enum { GL_APP_INFO_NONE, GL_APP_INFO_LDM_APPLICATION, GL_APP_GSETTINGS_APPLICATION };

/*
static GHashTable*
app_info_get_table ( )
{
        static GHashTable *infos = NULL;
        if(infos == NULL)
                infos = g_hash_table_new(g_str_hash,g_str_equal);
        return infos;
}


*/

// static GlAppInfo *__gui_process_info = NULL;

struct _GlAppInfoPrivate {
    GKeyFile *         key_file;
    gboolean           plugin_added;
    gboolean           is_run;
    GtkWidget *        box;
    GtkWidget *        spinner;
    GtkWidget *        waite_box;
    GtkSocket *        socket;
    GAppInfo *         appinfo;
    GAppLaunchContext *launch_content;
    GDBusActionGroup * group;
    gchar *            run_layout;
    gchar *            bus_id;
    guint              signal;
    GuiAppGapp *       proxy;
};

enum {
    GL_APP_INFO_PROP_NULL,
    GL_APP_INFO_APPLICATION,
    GL_APP_INFO_RUNNED,

};

enum { GL_APP_INFO_START, GL_APP_INFO_ACTIVATE_LDM_APPLICATION, GL_APP_INFO_ADD_LDM_APPLICATION, GL_APP_INFO_REMOVE_LDM_APPLICATION, GL_APP_INFO_LAST_SIGNAL };

static guint gl_app_info_signals[GL_APP_INFO_LAST_SIGNAL] = {0};

G_DEFINE_TYPE_WITH_PRIVATE(GlAppInfo, gl_app_info, G_TYPE_OBJECT);

// GL Application dbus client object manager functions -----------------------

static void gl_app_info_init(GlAppInfo *info) {
    g_return_if_fail(info != NULL);
    g_return_if_fail(GL_IS_APP_INFO(info));
    info->priv                 = gl_app_info_get_instance_private(info);
    info->priv->is_run         = FALSE;
    info->priv->key_file       = NULL;
    info->priv->socket         = NULL;
    info->priv->launch_content = g_app_launch_context_new();
    info->priv->run_layout     = NULL;
    info->priv->signal         = 0;
    info->priv->proxy          = NULL;

    //  Desktop window -----------------------------------------------------------
}

static void gl_app_info_finalize(GObject *object) {
    GlAppInfo *info = GL_APP_INFO(object);
    if (info->priv->appinfo) g_object_unref(info->priv->appinfo);
    if (info->priv->key_file) g_key_file_free(info->priv->key_file);
    if (info->priv->run_layout) g_free(info->priv->run_layout);
    G_OBJECT_CLASS(gl_app_info_parent_class)->finalize(object);
}

static gboolean app_info_plugin_add(gpointer user_data) {
    GlAppInfo *info = GL_APP_INFO(user_data);
    gtk_widget_show(GTK_WIDGET(info->priv->socket));
    gtk_widget_hide(GTK_WIDGET(info->priv->waite_box));
    info->priv->plugin_added = TRUE;
    return FALSE;
}

static gboolean gl_app_info_plugin_added(GtkSocket *socket_, gpointer user_data) {
    // GlAppInfo *info  = GL_APP_INFO ( user_data);
    g_timeout_add(999, app_info_plugin_add, user_data);
    return TRUE;
}

static gboolean gl_app_info_plugin_removed(GtkSocket *socket_, gpointer user_data) {
    GlAppInfo *info = GL_APP_INFO(user_data);
    g_signal_emit(info, gl_app_info_signals[GL_APP_INFO_REMOVE_LDM_APPLICATION], 0);

    return TRUE;
}

static void launch_application_real(GlAppInfo *info, const gchar *layout_name) {
    if (info->priv->run_layout) g_free(info->priv->run_layout);
    info->priv->run_layout = g_strdup(layout_name);
    GError *error          = NULL;
    if (!g_app_info_launch(info->priv->appinfo, NULL, info->priv->launch_content, &error)) {
        g_critical("Launch %s app fail - %s", g_app_info_get_id(info->priv->appinfo), error ? error->message : "unknown");
    }
    if (info->priv->proxy) {
    }
}

static void plugin_id_is_changed(GuiAppGapp *gapp, GParamSpec *pspec, GlAppInfo *info) {
    gtk_socket_add_id(info->priv->socket, (Window)gui_app_gapp_get_plug_id(gapp));
}

static void gapp_proxy_finish_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GlAppInfo *info   = GL_APP_INFO(user_data);
    GError *   error  = NULL;
    info->priv->proxy = gui_app_gapp_proxy_new_finish(res, &error);
    if (info->priv->proxy != NULL) {
        if (gui_app_gapp_get_plug_id(info->priv->proxy) > 0) {
            gtk_socket_add_id(info->priv->socket, (Window)gui_app_gapp_get_plug_id(info->priv->proxy));
        }
        g_signal_connect(info->priv->proxy, "notify::plug-id", G_CALLBACK(plugin_id_is_changed), info);
    }
}

static void on_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data) {
    GlAppInfo *info = GL_APP_INFO(user_data);
    gui_app_gapp_proxy_new(connection, G_DBUS_PROXY_FLAGS_NONE, info->priv->bus_id, "/tera/gapp", NULL, gapp_proxy_finish_callback, info);

    /* do stuff with proxy */
}

static void on_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    GlAppInfo *info = GL_APP_INFO(user_data);
    gtk_widget_show(GTK_WIDGET(info->priv->waite_box));
    gtk_spinner_start(GTK_SPINNER(info->priv->spinner));
    gtk_widget_hide(GTK_WIDGET(info->priv->socket));
    if (info->priv->proxy) g_object_unref(info->priv->proxy);
    info->priv->proxy = NULL;
}

static gchar *app_info_get_dbus_id(GAppInfo *info) {
    gchar *      dbus_id = NULL;
    const gchar *id      = g_app_info_get_id(info);
    gchar *      rstr    = g_strrstr(id, ".desktop");
    GString *    string  = g_string_new("");
    g_string_insert_len(string, 0, id, rstr - id);
    dbus_id = string->str;
    g_string_free(string, FALSE);
    return dbus_id;
}

static void gl_app_info_constructed(GObject *object) {
    GlAppInfo *info = GL_APP_INFO(object);
    if (info->priv->appinfo && G_DESKTOP_APP_INFO(info->priv->appinfo)) {
        const gchar *filename = g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(info->priv->appinfo));
        if (info->priv->key_file) g_key_file_free(info->priv->key_file);
        info->priv->key_file = g_key_file_new();
        g_key_file_load_from_file(info->priv->key_file, filename, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
    }
    info->priv->box       = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    info->priv->waite_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *label      = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(info->priv->waite_box), label, TRUE, TRUE, 0);
    gtk_widget_show(label);
    info->priv->spinner = gtk_spinner_new();
    gtk_widget_set_size_request(info->priv->spinner, 100, 100);
    gtk_box_pack_start(GTK_BOX(info->priv->waite_box), info->priv->spinner, TRUE, TRUE, 0);
    gtk_widget_show(info->priv->spinner);
    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(info->priv->waite_box), label, TRUE, TRUE, 0);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(info->priv->box), info->priv->waite_box, TRUE, TRUE, 0);
    info->priv->socket = GTK_SOCKET(gtk_socket_new());
    gtk_box_pack_start(GTK_BOX(info->priv->box), GTK_WIDGET(info->priv->socket), TRUE, TRUE, 0);
    gtk_widget_set_no_show_all(GTK_WIDGET(info->priv->waite_box), TRUE);
    gtk_widget_set_no_show_all(GTK_WIDGET(info->priv->socket), TRUE);
    gtk_widget_show(info->priv->waite_box);
    gl_desktop_local_add_app(info->priv->box, g_app_info_get_id(info->priv->appinfo));
    info->priv->bus_id = app_info_get_dbus_id(info->priv->appinfo);
    g_bus_watch_name(G_BUS_TYPE_SESSION, info->priv->bus_id, G_BUS_NAME_WATCHER_FLAGS_AUTO_START, on_name_appeared, on_name_vanished, info, NULL);

    g_signal_connect(info->priv->socket, "plug-added", G_CALLBACK(gl_app_info_plugin_added), info);
    g_signal_connect(info->priv->socket, "plug-removed", G_CALLBACK(gl_app_info_plugin_removed), info);
    // g_bus_watch_name(G_BUS_TYPE_SESSION,)

    /*g_debug("ID=%s",g_app_info_get_id (info->priv->appinfo));
    g_debug("NAME=%s",g_app_info_get_name (info->priv->appinfo));
    g_debug("DISPLAYNAME=%s",g_app_info_get_display_name (info->priv->appinfo));
    g_debug("DISCRIPTION=%s",g_app_info_get_description (info->priv->appinfo));
    g_debug("EXECUTABLE=%s",g_app_info_get_executable (info->priv->appinfo));
    g_debug("COMMANDLINE=%s",g_app_info_get_commandline (info->priv->appinfo));*/

    /* user_data */
    /* Export the object (@manager takes its own reference to @object) */
}

static void gl_app_info_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    ////TEST:
    g_return_if_fail(GL_IS_APP_INFO(object));
    GlAppInfo *info = GL_APP_INFO(object);
    switch (prop_id) {
    case GL_APP_INFO_APPLICATION:
        info->priv->appinfo = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_app_info_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Get (GL_MANAGER) property \n");
    g_return_if_fail(GL_IS_APP_INFO(object));
    GlAppInfo *info = GL_APP_INFO(object);
    switch (prop_id) {
    case GL_APP_INFO_APPLICATION:
        g_value_set_object(value, info->priv->appinfo);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_app_info_class_init(GlAppInfoClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize     = gl_app_info_finalize;
    object_class->constructed  = gl_app_info_constructed;
    object_class->set_property = gl_app_info_set_property;
    object_class->get_property = gl_app_info_get_property;

    g_object_class_install_property(object_class, GL_APP_INFO_APPLICATION,
                                    g_param_spec_object("app-info", "Desktop application file ", "Desktop application file ", G_TYPE_DESKTOP_APP_INFO,
                                                        G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

gboolean gl_app_info_stop(GlAppInfo *info) {
    g_return_val_if_fail(info != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_INFO(info), FALSE);
    // g_app_info_delete(info->priv->appinfo);
    return TRUE;
}

gboolean gl_app_info_start(GlAppInfo *info) {
    g_return_val_if_fail(info != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_INFO(info), FALSE);
    launch_application_real(info, "main");
    return TRUE;
}

gboolean gl_app_info_pause(GlAppInfo *info) {
    g_return_val_if_fail(info != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_INFO(info), FALSE);
    return TRUE;
}

GAppInfo *gl_app_info_app_info(GlAppInfo *info) {
    g_return_val_if_fail(info != NULL, NULL);
    g_return_val_if_fail(GL_IS_APP_INFO(info), NULL);
    return info->priv->appinfo;
}

const gchar *gl_app_info_get_id(GlAppInfo *info) {
    g_return_val_if_fail(info != NULL, NULL);
    g_return_val_if_fail(GL_IS_APP_INFO(info), NULL);
    return g_app_info_get_id(info->priv->appinfo);
}

/** @} */
