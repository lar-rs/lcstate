/*
 * file  gl-desktop.c	LGDM desktop
 * brief LGDM desktop
 *
 *
 * Copyright (C) LAR 2014-2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */
#include <mkt-log.h>
// #include "gl-select-device.h"
// #include "gl-service-log.h"

#include "gl-desktop-action.h"
#include "gl-layout.h"
#include "gl-status-action.h"
#include "lgdm-desktop-place.h"
#include "lgdm-desktop.h"
#include "lgdm-sidebar.h"
#include "lgdm-status.h"

#include <gtk/gtkx.h>
#include <string.h>

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>
#define DESKTOP_MAX_PARAMS 15

static LgdmDesktop *_lgd_local_desktop = NULL;


// static LgdmDesktop *__gui_process_desktop = NULL;

struct _LgdmDesktopPrivate {
    LgdmDesktopPlace *desktop_place;
    // LgdmLogBook               *logbook;
    LgdmSidebar *sidebar;
    LgdmStatus * status;
    GtkBox *     stack_halter;
    GtkBox *     sidebar_place;
    GtkBox *     status_place;
    GtkStack *   desktop_stack;
    GtkStack *   plugs_stack;
    GtkOverlay * desktop;
    GtkOverlay * login;

    // GtkWidget *service_log;
    // GtkWidget *select_device_win;

    GtkScrolledWindow *user_scrolled;
    GtkListBox *       user_list;
    GtkBox *           restart_block;
    GtkInfoBar *       device_fail_info;
    GtkSpinner *       loading_spinner;
    GtkImage *         critical_bild;
    GtkImage *         logo_image;
	GtkLabel            *user_name;
	GtkLabel            *user_level;
    GtkImage *         user_image;

    GtkLabel *restart_session;
    GtkLabel *restart_label;
    GtkLabel *copyright;

    gint       timer_tag;
    // GSettings *desktop_settings;
    // GSettings *status_settings;
    // GSettings *sidebar_settings;
    //  desktop windows ----------------------------
    GSList *last_plugin;
    GList * indicate;
    GList * widgets;

    GTimer *    info_timer;
    GHashTable *objects;
    GHashTable *launchers;

    GtkImage *   open_close_icon;
    GtkRevealer *restart_revaler;
    GtkButton *  show_restart;

    GtkBox *       meldung_box;
    GtkBox *       restart_box;

    gboolean is_booted;
    gboolean is_guard;
    gboolean critical;
    gboolean service_critical;
    gboolean session_critical;
    gboolean first_boot;
    guint    restart_tag;
    GTimer * warte_timer;
};

enum { LGDM_DESKTOP_PROP_NULL, LGDM_DESKTOP_PROP_SERVICE_BOOTED, LGDM_DESKTOP_PROP_SERVICE_CRITICAL };

enum { LGDM_DESKTOP_SEARCH_SIGNAL, LGDM_DESKTOP_LAST_SIGNAL };

// static guint lgdm_desktop_signals[LGDM_DESKTOP_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(LgdmDesktop, lgdm_desktop, GTK_TYPE_WINDOW);

static void restart_clicked_cb(LgdmDesktop *desktop, GtkButton *button) {
    if (desktop->priv->restart_tag) g_source_remove(desktop->priv->restart_tag);
    desktop->priv->restart_tag = 0;
    // if (mkt_pc_manager_client_get_device()){
        // larpc_device_call_reboot(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
    // }
    exit(1);
}

static void rsession_clicked_cb(LgdmDesktop *desktop, GtkButton *button) {
    if (desktop->priv->restart_tag) g_source_remove(desktop->priv->restart_tag);
    desktop->priv->restart_tag = 0;
    // if (mkt_pc_manager_client_get_device()){
        // larpc_device_call_restart(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
    // }
    exit(1);
}

// static void systemlog_clicked_cb (LgdmDesktop *desktop, GtkButton *button) {
    // lgdm_state_login();
// }

static gboolean waite_restart_session_callback(gpointer data) {
    LgdmDesktop *desktop = LGDM_DESKTOP(data);
    gdouble      elapsed = g_timer_elapsed(desktop->priv->warte_timer, NULL);
    if (elapsed > 30.0) {
        // desktop->priv->restart_tag = 0;
        // if (mkt_pc_manager_client_get_device()) {
            // mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "Automatical restart session");
            // larpc_device_call_restart(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
        // } else {
            // mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "Automatical restart session: system fail - please contact service (replase CF-Card)");
        // }
        exit(1);
    }

    gchar *sec_str = g_strdup_printf("new session(%.0f sec)", 30.0 - elapsed);
    gtk_label_set_text(desktop->priv->restart_session, sec_str);
    g_free(sec_str);
    return TRUE;
}

static gboolean waite_restart_system_callback(gpointer data) {
    LgdmDesktop *desktop = LGDM_DESKTOP(data);
    gdouble      elapsed = g_timer_elapsed(desktop->priv->warte_timer, NULL);

    if (elapsed > 30.0) {
        // if (mkt_pc_manager_client_get_device()) {
            // mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "Autoomatical reboot");
            // larpc_device_call_reboot(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
        // } else {
            // mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "Automatical reboot : system fail - please contact service (replase CF-Card)");
        //  }
        exit(1);
    }

    gchar *sec_str = g_strdup_printf("restart (%.0f sec)", 30.0 - elapsed);
    gtk_label_set_text(desktop->priv->restart_label, sec_str);
    g_free(sec_str);
    return TRUE;
}

static void stackShowLogin(LgdmDesktop *desktop) {
    gtk_stack_set_transition_type(desktop->priv->desktop_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_stack_set_visible_child(desktop->priv->desktop_stack, GTK_WIDGET(desktop->priv->login));
}

static void stackShowDesktop(LgdmDesktop *desktop) {
    gtk_stack_set_transition_type(desktop->priv->desktop_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_stack_set_visible_child(desktop->priv->desktop_stack, GTK_WIDGET(desktop->priv->desktop));
}

static void guardLoginCallback(LgdmState *state, LgdmDesktop *desktop) { stackShowDesktop(desktop); }

static void guardLogoutCallback(LgdmState *state, LgdmDesktop *desktop) {
    lgdm_desktop_local_show_app("lar.ams.lgdm.desktop");
    stackShowLogin(desktop);
    GList *launchers = lgdm_desktop_launcher();
    GList *l         = NULL;
    for (l = launchers; l != NULL; l = l->next) {
        if (!lgdm_app_launcher_is_autostart(LGDM_APP_LAUNCHER(l->data))) {
            lgdm_app_launcher_stop(LGDM_APP_LAUNCHER(l->data));
        }
    }
}

static void lgdm_desktop_application_change(LgdmDesktop *desktop, GParamSpec *pspec, gpointer user_data) {}

static void lgdm_desktop_resize_window(LgdmDesktop *desktop) {
    // GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW(desktop));
    // gint display_hight = 600;
    // gint display_width = 800;
    gtk_widget_set_size_request(GTK_WIDGET(desktop), 800, 600);
    gtk_window_resize(GTK_WINDOW(desktop), 800, 600);
    gtk_window_move(GTK_WINDOW(desktop), 0, 0);
}

static void lgdm_desktop_insert_to_table(LgdmDesktop *desktop, GObject *object, const gchar *id) {
    gpointer p = g_hash_table_lookup(desktop->priv->objects, (gconstpointer)id);
    if (p) g_hash_table_remove(desktop->priv->objects, (gconstpointer)id);
    g_hash_table_insert(desktop->priv->objects, (gpointer)id, (gpointer)object);
}

static void desktop_launches_destroy_key(gpointer data) {
    if (data) g_free(data);
}
static void desktop_launches_destroy_value(gpointer data) {
    if (data) g_object_unref(data);
}

static void lgdm_desktop_init(LgdmDesktop *desktop) {
    g_return_if_fail(desktop != NULL);
    g_return_if_fail(LGDM_IS_DESKTOP(desktop));

    desktop->priv                   = lgdm_desktop_get_instance_private(desktop);
    // desktop->priv->desktop_settings = g_settings_new("com.lar.LGDM.Desktop");
    // desktop->priv->status_settings  = g_settings_new("com.lar.LGDM.Status");
    // desktop->priv->sidebar_settings = g_settings_new("com.lar.LGDM.Sidebar");
    desktop->priv->indicate         = NULL;
    desktop->priv->last_plugin      = NULL;
    desktop->priv->objects          = g_hash_table_new(g_str_hash, g_str_equal);
    desktop->priv->launchers        = g_hash_table_new_full(g_str_hash, g_str_equal, desktop_launches_destroy_key, desktop_launches_destroy_value);
    gtk_widget_init_template(GTK_WIDGET(desktop));

    g_signal_connect(desktop, "notify::application", G_CALLBACK(lgdm_desktop_application_change), NULL);

    // g_signal_connect(desktop->priv->status_settings,"changed",G_CALLBACK(lgdm_desktop_status_settings_changed),desktop);
    // g_signal_connect(desktop->priv->sidebar_settings,"changed",G_CALLBACK(lgdm_desktop_sidebar_settings_changed),desktop);

    /*desktop->priv->logbook  = LGDM_LOG_BOOK(g_object_new(LGDM_TYPE_LOG_BOOK,NULL));
gtk_stack_add_named(desktop->priv->plugs_stack,GTK_WIDGET(desktop->priv->logbook),"com.lar.ldm.logbook");
lgdm_desktop_insert_to_table(desktop,G_OBJECT(desktop->priv->logbook),"com.lar.ldm.logbook");*/
    // gtk_widget_show(GTK_WIDGET(desktop->priv->logbook));
    // gtk_stack_set_visible_child_name(desktop->priv->plugs_stack,"com.lar.ldm.desktop");

    /*GtkBuilder *builder =
    gtk_builder_new_from_resource("/lgdm/ui/layout/desktop-actions.ui");
    if(builder)
    {
            //GtkWidget *widget =
    GTK_WIDGET(gtk_builder_get_object(builder,"desktop_scrolled"));
    }*/
    lgdm_desktop_resize_window(desktop);

    // GdkDisplay *      display        = gtk_widget_get_display(GTK_WIDGET(desktop));
    // GdkDeviceManager *device_manager = gdk_display_get_device_manager(display);
    // GList *           devices        = gdk_device_manager_list_devices(device_manager, GDK_DEVICE_TYPE_MASTER);
    //	g_debug("TOUCHSOURCE = %d",GDK_SOURCE_TOUCHSCREEN);
    /*	for(;devices!=NULL;devices=devices->next)
            {
                    if(gdk_x11_device_get_id(GDK_DEVICE(devices->data)) == 14)
       g_object_set(G_OBJECT(devices->data),"input-source",GDK_SOURCE_TOUCHSCREEN,NULL);


            }
            devices =    gdk_device_manager_list_devices
       (device_manager,GDK_DEVICE_TYPE_SLAVE);
            for(;devices!=NULL;devices=devices->next)
            {

            }
            g_debug(
       "----------------------------------------------------------------------------------");
            devices =    gdk_device_manager_list_devices
       (device_manager,GDK_DEVICE_TYPE_FLOATING);
            for(;devices!=NULL;devices=devices->next)
            {
                    g_debug("Device name = %s",gdk_device_get_name
       (GDK_DEVICE(devices->data)));
                    g_debug("Device id =
       %d",gdk_x11_device_get_id(GDK_DEVICE(devices->data)));
                    g_debug("Device source =
       %d",gdk_device_get_source(GDK_DEVICE(devices->data)));
            }*/

    //  Desktop window -----------------------------------------------------------
}

static void desktop_change_power_settings_icon(LgdmDesktop *desktop, const gchar *name) {
    GdkPixbuf *   buf   = NULL;
    GtkIconTheme *theme = gtk_icon_theme_get_default();
    buf                 = gtk_icon_theme_load_icon(theme, name, 24, GTK_ICON_LOOKUP_FORCE_SVG, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(desktop->priv->open_close_icon), buf);
    g_object_unref(buf);
}

static void lgdm_desktop_show_desktop_click_callback(LgdmStatus *sidebar, LgdmDesktop *desktop) {
    lgdm_desktop_local_show_app("lar.ams.lgdm.ui");
    lgdm_state_emit_show_desktop(lgdm_state());
    lgdm_status_set_application_name(lgdm_desktop_local_get_status(), "");
    gtk_window_set_default_size(GTK_WINDOW(lgdm_desktop_local_get()), 800, 600);
    gtk_widget_set_size_request(GTK_WIDGET(lgdm_desktop_local_get()), 800, 600);
}

static void lgdm_desktop_show_status_click_callback(LgdmStatus *sidebar, LgdmDesktop *desktop) {
    // lgdm_desktop_local_show_app("com.lar.ldm.logbook");
    lgdm_state_emit_show_desktop(lgdm_state());
    lgdm_status_set_application_name(lgdm_desktop_local_get_status(), "");
    gtk_window_set_default_size(GTK_WINDOW(lgdm_desktop_local_get()), 800, 600);
    gtk_widget_set_size_request(GTK_WIDGET(lgdm_desktop_local_get()), 800, 600);
}

static void     lgdm_desktop_app_start_app_callback(LgdmApp *app, const gchar *layout, LgdmDesktop *desktop) { lgdm_desktop_local_show_app(lgdm_app_get_app_id(app)); }

static void     lgdm_desktop_app_stopped_app_callback(LgdmApp *app, LgdmDesktop *desktop) {
    lgdm_desktop_local_show_app("lar.ams.lgdm.ui");
    lgdm_state_emit_show_desktop(lgdm_state());
    lgdm_status_set_application_name(lgdm_desktop_local_get_status(), "");
    gtk_window_set_default_size(GTK_WINDOW(lgdm_desktop_local_get()), 800, 600);
    gtk_widget_set_size_request(GTK_WIDGET(lgdm_desktop_local_get()), 800, 600);

}
static gboolean desktop_check_category_gapp(GAppInfo *app, const gchar *category) {
    const gchar *rstr = g_strrstr_len(g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(app)), 128, category);
    return rstr != NULL;
}

static void show_restart_clicked_cb(LgdmDesktop *desktop, GtkButton *button) {
    gtk_widget_set_visible(GTK_WIDGET(desktop->priv->restart_revaler), !gtk_widget_get_visible(GTK_WIDGET(desktop->priv->restart_revaler)));

    if (gtk_widget_get_visible(GTK_WIDGET(desktop->priv->restart_revaler)))
        desktop_change_power_settings_icon(desktop, "arrow-right-01");
    else
        desktop_change_power_settings_icon(desktop, "powerbuttons");
}

static void restart_system_clicked_cb(LgdmDesktop *desktop, GtkButton *button) {
    // mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "System reboot (User action)");
    // if (mkt_pc_manager_client_get_device()) larpc_device_call_reboot(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
    exit(0);
}

static void restart_session_clicked_cb(LgdmDesktop *desktop, GtkButton *button) {
    // mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "Session restart (User action)");
    // if (mkt_pc_manager_client_get_device()) larpc_device_call_restart(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
    exit(0);
}

static gboolean lgdm_desktop_app_create_action_is_new(LgdmDesktop *desktop, GAppInfo *appinfo) {

    if (g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(appinfo))) {
        gchar *          path     = g_strdup_printf("/lgdm/app/%s", g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(appinfo)));
        LgdmAppLauncher *launcher = LGDM_APP_LAUNCHER(g_dbus_object_manager_get_object(G_DBUS_OBJECT_MANAGER(lgdm_state_app_manager()), path));
        if (launcher == NULL) {
            LgdmAppLauncher *launcher = LGDM_APP_LAUNCHER(g_object_new(LGDM_TYPE_APP_LAUNCHER, "app-info", appinfo, "g-object-path", path, NULL));
            g_signal_connect(lgdm_object_get_app(LGDM_OBJECT(launcher)), "start", G_CALLBACK(lgdm_desktop_app_start_app_callback), desktop);
            g_signal_connect(lgdm_object_get_app(LGDM_OBJECT(launcher)), "stopped", G_CALLBACK(lgdm_desktop_app_stopped_app_callback), desktop);
            g_hash_table_insert(desktop->priv->launchers, (gpointer)g_app_info_get_id(appinfo), launcher);
            if (desktop_check_category_gapp(G_APP_INFO(appinfo), "Status")) {
                GtkWidget *action = GTK_WIDGET(g_object_new(GL_TYPE_STATUS_ACTION, "action-launcher", launcher, NULL));
                lgdm_status_add_action(desktop->priv->status, action);
            } else {
                GtkWidget *action = GTK_WIDGET(g_object_new(GL_TYPE_DESKTOP_ACTION, "action-launcher", launcher, NULL));
                lgdm_desktop_place_add_action(desktop->priv->desktop_place, action, 0, 0, lgdm_app_launcher_level(launcher));
            }
            g_dbus_object_manager_server_export(lgdm_state_app_manager(), G_DBUS_OBJECT_SKELETON(launcher));
            g_object_unref(launcher);
            return TRUE;
        }
        g_free(path);
    } else {
        g_warning("App Info Filename = %s", g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(appinfo)));
    }
    return FALSE;
}

static void login_button_clicked_cb (LgdmDesktop* desktop, GtkButton* button)
{
    lgdm_state_login     ();
}


static gboolean
binding_level_to_string      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_STRING(to_value),FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_UINT(from_value),FALSE);
    gchar *level_name = g_strdup_printf("LEVEL %d", g_value_get_uint(from_value));
	g_value_set_string(to_value,level_name);
	g_free(level_name);
    if(!user_data||LGDM_IS_DESKTOP(user_data))return TRUE;
    LgdmDesktop *desktop = LGDM_DESKTOP(user_data);
    gtk_label_set_text(desktop->priv->user_name,lgdm_state_user_name());
	return TRUE;
}


static void lgdm_desktop_init_users (LgdmDesktop* desktop)
{
	// GDBusObjectManager* user_manager = tera_security_client_get_users_manager();
	// GList*              users        = g_dbus_object_manager_get_objects (user_manager);
	// users                            = g_list_sort                       (users, users_sort_on_level);
	// GList*              lu           = NULL;
	// UsersObject*        object       = NULL;
    g_object_bind_property_full(lgdm_state(),"level",desktop->priv->user_level,"label",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE,
				binding_level_to_string,NULL,desktop,NULL);


	// for (lu=users; lu; lu=lu->next) {
	// 	g_object_bind_property      (users_object_get_user(USERS_OBJECT(lu->data)), "activated", row, "sensitive", G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT);

	// 	if (! object)
	// 		object = lu -> data;
	// }

	// if (TERA_GUARD()) {
		/*
		row = GTK_WIDGET (g_object_new (LGDM_TYPE_FAKE_USER, "fake-user", object, "fake-user-name", _("Expert Level"), "fake-user-level", 3, NULL));

		gtk_widget_set_size_request (row, -1, 50);
		gtk_container_add           (GTK_CONTAINER(desktop->priv->user_list), row);
		gtk_widget_show_all         (row);
		g_object_bind_property      (TERA_GUARD(), "level3", row, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

		row = GTK_WIDGET (g_object_new (LGDM_TYPE_FAKE_USER, "fake-user", object, "fake-user-name", _("Service"), "fake-user-level", 4, NULL));

		gtk_widget_set_size_request (row, -1, 50);
		gtk_container_add           (GTK_CONTAINER(desktop->priv->user_list), row);
		gtk_widget_show_all         (row);
		g_object_bind_property      (TERA_GUARD(), "level4", row, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
		*/

		// row = GTK_WIDGET (g_object_new (LGDM_TYPE_FAKE_USER, "fake-user", object, "fake-user-name", _("Factory Settings"), "fake-user-level", 5, NULL));

		// gtk_widget_set_size_request (row, -1, 50);
		// gtk_container_add           (GTK_CONTAINER(desktop->priv->user_list), row);
		// gtk_widget_show_all         (row);
		// g_object_bind_property      (TERA_GUARD(), "level5", row, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
	// }

	// if (users)
		// g_list_free (users);
}

static void guardChangeLevelCallback(LgdmState *state, GParamSpec *pspec, LgdmDesktop *desktop) {
    g_return_if_fail(desktop != NULL);
    if (lgdm_state_get_level(state) < 4) gtk_widget_set_visible(GTK_WIDGET(desktop->priv->restart_revaler), FALSE);
}

// Ansichtsänderungen nach unterschidliche Ereignisse.

static void restartBlockShow(LgdmDesktop *desktop) {
    gtk_widget_show(GTK_WIDGET(desktop->priv->restart_block));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->copyright));
}

static void loadingEffectShow(LgdmDesktop *desktop) {
    gtk_widget_show(GTK_WIDGET(desktop->priv->copyright));
    gtk_widget_show(GTK_WIDGET(desktop->priv->logo_image));
    gtk_widget_show(GTK_WIDGET(desktop->priv->loading_spinner));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->user_scrolled));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->restart_block));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->device_fail_info));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->critical_bild));
}

static void critacalErrorEffectShow(LgdmDesktop *desktop) {
    restartBlockShow(desktop);
    if (!desktop->priv->is_booted) {
        gtk_widget_show(GTK_WIDGET(desktop->priv->device_fail_info));
        gtk_widget_hide(GTK_WIDGET(desktop->priv->logo_image));
    }
    gtk_widget_show(GTK_WIDGET(desktop->priv->critical_bild));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->loading_spinner));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->user_scrolled));
}

static void doneEffectShow(LgdmDesktop *desktop) {
    gtk_widget_hide(GTK_WIDGET(desktop->priv->loading_spinner));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->restart_block));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->critical_bild));
    gtk_widget_hide(GTK_WIDGET(desktop->priv->device_fail_info));
    gtk_widget_show(GTK_WIDGET(desktop->priv->copyright));
    gtk_widget_show(GTK_WIDGET(desktop->priv->logo_image));
    gtk_widget_show(GTK_WIDGET(desktop->priv->user_scrolled));
}

// security service ready


static gint lgdm_desktop_sort_app(gconstpointer a, gconstpointer b) {
    const gchar *categA = g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(a));
    const gchar *categB = g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(b));
    return g_strcmp0(categA, categB);
}

static void control_state_ready(LgdmDesktop *desktop){
    g_debug("control ready");
    gtk_widget_hide(GTK_WIDGET(desktop->priv->loading_spinner));
    // if (!tera_client_is_critical(client)) {
        // if (tera_client_is_done(client)) {
    g_debug("control done");
    desktop->priv->status = LGDM_STATUS(g_object_new(LGDM_TYPE_STATUS, NULL));
    gtk_box_pack_start(GTK_BOX(desktop->priv->status_place), GTK_WIDGET(desktop->priv->status), TRUE, TRUE, 1);
    gtk_widget_show(GTK_WIDGET(desktop->priv->status));

            // Creeate side bar
    desktop->priv->sidebar = LGDM_SIDEBAR(g_object_new(LGDM_TYPE_SIDEBAR, NULL));
    gtk_box_pack_start(GTK_BOX(desktop->priv->sidebar_place), GTK_WIDGET(desktop->priv->sidebar), TRUE, TRUE, 1);
    gtk_widget_show(GTK_WIDGET(desktop->priv->sidebar));

    // Creeate desktop
    desktop->priv->plugs_stack = GTK_STACK(gtk_stack_new());
    gtk_stack_set_transition_duration(desktop->priv->plugs_stack, 200);
    gtk_stack_set_transition_type(desktop->priv->plugs_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_box_pack_start(GTK_BOX(desktop->priv->stack_halter), GTK_WIDGET(desktop->priv->plugs_stack), TRUE, TRUE, 1);
    gtk_widget_show(GTK_WIDGET(desktop->priv->plugs_stack));

    desktop->priv->desktop_place = LGDM_DESKTOP_PLACE(g_object_new(LGDM_TYPE_DESKTOP_PLACE, NULL));
    gtk_stack_add_named(desktop->priv->plugs_stack, GTK_WIDGET(desktop->priv->desktop_place), "lar.lgdm.ams.ui");
    gtk_widget_show(GTK_WIDGET(desktop->priv->desktop_place));
    lgdm_desktop_insert_to_table(desktop, G_OBJECT(desktop->priv->desktop_place), "lar.ams.lgdm.ui");
    gtk_stack_set_visible_child_name(desktop->priv->plugs_stack, "lar.ams.lgdm.ui");
    GList *list = g_app_info_get_all();
    list        = g_list_sort(list, lgdm_desktop_sort_app);
    for (; list != NULL; list = list->next) {
        if (g_str_has_prefix(g_app_info_get_id(G_APP_INFO(list->data)), "lar.ams")) {
            if (G_IS_DESKTOP_APP_INFO(list->data)) lgdm_desktop_app_create_action_is_new(desktop, G_APP_INFO(list->data));
        }
    }
    g_signal_connect(desktop->priv->status, "show-desktop", G_CALLBACK(lgdm_desktop_show_desktop_click_callback), desktop);
    g_signal_connect(desktop->priv->status, "show-status", G_CALLBACK(lgdm_desktop_show_status_click_callback), desktop);

    lgdm_status_set_application_name(desktop->priv->status, "");

    // g_object_bind_property(TERA_GUARD(), "level", desktop->priv->desktop_place, "desktop-level", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(TERA_GUARD(), "level", desktop->priv->sidebar, "lar-level", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_signal_connect(lgdm_state(), "notify::level", G_CALLBACK(guardChangeLevelCallback), desktop);
    g_signal_connect(lgdm_state(), "system-login", G_CALLBACK(guardLoginCallback), desktop);
    g_signal_connect(lgdm_state(), "system-logout", G_CALLBACK(guardLogoutCallback), desktop);
    lgdm_desktop_init_users(desktop);
    // gl_layout_LEVEL2_visible(GTK_WIDGET(desktop->priv->restart_box));

    doneEffectShow(desktop);
    desktop->priv->is_booted = TRUE;
    g_debug("control ready");
    return;
        // }
    // }
    // g_debug("control remove");
    // critacalErrorEffectShow(desktop);
    // lgdm_desktop_session_error();
}

gboolean after_constructed_timeout ( gpointer data  ){
    LgdmDesktop *desktop        = LGDM_DESKTOP(data);
    control_state_ready(desktop);
    return TRUE;
}

static void lgdm_desktop_constructed(GObject *object) {
    LgdmDesktop *desktop        = LGDM_DESKTOP(object);
    _lgd_local_desktop          = desktop;
    // desktop->priv->main_desktop = DESKTOP_OBJECT(desktop_object_skeleton_new("/com/lar/lgdm/desktop/main"));
    gtk_widget_show(GTK_WIDGET(desktop->priv->loading_spinner));
    GDateTime *dt   = g_date_time_new_now_local();
    gchar *    text = g_strdup_printf(_("Copyright © 1992-%d LAR Process Analysers AG"), g_date_time_get_year(dt));
    gtk_label_set_text(desktop->priv->copyright, text);
    g_free(text);
    g_date_time_unref(dt);
    desktop->priv->warte_timer = g_timer_new();
    g_timer_start(desktop->priv->warte_timer);
    // TeraClientObject *pcClient  = mkt_pc_manager_client_new();
    // TeraClientObject *canClient = mkt_can_manager_client_new();
	// TeraClientObject *securityClient = tera_security_manager_client_new();
    // TeraClientObject *controlClient = TERA_CLIENT_OBJECT(ultra_control_client_new("lar.ams.ultra.control"));

    // g_signal_connect(pcClient, "client-lost", G_CALLBACK(pc_service_lost), desktop);
    // g_signal_connect(canClient, "client-lost", G_CALLBACK(can_service_lost), desktop);
    // g_signal_connect(securityClient, "client-lost", G_CALLBACK(security_service_lost), desktop);
    // g_signal_connect(controlClient, "client-lost", G_CALLBACK(measurement_service_lost), desktop);
    // g_signal_connect(controlClient, "client-done", G_CALLBACK(measurement_service_ready_cb), desktop);
    // g_debug("lgdm constructed1");
    // tera_client_run(pcClient);
    // g_debug("lgdm constructed2");
    // tera_client_run(canClient);
    // g_debug("lgdm constructed3");
    // if(tera_client_startup(controlClient))
    // {
        // control_client_ready(controlClient,desktop);
        // return ;
    // }
    // tera_client_run(controlClient);
    loadingEffectShow(desktop);
    g_timeout_add(3,after_constructed_timeout,desktop);
    if (G_OBJECT_CLASS(lgdm_desktop_parent_class)->constructed) G_OBJECT_CLASS(lgdm_desktop_parent_class)->constructed(object);
}

static void lgdm_desktop_finalize(GObject *object) {
    LgdmDesktop *desktop = LGDM_DESKTOP(object);
    if (desktop->priv->info_timer) g_timer_destroy(desktop->priv->info_timer);
    G_OBJECT_CLASS(lgdm_desktop_parent_class)->finalize(object);
}

static void lgdm_desktop_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Set (LGDM_MANAGER) property \n");
    g_return_if_fail(LGDM_IS_DESKTOP(object));
    LgdmDesktop *desktop = LGDM_DESKTOP(object);
    switch (prop_id) {
        case LGDM_DESKTOP_PROP_SERVICE_BOOTED:
            desktop->priv->is_booted = g_value_get_boolean(value);
            break;
        case LGDM_DESKTOP_PROP_SERVICE_CRITICAL:
            desktop->priv->critical = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void lgdm_desktop_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Get (LGDM_MANAGER) property \n");
    g_return_if_fail(LGDM_IS_DESKTOP(object));
    LgdmDesktop *desktop = LGDM_DESKTOP(object);
    switch (prop_id) {
        case LGDM_DESKTOP_PROP_SERVICE_BOOTED:
            g_value_set_boolean(value, desktop->priv->is_booted);
            break;
        case LGDM_DESKTOP_PROP_SERVICE_CRITICAL:
            g_value_set_boolean(value, desktop->priv->critical);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

void lgdm_desktop_class_init(LgdmDesktopClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/layout/desktop.ui");
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, desktop_stack);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, desktop);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, login);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, stack_halter);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, status_place);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, sidebar_place);

    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, open_close_icon);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, restart_revaler);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, meldung_box);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, restart_box);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, show_restart);

    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, user_list);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, user_scrolled);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, restart_block);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, loading_spinner);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, critical_bild);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, restart_session);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, restart_label);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, device_fail_info);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, logo_image);
    gtk_widget_class_bind_template_child_private(widget_class, LgdmDesktop, copyright);

	// gtk_widget_class_bind_template_child_private (widget_class, LgdmUser, wrong_password);

	gtk_widget_class_bind_template_callback (widget_class, login_button_clicked_cb);


    gtk_widget_class_bind_template_callback(widget_class, restart_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, rsession_clicked_cb);
    // gtk_widget_class_bind_template_callback(widget_class, systemlog_clicked_cb);
    // gtk_widget_class_bind_template_callback(widget_class, select_device_clicked_cb);

    gtk_widget_class_bind_template_callback(widget_class, show_restart_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, restart_system_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, restart_session_clicked_cb);
    object_class->finalize     = lgdm_desktop_finalize;
    object_class->set_property = lgdm_desktop_set_property;
    object_class->get_property = lgdm_desktop_get_property;
    object_class->constructed  = lgdm_desktop_constructed;

    g_object_class_install_property(object_class, LGDM_DESKTOP_PROP_SERVICE_BOOTED,
                                    g_param_spec_boolean("is-booted", "System is ready", "System is ready", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property(object_class, LGDM_DESKTOP_PROP_SERVICE_CRITICAL,
                                    g_param_spec_boolean("critical", "Critical error", "Critical error", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));
}

// FIXME : plug in info window (last plugin ) Start last plugin from list..

void lgdm_desktop_local_init() {
    if (_lgd_local_desktop != NULL) {
        return;
    }
    _lgd_local_desktop = LGDM_DESKTOP(g_object_new(LGDM_TYPE_DESKTOP, NULL));
}

LgdmDesktop *lgdm_desktop_local_get() {
    if (_lgd_local_desktop == NULL) lgdm_desktop_local_init();
    return _lgd_local_desktop;
}

GList *lgdm_desktop_launcher(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(LGDM_IS_DESKTOP(_lgd_local_desktop), NULL);
    return g_hash_table_get_values(_lgd_local_desktop->priv->launchers);
}

LgdmStatus *lgdm_desktop_local_get_status(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(LGDM_IS_DESKTOP(_lgd_local_desktop), NULL);
    return _lgd_local_desktop->priv->status;
}

LgdmSidebar *lgdm_desktop_local_get_sidebar(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(LGDM_IS_DESKTOP(_lgd_local_desktop), NULL);
    return _lgd_local_desktop->priv->sidebar;
}

LgdmDesktopPlace *lgdm_desktop_local_get_action_place(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(LGDM_IS_DESKTOP(_lgd_local_desktop), NULL);
    return _lgd_local_desktop->priv->desktop_place;
}

static gboolean lgdm_desktop_show_app(LgdmDesktop *desktop, const gchar *name) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, FALSE);
    g_return_val_if_fail(LGDM_IS_DESKTOP(_lgd_local_desktop), FALSE);
    GtkWidget *widget = gtk_stack_get_visible_child(_lgd_local_desktop->priv->plugs_stack);
    gtk_widget_set_visible(widget, FALSE);
    widget = GTK_WIDGET(g_hash_table_lookup(desktop->priv->objects, name));
    if (widget) {
        gtk_widget_set_visible(widget, TRUE);
        gtk_stack_set_visible_child(_lgd_local_desktop->priv->plugs_stack, widget);
    }
    return TRUE;
}

gboolean lgdm_desktop_local_show_app(const gchar *name) { return lgdm_desktop_show_app(_lgd_local_desktop, name); }

gboolean lgdm_desktop_plug_removed(GtkWidget *app, LgdmDesktop *desktop) {
    if (((gpointer)app) == ((gpointer)gtk_stack_get_visible_child(desktop->priv->plugs_stack))) lgdm_desktop_show_app(desktop, "lar.ams.lgdm.desktop");
    gtk_container_remove(GTK_CONTAINER(_lgd_local_desktop->priv->plugs_stack), GTK_WIDGET(app));
    return TRUE;
}

void lgdm_desktop_local_add_app(GtkWidget *app, const gchar *name) {
    g_return_if_fail(_lgd_local_desktop != NULL);
    g_return_if_fail(app != NULL);
    gtk_stack_add_named(_lgd_local_desktop->priv->plugs_stack, GTK_WIDGET(app), name);
    lgdm_desktop_insert_to_table(_lgd_local_desktop, G_OBJECT(app), name);
}

void lgdm_desktop_local_remove_app(const gchar *name) {
    g_return_if_fail(_lgd_local_desktop != NULL);
    g_return_if_fail(name != NULL);
    GtkWidget *widget = GTK_WIDGET(g_hash_table_lookup(_lgd_local_desktop->priv->objects, name));
    if (widget) {
        gtk_container_remove(GTK_CONTAINER(_lgd_local_desktop->priv->plugs_stack), GTK_WIDGET(widget));
        g_hash_table_remove(_lgd_local_desktop->priv->objects, name);
    }
}
void lgdm_desktop_system_error() {
    LgdmDesktop *desktop = lgdm_desktop_local_get();
    critacalErrorEffectShow(desktop);
    stackShowLogin(desktop);
    g_timer_reset(desktop->priv->warte_timer);
    if (desktop->priv->restart_tag > 0) return;
    desktop->priv->restart_tag = g_timeout_add(100, waite_restart_system_callback, desktop);
}

void lgdm_desktop_session_error() {
    LgdmDesktop *desktop = lgdm_desktop_local_get();
    critacalErrorEffectShow(desktop);
    stackShowLogin(desktop);
    g_timer_reset(desktop->priv->warte_timer);
    if (desktop->priv->restart_tag > 0) return;
    desktop->priv->restart_tag = g_timeout_add(100, waite_restart_session_callback, desktop);
}
