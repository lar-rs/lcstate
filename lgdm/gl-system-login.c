/*
 * @ingroup GlSystemLogin
 * @{
 * @file  gl-desktop-place.c	generated object file
 * @brief generated object file
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-system-login.h"

#include <mktbus.h>
#include <mktlib.h>

#include "lgdm-app-launcher.h"
#include "lgdm-desktop.h"
#include "gl-select-device.h"
#include "gl-service-log.h"
#include "row-candaemon.h"
#include "row-fake-user.h"
#include "row-pcdaemon.h"
#include "row-sensors.h"
#include "row-service.h"
#include "row-user.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

// static GlSystemLogin *__gui_process_desktop = NULL;

struct _GlSystemLoginPrivate {
    GtkScrolledWindow *user_scrolled;
    GtkListBox *       user_list;
    GtkBox *           restart_block;
    GtkInfoBar *       device_fail_info;
    GtkSpinner *       loading_spinner;
    GtkImage *         critical_bild;
    GtkImage *         logo_image;

    GtkLabel *        restart_session;
    GtkLabel *        restart_label;
    GtkLabel *        copyright;
    TeraClientObject *can_client;
    TeraClientObject *pc_client;
    TeraClientObject *device_client;
    GtkWidget *       service_log;
    GtkWidget *       select_device_win;

    gboolean is_booted;
    gboolean is_guard;
    gboolean critical;
    gboolean service_critical;
    gboolean session_critical;
    gboolean first_boot;
    guint    restart_tag;
    GTimer * warte_timer;
};

enum { GL_SYSTEM_LOGIN_PROP_NULL, GL_SYSTEM_LOGIN_PROP_SERVICE_BOOTED, GL_SYSTEM_LOGIN_PROP_SERVICE_CRITICAL };

enum { GL_SYSTEM_LOGIN_LAST_SIGNAL };

// static guint gl_system_login_signals[GL_SYSTEM_LOGIN_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlSystemLogin, gl_system_login, GTK_TYPE_BOX);

static void restart_clicked_cb(GlSystemLogin *login, GtkButton *button) {
    if (login->priv->restart_tag) g_source_remove(login->priv->restart_tag);
    login->priv->restart_tag = 0;
    if (mkt_pc_manager_client_get_device()) larpc_device_call_reboot(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
}

static void rsession_clicked_cb(GlSystemLogin *login, GtkButton *button) {
    if (login->priv->restart_tag) g_source_remove(login->priv->restart_tag);
    login->priv->restart_tag = 0;
    if (mkt_pc_manager_client_get_device()) larpc_device_call_restart(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
}

static gboolean waite_restart_session_callback(gpointer data) {
    GlSystemLogin *login   = GL_SYSTEM_LOGIN(data);
    gdouble        elapsed = g_timer_elapsed(login->priv->warte_timer, NULL);

    if (elapsed > 30.0) {
        login->priv->restart_tag = 0;
        if (mkt_pc_manager_client_get_device()) larpc_device_call_restart(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);

        return FALSE;
    }

    gchar *sec_str = g_strdup_printf("new session(%.0f sec)", 30.0 - elapsed);
    gtk_label_set_text(login->priv->restart_session, sec_str);
    g_free(sec_str);
    return TRUE;
}

static gboolean waite_restart_system_callback(gpointer data) {
    GlSystemLogin *login   = GL_SYSTEM_LOGIN(data);
    gdouble        elapsed = g_timer_elapsed(login->priv->warte_timer, NULL);

    if (elapsed > 30.0) {
        if (mkt_pc_manager_client_get_device()) larpc_device_call_reboot(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
        return FALSE;
    }

    gchar *sec_str = g_strdup_printf("restart (%.0f sec)", 30.0 - elapsed);
    gtk_label_set_text(login->priv->restart_label, sec_str);
    g_free(sec_str);
    return TRUE;
}

static void critical_processing(GlSystemLogin *login) {
    g_timer_reset(login->priv->warte_timer);
    if (login->priv->restart_tag) g_source_remove(login->priv->restart_tag);
    if (login->priv->service_critical)
        login->priv->restart_tag = g_timeout_add(100, waite_restart_system_callback, login);
    else if (login->priv->session_critical)
        login->priv->restart_tag = g_timeout_add(100, waite_restart_session_callback, login);
}

static void systemlog_clicked_cb(GlSystemLogin *login, GtkButton *button) {
    gtk_widget_set_size_request(GTK_WIDGET(login->priv->service_log), 800, 600);
    gtk_window_resize(GTK_WINDOW(login->priv->service_log), 800, 600);
    gtk_window_move(GTK_WINDOW(login->priv->service_log), 0, 0);
    gtk_widget_set_visible(login->priv->service_log, TRUE);
    if (login->priv->restart_tag) g_source_remove(login->priv->restart_tag);
    login->priv->restart_tag = 0;
}

static void select_device_clicked_cb(GlSystemLogin *login, GtkButton *button) {
    if (login->priv->select_device_win) {
        LarpcApt *apt = mkt_pc_manager_client_get_apt();
        if (apt) {
            larpc_apt_call_update(apt, NULL, NULL, NULL);
        }
        gtk_widget_set_size_request(GTK_WIDGET(login->priv->select_device_win), 800, 600);
        gtk_window_resize(GTK_WINDOW(login->priv->select_device_win), 800, 600);
        gtk_window_move(GTK_WINDOW(login->priv->select_device_win), 0, 0);
        gtk_widget_set_visible(login->priv->select_device_win, TRUE);
        if (login->priv->restart_tag) g_source_remove(login->priv->restart_tag);
        login->priv->restart_tag = 0;
    }
}

static void gl_system_login_init(GlSystemLogin *gl_system_login) {
    g_return_if_fail(gl_system_login != NULL);
    g_return_if_fail(GL_IS_SYSTEM_LOGIN(gl_system_login));
    gl_system_login->priv            = gl_system_login_get_instance_private(gl_system_login);
    gl_system_login->priv->is_booted = FALSE;
    gtk_widget_init_template(GTK_WIDGET(gl_system_login));
}

static gboolean system_login_move_correct(gpointer user_data) {
    GlSystemLogin *login = GL_SYSTEM_LOGIN(user_data);
    gtk_widget_set_size_request(GTK_WIDGET(login), 800, 600);
    gtk_window_resize(GTK_WINDOW(login), 800, 600);
    gtk_window_move(GTK_WINDOW(login), 0, 0);
    return FALSE;
}

static void gl_system_login_check_service(GlSystemLogin *login) {
    GList *  list     = tera_clients_list();
    GList *  lw       = NULL;
    gboolean critical = FALSE;
    for (lw = list; lw != NULL; lw = lw->next) {
        if (tera_client_is_critical(TERA_CLIENT_OBJECT(lw->data))) {
            if ((gpointer)login->priv->can_client == (gpointer)lw->data)
                login->priv->service_critical = TRUE;
            else if ((gpointer)login->priv->pc_client == (gpointer)lw->data)
                login->priv->service_critical = TRUE;
            else
                login->priv->session_critical = TRUE;
            critical                          = TRUE;
        }
    }
    g_object_set(login, "is-booted", !critical, NULL);
    g_object_set(login, "critical", critical, NULL);
    if (critical) critical_processing(login);
}

static void security_system_login_callback(SecurityDevice *device, GlSystemLogin *login) {
    gl_system_login_check_service(login);
    if (login->priv->is_booted) {
        gtk_widget_hide(GTK_WIDGET(login));
    }
}

static void security_system_logout_callback(SecurityDevice *device, GlSystemLogin *login) {
    gl_desktop_local_show_app("com.lar.ldm.desktop");
    gtk_widget_show(GTK_WIDGET(login));
    gtk_widget_set_size_request(GTK_WIDGET(login), 800, 600);
    gtk_window_resize(GTK_WINDOW(login), 800, 600);
    gtk_window_move(GTK_WINDOW(login), 0, 0);
    GList *launchers = gl_desktop_launcher();
    GList *l         = NULL;
    for (l = launchers; l != NULL; l = l->next) {
        if (!gl_app_launcher_is_autostart(GL_APP_LAUNCHER(l->data))) {
            gl_app_launcher_stop(GL_APP_LAUNCHER(l->data));
        }
    }
}

static gint users_sort_on_level(gconstpointer a, gconstpointer b) {
    g_return_val_if_fail(USERS_IS_OBJECT(a), 0);
    g_return_val_if_fail(USERS_IS_OBJECT(b), 0);
    if (users_user_get_level(users_object_get_user(USERS_OBJECT(a))) == users_user_get_level(users_object_get_user(USERS_OBJECT(b)))) return 0;
    if (users_user_get_level(users_object_get_user(USERS_OBJECT(a))) < users_user_get_level(users_object_get_user(USERS_OBJECT(b)))) return -1;
    return 1;
}

static void gl_system_login_init_users(GlSystemLogin *login) {
    GDBusObjectManager *user_manager = tera_security_client_get_users_manager();
    GList *             users        = g_dbus_object_manager_get_objects(user_manager);
    users                            = g_list_sort(users, users_sort_on_level);
    GList *      lu                  = NULL;
    UsersObject *object              = NULL;
    for (lu = users; lu != NULL; lu = lu->next) {
        GtkWidget *row = GTK_WIDGET(g_object_new(ROW_TYPE_USER, "user-object", lu->data, NULL));
        gtk_widget_set_size_request(row, -1, 50);
        gtk_container_add(GTK_CONTAINER(login->priv->user_list), row);
        gtk_widget_show_all(row);
        g_object_bind_property(users_object_get_user(USERS_OBJECT(lu->data)), "activated", row, "sensitive", G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT);
        if (object == NULL) object = lu->data;
    }
    GtkWidget *row = GTK_WIDGET(g_object_new(ROW_TYPE_FAKE_USER, "fake-user", object, "fake-user-name", _("Expert Level"), "fake-user-level", 3, NULL));
    gtk_widget_set_size_request(row, -1, 50);
    gtk_container_add(GTK_CONTAINER(login->priv->user_list), row);
    gtk_widget_show_all(row);
    g_object_bind_property(TERA_GUARD(), "level3", row, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    row = GTK_WIDGET(g_object_new(ROW_TYPE_FAKE_USER, "fake-user", object, "fake-user-name", _("Service"), "fake-user-level", 4, NULL));
    gtk_widget_set_size_request(row, -1, 50);
    gtk_container_add(GTK_CONTAINER(login->priv->user_list), row);
    gtk_widget_show_all(row);
    g_object_bind_property(TERA_GUARD(), "level4", row, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    row = GTK_WIDGET(g_object_new(ROW_TYPE_FAKE_USER, "fake-user", object, "fake-user-name", _("Factory Settings"), "fake-user-level", 5, NULL));
    gtk_widget_set_size_request(row, -1, 50);
    gtk_container_add(GTK_CONTAINER(login->priv->user_list), row);
    gtk_widget_show_all(row);
    g_object_bind_property(TERA_GUARD(), "level5", row, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    if (users != NULL) g_list_free(users);
}

static void service_quit_unexpectedly(TeraClientObject *client, gboolean done, GlSystemLogin *login) {
    gl_system_login_check_service(login);
}

static void service_all_ready(TeraClientObject *client, gboolean done, GlSystemLogin *login) {
    login->priv->first_boot = FALSE;
    gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
    if (login->priv->is_guard) gl_system_login_init_users(login);
    if (!done && tera_client_proxy(client) == NULL) {
        g_object_set(login, "is-booted", FALSE, NULL);
        g_object_set(login, "critical", TRUE, NULL);
        login->priv->session_critical = TRUE;
        critical_processing(login);
    } else {
        g_signal_connect(client, "notify::critical-error", G_CALLBACK(service_quit_unexpectedly), login);
        gl_system_login_check_service(login);
    }
}
static void service_device_lost(TeraClientObject *client, GlSystemLogin *login) {
    if (login->priv->first_boot) {
        gtk_widget_show(GTK_WIDGET(login->priv->device_fail_info));
        gtk_widget_hide(GTK_WIDGET(login->priv->logo_image));
    }
    g_object_set(login, "is-booted", FALSE, NULL);
    g_object_set(login, "critical", TRUE, NULL);
    login->priv->session_critical = TRUE;
    gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
    g_critical(_("Measurement service is lost - %s"), tera_client_critical_message(client));
    critical_processing(login);
}

static void security_service_ready(TeraClientObject *client, gboolean done, GlSystemLogin *login) {
    g_signal_connect(client, "notify::critical-error", G_CALLBACK(service_quit_unexpectedly), login);
    if (!tera_client_is_critical(client)) {
        if (tera_client_is_done(client)) {
            login->priv->is_guard = TRUE;
            g_signal_connect(TERA_GUARD(), "system-login", G_CALLBACK(security_system_login_callback), login);
            g_signal_connect(TERA_GUARD(), "system-logout", G_CALLBACK(security_system_logout_callback), login);
        }
        TeraClientObject *client_object = TERA_CLIENT_OBJECT(ultra_control_client_new_default());
        login->priv->first_boot         = TRUE;
        g_object_set(client_object, "client-timeout", 50.0, NULL);
        g_signal_connect(client_object, "client-done", G_CALLBACK(service_all_ready), login);
        g_signal_connect(client_object, "client-lost", G_CALLBACK(service_device_lost), login);
        tera_client_run(client_object);
    } else {
        gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
        g_object_set(login, "critical", TRUE, NULL);
        mkt_log_error_message(_("Security service is fail - %s"), tera_client_critical_message(client));
        login->priv->session_critical = TRUE;
        critical_processing(login);
    }
}

static void security_service_lost(TeraClientObject *client, GlSystemLogin *login) {
    // mkt_log_error_message(_("CAN service is lost - %s"),tera_client_critical_message(client));
    g_critical(_("Security service is lost - %s"), tera_client_critical_message(client));

    g_object_set(login, "is-booted", FALSE, NULL);
    g_object_set(login, "critical", TRUE, NULL);
    login->priv->session_critical = TRUE;
    gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
    critical_processing(login);
}

static void can_service_ready(TeraClientObject *client, gboolean done, GlSystemLogin *login) {
    g_signal_connect(client, "notify::critical-error", G_CALLBACK(service_quit_unexpectedly), login);
    if (!tera_client_is_critical(client)) {
        TeraClientObject *client_object = tera_security_manager_client_new();
        g_signal_connect(client_object, "client-done", G_CALLBACK(security_service_ready), login);
        g_signal_connect(client_object, "client-lost", G_CALLBACK(security_service_lost), login);
        tera_client_run(client_object);
    } else {
        gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
        g_object_set(login, "critical", TRUE, NULL);
        login->priv->service_critical = TRUE;
        mkt_log_error_message(_("CAN service - %s"), tera_client_critical_message(client));
        critical_processing(login);
    }
}

static void can_sevice_lost(TeraClientObject *client, GlSystemLogin *login) {
    g_signal_handlers_disconnect_by_func(login->priv->can_client, can_sevice_lost, login);
    g_critical(_("CAN service is lost - %s"), tera_client_critical_message(client));
    g_object_set(login, "critical", TRUE, NULL);
    gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
    login->priv->service_critical = TRUE;
    critical_processing(login);
}

static void pc_sevice_ready(TeraClientObject *client, gboolean done, GlSystemLogin *login) {
    g_signal_connect(client, "notify::critical-error", G_CALLBACK(service_quit_unexpectedly), login);
    if (!tera_client_is_critical(client)) {
        login->priv->select_device_win = GTK_WIDGET(g_object_new(GL_TYPE_SELECT_DEVICE, NULL));
        login->priv->can_client        = mkt_can_manager_client_new();
        g_signal_connect(login->priv->can_client, "client-done", G_CALLBACK(can_service_ready), login);
        g_signal_connect(login->priv->can_client, "client-lost", G_CALLBACK(can_sevice_lost), login);
        tera_client_run(login->priv->can_client);
    } else {
        g_object_set(login, "critical", TRUE, NULL);
        gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
        login->priv->service_critical = TRUE;
        mkt_log_error_message(_("PC service - %s"), tera_client_critical_message(client));
        critical_processing(login);
    }
}

static void pc_sevice_lost(TeraClientObject *client, GlSystemLogin *login) {
    g_signal_handlers_disconnect_by_func(login->priv->pc_client, pc_sevice_lost, login);
    // mkt_log_error_message(_("PC service is lost %s"), tera_client_critical_message(client));
    g_critical(_("PC service is lost %s"), tera_client_critical_message(client));
    g_object_set(login, "critical", TRUE, NULL);
    gtk_widget_hide(GTK_WIDGET(login->priv->loading_spinner));
    login->priv->service_critical = TRUE;
    critical_processing(login);
}

static void system_login_visible(GlSystemLogin *login, GParamSpec *pspec, gpointer data) {
    if (gtk_widget_get_visual(GTK_WIDGET(login))) g_timeout_add(20, system_login_move_correct, login);
}

static void gl_system_login_init_service(GlSystemLogin *login) {
    login->priv->pc_client = mkt_pc_manager_client_new();
    g_signal_connect(login->priv->pc_client, "client-done", G_CALLBACK(pc_sevice_ready), login);
    g_signal_connect(login->priv->pc_client, "client-lost", G_CALLBACK(pc_sevice_lost), login);
    tera_client_run(login->priv->pc_client);
    gtk_widget_show(GTK_WIDGET(login->priv->loading_spinner));
}

static void gl_system_login_constructed(GObject *object) {
    GlSystemLogin *login     = GL_SYSTEM_LOGIN(object);
    login->priv->warte_timer = g_timer_new();
    g_timer_start(login->priv->warte_timer);
    g_object_bind_property(login, "is-booted", login->priv->user_list, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(login, "is-booted", login->priv->user_scrolled, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    login->priv->is_guard = FALSE;
    g_object_bind_property(login, "critical", login->priv->restart_block, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(login, "critical", login->priv->critical_bild, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(login, "critical", login->priv->copyright, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

    gl_system_login_init_service(login);
    g_signal_connect(login, "notify::visible", G_CALLBACK(system_login_visible), NULL);
    //	g_signal_connect(login,"notify::is-booted",G_CALLBACK(system_login_changed_is_booted),NULL);

    g_timeout_add(20, system_login_move_correct, login);

    login->priv->service_log = GTK_WIDGET(g_object_new(GL_TYPE_SERVICE_LOG, NULL));

    if (G_OBJECT_CLASS(gl_system_login_parent_class)->constructed) G_OBJECT_CLASS(gl_system_login_parent_class)->constructed(object);
}

static void gl_system_login_finalize(GObject *object) {
    GlSystemLogin *login = GL_SYSTEM_LOGIN(object);
    g_timer_destroy(login->priv->warte_timer);
    G_OBJECT_CLASS(gl_system_login_parent_class)->finalize(object);
}

static void gl_system_login_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_SYSTEM_LOGIN(object));
    GlSystemLogin *gl_system_login = GL_SYSTEM_LOGIN(object);
    switch (prop_id) {
    case GL_SYSTEM_LOGIN_PROP_SERVICE_BOOTED:
        gl_system_login->priv->is_booted = g_value_get_boolean(value);
        break;
    case GL_SYSTEM_LOGIN_PROP_SERVICE_CRITICAL:
        gl_system_login->priv->critical = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_system_login_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_SYSTEM_LOGIN(object));
    GlSystemLogin *gl_system_login = GL_SYSTEM_LOGIN(object);
    switch (prop_id) {
    case GL_SYSTEM_LOGIN_PROP_SERVICE_BOOTED:
        g_value_set_boolean(value, gl_system_login->priv->is_booted);
        break;
    case GL_SYSTEM_LOGIN_PROP_SERVICE_CRITICAL:
        g_value_set_boolean(value, gl_system_login->priv->critical);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_system_login_class_init(GlSystemLoginClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    // GlLayoutClass        *layout_class     =  GL_LAYOUT_CLASS (klass);
    object_class->finalize     = gl_system_login_finalize;
    object_class->set_property = gl_system_login_set_property;
    object_class->get_property = gl_system_login_get_property;
    object_class->constructed  = gl_system_login_constructed;

    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/ui/layout/user-login.ui");

    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, user_list);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, user_scrolled);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, restart_block);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, loading_spinner);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, critical_bild);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, restart_session);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, restart_label);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, device_fail_info);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, logo_image);
    gtk_widget_class_bind_template_child_private(widget_class, GlSystemLogin, copyright);
	gtk_widget_class_bind_template_child_private (widget_class,  GlSystemLogin, user_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlSystemLogin, user_level);

    gtk_widget_class_bind_template_callback(widget_class, restart_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, rsession_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, systemlog_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, select_device_clicked_cb);

    g_object_class_install_property(
        object_class, GL_SYSTEM_LOGIN_PROP_SERVICE_BOOTED,
        g_param_spec_boolean("is-booted", "System is ready", "System is ready", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property(
        object_class, GL_SYSTEM_LOGIN_PROP_SERVICE_CRITICAL,
        g_param_spec_boolean("critical", "Critical error", "Critical error", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));
}

/** @} */
