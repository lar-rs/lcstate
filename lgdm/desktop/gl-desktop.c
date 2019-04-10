/*
 * @ingroup GlDesktop
 * @{
 * @file  gl-desktop.c	LGDM desktop
 * @brief LGDM desktop
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-app-info.h"
#include "gl-desktop-action.h"
#include "gl-desktop-place.h"
#include "gl-desktop.h"
#include "gl-layout.h"
#include "gl-sidebar.h"
#include "gl-status-action.h"
#include "gl-status.h"
#include "lgdm-ui-collection.h"
#include <gtk/gtkx.h>
#include <mktbus.h>
#include <mktlib.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>
#define DESKTOP_MAX_PARAMS 15

static GlDesktop *_lgd_local_desktop = NULL;

// static GlDesktop *__gui_process_desktop = NULL;

struct _GlDesktopPrivate {

    GlDesktopPlace *desktop_place;
    // GlLogBook               *logbook;
    GlSidebar *sidebar;
    GlStatus * status;
    GtkBox *   stack_halter;
    GtkBox *   sidebar_place;
    GtkBox *   status_place;
    GtkStack * plugs_stack;

    gint       timer_tag;
    GSettings *desktop_settings;
    GSettings *status_settings;
    GSettings *sidebar_settings;
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
    DesktopObject *main_desktop;
};

enum {
    GL_DESKTOP_PROP_NULL,
};

enum { GL_DESKTOP_SEARCH_SIGNAL, GL_DESKTOP_LAST_SIGNAL };

// static guint gl_desktop_signals[GL_DESKTOP_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlDesktop, gl_desktop, GTK_TYPE_WINDOW);

/*
static void
gl_desktop_change_level(GlManager *manager,GlLevelManager *level)
{

}


*/

static void gl_desktop_application_change(GlDesktop *desktop, GParamSpec *pspec, gpointer user_data) {
}

static void gl_desktop_resize_window(GlDesktop *desktop) {
    // GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW(desktop));
    // gint display_hight = 600;
    // gint display_width = 800;
    gtk_widget_set_size_request(GTK_WIDGET(desktop), 800, 600);
    gtk_window_resize(GTK_WINDOW(desktop), 800, 600);
    gtk_window_move(GTK_WINDOW(desktop), 0, 0);
}

// /*
static void
gl_desktop_status_settings_changed (GSettings *settings, gchar     *key,
gpointer   user_data)
{
        GlDesktop *desktop = GL_DESKTOP(user_data);
        gl_desktop_resize_window(desktop);
}
static void
gl_desktop_sidebar_settings_changed (GSettings *settings, gchar     *key,
gpointer   user_data)
{
        GlDesktop *desktop = GL_DESKTOP(user_data);
        gl_desktop_resize_window(desktop);
}
*/

static void gl_desktop_insert_to_table(GlDesktop *desktop, GObject *object, const gchar *id) {
    gpointer p = g_hash_table_lookup(desktop->priv->objects, (gconstpointer)id);
    if (p)
        g_hash_table_remove(desktop->priv->objects, (gconstpointer)id);
    g_hash_table_insert(desktop->priv->objects, (gpointer)id, (gpointer)object);
}

static void desktop_launches_destroy_key(gpointer data) {
    if (data)
        g_free(data);
}
static void desktop_launches_destroy_value(gpointer data) {
    if (data)
        g_object_unref(data);
}

static void gl_desktop_init(GlDesktop *desktop) {
    g_return_if_fail(desktop != NULL);
    g_return_if_fail(GL_IS_DESKTOP(desktop));
    desktop->priv                   = gl_desktop_get_instance_private(desktop);
    desktop->priv->desktop_settings = g_settings_new("com.lar.LGDM.Desktop");
    desktop->priv->status_settings  = g_settings_new("com.lar.LGDM.Status");
    desktop->priv->sidebar_settings = g_settings_new("com.lar.LGDM.Sidebar");
    desktop->priv->indicate         = NULL;
    desktop->priv->last_plugin      = NULL;
    desktop->priv->objects          = g_hash_table_new(g_str_hash, g_str_equal);
    desktop->priv->launchers        = g_hash_table_new_full(g_str_hash, g_str_equal, desktop_launches_destroy_key, desktop_launches_destroy_value);

    gtk_widget_init_template(GTK_WIDGET(desktop));
    g_signal_connect(desktop, "notify::application", G_CALLBACK(gl_desktop_application_change), NULL);
    // g_signal_connect(desktop->priv->status_settings,"changed",G_CALLBACK(gl_desktop_status_settings_changed),desktop);
    // g_signal_connect(desktop->priv->sidebar_settings,"changed",G_CALLBACK(gl_desktop_sidebar_settings_changed),desktop);

    desktop->priv->status = GL_STATUS(g_object_new(GL_TYPE_STATUS, NULL));
    gtk_box_pack_start(GTK_BOX(desktop->priv->status_place), GTK_WIDGET(desktop->priv->status), TRUE, TRUE, 1);

    gtk_widget_show(GTK_WIDGET(desktop->priv->status));

    // Creeate side bar
    desktop->priv->sidebar = GL_SIDEBAR(g_object_new(GL_TYPE_SIDEBAR, NULL));
    gtk_box_pack_start(GTK_BOX(desktop->priv->sidebar_place), GTK_WIDGET(desktop->priv->sidebar), TRUE, TRUE, 1);
    gtk_widget_show(GTK_WIDGET(desktop->priv->sidebar));

    // Creeate desktop
    desktop->priv->plugs_stack = GTK_STACK(gtk_stack_new());
    gtk_stack_set_transition_duration(desktop->priv->plugs_stack, 200);
    gtk_stack_set_transition_type(desktop->priv->plugs_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_box_pack_start(GTK_BOX(desktop->priv->stack_halter), GTK_WIDGET(desktop->priv->plugs_stack), TRUE, TRUE, 1);
    gtk_widget_show(GTK_WIDGET(desktop->priv->plugs_stack));

    desktop->priv->desktop_place = GL_DESKTOP_PLACE(g_object_new(GL_TYPE_DESKTOP_PLACE, NULL));
    gtk_stack_add_named(desktop->priv->plugs_stack, GTK_WIDGET(desktop->priv->desktop_place), "com.lar.ldm.desktop");
    gtk_widget_show(GTK_WIDGET(desktop->priv->desktop_place));
    gl_desktop_insert_to_table(desktop, G_OBJECT(desktop->priv->desktop_place), "com.lar.ldm.desktop");
    gtk_stack_set_visible_child_name(desktop->priv->plugs_stack, "com.lar.ldm.desktop");

    /*desktop->priv->logbook  = GL_LOG_BOOK(g_object_new(GL_TYPE_LOG_BOOK,NULL));
    gtk_stack_add_named(desktop->priv->plugs_stack,GTK_WIDGET(desktop->priv->logbook),"com.lar.ldm.logbook");
    gl_desktop_insert_to_table(desktop,G_OBJECT(desktop->priv->logbook),"com.lar.ldm.logbook");*/
    // gtk_widget_show(GTK_WIDGET(desktop->priv->logbook));
    // gtk_stack_set_visible_child_name(desktop->priv->plugs_stack,"com.lar.ldm.desktop");

    /*GtkBuilder *builder =
    gtk_builder_new_from_resource("/lgdm/ui/layout/desktop-actions.ui");
    if(builder)
    {
            //GtkWidget *widget =
    GTK_WIDGET(gtk_builder_get_object(builder,"desktop_scrolled"));
    }*/
    gl_desktop_resize_window(desktop);

    GdkDisplay *      display        = gtk_widget_get_display(GTK_WIDGET(desktop));
    GdkDeviceManager *device_manager = gdk_display_get_device_manager(display);
    GList *           devices        = gdk_device_manager_list_devices(device_manager, GDK_DEVICE_TYPE_MASTER);
}

static void desctop_chenge_power_settings_icon(GlDesktop *desktop, const gchar *name) {
    GdkPixbuf *   buf   = NULL;
    GtkIconTheme *theme = gtk_icon_theme_get_default();
    buf                 = gtk_icon_theme_load_icon(theme, name, 24, GTK_ICON_LOOKUP_FORCE_SVG, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(desktop->priv->open_close_icon), buf);
    g_object_unref(buf);
}

static void gl_desktop_show_desktop_click_callback(GlStatus *sidebar, GlDesktop *desktop) {
    gl_desktop_local_show_app("com.lar.ldm.desktop");
    desktop_desktop_emit_show_desktop(desktop_object_get_desktop(desktop->priv->main_desktop));
    gl_status_set_application_name(gl_desktop_local_get_status(), "");
    gtk_window_set_default_size(GTK_WINDOW(gl_desktop_local_get()), 800, 600);
    gtk_widget_set_size_request(GTK_WIDGET(gl_desktop_local_get()), 800, 600);
}

static void gl_desktop_show_status_click_callback(GlStatus *sidebar, GlDesktop *desktop) {
    // gl_desktop_local_show_app("com.lar.ldm.logbook");
    desktop_desktop_emit_show_desktop(desktop_object_get_desktop(desktop->priv->main_desktop));
    gl_status_set_application_name(gl_desktop_local_get_status(), "");
    gtk_window_set_default_size(GTK_WINDOW(gl_desktop_local_get()), 800, 600);
    gtk_widget_set_size_request(GTK_WIDGET(gl_desktop_local_get()), 800, 600);
}

static void gl_desktop_app_start_app_callback(LgdmApp *app, const gchar *layout, GlDesktop *desktop) {
    gl_desktop_local_show_app(lgdm_app_get_app_id(app));
}

static void gl_desktop_app_stopped_app_callback(LgdmApp *app GlDesktop *desktop){
    desktop_desktop_emit_show_desktop(desktop_object_get_desktop(desktop->priv->main_desktop));
    gl_status_set_application_name(gl_desktop_local_get_status(), "");
    gtk_window_set_default_size(GTK_WINDOW(gl_desktop_local_get()), 800, 600);
    gtk_widget_set_size_request(GTK_WIDGET(gl_desktop_local_get()), 800, 600);
 
}

static gboolean desktop_check_category_gapp(GAppInfo *app, const gchar *category) {
    const gchar *rstr = g_strrstr_len(g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(app)), 128, category);
    return rstr != NULL;
}

static void show_restart_clicked_cb(GlDesktop *desktop, GtkButton *button) {
    gtk_widget_set_visible(GTK_WIDGET(desktop->priv->restart_revaler), !gtk_widget_get_visible(GTK_WIDGET(desktop->priv->restart_revaler)));

    if (gtk_widget_get_visible(GTK_WIDGET(desktop->priv->restart_revaler)))
        desctop_chenge_power_settings_icon(desktop, "arrow-right-01");
    else
        desctop_chenge_power_settings_icon(desktop, "powerbuttons");
}
static void restart_system_clicked_cb(GlDesktop *desktop, GtkButton *button) {
    mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "System reboot (User action)");
    if (mkt_pc_manager_client_get_device())
        larpc_device_call_reboot(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
}
static void restart_session_clicked_cb(GlDesktop *desktop, GtkButton *button) {
    mkt_log_message_sync(MKT_LOG_STATE_SYSTEM, "Session restart (User action)");
    if (mkt_pc_manager_client_get_device())
        larpc_device_call_restart(mkt_pc_manager_client_get_device(), NULL, NULL, NULL);
}

static gboolean gl_desktop_app_create_action_is_new (GlDesktop* desktop, GAppInfo* appinfo)
{
    if (g_desktop_app_info_get_startup_wm_class (G_DESKTOP_APP_INFO (appinfo))) {
        gchar*         path     = g_strdup_printf ("/com/lar/lgdm/app/%s", g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(appinfo)));
        GlAppLauncher* launcher = GL_APP_LAUNCHER (g_dbus_object_manager_get_object(G_DBUS_OBJECT_MANAGER(LGDM_APP_MANAGER()), path));

        if (! launcher) {
            GlAppLauncher* launcher = GL_APP_LAUNCHER (g_object_new (GL_TYPE_APP_LAUNCHER, "app-info", appinfo, "g-object-path", path, NULL));

            g_signal_connect    (lgdm_object_get_app(LGDM_OBJECT(launcher)), "start",                              G_CALLBACK(gl_desktop_app_start_app_callback),   desktop);
            g_signal_connect    (lgdm_object_get_app(LGDM_OBJECT(launcher)), "stopped",                            G_CALLBACK(gl_desktop_app_stopped_app_callback), desktop);
            g_hash_table_insert (desktop->priv->launchers,                   (gpointer)g_app_info_get_id(appinfo), launcher);

            if (desktop_check_category_gapp(G_APP_INFO(appinfo), "Status")) {
                GtkWidget* action = GTK_WIDGET (g_object_new (GL_TYPE_STATUS_ACTION, "action-launcher", launcher, NULL));
                gl_status_add_action (desktop->priv->status, action);
            }
            else {
                GtkWidget* action = GTK_WIDGET (g_object_new (GL_TYPE_DESKTOP_ACTION, "action-launcher", launcher, NULL));
                gl_desktop_place_add_action (desktop->priv->desktop_place, action, 0, 0, gl_app_launcher_level(launcher));
            }

            g_dbus_object_manager_server_export (LGDM_APP_MANAGER(), G_DBUS_OBJECT_SKELETON(launcher));
            g_object_unref                      (launcher);

            return TRUE;
        }

        g_free(path);
    }
    else
        g_warning ("App Info Filename = %s", g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(appinfo)));

    return FALSE;
}

static gint gl_desktop_sort_app(gconstpointer a, gconstpointer b) {
    const gchar *categA = g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(a));
    const gchar *categB = g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(b));
    return g_strcmp0(categA, categB);
}

static void tera_change_level_callback(SecurityDevice *guard, GParamSpec *pspec, GlDesktop *desktop) {
    g_return_if_fail(desktop != NULL);
    if (security_device_get_level(guard) < 4)
        gtk_widget_set_visible(GTK_WIDGET(desktop->priv->restart_revaler), FALSE);
}

static void gl_desktop_constructed(GObject *object) {
    GlDesktop *desktop          = GL_DESKTOP(object);
    _lgd_local_desktop          = desktop;
    desktop->priv->main_desktop = DESKTOP_OBJECT(desktop_object_skeleton_new("/com/lar/lgdm/desktop/main"));
    DesktopDesktop *desk        = desktop_desktop_skeleton_new();
    desktop_object_skeleton_set_desktop(DESKTOP_OBJECT_SKELETON(desktop->priv->main_desktop), desk);
    // g_object_bind_property(TERA_GUARD(),"handle-need-session-restart",desktop->priv->sidebar,"lar-level",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

    g_dbus_object_manager_server_export(LGDM_SERVER_MANAGER(), G_DBUS_OBJECT_SKELETON(desktop->priv->main_desktop));
    GList *list = g_app_info_get_all();
    list        = g_list_sort(list, gl_desktop_sort_app);
    for (; list != NULL; list = list->next) {

        if (g_str_has_prefix(g_app_info_get_id(G_APP_INFO(list->data)), "com.lar")) {

            if (G_IS_DESKTOP_APP_INFO(list->data))
                gl_desktop_app_create_action_is_new(desktop, G_APP_INFO(list->data));
        }
    }
    // desktop_desktop_se
    g_signal_connect(desktop->priv->status, "show-desktop", G_CALLBACK(gl_desktop_show_desktop_click_callback), desktop);
    g_signal_connect(desktop->priv->status, "show-status", G_CALLBACK(gl_desktop_show_status_click_callback), desktop);

    gl_status_set_application_name(desktop->priv->status, "");

    g_object_bind_property(TERA_GUARD(), "level", desktop->priv->desktop_place, "desktop-level", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(TERA_GUARD(), "level", desktop->priv->sidebar, "lar-level", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    g_signal_connect(TERA_GUARD(), "notify::level", G_CALLBACK(tera_change_level_callback), desktop);
    gl_layout_LEVEL3_visible(GTK_WIDGET(desktop->priv->show_restart));

    if (G_OBJECT_CLASS(gl_desktop_parent_class)->constructed)
        G_OBJECT_CLASS(gl_desktop_parent_class)->constructed(object);
}

static void gl_desktop_finalize(GObject *object) {
    GlDesktop *desktop = GL_DESKTOP(object);
    if (desktop->priv->info_timer)
        g_timer_destroy(desktop->priv->info_timer);
    G_OBJECT_CLASS(gl_desktop_parent_class)->finalize(object);
}

static void gl_desktop_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Set (GL_MANAGER) property \n");
    g_return_if_fail(GL_IS_DESKTOP(object));
    // GlDesktop* desktop = GL_DESKTOP(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_desktop_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Get (GL_MANAGER) property \n");
    g_return_if_fail(GL_IS_DESKTOP(object));
    // GlDesktop* desktop = GL_DESKTOP(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void gl_desktop_class_init(GlDesktopClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/ui/layout/desktop.ui");
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, stack_halter);
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, status_place);
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, sidebar_place);

    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, open_close_icon);
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, restart_revaler);
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, meldung_box);
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, restart_box);
    gtk_widget_class_bind_template_child_private(widget_class, GlDesktop, show_restart);

    // gtk_widget_class_bind_template_child_private (widget_class, GlDesktop,
    // desktop_action_fail);
    // gtk_widget_class_bind_template_child_private (widget_class, GlDesktop,
    // desktop_scrolled);
    // gtk_widget_class_bind_template_child_private (widget_class, GlDesktop,
    // desktop_viewport);

    // gtk_widget_class_bind_template_child_private (widget_class, GlDesktop,
    // noob_actions);
    // gtk_widget_class_bind_template_child_private (widget_class, GlDesktop,
    // service_actions);
    // gtk_widget_class_bind_template_child_private (widget_class, GlDesktop,
    // level_separator);

    gtk_widget_class_bind_template_callback(widget_class, show_restart_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, restart_system_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, restart_session_clicked_cb);
    object_class->finalize     = gl_desktop_finalize;
    object_class->set_property = gl_desktop_set_property;
    object_class->get_property = gl_desktop_get_property;
    object_class->constructed  = gl_desktop_constructed;
}

// FIXME : plug in info window (last plugin ) Start last plugin from list..

void gl_desktop_local_init() {
    if (_lgd_local_desktop != NULL) {
        // g_error("More as too lgd desktop local initialized");
        return;
    }
    _lgd_local_desktop = GL_DESKTOP(g_object_new(GL_TYPE_DESKTOP, NULL));
}

GlDesktop *gl_desktop_local_get() {
    if (_lgd_local_desktop == NULL)
        gl_desktop_local_init();
    return _lgd_local_desktop;
}

GList *gl_desktop_launcher(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(GL_IS_DESKTOP(_lgd_local_desktop), NULL);
    return g_hash_table_get_values(_lgd_local_desktop->priv->launchers);
}

GlStatus *gl_desktop_local_get_status(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(GL_IS_DESKTOP(_lgd_local_desktop), NULL);
    return _lgd_local_desktop->priv->status;
}

GlSidebar *gl_desktop_local_get_sidebar(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(GL_IS_DESKTOP(_lgd_local_desktop), NULL);
    return _lgd_local_desktop->priv->sidebar;
}

GlDesktopPlace *gl_desktop_local_get_action_place(void) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, NULL);
    g_return_val_if_fail(GL_IS_DESKTOP(_lgd_local_desktop), NULL);
    return _lgd_local_desktop->priv->desktop_place;
}

static gboolean gl_desktop_show_app(GlDesktop *desktop, const gchar *name) {
    g_return_val_if_fail(_lgd_local_desktop != NULL, FALSE);
    g_return_val_if_fail(GL_IS_DESKTOP(_lgd_local_desktop), FALSE);
    GtkWidget *widget = gtk_stack_get_visible_child(_lgd_local_desktop->priv->plugs_stack);
    gtk_widget_set_visible(widget, FALSE);
    widget = GTK_WIDGET(g_hash_table_lookup(desktop->priv->objects, name));
    if (widget) {
        gtk_widget_set_visible(widget, TRUE);
        gtk_stack_set_visible_child(_lgd_local_desktop->priv->plugs_stack, widget);
    }
    /*widget = gtk_stack_get_visible_child(_lgd_local_desktop->priv->plugs_stack);
    gtk_widget_set_visible(widget,TRUE);*/

    return TRUE;
}

gboolean gl_desktop_local_show_app(const gchar *name) {
    return gl_desktop_show_app(_lgd_local_desktop, name);
}

gboolean gl_desktop_plug_removed(GtkWidget *app, GlDesktop *desktop) {
    if (((gpointer)app) == ((gpointer)gtk_stack_get_visible_child(desktop->priv->plugs_stack)))
        gl_desktop_show_app(desktop, "com.lar.ldm.desktop");
    gtk_container_remove(GTK_CONTAINER(_lgd_local_desktop->priv->plugs_stack), GTK_WIDGET(app));
    return TRUE;
}

void gl_desktop_local_add_app(GtkWidget *app, const gchar *name) {
    g_return_if_fail(_lgd_local_desktop != NULL);
    g_return_if_fail(app != NULL);
    gtk_stack_add_named(_lgd_local_desktop->priv->plugs_stack, GTK_WIDGET(app), name);
    gl_desktop_insert_to_table(_lgd_local_desktop, G_OBJECT(app), name);
}

void gl_desktop_local_remove_app(const gchar *name) {
    g_return_if_fail(_lgd_local_desktop != NULL);
    g_return_if_fail(name != NULL);
    GtkWidget *widget = GTK_WIDGET(g_hash_table_lookup(_lgd_local_desktop->priv->objects, name));
    if (widget) {
        gtk_container_remove(GTK_CONTAINER(_lgd_local_desktop->priv->plugs_stack), GTK_WIDGET(widget));
        g_hash_table_remove(_lgd_local_desktop->priv->objects, name);
    }
}

/** @} */
