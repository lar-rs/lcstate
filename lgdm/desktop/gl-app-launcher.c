/*
 * @ingroup GlAppLauncher
 * @{
 * @file  gl-launcher-action.c	LGDM launcher application launcher
 * @brief LGDM launcher application launcher.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-app-launcher.h"
#include "gl-application-object.h"
#include "gl-desktop.h"
#include "lgdm-app-generated-code.h"
#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <mkt-utils.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

enum { GL_APP_LAUNCHER_NONE, GL_APP_LAUNCHER_LDM_APPLICATION, GL_APP_GSETTINGS_APPLICATION };

// static GlAppLauncher *__gui_process_launcher = NULL;

struct _GlAppLauncherPrivate {
    GKeyFile *         key_file;
    gint               level;
    gboolean           plugin_added;
    gboolean           is_run;
    GtkWidget *        box;
    GtkWidget *        spinner;
    GtkWidget *        waite_box;
    GtkSocket *        socket;
    GAppInfo *         appinfo;
    GAppLaunchContext *launch_content;
    GDBusActionGroup * group;
    GSubprocess *      process;
    gchar *            run_layout;
    guint              signal;
};

enum {
    GL_APP_LAUNCHER_PROP_NULL,
    GL_APP_LAUNCHER_APPLICATION,
    GL_APP_LAUNCHER_RUNNED,

};

enum { GL_APP_LAUNCHER_START, GL_APP_LAUNCHER_ACTIVATE_LDM_APPLICATION, GL_APP_LAUNCHER_ADD_LDM_APPLICATION, GL_APP_LAUNCHER_REMOVE_LDM_APPLICATION, GL_APP_LAUNCHER_LAST_SIGNAL };

static guint gl_app_launcher_signals[GL_APP_LAUNCHER_LAST_SIGNAL] = {0};

G_DEFINE_TYPE_WITH_PRIVATE(GlAppLauncher, gl_app_launcher, LGDM_TYPE_OBJECT_SKELETON);

// GL Application dbus client object manager functions -----------------------

static void gl_app_launcher_init(GlAppLauncher *launcher) {
    g_return_if_fail(launcher != NULL);
    g_return_if_fail(GL_IS_APP_LAUNCHER(launcher));
    launcher->priv             = gl_app_launcher_get_instance_private(launcher);
    launcher->priv->is_run     = FALSE;
    launcher->priv->key_file   = NULL;
    launcher->priv->socket     = NULL;
    launcher->priv->process    = NULL;
    launcher->priv->run_layout = NULL;
    launcher->priv->signal     = 0;

    //  Desktop window -----------------------------------------------------------
}

static void gl_app_launcher_finalize(GObject *object) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(object);
    if (launcher->priv->appinfo) g_object_unref(launcher->priv->appinfo);
    if (launcher->priv->key_file) g_key_file_free(launcher->priv->key_file);
    if (launcher->priv->run_layout) g_free(launcher->priv->run_layout);
    G_OBJECT_CLASS(gl_app_launcher_parent_class)->finalize(object);
}

static gboolean app_launcher_plugin_add(gpointer user_data) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(user_data);
    gtk_widget_show(GTK_WIDGET(launcher->priv->socket));
    gtk_widget_hide(GTK_WIDGET(launcher->priv->waite_box));
    launcher->priv->plugin_added = TRUE;
    return FALSE;
}

static gboolean gl_app_launcher_plugin_added(GtkSocket *socket_, gpointer user_data) {
    // GlAppLauncher *launcher  = GL_APP_LAUNCHER ( user_data);
    g_timeout_add(999, app_launcher_plugin_add, user_data);
    return TRUE;
}

static gboolean gl_app_launcher_plugin_removed(GtkSocket *socket_, gpointer user_data) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(user_data);
    // g_signal_emit(launcher,gl_app_launcher_signals[GL_APP_LAUNCHER_REMOVE_LDM_APPLICATION],0);
    gtk_widget_show(GTK_WIDGET(launcher->priv->waite_box));
    gtk_spinner_start(GTK_SPINNER(launcher->priv->spinner));
    gtk_widget_hide(GTK_WIDGET(launcher->priv->socket));
    LgdmApp *app = lgdm_object_get_app(LGDM_OBJEC(launcher));
    lgdm_app_emit_stopped(app);
    return TRUE;
}

static void gl_app_launcher_change_plugin(GObject *object, GParamSpec *pspec, gpointer user_data) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(user_data);
    guint64        plug_id  = lgdm_app_get_plug_id(LGDM_APP(object));
    if (plug_id > 0) gtk_socket_add_id(launcher->priv->socket, plug_id);
}

/*
static void
gl_app_launcher_test_visible ( GObject *object , GParamSpec *pspec , gpointer
user_data )
{
        GlAppLauncher *launcher  = GL_APP_LAUNCHER ( user_data);
}
*/
/*
static void
gl_app_launcher_launched_callback (GAppLaunchContext *context,
               GAppInfo          *info,
               GVariant          *platform_data,
                           GlAppLauncher     *launcher)
{


}

static void
gl_app_launcher_launch_failed_callback (GAppLaunchContext *context,
               gchar             *startup_notify_id,
               gpointer           user_data)
{
        g_debug("LAUNCH FAILED");
}
*/

static void gl_app_launcher_waite_process(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(user_data);
    LgdmApp *      app      = lgdm_object_get_app(LGDM_OBJECT(launcher));
    g_object_unref(source_object);
    launcher->priv->process = NULL;
    lgdm_app_set_started(lgdm_object_get_app(LGDM_OBJECT(launcher)), FALSE);
    lgdm_app_emit_stopped(app);
}

static void gl_app_layuncher_start_application_real(GlAppLauncher *launcher, const gchar *layout_name) {
    if (launcher->priv->process == NULL) {
        if (launcher->priv->run_layout) g_free(launcher->priv->run_layout);
        launcher->priv->run_layout = g_strdup(layout_name);
        g_object_ref(launcher->priv->socket);
        gtk_spinner_start(GTK_SPINNER(launcher->priv->spinner));
        g_signal_connect(launcher->priv->socket, "plug-added", G_CALLBACK(gl_app_launcher_plugin_added), launcher);
        g_signal_connect(launcher->priv->socket, "plug-removed", G_CALLBACK(gl_app_launcher_plugin_removed), launcher);
        launcher->priv->process = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, NULL, g_app_info_get_executable(launcher->priv->appinfo), NULL);
        g_subprocess_wait_async(launcher->priv->process, NULL, gl_app_launcher_waite_process, launcher);
        lgdm_app_emit_start(lgdm_object_get_app(LGDM_OBJECT(launcher)), launcher->priv->run_layout);
    }
}

static void gl_app_launcher_start_application_callback(LgdmApp *dapp, GDBusMethodInvocation *invocation, const gchar *layout_name, gpointer user_data) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(user_data);
    gl_app_layuncher_start_application_real(launcher, layout_name);
}

static void gl_app_launcher_constructed(GObject *object) {
    GlAppLauncher *launcher = GL_APP_LAUNCHER(object);
    if (launcher->priv->appinfo && G_DESKTOP_APP_INFO(launcher->priv->appinfo)) {
        const gchar *filename = g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(launcher->priv->appinfo));
        if (launcher->priv->key_file) g_key_file_free(launcher->priv->key_file);
        launcher->priv->key_file = g_key_file_new();
        g_key_file_load_from_file(launcher->priv->key_file, filename, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
    }
    launcher->priv->box       = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    launcher->priv->waite_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *label          = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(launcher->priv->waite_box), label, TRUE, TRUE, 0);
    gtk_widget_show(label);
    launcher->priv->spinner = gtk_spinner_new();
    gtk_widget_set_size_request(launcher->priv->spinner, 100, 100);
    gtk_box_pack_start(GTK_BOX(launcher->priv->waite_box), launcher->priv->spinner, TRUE, TRUE, 0);
    gtk_widget_show(launcher->priv->spinner);
    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(launcher->priv->waite_box), label, TRUE, TRUE, 0);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(launcher->priv->box), launcher->priv->waite_box, TRUE, TRUE, 0);
    launcher->priv->socket = GTK_SOCKET(gtk_socket_new());
    gtk_box_pack_start(GTK_BOX(launcher->priv->box), GTK_WIDGET(launcher->priv->socket), TRUE, TRUE, 0);
    gtk_widget_set_no_show_all(GTK_WIDGET(launcher->priv->waite_box), TRUE);
    gtk_widget_set_no_show_all(GTK_WIDGET(launcher->priv->socket), TRUE);
    gtk_widget_show(launcher->priv->waite_box);
    gl_desktop_local_add_app(launcher->priv->box, g_app_info_get_id(launcher->priv->appinfo));

    LgdmApp *dapp = NULL;
    dapp          = lgdm_app_skeleton_new();
    lgdm_object_skeleton_set_app(LGDM_OBJECT_SKELETON(launcher), dapp);
    lgdm_app_set_app_id(dapp, g_app_info_get_id(launcher->priv->appinfo));
    g_signal_connect(dapp, "notify::plug-id", G_CALLBACK(gl_app_launcher_change_plugin), launcher);
    g_signal_connect(dapp, "handle-start-layout", G_CALLBACK(gl_app_launcher_start_application_callback), launcher);
    g_object_bind_property(launcher->priv->box, "visible", dapp, "activated", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    // g_signal_connect(launcher->priv->box,"notify::visible",G_CALLBACK(gl_app_launcher_test_visible),launcher);
    g_object_unref(dapp);

    if (g_key_file_has_key(launcher->priv->key_file, "Desktop Entry", "Autostart", NULL)) {
        gboolean is_autostart = g_key_file_get_boolean(launcher->priv->key_file, "Desktop Entry", "Autostart", NULL);
        if (is_autostart) {
            g_object_ref(launcher->priv->socket);
            gtk_spinner_start(GTK_SPINNER(launcher->priv->spinner));
            /*launcher->priv->launch_content = g_app_launch_context_new ();
                                    g_signal_connect(launcher->priv->launch_content,"launched",G_CALLBACK(gl_app_launcher_launched_callback),launcher);
                                    g_signal_connect(launcher->priv->launch_content,"launch-failed",G_CALLBACK(gl_app_launcher_launch_failed_callback),launcher);*/
            g_signal_connect(launcher->priv->socket, "plug-added", G_CALLBACK(gl_app_launcher_plugin_added), launcher);
            g_signal_connect(launcher->priv->socket, "plug-removed", G_CALLBACK(gl_app_launcher_plugin_removed), launcher);
            launcher->priv->process = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, NULL, g_app_info_get_executable(launcher->priv->appinfo), NULL);
            g_subprocess_wait_async(launcher->priv->process, NULL, gl_app_launcher_waite_process, launcher);
        }
    }

    /*g_debug("ID=%s",g_app_info_get_id (launcher->priv->appinfo));
    g_debug("NAME=%s",g_app_info_get_name (launcher->priv->appinfo));
    g_debug("DISPLAYNAME=%s",g_app_info_get_display_name
    (launcher->priv->appinfo));
    g_debug("DISCRIPTION=%s",g_app_info_get_description
    (launcher->priv->appinfo));
    g_debug("EXECUTABLE=%s",g_app_info_get_executable (launcher->priv->appinfo));
    g_debug("COMMANDLINE=%s",g_app_info_get_commandline
    (launcher->priv->appinfo));*/

    /* user_data */
    /* Export the object (@manager takes its own reference to @object) */
}

static void gl_app_launcher_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    ////TEST:
    g_return_if_fail(GL_IS_APP_LAUNCHER(object));
    GlAppLauncher *launcher = GL_APP_LAUNCHER(object);
    switch (prop_id) {
        case GL_APP_LAUNCHER_APPLICATION:
            launcher->priv->appinfo = g_value_dup_object(value);
            break;
        case GL_APP_LAUNCHER_RUNNED:
            launcher->priv->is_run = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gl_app_launcher_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Get (GL_MANAGER) property \n");
    g_return_if_fail(GL_IS_APP_LAUNCHER(object));
    GlAppLauncher *launcher = GL_APP_LAUNCHER(object);
    switch (prop_id) {
        case GL_APP_LAUNCHER_RUNNED:
            g_value_set_boolean(value, launcher->priv->is_run);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gl_app_launcher_class_init(GlAppLauncherClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize     = gl_app_launcher_finalize;
    object_class->constructed  = gl_app_launcher_constructed;
    object_class->set_property = gl_app_launcher_set_property;
    object_class->get_property = gl_app_launcher_get_property;

    g_object_class_install_property(object_class, GL_APP_LAUNCHER_RUNNED,
                                    g_param_spec_boolean("runned", "Desktop action button name", "Desktop action button name", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE));
    g_object_class_install_property(object_class, GL_APP_LAUNCHER_APPLICATION,
                                    g_param_spec_object("app-info", "Desktop application file ", "Desktop application file ", G_TYPE_DESKTOP_APP_INFO, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(object_class, GL_APP_LAUNCHER_RUNNED,
                                    g_param_spec_boolean("dependencies", "Desktop action button name", "Desktop action button name", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE));
}

gboolean gl_app_launcher_stop(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), FALSE);
    if (launcher->priv->process) {
        g_subprocess_send_signal(launcher->priv->process, 9);
    }
    // g_app_info_delete(launcher->priv->appinfo);
    return TRUE;
}

gboolean gl_app_launcher_start(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), FALSE);
    gl_app_layuncher_start_application_real(launcher, "main");
    /*if(gtk_socket_get_plug_window(launcher->priv->socket))
    {

    }
    else
    {
            if(!g_app_info_launch(launcher->priv->appinfo,NULL,launcher->priv->launch_content,&error))
            {
                    g_message("Start app error : %s",error->message);
            }
    }*/
    return TRUE;
}
gboolean gl_app_launcher_is_autostart(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), FALSE);
    gboolean is_autostart = g_key_file_get_boolean(launcher->priv->key_file, "Desktop Entry", "Autostart", NULL);
    return is_autostart;
}

gboolean gl_app_launcher_pause(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, FALSE);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), FALSE);
    return TRUE;
}

GAppInfo *gl_app_launcher_app_info(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, NULL);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), NULL);
    return launcher->priv->appinfo;
}

gint gl_app_launcher_level(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, 4);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), 4);
    return launcher->priv->level;
}

const gchar *gl_app_launcher_get_id(GlAppLauncher *launcher) {
    g_return_val_if_fail(launcher != NULL, NULL);
    g_return_val_if_fail(GL_IS_APP_LAUNCHER(launcher), NULL);
    return g_app_info_get_id(launcher->priv->appinfo);
}

/** @} */
