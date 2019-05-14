/* 
 * 
 */

#include <ultimate-library.h>

#include "psanalyse-task.h"
#include "ultimate-channel.h"
#include "ultra-control-process.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _PSAnalyseTaskPrivate {
    UltraStreamObject *stream;
    gdouble            analyse;
    GList *            channels;
    gboolean           sampling_done;
    gboolean           stabilization_done;
    GCancellable *     cancellable;
    gulong             handler_id;
    gdouble            airflow_current;
    GTimer *           analyse_timeout;
    GTimer *           wait_after;
    gboolean           stoped;
    gboolean           complited;
};

/* signals */

enum {
    PANALYSE_PROP0,
    PSANALYSE_STREAM,
    PSANALYSE_TIME,
};

G_DEFINE_TYPE_WITH_PRIVATE(PSAnalyseTask, psanalyse_task, MKT_TYPE_TASK_OBJECT);

static void waitProcessRinsingAfterTime_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    GError *error = NULL;
    if (!lar_timer_default_finish(res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    gdouble after = streams_ultra_get_rinsing_wait_after(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    if (g_timer_elapsed(psanalyse_task->priv->wait_after, NULL) > after) {
        g_task_return_boolean(subTask, TRUE);
        psanalyse_task->priv->complited = TRUE;
        g_object_unref(subTask);
        return;
    }
    lar_timer_default_run(g_task_get_cancellable(subTask), waitProcessRinsingAfterTime_callback, 2.0, subTask);
}

static void afterHoldDone(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }

    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    gboolean       result         = FALSE;
    GError *       error          = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);

        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), _("sequence %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
            sequence_workers_process_get_status(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER())));
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    } else {
        lar_timer_default_run(g_task_get_cancellable(subTask), waitProcessRinsingAfterTime_callback, 2.0, subTask);
    }
}

static void afterRinsingDone_runAfterHold(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }

    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    gboolean       result         = FALSE;
    GError *       error          = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);

        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), _("sequence %s failed - %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
            sequence_workers_process_get_status(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER())));
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    } else {
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(sequence_object_get_workers_process(ULTRA_AXISHOLD_SEQUENCE_WORKER())), 20000);
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_AXISHOLD_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), afterHoldDone, subTask);
        control_was_rinsed();
    }
}

static void rurAfterRinsing(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));

    VesselsObject *drain = ultra_stream_get_drain(psanalyse_task->priv->stream);
    if (psanalyse_task->priv->wait_after) g_timer_destroy(psanalyse_task->priv->wait_after);
    psanalyse_task->priv->wait_after = g_timer_new();
    g_timer_start(psanalyse_task->priv->wait_after);
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), _("run sequence %s"), g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())));
    sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(drain)));
    g_object_unref(drain);
    guint rinsing_repeat = streams_ultra_get_rinsing_count(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    sequence_workers_sample_set_repeat(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()), rinsing_repeat);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER())), 100000);
    sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), afterRinsingDone_runAfterHold, subTask);
}

static void doneInjection_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(subTask), _("ptp - operation failed - %s"), error != NULL ? error->message : "unknown");
        g_task_return_error(subTask, error);
        g_object_unref(subTask); // TODO:Add movement error
        return;
    }
    rurAfterRinsing(subTask);
}

static void isFurnaseClosed_GoYHold_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!vessels_furnace_call_close_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(subTask), "furnace close failed - %s", error != NULL ? error->message : "unknown");
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
    } else {
        GString *commands = g_string_new("");
        g_string_append_printf(commands, "MoveY(1,1,1);SensorY();HoldY(1,1);");
        g_string_append_printf(commands, "MoveX(1,1,1);SensorX();HoldX(1,1);");
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(subTask), doneInjection_callback, subTask);
        g_string_free(commands, TRUE);
    }
}
// Extra rinsing process

static void closeFurnaseAfterRinsing(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    gboolean result = FALSE;
    // PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));

    GError *error = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    vessels_furnace_call_close(vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(subTask), isFurnaseClosed_GoYHold_callback, subTask);
}

static void runProcessRinsing(GTask *subTask) {
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    if (Z_AXIS() == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, _("Z Axis not found")));
        mkt_log_error_message("Stream %s neet z axis to process rinsing start", streams_simple_get_name(streams_object_get_simple(STREAMS_OBJECT(psanalyse_task->priv->stream))));
        g_object_unref(subTask);
        return;
    }
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
    if (injection == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, _("Injection not found")));
        mkt_log_error_message("Stream %s neet injection to process rinsing start", streams_simple_get_name(streams_object_get_simple(STREAMS_OBJECT(psanalyse_task->priv->stream))));
        g_object_unref(subTask);
        return;
    }
    guint    y2Pos      = streams_ultra_get_rinsing_pos_y2(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    guint    zVol       = streams_ultra_get_rinsing_volume(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    // guint    air        = achsen_injection_get_air(injection);
    gdouble  zSteps     = zVol * 2.5;
    gint     replicates = streams_ultra_get_rinsing_replicate(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    GString *commands   = g_string_new("");

    // TODO: Message durch eine Fehler ersetzen
    gdouble curPos  = ((gdouble)achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()))));
    gint    sollPos = achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS())));
    gint    step    = 250;
    gint    volume  = (guint)(curPos + zSteps);
    g_string_append_printf(commands, "MoveY(%d,1,0);", y2Pos);
    int rep = 0;
    for (rep = 1; rep <= replicates; rep++) {
        g_string_append_printf(commands, "V3O();MoveInj(%d,2,0);V3C();", (guint)(volume < 0 ? 0 : volume));
        gint next_volume = volume;
        next_volume = next_volume - step;
        if (next_volume - 75 < sollPos) {
            next_volume = sollPos;
        }
        g_string_append_printf(commands, "MoveInj(%d,1,0);", (guint)(next_volume < 0 ? 0 : next_volume));
        for (; next_volume > sollPos;) {
            next_volume = next_volume - step;
            if (next_volume - 75 < sollPos) {
                next_volume = sollPos;
            }
            g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,1,0);", (guint)(next_volume < 0 ? 0 : next_volume));
        }
        volume = sollPos + zSteps;
    }

    // g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,1,0);", ((guint)(air * 2.5)));
    guint needle_pos = vessels_furnace_get_needle_pos(vessels_object_get_furnace(ULTRA_FURNACE()));
    g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,1,0);MoveY(%d,1,0);",sollPos+250,needle_pos);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 50000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(subTask), closeFurnaseAfterRinsing, subTask);
    g_string_free(commands, TRUE);
}

static void waitDoneAnalyse_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    GError *       error          = NULL;
    if (!lar_timer_default_finish(res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    gboolean is_integrating = FALSE;
    GList *  ch             = NULL;
    for (ch = psanalyse_task->priv->channels; ch != NULL; ch = ch->next) {
        if (ultimate_channel_integration_is_runned(ULTIMATE_CHANNEL(ch->data))) is_integrating = TRUE;
    }
    if (!is_integrating) {
        runProcessRinsing(subTask);
        return;
    }
    if (g_timer_elapsed(psanalyse_task->priv->analyse_timeout, NULL) > psanalyse_task->priv->analyse) {
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), _("Analyse time out "));
        g_task_return_error(
            subTask, g_error_new(control_error_quark(), 1001, _("analyse stream %d timeout"), streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(psanalyse_task->priv->stream)))));
        g_object_unref(subTask);
        return;
    }
    lar_timer_default_run(g_task_get_cancellable(subTask), waitDoneAnalyse_callback, 2.0, subTask);
}

static void onWaitPos_waitAnalyse_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(task));
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), _("run integration"));
    GList *ch = NULL;
    for (ch = psanalyse_task->priv->channels; ch != NULL; ch = ch->next) {
        ultimate_channel_integration(ULTIMATE_CHANNEL(ch->data));
    }
    lar_timer_default_run(g_task_get_cancellable(task), waitDoneAnalyse_callback, 2.0, task);
}

static void injectionDone_goWaitPos(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(task));
    GString *      commands       = g_string_new("");

    guint needle_pos = streams_ultra_get_rinsing_pos_y1(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    g_string_append_printf(commands, "Wait(2.0);MoveY(%d,1,0);", needle_pos);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), onWaitPos_waitAnalyse_callback, task);
    g_string_free(commands, TRUE);
}

static void furnaceIsOpen_codoInjection(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!vessels_furnace_call_open_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(task), "furnace open failed - %s", error != NULL ? error->message : "unknown");
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    } else {

        GString *commands     = g_string_new("");
        guint    injectio_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(ULTRA_FURNACE()));
        g_string_append_printf(commands, "MoveY(%d,1,0);", injectio_pos);
        AchsenObject *   axis_object = Z_AXIS();
        AchsenAchse *    achse       = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
        AchsenInjection *injection   = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
        guint luftpolster = (guint)(achsen_achse_get_hold(achse) + ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rest(injection)) * 2.5));

        gint    injection_pos     = (guint)(achsen_achse_get_hold(achse) + ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection)) * 2.5));
        gdouble volume            = ((gdouble)achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)))) - injection_pos;
        gint    injection_counter = (gint)((volume / 2.5) / 100.0);
        guint   injpar            = achsen_injection_get_injection_stepper_parameter(injection);
        if (injection_counter < 1)
            injection_counter = 1;
        else if (injection_counter > 4)
            injection_counter = 4;
        if (injection_counter == 1) {
            g_string_append_printf(commands, "MoveInj(%d,%d,0);", injection_pos, injpar);
        } else {
            guint injection_volume = (guint)(volume / (gdouble)injection_counter);
            gint  next_volume      = achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)));
            next_volume = next_volume - injection_volume;
            if (next_volume - 75 < injection_pos) {
                next_volume = injection_pos;
            }
            g_string_append_printf(commands, "MoveInj(%d,%d,0);", next_volume, injpar);
            for (; next_volume > injection_pos;) {
                next_volume = next_volume - injection_volume;
                if (next_volume - 75 < injection_pos) {
                    next_volume = injection_pos;
                }
                g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,%d,0);", next_volume, injpar);
            }
        }
        g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,2,0)", luftpolster);
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), injectionDone_goWaitPos, task);
        g_string_free(commands, TRUE);
    }
}

static void furnaceIsOpen_basicInjection(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!vessels_furnace_call_open_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(task), "furnace open failed - %s", error != NULL ? error->message : "unknown");
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    } else {
        GString *commands = g_string_new("");
        guint injectio_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(ULTRA_FURNACE()));
        g_string_append_printf(commands, "MoveY(%d,1,0);", injectio_pos);
        AchsenObject *   axis_object       = Z_AXIS();
        AchsenAchse *    achse             = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
        AchsenInjection *injection         = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
        gdouble          air               = ((((gdouble)achsen_injection_get_air(injection)) * 2.5) / 2.0);
        gint             injection_pos     = achsen_achse_get_hold(achse) + (guint)air;
        gdouble          volume            = ((gdouble)achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)))) - injection_pos;
        gint             injection_counter = (gint)((volume / 2.5) / 100.0);
        guint            injpar            = achsen_injection_get_injection_stepper_parameter(injection);
        if (injection_counter < 1)
            injection_counter = 1;
        else if (injection_counter > 4)
            injection_counter = 4;
        if (injection_counter == 1) {
            g_string_append_printf(commands, "MoveInj(%d,%d,0);", injection_pos, injpar);
        } else {
            guint injection_volume = (guint)(volume / (gdouble)injection_counter);
            gint  next_volume      = achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)));
            next_volume = next_volume - injection_volume;
            if (next_volume - 75 < injection_pos) {
                next_volume = injection_pos;
            }
            g_string_append_printf(commands, "MoveInj(%d,%d,0);", next_volume, injpar);
            for (; next_volume > injection_pos;) {
                next_volume = next_volume - injection_volume;
                if (next_volume - 75 < injection_pos) {
                    next_volume = injection_pos;
                }
                g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,%d,0);", next_volume, injpar);
            }
        }
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), injectionDone_goWaitPos, task);
        g_string_free(commands, TRUE);
    }
}
static void isOnFurnace_OpenFurnace(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(task));
    if (streams_ultra_get_codo_injection(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream))))
        vessels_furnace_call_open(vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), furnaceIsOpen_codoInjection, task);
    else
        vessels_furnace_call_open(vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), furnaceIsOpen_basicInjection, task);
}

static void doneAirFlowAnalyse(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *        subTask        = G_TASK(user_data);
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    if (psanalyse_task->priv->complited || g_cancellable_is_cancelled(g_task_get_cancellable(subTask))) {
        g_object_unref(subTask);
        return;
    }
    GError * error     = NULL;
    gboolean out_value = FALSE;
    if (!airflow_sensor_call_injection_analyse_out_finish(airflow_object_get_sensor(ULTRA_AIRFLOW()), &out_value, res, &error)) {
        if (error) g_error_free(error);
    } else {
        gint stream = streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(psanalyse_task->priv->stream))) - 1;
        if (!out_value)
            mkt_errors_come(E1841 + stream);
        else
            mkt_errors_clean(E1841 + stream);
    }
    g_object_unref(subTask);
}

static void analyzeAirFlow_ForInjectionError(GTask *subTask) {
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "run airflow analyse %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_AIRFLOW())));
    // mkt_error_gone(1840+streams_simple_get_number(streams_object_get_simple(task->priv->stream)));
    airflow_sensor_set_inj_analyse_timeout(airflow_object_get_sensor(ULTRA_AIRFLOW()), 20.0);
    airflow_sensor_call_injection_analyse_out(airflow_object_get_sensor(ULTRA_AIRFLOW()), g_task_get_cancellable(subTask), doneAirFlowAnalyse, g_object_ref(subTask));
}

static void injectionRun(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    GString *commands = g_string_new("");
    if (achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()))) != achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))) {
        g_string_append_printf(commands, "MoveY(1,1,1);SensorY();HoldY(1,1);");
    }
    guint furnace_pos = vessels_simple_get_pos_xachse(vessels_object_get_simple(ULTRA_FURNACE()));
    guint needle_pos  = vessels_furnace_get_needle_pos(vessels_object_get_furnace(ULTRA_FURNACE()));
    g_string_append_printf(commands, "MoveX(100,1,1);HoldX(1,1);SensorX();MoveX(%d,1,4);MoveY(%d,1,1);", furnace_pos, needle_pos);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 20000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(subTask), isOnFurnace_OpenFurnace, subTask);
    g_string_free(commands, TRUE);
    analyzeAirFlow_ForInjectionError(subTask);
}

static void psJustificationDone(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));

    airflow_sensor_set_smooth(airflow_object_get_sensor(ULTRA_AIRFLOW()), FALSE);
    GList *ch                             = NULL;
    psanalyse_task->priv->airflow_current = airflow_sensor_get_air_out(airflow_object_get_sensor(ULTRA_AIRFLOW()));
    for (ch = psanalyse_task->priv->channels; ch != NULL; ch = ch->next) {
        ultimate_channel_calculate_justification(ULTIMATE_CHANNEL(ch->data));
    }
    injectionRun(subTask);
}

static void psAnalyseSart(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));

    GList *ch = NULL;
    for (ch = psanalyse_task->priv->channels; ch != NULL; ch = ch->next) {
        ultimate_channel_start_analyse(ULTIMATE_CHANNEL(ch->data));
        ultimate_channel_justification(ULTIMATE_CHANNEL(ch->data));
    }
    psanalyse_task->priv->analyse_timeout = g_timer_new();
    g_timer_start(psanalyse_task->priv->analyse_timeout);
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "run justification");
    gdouble just_time = 15.0;
    lar_timer_default_run(g_task_get_cancellable(subTask), psJustificationDone, just_time, subTask);
    airflow_sensor_set_smooth(airflow_object_get_sensor(ULTRA_AIRFLOW()), TRUE);
}

static void psSamplingDone(GObject *source_object, GAsyncResult *res, gpointer user_data) {

    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    gboolean       result         = FALSE;
    GError *       error          = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "sequence %s failed - %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(source_object)), error != NULL ? error->message : "unknown");
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "done");
    psanalyse_task->priv->sampling_done = TRUE;
    psAnalyseSart(subTask);
    mkt_task_object_free_res (MKT_TASK_OBJECT(psanalyse_task),1);
}

static void psSamplingRun(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task   = PSANALYSE_TASK(g_task_get_source_object(subTask));
    gint           injection_volume = streams_ultra_get_injection_volume(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    VesselsObject *sample           = NULL;
    VesselsObject *drain            = ultra_stream_get_drain(psanalyse_task->priv->stream);
    if (streams_ultra_get_is_dilution(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)))) {
        sample = g_object_ref(ULTRA_VESSEL6());
    } else {
        sample = ultra_stream_get_sample(psanalyse_task->priv->stream);
    }
    if (streams_ultra_get_codo_injection(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)))) {
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "run sequence %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_SAMPLINGCOD_SEQUENCE_WORKER())));
        sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_SAMPLINGCOD_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
        sequence_workers_sample_set_volume(sequence_object_get_workers_sample(ULTRA_SAMPLINGCOD_SEQUENCE_WORKER()), injection_volume);
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_SAMPLINGCOD_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), psSamplingDone, subTask);
    } else {
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "run sequence %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_SAMPLING_SEQUENCE_WORKER())));
        sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_SAMPLING_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
        sequence_workers_sample_set_volume(sequence_object_get_workers_sample(ULTRA_SAMPLING_SEQUENCE_WORKER()), injection_volume);
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_SAMPLING_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), psSamplingDone, subTask);
    }
    control_need_hold();
    control_need_rinsing(drain);
    g_object_unref(sample);
    g_object_unref(drain);
}

static void psDilutionDone(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));
    gboolean       result         = FALSE;
    GError *       error          = NULL;
    if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error)) {
        if (error) g_dbus_error_strip_remote_error(error);
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "sequence %s failed - %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(source_object)), error != NULL ? error->message : "unknown");
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    psSamplingRun(subTask);
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "wait sampling");
}
static void psStabilizationDone(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    PSAnalyseTask *psanalyse_task            = PSANALYSE_TASK(g_task_get_source_object(subTask));
    psanalyse_task->priv->stabilization_done = TRUE;

    if (streams_ultra_get_is_dilution(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream))) &&
        streams_ultra_get_on_replicte(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)))) {
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "run sequence %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_DILUTION_SEQUENCE_WORKER())));
        VesselsObject *sample = ultra_stream_get_sample(psanalyse_task->priv->stream);
        VesselsObject *drain  = ultra_stream_get_drain(psanalyse_task->priv->stream);
        sequence_workers_sample_set_sample_main(sequence_object_get_workers_sample(ULTRA_DILUTION_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
        sequence_workers_sample_set_sample_second(sequence_object_get_workers_sample(ULTRA_DILUTION_SEQUENCE_WORKER()), g_dbus_object_get_object_path(G_DBUS_OBJECT(drain)));
        g_object_unref(sample);
        g_object_unref(drain);
        StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(STREAMS_OBJECT(psanalyse_task->priv->stream)));
        sequence_workers_dilution_set_fill_time(sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()), (gdouble)streams_ultra_get_dilution_pump_time(ultra));
        sequence_workers_dilution_set_dilution_time(sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()), streams_ultra_get_dilution_wait_time(ultra));
        sequence_workers_dilution_set_proportion(sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()), streams_ultra_get_dilution_factor(ultra));
        sequence_workers_process_call_run(sequence_object_get_workers_process(ULTRA_DILUTION_SEQUENCE_WORKER()), g_task_get_cancellable(subTask), psDilutionDone, subTask);
        control_need_hold();
        control_need_rinsing(drain);
    } else {
        psSamplingRun(subTask);
        mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "wait sampling");
    }
}

static gboolean psanalyse_task_start(GTask *subTask) {

    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));

    psanalyse_task->priv->sampling_done      = FALSE;
    psanalyse_task->priv->stabilization_done = FALSE;
    gdouble stabilization_time               = streams_ultra_get_delay(streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream)));
    mkt_task_object_set_status(MKT_TASK_OBJECT(psanalyse_task), "gas stabilization");
    airflow_sensor_set_furnace_way(airflow_object_get_sensor(ULTRA_AIRFLOW()), FALSE);
    lar_timer_default_run(g_task_get_cancellable(subTask), psStabilizationDone, stabilization_time, subTask);
    return TRUE;
}

static gboolean check_ps_task_parameter(GTask *subTask) {
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(subTask));

    if (psanalyse_task->priv->stream == NULL || !ULTRA_IS_STREAM_OBJECT(psanalyse_task->priv->stream)) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "stream object not found"));
        g_object_unref(subTask);
        return FALSE;
    }
    StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(psanalyse_task->priv->stream));
    if (ultra == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "task stream ultra interface not found"));
        g_object_unref(subTask);
        return FALSE;
    }
    VesselsObject *drain = ultra_stream_get_drain(psanalyse_task->priv->stream);
    if (drain == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "drain vessel %s not found", streams_ultra_get_drain_vessel(ultra)));
        g_object_unref(subTask);
        return FALSE;
    }
    g_object_unref(drain);
    VesselsObject *sample = ultra_stream_get_sample(psanalyse_task->priv->stream);
    if (sample == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "sample vessel not found"));
        g_object_unref(subTask);
        return FALSE;
    }
    g_object_unref(sample);
    if (psanalyse_task->priv->channels == NULL) {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, "activated channels not found"));
        // TODO: Message durch eine Fehler ersetzen
        mkt_log_error_message("Stream %s activated channels not found", streams_simple_get_name(streams_object_get_simple(STREAMS_OBJECT(psanalyse_task->priv->stream))));
        g_object_unref(subTask);
        return FALSE;
    }

    return TRUE;
}
static void analyseps_stop_intern(PSAnalyseTask *psanalyse_task) {
    if (psanalyse_task->priv->stoped) return;
    psanalyse_task->priv->stoped = TRUE;
    airflow_sensor_set_smooth(airflow_object_get_sensor(ULTRA_AIRFLOW()), FALSE);

    if (psanalyse_task && PSANALYSE_IS_TASK(psanalyse_task)) {
        GList *ch = NULL;
        for (ch = psanalyse_task->priv->channels; ch != NULL; ch = ch->next) {
            ultimate_channel_analyse_stop(ULTIMATE_CHANNEL(ch->data));
            channels_simple_set_measure(channels_object_get_simple(CHANNELS_OBJECT(ch->data)), FALSE);
        }
    }
    airflow_sensor_set_furnace_way(airflow_object_get_sensor(ULTRA_AIRFLOW()), FALSE);
}

static void psanalyse_cancelled(GCancellable *cancel, GTask *task) {
    PSAnalyseTask *psanalyse_task = PSANALYSE_TASK(g_task_get_source_object(task));
    analyseps_stop_intern(psanalyse_task);
}

static void psanalyse_task_run(MktTaskObject *task, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *        subTask            = g_task_new(task, cancellable, callback, user_data);
    PSAnalyseTask *psanalyse_task     = PSANALYSE_TASK(task);
    psanalyse_task->priv->cancellable = g_object_ref(cancellable);
    psanalyse_task->priv->complited   = FALSE;
    psanalyse_task->priv->handler_id  = g_cancellable_connect(psanalyse_task->priv->cancellable, G_CALLBACK(psanalyse_cancelled), subTask, NULL);
    if (check_ps_task_parameter(subTask)) {
        psanalyse_task_start(subTask);
    }
}

static gboolean psanalyse_task_finish(MktTaskObject *task, GTask *res, GError **error) {
    g_return_val_if_fail(task != NULL, FALSE);
    g_return_val_if_fail(res != NULL, FALSE);
    g_return_val_if_fail(G_IS_TASK(res), FALSE);
    PSAnalyseTask *psanalyse_task   = PSANALYSE_TASK(task);
    psanalyse_task->priv->complited = TRUE;
    analyseps_stop_intern(psanalyse_task);
    gboolean result = g_task_propagate_boolean(G_TASK(res), error);
    return result;
}

static void psanalyse_task_init(PSAnalyseTask *psanalyse_task) {
    psanalyse_task->priv = psanalyse_task_get_instance_private(psanalyse_task);
    tera_pumps_manager_client_new();
    ultra_vessels_manager_client_new();

    psanalyse_task->priv->channels        = NULL;
    psanalyse_task->priv->stream          = NULL;
    psanalyse_task->priv->analyse_timeout = NULL;
    psanalyse_task->priv->stoped          = FALSE;
    /* TODO: Add initialization code here */
}

static void psanalyse_task_finalize(GObject *object) {
    /* TODO: Add deinitalization code here */
    PSAnalyseTask *task = PSANALYSE_TASK(object);
    if (task->priv->cancellable) {
        g_cancellable_disconnect(task->priv->cancellable, task->priv->handler_id);
        g_object_unref(task->priv->cancellable);
    }
    if (task->priv->channels) g_list_free(task->priv->channels);
    if (task->priv->stream) g_object_unref(task->priv->stream);
    if (task->priv->analyse_timeout) g_timer_destroy(task->priv->analyse_timeout);
    G_OBJECT_CLASS(psanalyse_task_parent_class)->finalize(object);
}

static void psanalyse_task_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    PSAnalyseTask *task = PSANALYSE_TASK(object);
    switch (prop_id) {
    case PSANALYSE_STREAM:
        if (task->priv->stream) g_object_unref(task->priv->stream);
        task->priv->stream = g_value_dup_object(value);
        break;
    case PSANALYSE_TIME:
        task->priv->analyse = g_value_get_double(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void psanalyse_task_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    PSAnalyseTask *task = PSANALYSE_TASK(object);
    switch (prop_id) {
    case PSANALYSE_STREAM:
        g_value_set_object(value, task->priv->stream);
        break;
    case PSANALYSE_TIME:
        g_value_set_double(value, task->priv->analyse);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void psanalyse_task_class_init(PSAnalyseTaskClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // object_class->dispose           = psanalyse_atom_dispose;
    object_class->finalize               = psanalyse_task_finalize;
    object_class->set_property           = psanalyse_task_set_property;
    object_class->get_property           = psanalyse_task_get_property;
    MKT_TASK_OBJECT_CLASS(klass)->run    = psanalyse_task_run;
    MKT_TASK_OBJECT_CLASS(klass)->finish = psanalyse_task_finish;

    g_object_class_install_property(object_class, PSANALYSE_STREAM,
        g_param_spec_object("analyze-stream", "PSAnalyse stream object", "PSAnalyse stream object", STREAMS_TYPE_OBJECT, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(
        object_class, PSANALYSE_TIME, g_param_spec_double("analyze-timeout", "PSAnalyse timeout", "PSAnalyse timeout", 250.0, 2000.0, 700.0, G_PARAM_WRITABLE | G_PARAM_READABLE));
}

void psanalyse_task_set_channels(PSAnalyseTask *psanalyse, GList *channels) {
    g_return_if_fail(psanalyse != NULL);
    g_return_if_fail(psanalyse != NULL);

    if (psanalyse->priv->channels) g_list_free(psanalyse->priv->channels);
    psanalyse->priv->channels = g_list_copy(channels);
}

gdouble psanalyse_justification_airflow(PSAnalyseTask *task) {
    g_return_val_if_fail(task != NULL, FALSE);
    return task->priv->airflow_current;
}
