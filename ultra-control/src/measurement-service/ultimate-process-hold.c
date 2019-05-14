/*
 * @ingroup UltimateProcessHold
 * @{
 * @file  ultimate-process_hold.c	ProcessObject object
 * @brief This is ProcessObject object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "ultimate-process-hold.h"

#include <mktbus.h>
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltimateProcessHoldPrivate {
    GCancellable * cancelable;
    gboolean       reinit;
    gboolean       need_rinsing;
    gboolean       need_hold;
    VesselsObject *drain;
};

enum {
    HOLD_PROP0,
    HOLD_NEED_HOLD,
    HOLD_NEED_RINSE,
    HOLD_DRAIN_VESSEL,

};

#define ULTIMATE_PROCESS_HOLD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTIMATE_TYPE_PROCESS_HOLD, UltimateProcessHoldPrivate))

G_DEFINE_TYPE(UltimateProcessHold, ultimate_process_hold, MKT_TYPE_PROCESS_OBJECT)

static void ultra_control_process_HOLD_POSITION_AUTOSTART(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask) ) {
        g_object_unref(subTask);
        return;
    }

    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(g_task_get_source_object(subTask));
    gboolean             result       = FALSE;
    GError *             error        = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_process_object_critical(MKT_PROCESS_OBJECT(process_hold), _("sequense %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
            error != NULL ? error->message : "unknown error");
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    g_task_return_boolean(subTask, TRUE);
    g_object_unref(subTask);
    process_hold->priv->reinit = FALSE;
}

static void hold_task_RINSING_DONE(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask) ) {
        g_object_unref(subTask);
        return;
    }
    
    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(g_task_get_source_object(subTask));
    gboolean             result       = FALSE;
    GError *             error        = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_process_object_critical(MKT_PROCESS_OBJECT(process_hold), _("sequence %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
            error != NULL ? error->message : "unknown error");
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    mkt_process_object_status(MKT_PROCESS_OBJECT(process_hold), _("set all hold"));
    sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_HOLD_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), ultra_control_process_HOLD_POSITION_AUTOSTART, subTask);
    process_hold->priv->need_rinsing = FALSE;
}

static void ultra_control_process_HOLDRINSING_POSITION_AUTOSTART(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask) ) {
        g_object_unref(subTask);
        return;
    }
    
    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(g_task_get_source_object(subTask));
    gboolean             result       = FALSE;
    GError *             error        = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_process_object_critical(MKT_PROCESS_OBJECT(process_hold), _("sequense %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
            error != NULL ? error->message : "unknown error");
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    sequence_workers_sample_set_repeat(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()),1);
    sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(process_hold->priv->drain)));
    sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER()),g_task_get_cancellable(subTask), hold_task_RINSING_DONE, subTask);
}




static void HOLD_wait_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask) ) {
        g_object_unref(subTask);
        return;
    }
    GError *error = NULL;
    if (!lar_timer_default_finish(res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    g_task_return_boolean(subTask, TRUE);
    g_object_unref(subTask);

}

static void ultimate_process_hold_run(MktProcessObject *process, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(process);
    GTask *subTask = g_task_new(process, cancellable, callback, user_data);
    if (process_hold->priv->reinit) {
        mkt_process_object_status(MKT_PROCESS_OBJECT(process_hold), _("Set all hold"));
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_HOLD_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), ultra_control_process_HOLD_POSITION_AUTOSTART, subTask);
    } else if (process_hold->priv->need_rinsing && process_hold->priv->drain) {
        mkt_process_object_status(MKT_PROCESS_OBJECT(process_hold), _("Set all hold"));
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_HOLD_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), ultra_control_process_HOLDRINSING_POSITION_AUTOSTART, subTask);
      } else if (process_hold->priv->need_hold) {
        mkt_process_object_status(MKT_PROCESS_OBJECT(process_hold), _("Set all hold"));
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_HOLD_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), ultra_control_process_HOLD_POSITION_AUTOSTART, subTask);
    } else {
        lar_timer_default_run(g_task_get_cancellable(subTask), HOLD_wait_done, 0.5, subTask);
    }
}
static void ultimate_process_hold_init(UltimateProcessHold *ultimate_process_hold) {
    ultimate_process_hold->priv               = ULTIMATE_PROCESS_HOLD_PRIVATE(ultimate_process_hold);
}

static void ultimate_process_hold_finalize(GObject *object) {
    G_OBJECT_CLASS(ultimate_process_hold_parent_class)->finalize(object);
}

static void ultimate_process_hold_constructed(GObject *object) {
    /* TODO: Add deinitalization code here */
    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(object);

    if (G_OBJECT_CLASS(ultimate_process_hold_parent_class)->constructed) G_OBJECT_CLASS(ultimate_process_hold_parent_class)->constructed(object);

    process_simple_set_interruptible(process_object_get_simple(PROCESS_OBJECT(process_hold)),FALSE);

    //
}

static void ultimate_process_hold_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(object);
    switch (prop_id) {
    case HOLD_NEED_HOLD:
        process_hold->priv->need_hold = g_value_get_boolean(value);
        break;
    case HOLD_NEED_RINSE:
        process_hold->priv->need_rinsing = g_value_get_boolean(value);
        break;
    case HOLD_DRAIN_VESSEL:
        if (process_hold->priv->drain) g_object_unref(process_hold->priv->drain);
        process_hold->priv->drain = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultimate_process_hold_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    UltimateProcessHold *process_hold = ULTIMATE_PROCESS_HOLD(object);
    switch (prop_id) {
    case HOLD_NEED_HOLD:
        g_value_set_boolean(value, process_hold->priv->need_hold);
        break;
    case HOLD_NEED_RINSE:
        g_value_set_boolean(value, process_hold->priv->need_rinsing);
        break;
    case HOLD_DRAIN_VESSEL:
        g_value_set_object(value, process_hold->priv->drain);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/* signals */

static void ultimate_process_hold_class_init(UltimateProcessHoldClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltimateProcessHoldClass));
    MktProcessObjectClass *pclass = MKT_PROCESS_OBJECT_CLASS(klass);
    // object_class->dispose              = ultimate_atom_dispose;
    object_class->finalize     = ultimate_process_hold_finalize;
    object_class->set_property = ultimate_process_hold_set_property;
    object_class->get_property = ultimate_process_hold_get_property;
    object_class->constructed  = ultimate_process_hold_constructed;
    pclass->run                = ultimate_process_hold_run;

    g_object_class_install_property(object_class, HOLD_NEED_HOLD, g_param_spec_boolean("need-hold", "Need hold", "Need hold", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE));

    g_object_class_install_property(object_class, HOLD_NEED_RINSE, g_param_spec_boolean("need-rinsing", "Need rinsing", "Need rinsing", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE));

    g_object_class_install_property(object_class, HOLD_DRAIN_VESSEL, g_param_spec_object("drain-vessel", "Drain vessel", "Drain vessel", VESSELS_TYPE_OBJECT, G_PARAM_WRITABLE | G_PARAM_READABLE));

    /*	ultimate_action_signals[PROCESS_OBJECT_RUN] =
                            g_signal_new ("process_hold-run",
                                            G_TYPE_FROM_CLASS (klass),
                                            G_SIGNAL_RUN_LAST ,
                                            0,
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE,
                                            0,
                                            G_TYPE_NONE);*/
}

void ultimate_process_hold_reinit(UltimateProcessHold *hold) { hold->priv->reinit = TRUE; }

/** @} */
