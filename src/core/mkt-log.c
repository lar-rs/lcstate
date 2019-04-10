/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-log.c
 * Copyright (C) 2018 LAR
 */

#include "mkt-log.h"
#include <math.h>
#include "market-time.h"


enum {
    PROP_0,
    PROP_MESSAGE,
    PROP_STATE,
    PROP_CHANGED,
    PROP_NOTE,
};

struct _MktLogPrivate {
    gchar * message;
    gchar * note;
    gdouble changed;
    guint   state;
};

#define SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT = 100

#define MKT_LOG_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), MKT_TYPE_LOG, MktLogPrivate))

G_DEFINE_TYPE(MktLog, mkt_log, G_TYPE_OBJECT)

static void mkt_log_init(MktLog *mkt_log) {
    MktLogPrivate *priv = MKT_LOG_PRIVATE(mkt_log);
    priv->message       = g_strdup("empty message");
    priv->changed       = market_db_time_now();
    priv->state         = 0;
    priv->note          = g_strdup("-");
    mkt_log->priv       = priv;
    /* TODO: Add initialization code here */
}

static void mkt_log_finalize(GObject *object) {
    MktLog *data = MKT_LOG(object);
    if (data->priv->message) g_free(data->priv->message);
    if (data->priv->note) g_free(data->priv->note);
    G_OBJECT_CLASS(mkt_log_parent_class)->finalize(object);
}

static void mkt_log_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(MKT_IS_LOG(object));
    MktLog *data = MKT_LOG(object);
    switch (prop_id) {
        case PROP_MESSAGE:
            if (data->priv->message != NULL) g_free(data->priv->message);
            data->priv->message = g_value_dup_string(value);
            break;
        case PROP_CHANGED:
            data->priv->changed = g_value_get_double(value);
            break;
        case PROP_STATE:
            data->priv->state = g_value_get_uint(value);
            break;
        case PROP_NOTE:
            if (data->priv->note != NULL) g_free(data->priv->note);
            data->priv->note = g_value_dup_string(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void mkt_log_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(MKT_IS_LOG(object));
    MktLog *data = MKT_LOG(object);
    switch (prop_id) {
        case PROP_MESSAGE:
            g_value_set_string(value, data->priv->message);
            break;
        case PROP_CHANGED:
            g_value_set_double(value, data->priv->changed);
            break;
        case PROP_STATE:
            g_value_set_uint(value, data->priv->state);
            break;
        case PROP_NOTE:
            g_value_set_string(value, data->priv->note);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void mkt_log_class_init(MktLogClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // MktModelClass* parent_class = MKT_MODEL_CLASS (klass);
    g_type_class_add_private(klass, sizeof(MktLogPrivate));
    object_class->finalize     = mkt_log_finalize;
    object_class->set_property = mkt_log_set_property;
    object_class->get_property = mkt_log_get_property;
    g_object_class_install_property(object_class, PROP_MESSAGE, g_param_spec_string("message", "Log message prop", "Set|Get message property", "nil", G_PARAM_READWRITE));
    g_object_class_install_property(object_class, PROP_CHANGED, g_param_spec_double("changed", "changed", "changed", 0., G_MAXDOUBLE, 0., G_PARAM_READWRITE));
    g_object_class_install_property(object_class, PROP_STATE, g_param_spec_uint("state", "State", "state", 0, G_MAXUINT32, 0, G_PARAM_READWRITE));
    g_object_class_install_property(object_class, PROP_NOTE, g_param_spec_string("note", "note", "note", "-", G_PARAM_READWRITE));
}

#define DB_DIR "db"
#define DB_NAME "log.sqlite"
#define TABLE_NAME MKT_LOG_TABLE_NAME
#define JOURNAL "WAL"

static gboolean _check_table = FALSE;

// MktSqlite *     mkt_log_connection() {
//     gchar *    dbpath = g_build_filename(g_get_home_dir(), DB_DIR, DB_NAME, NULL);
//     GObject *  model  = g_object_new(MKT_TYPE_LOG, NULL);
//     MktSqlite *conn   = mkt_sqlite_new_full(model, dbpath, TABLE_NAME,JOURNAL);
//     g_free(dbpath);
//     g_object_unref(model);
    // return conn;
// }

// static void log_check_create_table(MktSqlite *conn){
//     if(!_check_table){
//         mkt_sqlite_open(conn);
//         mkt_sqlite_create_table(conn);
//     }
// }

#undef DB_DIR
#undef DB_NAME
#undef TABLE_NAME
#undef JOURNAL

// static void sqliteInsertThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
//     MktSqlite *conn = mkt_log_connection();
//     mkt_sqlite_open(conn);
//     log_check_create_table(conn);
//     guint64 ref = mkt_sqlite_insert_object(conn, G_OBJECT(source_object));
//     g_object_unref(conn);
//     g_task_return_boolean(task, ref != 0);
//     return;
// }

void mkt_log_write_message(MktLog *log) {
    // GTask *task = g_task_new(log, NULL, NULL, NULL);
    // g_task_run_in_thread(task, sqliteInsertThread);
    // g_object_unref(task);
}

void mkt_log_write_message_sync(MktLog *log) {
    // MktSqlite *conn = mkt_log_connection();
    // mkt_sqlite_open(conn);
    // log_check_create_table(conn);
    // mkt_sqlite_insert_object(conn, G_OBJECT(log));
    // g_object_unref(conn);
}

const gchar *mkt_log_get_message(MktLog *log) {
    g_return_val_if_fail(log != NULL, NULL);
    g_return_val_if_fail(MKT_IS_LOG(log), NULL);
    return log->priv->message;
}

guint mkt_log_get_state(MktLog *log) {
    g_return_val_if_fail(log != NULL, 0);
    g_return_val_if_fail(MKT_IS_LOG(log), 0);
    return log->priv->state;
}

const gchar *mkt_log_get_note(MktLog *log) {
    g_return_val_if_fail(log != NULL, 0);
    g_return_val_if_fail(MKT_IS_LOG(log), 0);
    return log->priv->note;
}

gdouble mkt_log_get_changed(MktLog *log) {
    g_return_val_if_fail(log != NULL, 0.);
    g_return_val_if_fail(MKT_IS_LOG(log), 0.);
    return log->priv->changed;
}

void mkt_log_message_sync(MktLogState state, const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", state, NULL));
        mkt_log_write_message_sync(msg);
        g_object_unref(msg);
        g_free(message);
    }
}
void mkt_log_message(MktLogState state, const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", state, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_message_note(MktLogState type, const gchar *note, const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", type, "note", note, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_status_message(const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_STATUS, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_system_message(const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_SYSTEM, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_error_message(const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_ERRORS, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}
void mkt_log_error_message_sync(const gchar *format, ... ){
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_ERRORS, NULL));
        mkt_log_write_message_sync(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_error_message_with_note(const gchar *note, const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_ERRORS, "note", note, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_warning_message(const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_WARNING, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

void mkt_log_limit_message(const gchar *format, ...) {
    va_list args;
    gchar * message = NULL;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    if (message != NULL) {
        MktLog *msg = MKT_LOG(g_object_new(MKT_TYPE_LOG, "message", message, "state", MKT_LOG_STATE_LIMIT, NULL));
        mkt_log_write_message(msg);
        g_object_unref(msg);
        g_free(message);
    }
}

// GSList *mkt_log_select(const gchar *match, ...) {
//     MktSqlite *conn = mkt_log_connection();
//     mkt_sqlite_open(conn);
//     log_check_create_table(conn);
//     GSList *messages = mkt_sqlite_select_objects(conn, "%s", match);
//     g_object_unref(conn);
//     return messages;
// }

// GQuark mkt_log_quark() {
//     static GQuark error;
//     if (!error) error = g_quark_from_static_string("log");
//     return error;
// }

// static void destroy_list(gpointer list) {
//     // g_print("destroy Async Object list\n");
//     if (list != NULL) {
//         g_slist_free_full((GSList *)list, g_object_unref);
//     }
// }

// static void selectQueryThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
//     MktSqlite *conn = MKT_SQLITE(source_object);
//     if (g_task_return_error_if_cancelled(task)) {
//         return;
//     }
//     if (!mkt_sqlite_open(conn)) {
//         g_task_return_error(task, g_error_new(mkt_log_quark(), 100, "Open database error"));
//         return;
//     }
//     const gchar *match = g_task_get_task_data(task);
//     GSList *messages = NULL;
//     if(match)
//         messages = mkt_sqlite_select_objects(conn, "%s",match);
//     else
//         messages = mkt_sqlite_select_objects(conn,";");
//     mkt_sqlite_close(conn);
//     if (g_task_return_error_if_cancelled(task)) {
//         g_slist_free_full(messages, g_object_unref);
//         return;
//     }
//     g_task_return_pointer(task,messages,destroy_list);
//     return;
// }

// void mkt_log_async(GCancellable *cancel, GAsyncReadyCallback callback, gpointer user_data, const gchar *match, ...){
//     MktSqlite *conn = mkt_log_connection();
//     GTask *    task = g_task_new(conn, cancel, callback, user_data);
//     gchar * old_locale = g_strdup(setlocale(LC_ALL, NULL));
//     va_list args;
//     setlocale(LC_ALL, "en_US.UTF-8");
//     gchar *build_match;
//     va_start(args, match);
//     build_match = g_strdup_vprintf(match, args);
//     va_end(args);
//     setlocale(LC_ALL, old_locale);
//     g_free(old_locale);
//     g_task_set_task_data(task, build_match,g_free);
//     g_task_run_in_thread(task, selectQueryThread);
//     g_object_unref(task);
//     g_object_unref(conn);
// }
