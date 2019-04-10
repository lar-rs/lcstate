/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-log.c
 * Copyright (C) 2018 LAR
 */

#ifndef _MKT_LOG_H_
#define _MKT_LOG_H_

// #include "mkt-sqlite.h"
#include <glib.h>
#include <gio/gio.h>

#define MKT_TYPE_LOG (mkt_log_get_type())
#define MKT_LOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MKT_TYPE_LOG, MktLog))
#define MKT_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MKT_TYPE_LOG, MktLogClass))
#define MKT_IS_LOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MKT_TYPE_LOG))
#define MKT_IS_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MKT_TYPE_LOG))
#define MKT_LOG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MKT_TYPE_LOG, MktLogClass))

typedef struct _MktLogClass   MktLogClass;
typedef struct _MktLog        MktLog;
typedef struct _MktLogPrivate MktLogPrivate;

struct _MktLogClass {
    GObjectClass parent_class;
};

struct _MktLog {
    GObject        parent_instance;
    MktLogPrivate *priv;
};

GType mkt_log_get_type(void) G_GNUC_CONST;

// typedef enum                 /*< flags >*/
// { LOG_X_AXIS      = 1 << 0,  /* < nick= X-Axis>*/
//   LOG_Y_AXIS      = 1 << 1,  /* < nick= X-Axis>*/
//   LOG_INJECTION   = 1 << 2,  /* < nick= X-Axis>*/
//   LOG_FURNACE     = 1 << 3,  /* < nick= X-Axis>*/
//   LOG_TIC         = 1 << 4,  /* < nick= X-Axis>*/
//   LOG_COOLER      = 1 << 5,  /* < nick= X-Axis>*/
//   LOG_FLOW_SENSOR = 1 << 6,  /* < nick= X-Axis>*/
//   LOG_NDIR1       = 1 << 7,  /* < nick= X-Axis>*/
//   LOG_NDIR2       = 1 << 8,  /* < nick= X-Axis>*/
//   LOG_TNb         = 1 << 9,  /* < nick= X-Axis>*/
//   LOG_CODo        = 1 << 10, /* < nick= X-Axis>*/
//   LOG_HUMIDITY    = 1 << 11, /* < nick= X-Axis>*/
//   LOG_PRESURE     = 1 << 12, /* < nick= X-Axis>*/
//   LOG_CANDEVICE   = 1 << 13, /* < nick= X-Axis>*/
//   LOG_ANALOG      = 1 << 14, /* < nick= X-Axis>*/
//   LOG_MOTOR       = 1 << 15, /* < nick= X-Axis>*/
// } MktLogNote;

typedef enum {
    MKT_LOG_STATE_STATUS,           /*< nick=Status>*/
    MKT_LOG_STATE_SYSTEM,           /*< nick=System>*/
    MKT_LOG_STATE_ERRORS,           /*< nick=Error>*/
    MKT_LOG_STATE_WARNING,          /*< nick=Warning>*/
    MKT_LOG_STATE_LIMIT,            /*< nick=Limit>*/
    MKT_LOG_STATE_PARAMETER_CHANGE, /*< nick=Parameter Change>*/
    MKT_LOG_STATE_SERVICE_LOG,      /*< nick=Service Log>*/
    MKT_LOG_STATE_OPERATOR,         /*< nick=Operator>*/
    MKT_LOG_STATE_MEASUREMENT,      /*< nick=Measurement>*/
    MKT_LOG_STATE_CALIBRATE,        /*< nick=Calibration>*/
    MKT_LOG_STATE_UNKNOWN,          /*< nick=Unknown>*/
    MKT_LOG_STATE_LAST,
} MktLogState;

#define MKT_LOG_TABLE_NAME "messages"

const gchar *mkt_log_get_message(MktLog *log);
guint        mkt_log_get_state(MktLog *log);
const gchar *mkt_log_get_note(MktLog *log);
gdouble      mkt_log_get_changed(MktLog *log);
void         mkt_log_message(MktLogState state, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void         mkt_log_message_sync(MktLogState state, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void         mkt_log_message_note(MktLogState state, const gchar *note, const gchar *format, ...) G_GNUC_PRINTF(3, 4);
void         mkt_log_status_message(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void         mkt_log_system_message(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void         mkt_log_error_message(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void         mkt_log_error_message_sync(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void         mkt_log_warning_message(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void         mkt_log_error_message_with_note(const gchar *note, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void         mkt_log_limit_message(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
// GSList *     mkt_log_select(const gchar *match, ...) G_GNUC_PRINTF(1, 2);
// void         mkt_log_async(GCancellable *cancel, GAsyncReadyCallback callback, gpointer user_data, const gchar *match, ...) G_GNUC_PRINTF(4, 5);
// MktSqlite *  mkt_log_connection();
// GSList      *mkt_log_select(const gchar *format, ...) G_GNUC_PRINTF(1, 2);

G_END_DECLS

#endif
