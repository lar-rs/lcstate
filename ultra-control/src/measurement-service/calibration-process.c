/*
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "calibration-process.h"

#include <ultimate-library.h>

#include "../../config.h"

#include "analyze-task.h"
#include "prepare-task.h"
#include "ultra-channel-object.h"
#include "ultra-control-process.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"

#include <glib/gi18n-lib.h>
struct _CalibrationProcessPrivate {
    GList * calibrations_channels;
    GList * meas_TC;
    GList * meas_TIC;
    GList * meas_channels;
    guint   solution;
    GArray *array_airflow;
    gulong  next_sol_handlerId;
    gulong  activate_handlerId;
};

enum {
    CALIBRATION_PROP0,

};

#define CALIBRATION_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CALIBRATION_TYPE_PROCESS, CalibrationProcessPrivate))

G_DEFINE_TYPE(CalibrationProcess, calibration_process, ULTIMATE_TYPE_PROCESS_OBJECT)

static void calibration_process_set_analyze(CalibrationProcess *process, GList *channels, gboolean analyze) {
    GList *chl = NULL;
    for (chl = channels; chl != NULL; chl = chl->next) {
        ChannelsCalibration *cal = channels_object_get_calibration(CHANNELS_OBJECT(chl->data));
        if (cal) channels_calibration_set_measure(cal, analyze);
    }
}
static void calibration_process_break(CalibrationProcess *process) {
    calibration_process_set_analyze(process, process->priv->calibrations_channels, FALSE);
    process_simple_set_wait_user_next(process_object_get_simple(PROCESS_OBJECT(process)), FALSE);
}

static gboolean calibration_start_calibration(CalibrationProcess *calibration) {
    if (calibration->priv->calibrations_channels) g_list_free(calibration->priv->calibrations_channels);
    calibration->priv->calibrations_channels = NULL;
    MktProcess *process                      = mkt_process_object_get_original(MKT_PROCESS_OBJECT(calibration));
    if (process == NULL) {
        mkt_process_object_critical(MKT_PROCESS_OBJECT(calibration), "Process is defect - original data model not found (mkt_process_object_get_original == NULL)");
        return FALSE;
    }
    GList *channels = mkt_process_object_channels(MKT_PROCESS_OBJECT(calibration));
    GList *chl      = NULL;
    gint   status   = 0;
    for (chl = channels; chl != NULL; chl = chl->next) {
        status = ultra_channel_start_calibration(ULTRA_CHANNEL_OBJECT(chl->data), MKT_PROCESS_OBJECT(calibration));
        if (status == 1)
            calibration->priv->calibrations_channels = g_list_append(calibration->priv->calibrations_channels, chl->data);
        else if (status < 0)
            break;
    }
    return calibration->priv->calibrations_channels != NULL && status >= 0;
}

static void calibration_transmit_autocal(CalibrationProcess *process) {
    GList *chl = NULL;
    for (chl = process->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        ultra_channel_activate_calibration(ULTRA_CHANNEL_OBJECT(chl->data), FALSE);
    }
}

static gboolean calibration_transmit_value(CalibrationProcess *process) {
    gboolean fail         = FALSE;
    gboolean autocal_done = TRUE;
    if (process->priv->calibrations_channels == NULL) fail = TRUE;
    GList *        chl    = NULL;
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
    for (chl = process->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        MktCalibration *last   = ultra_channel_calibration_model(ULTRA_CHANNEL_OBJECT(chl->data));
        MktCalibration *grund  = mkt_calibration_main_for_channel(channels_simple_get_link(channels_object_get_simple(CHANNELS_OBJECT(chl->data))));
        MktCalibration *active = mkt_calibration_activated_for_channel(channels_simple_get_link(channels_object_get_simple(CHANNELS_OBJECT(chl->data))));

        MktCalPoint *cal_point = mkt_calibration_max_cv(last);
        if (last == NULL || cal_point == NULL) {
            fail = TRUE;
        } else {
            mkt_calibration_update_time(last);
            mkt_calibration_calculate(last);
            ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
            gdouble        max_cv = process_simple_get_max_cv(simple);
            g_object_set(last, "calibration-done", TRUE, "calibration-cv", max_cv, NULL);

            if (max_cv < mkt_cal_point_cv(cal_point)) {
                fail = TRUE;
            }
            StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
            if (stream) {
                if (grund) {
                    gdouble defference    = mkt_calibration_max_deviation(last, mkt_calibration_slope(grund), mkt_calibration_intercept(grund));
                    gdouble max_deviation = streams_ultra_get_allowed_deviation(streams_object_get_ultra(STREAMS_OBJECT(stream)));
                    if (max_deviation < defference) fail = TRUE;
                    g_object_set(last, "calibration-deviation", max_deviation, NULL);
                }
                if (active) {
                    gdouble defference  = (gdouble)ABS(100. - ((mkt_calibration_slope(last) / mkt_calibration_slope(active)) * 100.));
                    gdouble max_autocal = streams_ultra_get_autocal_deviation(streams_object_get_ultra(STREAMS_OBJECT(stream)));
                    if (max_autocal < defference) autocal_done = FALSE;
                }
            }
        }
        if (active) g_object_unref(active);
        if (grund) g_object_unref(grund);

        if (cal_point) g_object_unref(cal_point);
    }
    if ((!fail) && process_simple_get_is_online(simple) && autocal_done) {
        calibration_transmit_autocal(process);
        return TRUE;
    } else if (!process_simple_get_is_online(simple))
        process_simple_set_can_activation(process_object_get_simple(PROCESS_OBJECT(process)), (!fail));

    return process_simple_get_is_online(simple);
}

static void calibration_transmit_integration_value(CalibrationProcess *process) {
    GList *channels = NULL;
    for (channels = process->priv->meas_channels; channels != NULL; channels = channels->next) {
        ultra_channel_transmit_integration(ULTRA_CHANNEL_OBJECT(channels->data), MKT_PROCESS_OBJECT(process));
        channels_calibration_emit_update(channels_object_get_calibration(CHANNELS_OBJECT(channels->data)));
    }
}

static void calibration_recalculate_statistic_value(CalibrationProcess *process) {
    GList *channels = NULL;
    for (channels = process->priv->meas_channels; channels != NULL; channels = channels->next) {
        ultra_channel_calibration_calculate_statistic(ULTRA_CHANNEL_OBJECT(channels->data), MKT_PROCESS_OBJECT(process));
    }
}

static gboolean calibration_process_realize_channels(CalibrationProcess *calibration) {
    if (calibration->priv->meas_TC) g_list_free(calibration->priv->meas_TC);
    if (calibration->priv->meas_TIC) g_list_free(calibration->priv->meas_TIC);
    if (calibration->priv->meas_channels) g_list_free(calibration->priv->meas_channels);
    calibration->priv->meas_TC       = NULL;
    calibration->priv->meas_TIC      = NULL;
    calibration->priv->meas_channels = NULL;
    GList *chl                       = NULL;
    for (chl = calibration->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        ChannelsSimple *simple = NULL;
        simple                 = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
        if (ultra_channel_calibration_current_point(ULTRA_CHANNEL_OBJECT(chl->data)) != NULL && !mkt_cal_point_done(ultra_channel_calibration_current_point(ULTRA_CHANNEL_OBJECT(chl->data)))) {
            calibration->priv->meas_channels = g_list_append(calibration->priv->meas_channels, chl->data);
            if (!channels_simple_get_tic(simple))
                calibration->priv->meas_TC = g_list_append(calibration->priv->meas_TC, chl->data);
            else
                calibration->priv->meas_TIC = g_list_append(calibration->priv->meas_TIC, chl->data);
        }
    }
    return calibration->priv->meas_channels != NULL;
}

static void calibration_process_analyse_done(GTask *subTask);
static void calibration_run_analyse(GTask *subTask);
static void calibration_process_clean(CalibrationProcess *calibration) {
    GList *chl = NULL;
    for (chl = calibration->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        ultimate_channel_clean(ULTIMATE_CHANNEL(chl->data));
    }
    process_simple_set_wait_activation(process_object_get_simple(PROCESS_OBJECT(calibration)), FALSE);
    MktProcess *process_model = mkt_process_object_get_original(MKT_PROCESS_OBJECT(calibration));
    if (process_model) g_object_set(process_model, "process-wait-action", TRUE, NULL);
}

static gboolean calibration_is_more_points(CalibrationProcess *calibration_process) {
    gboolean result = FALSE;
    GList *  chl    = NULL;
    for (chl = calibration_process->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        if (ultra_channel_calibration_next_point(ULTRA_CHANNEL_OBJECT(chl->data))) result = TRUE;
    }
    return result;
}

static void calibration_next_user_solution(CalibrationProcess *calibration_process, GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    if (calibration_process->priv->next_sol_handlerId) g_signal_handler_disconnect(calibration_process, calibration_process->priv->next_sol_handlerId);
    ProcessSimple *psimple = process_object_get_simple(PROCESS_OBJECT(calibration_process));
    if (!process_simple_get_wait_user_next(psimple)) {
        g_object_unref(psimple);
        return;
    }
    process_simple_set_wait_user_next(psimple, FALSE);
    process_simple_set_current_replicate(psimple, 1);
    GList *chl = NULL;
    for (chl = calibration_process->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        ultra_channel_calibration_next_point(ULTRA_CHANNEL_OBJECT(chl->data));
    }
    calibration_process_realize_channels(calibration_process);
    if (calibration_process->priv->meas_TC || calibration_process->priv->meas_TIC) {
        calibration_run_analyse(subTask);
    } else {
        mkt_process_object_status(MKT_PROCESS_OBJECT(calibration_process), _("Next solution fail"));
        mkt_process_object_critical(MKT_PROCESS_OBJECT(calibration_process), "Solution not found");
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, _("Solution not found")));
    }
}

static gboolean calibration_process_activate_handle_callback(ProcessSimple *interface, GDBusMethodInvocation *invocation, CalibrationProcess *calibration) {
    MktProcessObject *process = MKT_PROCESS_OBJECT(calibration);
    if (!process_simple_get_wait_activation(process_object_get_simple(PROCESS_OBJECT(calibration)))) return TRUE;
    GList * chl      = NULL;
    gdouble air_flow = airflow_sensor_get_soll_value(airflow_object_get_sensor(ULTRA_AIRFLOW()));
    if (calibration->priv->array_airflow!= NULL) {
        air_flow = 0.0;
        guint i = 0;
        for (i = 0; i < calibration->priv->array_airflow->len; i++) {
            air_flow += g_array_index(calibration->priv->array_airflow, gdouble, i);
        }
        air_flow /= calibration->priv->array_airflow->len;
    }
    g_array_free(calibration->priv->array_airflow, TRUE);
    calibration->priv->array_airflow = NULL;

    airflow_sensor_set_soll_value(airflow_object_get_sensor(ULTRA_AIRFLOW()), air_flow);
    for (chl = calibration->priv->calibrations_channels; chl != NULL; chl = chl->next) {
        ultra_channel_activate_calibration(ULTRA_CHANNEL_OBJECT(chl->data), security_device_get_level3(TERA_GUARD()));
    }
    process_simple_set_wait_activation(process_object_get_simple(PROCESS_OBJECT(calibration)), FALSE);
    mkt_process_object_update_stop_time(MKT_PROCESS_OBJECT(calibration));
    process_simple_emit_update(process_object_get_simple(PROCESS_OBJECT(process)));
    return TRUE;
}

static gboolean calibration_process_clean_handle_callback(ProcessSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    MktProcessObject *process = MKT_PROCESS_OBJECT(user_data);
    if (!process_simple_get_wait_activation(process_object_get_simple(PROCESS_OBJECT(process)))) return TRUE;
    calibration_process_clean(CALIBRATION_PROCESS(process));
    mkt_process_object_update_stop_time(MKT_PROCESS_OBJECT(process));
    return TRUE;
}

void calibration_process_analyse_done(GTask *subTask) {
    CalibrationProcess *calibration = CALIBRATION_PROCESS(g_task_get_source_object(subTask));

    calibration_process_set_analyze(calibration, calibration->priv->calibrations_channels, FALSE);
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(calibration));
    calibration_transmit_integration_value(calibration);
    calibration_recalculate_statistic_value(calibration);
    if (calibration_process_realize_channels(calibration)) {
        process_simple_set_current_replicate(simple, (process_simple_get_current_replicate(simple) + 1));
        calibration_run_analyse(subTask);
        return;
    }
    if (calibration_is_more_points(calibration)) {
        if (process_simple_get_is_online(process_object_get_simple(PROCESS_OBJECT(calibration)))) {
            mkt_process_object_critical(MKT_PROCESS_OBJECT(calibration), "Autokaliebrirung kann nicht gestartet werden - Mehrpunktkalibrierung");
            g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, _("Autokaliebrirung kann nicht gestartet werden - Mehrpunktkalibrierung")));
        } else {
            process_simple_set_wait_user_next(process_object_get_simple(PROCESS_OBJECT(calibration)), TRUE);
            g_signal_handlers_disconnect_by_func(calibration, calibration_next_user_solution, calibration);
            if (calibration->priv->next_sol_handlerId) g_signal_handler_disconnect(calibration, calibration->priv->next_sol_handlerId);
            calibration->priv->next_sol_handlerId = g_signal_connect(calibration, "process-will-be-next", G_CALLBACK(calibration_next_user_solution), subTask);
            mkt_process_object_status(MKT_PROCESS_OBJECT(calibration), _("wait next solution"));
            process_simple_emit_update(process_object_get_simple(PROCESS_OBJECT(calibration)));
        }
    } else {
        if (!calibration_transmit_value(calibration)) {
            process_simple_set_wait_activation(process_object_get_simple(PROCESS_OBJECT(calibration)), TRUE);
            mkt_process_object_status(MKT_PROCESS_OBJECT(calibration), _("wait activation"));
            process_simple_emit_update(process_object_get_simple(PROCESS_OBJECT(calibration)));
        }
        process_simple_emit_update(simple);
        g_task_return_boolean(subTask, TRUE);
    }

    return;
}

static void analyze_TIC_task_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    GError *error = NULL;
    if (!mkt_task_object_finish(MKT_TASK_OBJECT(source_object), res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    CalibrationProcess *calibration_process = CALIBRATION_PROCESS(g_task_get_source_object(subTask));
    calibration_process_set_analyze(calibration_process, calibration_process->priv->meas_TIC, FALSE);
    calibration_process_analyse_done(subTask);
}

static void analyze_TC_task_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    GError *error = NULL;
    if (!mkt_task_object_finish(MKT_TASK_OBJECT(source_object), res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    CalibrationProcess *calibration_process = CALIBRATION_PROCESS(g_task_get_source_object(subTask));
    calibration_process_set_analyze(calibration_process, calibration_process->priv->meas_TC, FALSE);
    gdouble airflow                          = analyse_justification_airflow(ANALYZE_TASK(source_object));
    calibration_process->priv->array_airflow = g_array_append_val(calibration_process->priv->array_airflow, airflow);

    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(calibration_process)));
    if (calibration_process->priv->meas_TIC != NULL) {
        MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(ANALYZE_TYPE_TASK, "name", "TC-Analyze", "analyze-is-tic", TRUE, "analyze-stream", stream, "analyze-timeout", 600., NULL));
        calibration_process_set_analyze(calibration_process, calibration_process->priv->meas_TIC, TRUE);
        analyze_task_set_channels(ANALYZE_TASK(analyze), calibration_process->priv->meas_TIC);
        mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TIC_task_done, subTask);
        g_object_unref(analyze);
    } else {
        calibration_process_analyse_done(subTask);
    }
}

void calibration_run_analyse(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }

    CalibrationProcess *calibration_process = CALIBRATION_PROCESS(g_task_get_source_object(subTask));
    UltraStreamObject * stream              = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(calibration_process)));
    if (calibration_process->priv->meas_TC != NULL) {
        MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(ANALYZE_TYPE_TASK, "name", "TC-Analyze", "analyze-is-tic", FALSE, "analyze-stream", stream, "analyze-timeout", 600., NULL));
        calibration_process_set_analyze(calibration_process, calibration_process->priv->meas_TC, TRUE);
        analyze_task_set_channels(ANALYZE_TASK(analyze), calibration_process->priv->meas_TC);
        mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TC_task_done, subTask);
        g_object_unref(analyze);
        process_simple_emit_update(process_object_get_simple(PROCESS_OBJECT(calibration_process)));
    } else if (calibration_process->priv->meas_TIC != NULL) {
        MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(ANALYZE_TYPE_TASK, "name", "TC-Analyze", "analyze-is-tic", TRUE, "analyze-stream", stream, "analyze-timeout", 600., NULL));
        calibration_process_set_analyze(calibration_process, calibration_process->priv->meas_TIC, TRUE);
        analyze_task_set_channels(ANALYZE_TASK(analyze), calibration_process->priv->meas_TIC);
        mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TIC_task_done, subTask);
        g_object_unref(analyze);
        process_simple_emit_update(process_object_get_simple(PROCESS_OBJECT(calibration_process)));
    } else {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, _("Measurement process channels not found")));
        g_object_unref(subTask);
        return;
    }
}

static void prepare_task_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    GError *error = NULL;
    if (!mkt_task_object_finish(MKT_TASK_OBJECT(source_object), res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    CalibrationProcess *calibration_process = CALIBRATION_PROCESS(g_task_get_source_object(subTask));
    if (calibration_process->priv->array_airflow) g_array_free(calibration_process->priv->array_airflow, TRUE);
    calibration_process->priv->array_airflow = g_array_new(TRUE, TRUE, sizeof(gdouble));

    calibration_start_calibration(calibration_process);
    calibration_process_realize_channels(calibration_process);
    calibration_run_analyse(subTask);
}

static void calibration_process_run(MktProcessObject *process, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *            subTask = g_task_new(process, cancellable, callback, user_data);
    UltraStreamObject *stream  = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    ultra_stream_set_state(stream, mkt_process_object_get_state(process));
    ultra_stream_no_sampling(stream);
    MktTaskObject *prepare = MKT_TASK_OBJECT(g_object_new(PREPARE_TYPE_TASK, "name", "Prepare", "prepare-stream", stream, NULL));
    mkt_task_object_run(prepare, g_task_get_cancellable(subTask), prepare_task_done, subTask);
    g_object_unref(prepare);
}

static gboolean calibration_process_finish(MktProcessObject *process, GTask *task, GError **error) {
    CalibrationProcess *calibration_process = CALIBRATION_PROCESS(process);
    g_signal_handlers_disconnect_by_func(process, calibration_next_user_solution, calibration_process);
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));

    ultra_stream_on_sampling(stream);
    calibration_process_break(calibration_process);
    return g_task_propagate_boolean(task, error);
}

static void calibration_process_init(CalibrationProcess *calibration_process) {
    calibration_process->priv                        = CALIBRATION_PROCESS_PRIVATE(calibration_process);
    calibration_process->priv->meas_TC               = NULL;
    calibration_process->priv->meas_TIC              = NULL;
    calibration_process->priv->array_airflow         = NULL;
    calibration_process->priv->calibrations_channels = NULL;
    calibration_process->priv->next_sol_handlerId    = 0;
    calibration_process->priv->activate_handlerId    = 0;

    /* TODO: Add initialization code here */
}

static void calibration_process_finalize(GObject *object) {
    /* TODO: Add deinitalization code here */
    CalibrationProcess *process = CALIBRATION_PROCESS(object);
    if (process->priv->meas_TC) g_list_free(process->priv->meas_TC);
    if (process->priv->meas_TIC) g_list_free(process->priv->meas_TIC);
    if (process->priv->calibrations_channels) g_list_free(process->priv->calibrations_channels);
    if (process->priv->array_airflow) g_array_free(process->priv->array_airflow, TRUE);
    if (process->priv->next_sol_handlerId) g_signal_handler_disconnect(process, process->priv->next_sol_handlerId);
    G_OBJECT_CLASS(calibration_process_parent_class)->finalize(object);
}

static void calibration_process_constructed(GObject *object) {
    /* TODO: Add deinitalization code here */
    //
    if (G_OBJECT_CLASS(calibration_process_parent_class)->constructed) G_OBJECT_CLASS(calibration_process_parent_class)->constructed(object);
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(object));
    if (simple) {
        g_signal_connect(simple, "handle-clean", G_CALLBACK(calibration_process_clean_handle_callback), object);
        g_signal_connect(simple, "handle-activate", G_CALLBACK(calibration_process_activate_handle_callback), object);
    }

    /*MktModel *pmodel = mkt_model_select_one(MKT_TYPE_PROCESS_MODEL,"select * from $tablename where process_path = '%s' ORDER BY ref_id DESC LIMIT
    1",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)));
    if(pmodel&&simple)
    {
            process_simple_set_wait_activation(simple,mkt_process_wait_action(MKT_PROCESS(pmodel)));
            g_object_unref(pmodel);
    }*/
}

static void calibration_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    // CalibrationProcess *process = CALIBRATION_PROCESS( object );
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void calibration_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    // CalibrationProcess *process = CALIBRATION_PROCESS( object );
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/* signals */

static void calibration_process_class_init(CalibrationProcessClass *klass) {
    GObjectClass *         object_class = G_OBJECT_CLASS(klass);
    MktProcessObjectClass *pclass       = MKT_PROCESS_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(CalibrationProcessClass));
    // object_class->dispose              = calibration_atom_dispose;
    object_class->finalize     = calibration_process_finalize;
    object_class->set_property = calibration_process_set_property;
    object_class->get_property = calibration_process_get_property;
    object_class->constructed  = calibration_process_constructed;
    pclass->run                = calibration_process_run;
    pclass->finish             = calibration_process_finish;

    /*	calibration_action_signals[PROCESS_RUN] =
                            g_signal_new ("process-run",
                                            G_TYPE_FROM_CLASS (klass),
                                            G_SIGNAL_RUN_LAST ,
                                            0,
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE,
                                            0,
                                            G_TYPE_NONE);*/
}
