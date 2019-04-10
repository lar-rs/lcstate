/*
 *
 *
 *  Copyright (C) LAR 2015
 *  author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "lgdm-desktop-place.h"
#include "gl-service-log.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

#include <mktlib.h>
#include <mktbus.h>
#include <fcntl.h>
// static GlServiceLog *__gui_process_desktop = NULL;

struct _GlServiceLogPrivate {
    GtkTreeView *  log_tree;
    GtkListBoxRow *close_service_log;
    GtkSpinner *   load_log;

    GtkScrolledWindow *scrolledwindow;
    GtkButton *        save_button;
    GtkButton *        next_messages;
    LarpcDevice *      pc_device;
    GEnumClass *       message_enum;

    gboolean log_load;
    gboolean log_data;
    gboolean more_data;

    guint         offset;
    guint         save_offset;
    guint         msg_counter;
    GCancellable *log_cancel;
    FILE *        copy_file;
};

enum {
    GL_SERVICE_LOG_PROP_NULL,
    GL_SERVICE_LOG_PROP_LOG_DATA,
    GL_SERVICE_LOG_PROP_LOG_LOAD,

};

enum { GL_SERVICE_LOG_LAST_SIGNAL };

// static guint gl_service_log_signals[GL_SERVICE_LOG_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlServiceLog, gl_service_log, GTK_TYPE_WINDOW);

static const char *date_file_format(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%d_%m_%Y-%H_%M", timeinfo);
    return (buf);
}

#define MAX_STEP_DATA 18

// -------------------------------------- Bitte hier alle Error Verwaltungsfunktionen ---------------------------------------------

// ------------------------------------------------------- Logbook Functions ----------------------------------------

static void log_save_messages(GlServiceLog *logbook, GSList *lm) {
    if (lm) {
        if (logbook->priv->copy_file) {
            GSList *l = NULL;
            for (l = lm; l != NULL; l = l->next) {
                logbook->priv->save_offset++;
                guint        kind  = (guint)mkt_log_get_state(MKT_LOG(lm->data));
                const gchar *tname = _("Unknown");
                GEnumValue * enum_value;
                enum_value = g_enum_get_value(logbook->priv->message_enum, kind);
                if (enum_value) tname = enum_value->value_nick;
                if (0 > fprintf(logbook->priv->copy_file, "%s,%s,%s\n", market_db_get_date_string(mkt_log_get_changed(MKT_LOG(l->data))), tname, mkt_log_get_message(MKT_LOG(l->data)))) break;
            }
            fflush(logbook->priv->copy_file);
        }
    }
}

static void destroy_list(gpointer list) {
    // g_print("destroy Async Object list\n");
    if (list != NULL) {
        g_slist_free_full((GSList *)list, g_object_unref);
    }
}

static void saveLogDataThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    GSList *messages = mkt_log_select("ORDER BY changed DESC LIMIT %d", 50000);
    if (g_task_return_error_if_cancelled(task)) {
        g_slist_free_full(messages, g_object_unref);
    }
    g_task_return_pointer(task, messages, destroy_list);
    return;
}
static void gl_service_log_save_system_log(GlServiceLog *logbook) {
    if (logbook->priv->pc_device == NULL) return;
    GString *file_path = g_string_new("");
    g_string_append_printf(file_path, "%s/start-log", larpc_device_get_usb_path(logbook->priv->pc_device));
    // g_string_append_printf(file_path,".");
    file_path = g_string_ascii_down(file_path);
    if (0 >= g_mkdir_with_parents(file_path->str, 666)) {
        GDir * dir;
        gchar *check_dir = "/var/log/tera/";
        dir              = g_dir_open(check_dir, 0, NULL);
        if (dir) {
            const gchar *name = NULL;
            while ((name = g_dir_read_name(dir))) {
                gchar *source         = g_strdup_printf("/var/log/tera/%s", name);
                gchar *source_content = NULL;
                gsize  len            = 0;
                if (g_file_get_contents(source, &source_content, &len, NULL)) {
                    gchar *dest = g_strdup_printf("%s/%s", file_path->str, name);
                    g_file_set_contents(dest, source_content, len, NULL);
                    g_free(dest);
                    g_free(source_content);
                }
                g_free(source);
            }
        }
    }
}

static void gl_service_log_save_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GlServiceLog *logbook = GL_SERVICE_LOG(source_object);
    GError *      error   = NULL;
    GSList *      models  = g_task_propagate_pointer(G_TASK(res), &error);
    gl_service_log_save_system_log(logbook);
    if (error == NULL) {
        log_save_messages(logbook, models);
        if (logbook->priv->copy_file) {
            fflush(logbook->priv->copy_file);
            fclose(logbook->priv->copy_file);
            logbook->priv->copy_file = NULL;
        }
        g_object_set(logbook, "log-is-load", FALSE, NULL);

    } else {
        g_error_free(error);
        if (logbook->priv->copy_file) {
            fflush(logbook->priv->copy_file);
            fclose(logbook->priv->copy_file);
            logbook->priv->copy_file = NULL;
        }
        g_object_set(logbook, "log-is-load", FALSE, NULL);
    }
}

static void log_select_save_run(GlServiceLog *logbook) {
    if (logbook->priv->log_cancel) {
        g_cancellable_cancel(logbook->priv->log_cancel);
        g_object_unref(logbook->priv->log_cancel);
    }
    logbook->priv->log_cancel = g_cancellable_new();
    GTask *task               = g_task_new(logbook, logbook->priv->log_cancel, gl_service_log_save_callback, NULL);
    g_task_run_in_thread(task, saveLogDataThread);
    g_object_unref(task);
}

static void save_button_clicked_cb(GlServiceLog *logbook, GtkButton *button) {
    if (logbook->priv->pc_device == NULL) return;
    g_object_set(logbook, "log-is-load", TRUE, NULL);
    g_cancellable_cancel(logbook->priv->log_cancel);
    g_cancellable_reset(logbook->priv->log_cancel);
    logbook->priv->save_offset = 0;

    GString *file_path = g_string_new("");
    g_string_append_printf(file_path, "%s/start-log", larpc_device_get_usb_path(logbook->priv->pc_device));
    // g_string_append_printf(file_path,".");
    file_path = g_string_ascii_down(file_path);
    g_mkdir_with_parents(file_path->str, 0777);
    const gchar *fromStr = date_file_format(market_db_time_now());
    const gchar *tname   = _("all");
    g_string_append_printf(file_path, "/servicelog-%s_%s-%d-messages.csv", tname, fromStr, logbook->priv->msg_counter);
    // GError *error = NULL;
    file_path                = g_string_ascii_down(file_path);
    logbook->priv->copy_file = fopen(file_path->str, "w");
    if (logbook->priv->copy_file != NULL) {
        fprintf(logbook->priv->copy_file, "timestamp,type,message\n");
        log_select_save_run(logbook);
    } else {
        g_object_set(logbook, "log-is-load", FALSE, NULL);
    }
    g_string_free(file_path, TRUE);
}
static void log_insert_messages(GlServiceLog *logbook, GSList *lm, gboolean autoupdate) {
    GtkTreeStore *tree_store = GTK_TREE_STORE(gtk_tree_view_get_model(logbook->priv->log_tree));
    guint         anzahl     = 0;
    for (; lm != NULL; lm = lm->next) {
        anzahl++;
        logbook->priv->offset++;
        logbook->priv->msg_counter++;
        guint kind = (guint)mkt_log_get_state(MKT_LOG(lm->data));
        if (kind >= MKT_LOG_STATE_LAST) kind = MKT_LOG_STATE_UNKNOWN;
        const gchar *tname = _("Unknown");
        GEnumValue * enum_value;
        enum_value = g_enum_get_value(logbook->priv->message_enum, kind);
        if (enum_value) tname = enum_value->value_nick;
        GtkTreeIter child;
        gtk_tree_store_append(GTK_TREE_STORE(tree_store), &child, NULL);
        gtk_tree_store_set(GTK_TREE_STORE(tree_store), &child, 0, tname, 1, market_db_get_date_hmydm(mkt_log_get_changed(MKT_LOG(lm->data))), 2, mkt_log_get_message(MKT_LOG(lm->data)), -1);
    }
    if (anzahl < MAX_STEP_DATA) {
        gint i = 0;
        for (i = 0; i < 5; i++) {
            GtkTreeIter extra_child;
            gtk_tree_store_append(GTK_TREE_STORE(tree_store), &extra_child, NULL);
        }
    }
}

void gl_service_log_selection_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GlServiceLog *logbook = GL_SERVICE_LOG(user_data);
    GError *      error   = NULL;
    GSList *      models  = g_task_propagate_pointer(G_TASK(res), &error);
    if (error) {
        g_critical("service log select data error");
        g_error_free(error);
    } else if (models) {
        gl_service_log_save_system_log(logbook);
        log_insert_messages(logbook, models, FALSE);
    }
    if (logbook->priv->offset > 0) g_object_set(logbook, "log-is-data", TRUE, NULL);
    g_object_set(logbook, "log-is-load", FALSE, NULL);
}

static void log_select_run(GlServiceLog *logbook) {
    if (logbook->priv->log_cancel) {
        g_cancellable_cancel(logbook->priv->log_cancel);
        g_object_unref(logbook->priv->log_cancel);
    }
    logbook->priv->log_cancel = g_cancellable_new();
    mkt_log_async(logbook->priv->log_cancel, gl_service_log_selection_async_callback, logbook, "ORDER BY changed DESC LIMIT %d OFFSET %d", MAX_STEP_DATA, logbook->priv->offset);
}

static void gl_service_log_reload_start(GlServiceLog *logbook) {
    g_cancellable_cancel(logbook->priv->log_cancel);
    g_cancellable_reset(logbook->priv->log_cancel);
    logbook->priv->offset      = 0;
    logbook->priv->msg_counter = 0;
    GtkTreeModel *model        = gtk_tree_view_get_model(logbook->priv->log_tree);
    gtk_tree_store_clear(GTK_TREE_STORE(model));
    g_object_set(logbook, "log-is-load", TRUE, NULL);
    g_object_set(logbook, "log-is-data", FALSE, NULL);

    log_select_run(logbook);
    // g_debug("no auto update TYPE=%d",logbook->priv->message_type_value);
}

static void next_messages_clicked_cb(GlServiceLog *logbook, GtkButton *button) {
    g_cancellable_cancel(logbook->priv->log_cancel);
    g_cancellable_reset(logbook->priv->log_cancel);
    g_object_set(logbook, "log-is-load", TRUE, NULL);
    log_select_run(logbook);
}

static void show_logbook_clicked_cb(GlServiceLog *gl_service_log, GtkButton *button) { g_object_set(gl_service_log, "log-activated", TRUE, NULL); }

static void show_errors_clicked_cb(GlServiceLog *gl_service_log, GtkButton *button) { g_object_set(gl_service_log, "log-activated", FALSE, NULL); }

static void logbook_settings_row_activated_cb(GlServiceLog *gl_service_log, GtkListBoxRow *row, GtkListBox *list_box) { gtk_widget_set_visible(GTK_WIDGET(gl_service_log), FALSE); }

static void gl_service_log_init(GlServiceLog *gl_service_log) {
    g_return_if_fail(gl_service_log != NULL);
    g_return_if_fail(GL_IS_SERVICE_LOG(gl_service_log));
    gl_service_log->priv               = gl_service_log_get_instance_private(gl_service_log);
    gl_service_log->priv->log_cancel   = g_cancellable_new();
    gl_service_log->priv->message_enum = g_type_class_ref(MKT_TYPE_LOG_STATE);
    gtk_widget_init_template(GTK_WIDGET(gl_service_log));
}

static void realize_save_button(GlServiceLog *logbook) {
    gboolean has_usb = FALSE;
    if (logbook->priv->pc_device != NULL) {
        has_usb = larpc_device_get_has_usb(logbook->priv->pc_device);
    }
    gtk_widget_set_visible(GTK_WIDGET(logbook->priv->save_button), logbook->priv->log_data && has_usb);
}

static void logbook_has_usb_device(LarpcDevice *pc_device, GParamSpec *pspec, GlServiceLog *logbook) { realize_save_button(logbook); }

static void service_log_call_start(GlServiceLog *servicelog, GParamSpec *pspec, gpointer data) {
    if (gtk_widget_get_visible(GTK_WIDGET(servicelog))) {
        if (servicelog->priv->pc_device == NULL) {
            servicelog->priv->pc_device = mkt_pc_manager_client_get_device();
            if (servicelog->priv->pc_device != NULL) {
                g_signal_connect(servicelog->priv->pc_device, "notify::has-usb", G_CALLBACK(logbook_has_usb_device), servicelog);

                realize_save_button(servicelog);
            }
        }
        gl_service_log_reload_start(servicelog);
    }
}

static void gl_service_log_constructed(GObject *object) {
    GlServiceLog *logbook = GL_SERVICE_LOG(object);

    g_signal_connect(logbook, "notify::visible", G_CALLBACK(service_log_call_start), logbook);

    g_object_bind_property(logbook, "log-is-load", logbook->priv->load_log, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(logbook, "log-is-load", logbook->priv->load_log, "active", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    g_object_bind_property(logbook, "log-is-load", logbook->priv->save_button, "sensitive", G_BINDING_DEFAULT | G_BINDING_INVERT_BOOLEAN);
    g_object_bind_property(logbook, "log-is-load", logbook->priv->next_messages, "sensitive", G_BINDING_DEFAULT | G_BINDING_INVERT_BOOLEAN);

    g_object_bind_property(logbook, "log-is-data", logbook->priv->load_log, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    G_OBJECT_CLASS(gl_service_log_parent_class)->constructed(object);
}

static void gl_service_log_finalize(GObject *object) {
    GlServiceLog *gl_service_log = GL_SERVICE_LOG(object);
    g_type_class_unref(gl_service_log->priv->message_enum);
    G_OBJECT_CLASS(gl_service_log_parent_class)->finalize(object);
}

static void gl_service_log_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_SERVICE_LOG(object));
    GlServiceLog *gl_service_log = GL_SERVICE_LOG(object);
    switch (prop_id) {
        case GL_SERVICE_LOG_PROP_LOG_DATA:
            gl_service_log->priv->log_data = g_value_get_boolean(value);
            realize_save_button(gl_service_log);
            break;
        case GL_SERVICE_LOG_PROP_LOG_LOAD:
            gl_service_log->priv->log_load = g_value_get_boolean(value);
            realize_save_button(gl_service_log);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gl_service_log_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_SERVICE_LOG(object));
    GlServiceLog *gl_service_log = GL_SERVICE_LOG(object);
    switch (prop_id) {
        case GL_SERVICE_LOG_PROP_LOG_DATA:
            g_value_set_boolean(value, gl_service_log->priv->log_data);
            break;
        case GL_SERVICE_LOG_PROP_LOG_LOAD:
            g_value_set_boolean(value, gl_service_log->priv->log_load);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gl_service_log_class_init(GlServiceLogClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    // GlLayoutClass        *layout_class     =  GL_LAYOUT_CLASS (klass);
    object_class->finalize     = gl_service_log_finalize;
    object_class->set_property = gl_service_log_set_property;
    object_class->get_property = gl_service_log_get_property;
    object_class->constructed  = gl_service_log_constructed;

    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/ui/layout/service-log.ui");

    gtk_widget_class_bind_template_child_private(widget_class, GlServiceLog, log_tree);
    gtk_widget_class_bind_template_child_private(widget_class, GlServiceLog, close_service_log);
    gtk_widget_class_bind_template_child_private(widget_class, GlServiceLog, scrolledwindow);
    gtk_widget_class_bind_template_child_private(widget_class, GlServiceLog, save_button);
    gtk_widget_class_bind_template_child_private(widget_class, GlServiceLog, next_messages);
    gtk_widget_class_bind_template_child_private(widget_class, GlServiceLog, load_log);

    gtk_widget_class_bind_template_callback(widget_class, logbook_settings_row_activated_cb);

    gtk_widget_class_bind_template_callback(widget_class, save_button_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, next_messages_clicked_cb);

    // gtk_widget_class_bind_template_callback (widget_class, map_event_test);
    gtk_widget_class_bind_template_callback(widget_class, show_logbook_clicked_cb);
    gtk_widget_class_bind_template_callback(widget_class, show_errors_clicked_cb);

    g_object_class_install_property(object_class, GL_SERVICE_LOG_PROP_LOG_DATA,
                                    g_param_spec_boolean("log-is-data", "Log is data", "Log is data", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));
    g_object_class_install_property(object_class, GL_SERVICE_LOG_PROP_LOG_LOAD,
                                    g_param_spec_boolean("log-is-load", "Log is data", "Log is data", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));
}

static GQuark serviceLogQuark(void) {
    static GQuark error;
    if (!error) error = g_quark_from_static_string("service-log");
    return error;
}



static void saveCanInternalLogThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
    gchar *path = (gchar *)g_task_get_task_data(task);
    if (path == NULL) {
        g_task_return_error(task, g_error_new(serviceLogQuark(), 1, "Create service log fail:save path == NULL"));
        return;
    }
    if (0 > g_mkdir_with_parents(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
        g_task_return_error(task, g_error_new(serviceLogQuark(), 1, "Create service log fail - create directory"));
        return;
    }
    if (g_task_return_error_if_cancelled(task)) return;
    GDir *  dir   = NULL;
    GError *error = NULL;
    dir           = g_dir_open("/var/log/tera", 0, &error);
    if (dir == NULL || error != NULL) {
        g_task_return_error(task, g_error_new(serviceLogQuark(), 1, "Create service log fail - %s", error != NULL ? error->message : "can`t open log directory"));
        if (error) g_error_free(error);
        return;
    }
    const gchar *name = NULL;
    while ((name = g_dir_read_name(dir))) {
        if (g_strstr_len(name, -1, "scan-fail")) {
            gchar *src_path  = g_build_filename("/var/log/tera", name, NULL);
            gchar *dest_path = g_build_filename(path, name, NULL);
            GFile *src       = g_file_new_for_path(src_path);
            GFile *dest      = g_file_new_for_path(dest_path);
            g_file_copy(src, dest, G_FILE_COPY_OVERWRITE, g_task_get_cancellable(task), NULL, NULL, NULL);
            g_object_unref(src);
            g_object_unref(dest);
            g_free(src_path);
            g_free(dest_path);
        }
        if (g_task_return_error_if_cancelled(task)){
            g_dir_close(dir);
            return;
        }
    }
    g_dir_close(dir);
    g_task_return_boolean(task, TRUE);
    // g_object_unref(task);
    return;
}

void gl_service_log_copy_can_fail(const gchar *path, GCancellable *cancel, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_if_fail(path != NULL);
    GTask *task = g_task_new(NULL, cancel, callback, user_data);
    g_task_set_task_data(task, g_strdup(path), (GDestroyNotify)g_free);
    g_task_run_in_thread(task,saveCanInternalLogThread);
    g_object_unref(task);
}

#define LAR_CORE_DUMP "/var/log/tera/coredump"

static void saveAppCoreDumpThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
    GDir *  dir   = NULL;
    gchar *path = (gchar *)g_task_get_task_data(task);
    if (path == NULL) {
        g_task_return_error(task, g_error_new(serviceLogQuark(), 1, "Save app core dump error - destination path null is not allowed"));
        return;
    }
    if (0 > g_mkdir_with_parents(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
        g_task_return_error(task, g_error_new(serviceLogQuark(), 1, "Save app core dump create directory %s error",path));
        return;
    }
    if (g_task_return_error_if_cancelled(task))return;
    GError *error = NULL;
    dir           = g_dir_open(LAR_CORE_DUMP, 0, &error);
    if (dir == NULL || error != NULL) {
        g_task_return_error(task, g_error_new(serviceLogQuark(), 1, "Save app core dump error - can`t open %s directory",LAR_CORE_DUMP));
        if (error) g_error_free(error);
        return;
    }
    const gchar *name = NULL;
    while ((name = g_dir_read_name(dir))) {
        if (g_strstr_len(name, -1, "kd-")) {
            gchar *src_path  = g_build_filename(LAR_CORE_DUMP, name, NULL);
            gchar *dest_path = g_build_filename(path, name, NULL);
            GFile *src       = g_file_new_for_path(src_path);
            GFile *dest      = g_file_new_for_path(dest_path);
            g_file_copy(src, dest, G_FILE_COPY_OVERWRITE, g_task_get_cancellable(task), NULL, NULL, NULL);
            g_object_unref(src);
            g_object_unref(dest);
            g_free(src_path);
            g_free(dest_path);
        }
        if (g_task_return_error_if_cancelled(task)){
            g_dir_close(dir);
            return;
        } 
    }

    g_dir_close(dir);
    g_task_return_boolean(task, TRUE);
    return;
    // g_object_unref(task);
}

void gl_service_log_copy_app_core_dump(const gchar *path, GCancellable *cancel, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_if_fail(path != NULL);
    GTask *task = g_task_new(NULL, cancel, callback, user_data);
    g_task_set_task_data(task, g_strdup(path), (GDestroyNotify)g_free);
    g_task_run_in_thread(task,saveAppCoreDumpThread);
    g_object_unref(task);
}