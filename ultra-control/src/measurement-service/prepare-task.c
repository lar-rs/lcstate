/*
 * @ingroup PrepareTask
 * @{
 * @file  prepare-task.c	Task object
 * @brief This is Task object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "prepare-task.h"
#include "ultra-control-process.h"
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _PrepareTaskPrivate {
    UltraStreamObject *stream;
    gboolean           ssp_done;
    gboolean           rinsing_done;
    gboolean           complited;
    GCancellable *     cancellable;
    gulong             handler_id;
};

enum {
    TASK_PROP0,
    PREPARE_STREAM,
    PREPARE_STRIPPING,
    PREPARE_STRIPPING_TIME,
    PREPARE_FILL_SAMPLE_TIME,
};

G_DEFINE_TYPE_WITH_PRIVATE(PrepareTask, prepare_task, MKT_TYPE_TASK_OBJECT);

static void prepare_task_DILUTION_DONE(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PrepareTask *task   = PREPARE_TASK(g_task_get_source_object(subTask));
    gboolean     result = FALSE;
    GError *     error  = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(task), _("sequence %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_DILUTION_SEQUENCE_WORKER())),
            sequence_workers_process_get_status(sequence_object_get_workers_process(ULTRA_DILUTION_SEQUENCE_WORKER())));
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    g_task_return_boolean(subTask, TRUE);
    g_object_unref(subTask);
}

static void prepare_task_DILUTION(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PrepareTask *  task   = PREPARE_TASK(g_task_get_source_object(subTask));
    VesselsObject *drain  = ultra_stream_get_drain(task->priv->stream);
    VesselsObject *sample = ultra_stream_get_sample(task->priv->stream);
    // TODO: ist es immer noch notig zwei dilution vorgange zu unterstutzen? !streams_ultra_get_on_replicte(streams_object_get_ultra(task->priv->stream))
    if (streams_ultra_get_is_dilution(streams_object_get_ultra(STREAMS_OBJECT(task->priv->stream))) && !streams_ultra_get_on_replicte(streams_object_get_ultra(STREAMS_OBJECT(task->priv->stream)))) {
        mkt_task_object_set_status(MKT_TASK_OBJECT(task), _("run sequence %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_DILUTION_SEQUENCE_WORKER())));
        sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_DILUTION_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
        sequence_workers_sample_set_sample_second(sequence_object_get_workers_sample(ULTRA_DILUTION_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(drain)));
        StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(task->priv->stream));
        sequence_workers_dilution_set_fill_time(sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()), (gdouble)streams_ultra_get_dilution_pump_time(ultra));
        sequence_workers_dilution_set_dilution_time(sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()), streams_ultra_get_dilution_wait_time(ultra));
        sequence_workers_dilution_set_proportion(sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()), streams_ultra_get_dilution_factor(ultra));
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_DILUTION_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), prepare_task_DILUTION_DONE, subTask);
        control_need_hold();
        control_need_rinsing(drain);
    } else {
        g_task_return_boolean(subTask, TRUE);
        g_object_unref(subTask);
    }
    g_object_unref(drain);
    g_object_unref(sample);
}

// prepare_task_RINSING_DONE Rinsing is done run dilution if it activated
static void prepare_task_RINSING_DONE(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *      subTask = G_TASK(user_data);
    PrepareTask *task    = PREPARE_TASK(g_task_get_source_object(subTask));

    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(task), _("sequence %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
            sequence_workers_process_get_status(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER())));
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    } 
    prepare_task_DILUTION(subTask);
}

static void prepare_task_PRE_RINSING(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PrepareTask *  task  = PREPARE_TASK(g_task_get_source_object(subTask));
    VesselsObject *drain = ultra_stream_get_drain(task->priv->stream);
    mkt_task_object_set_status(MKT_TASK_OBJECT(task), _("run sequence %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())));
    guint rinsing_count = streams_ultra_get_rinsing_count(streams_object_get_ultra(STREAMS_OBJECT(task->priv->stream)));
    sequence_workers_sample_set_repeat(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()), rinsing_count);
    sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(drain)));
    sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), prepare_task_RINSING_DONE, subTask);
    control_need_hold();
    control_need_rinsing(drain);
    g_object_unref(drain);
}

// FILL_SAMPLE AND STRIPPING
// XXXXX

static gboolean prepare_task_start(GTask *subTask) {
    prepare_task_PRE_RINSING(subTask);
    return TRUE;
}

static gboolean check_task_parameter(GTask *subTask) {
    PrepareTask *                           prepare_task = PREPARE_TASK(g_task_get_source_object(subTask));

    if (prepare_task->priv->stream == NULL || !ULTRA_IS_STREAM_OBJECT(prepare_task->priv->stream)) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "prepare stream object not found"));
        g_object_unref(subTask);
        return FALSE;
    }
    StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(prepare_task->priv->stream));
    if (ultra == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "prepare stream ultra interface not found"));
        g_object_unref(subTask);
        return FALSE;
    }
    VesselsObject *drain = ultra_stream_get_drain(prepare_task->priv->stream);
    if (drain == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "drain vessel %s not found", streams_ultra_get_drain_vessel(ultra)));
        g_object_unref(subTask);
        return FALSE;
    }
    g_object_unref(drain);

    return TRUE;
}
static void prepare_cancelled(GCancellable *cancel, GTask *task) {
    PrepareTask *prepare_task = PREPARE_TASK(g_task_get_source_object(task));
    gboolean     is_done      = FALSE;
    if (ultra_stream_get_pump(prepare_task->priv->stream)) {
        pumps_pump_call_stop_sync(pumps_object_get_pump(ultra_stream_get_pump(prepare_task->priv->stream)), &is_done, NULL, NULL);
    }
}

static void prepare_task_run(MktTaskObject *task, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *subTask = g_task_new(task, cancellable, callback, user_data);

    PrepareTask *prepare_task       = PREPARE_TASK(task);
    prepare_task->priv->complited   = FALSE;
    prepare_task->priv->cancellable = g_object_ref(cancellable);
    prepare_task->priv->handler_id  = g_cancellable_connect(g_task_get_cancellable(subTask), G_CALLBACK(prepare_cancelled), subTask, NULL);

    if (check_task_parameter(subTask)) prepare_task_start(subTask);
}

static gboolean prepare_task_finish(MktTaskObject *self, GTask *task, GError **error) {
    PREPARE_TASK(self)->priv->complited = TRUE;
    gboolean result                     = g_task_propagate_boolean(G_TASK(task), error);
    gboolean is_done;
    if (ultra_stream_get_pump(PREPARE_TASK(self)->priv->stream)) {
        pumps_pump_call_stop_sync(pumps_object_get_pump(ultra_stream_get_pump(PREPARE_TASK(self)->priv->stream)), &is_done, NULL, NULL);
    }

    // g_object_unref(self);
    return result;
}

static void prepare_task_init(PrepareTask *prepare_task) {
    prepare_task->priv = prepare_task_get_instance_private(prepare_task);
    tera_pumps_manager_client_new();
    ultra_vessels_manager_client_new();
    prepare_task->priv->stream = NULL;
}

static void prepare_task_finalize(GObject *object) {
    PrepareTask *task = PREPARE_TASK(object);
    if (task->priv->cancellable) {
        g_cancellable_disconnect(task->priv->cancellable, task->priv->handler_id);
        g_object_unref(task->priv->cancellable);
    }

    // if (task->priv->stream) g_object_unref(task->priv->stream);
    G_OBJECT_CLASS(prepare_task_parent_class)->finalize(object);
}

static void prepare_task_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    PrepareTask *task = PREPARE_TASK(object);
    switch (prop_id) {
    case PREPARE_STREAM:
        if (task->priv->stream) g_object_unref(task->priv->stream);
        task->priv->stream = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void prepare_task_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    PrepareTask *task = PREPARE_TASK(object);
    switch (prop_id) {
    case PREPARE_STREAM:
        g_value_set_object(value, task->priv->stream);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void prepare_task_class_init(PrepareTaskClass *klass) {
    GObjectClass *                                    object_class = G_OBJECT_CLASS(klass);
    // object_class->dispose           = prepare_atom_dispose;
    object_class->finalize               = prepare_task_finalize;
    object_class->set_property           = prepare_task_set_property;
    object_class->get_property           = prepare_task_get_property;
    MKT_TASK_OBJECT_CLASS(klass)->run    = prepare_task_run;
    MKT_TASK_OBJECT_CLASS(klass)->finish = prepare_task_finish;

    g_object_class_install_property(object_class, PREPARE_STREAM,
        g_param_spec_object("prepare-stream", "Prepare stream object", "Prepare stream object", ULTRA_TYPE_STREAM_OBJECT, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

/** @} */
