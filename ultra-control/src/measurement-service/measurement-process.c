/*
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "measurement-process.h"
#include <ultimate-library.h>
#include "analyze-task.h"
#include "prepare-task.h"
#include "psanalyse-task.h"
#include "ultra-channel-object.h"
#include "ultra-control-process.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _MeasurementProcessPrivate {
    GList *       values_TC;
    GList *       values_TIC;
    GList *       values_TOC;
    GList *       measurement_channels;
    GArray *      amount;
    NodesObject * digital;
    MktStatus *   wait_status;
    GTimer *      prepare_timer;
    gdouble       prepare_time;
    GCancellable *prepare;
    gboolean      preparation_run;
    gboolean      preparation_done;
    gdouble       sampling_time;
    gdouble       stripping_time;
    GTask *       sub_task;
    GTimer *      sampling_timer;
};

enum {
    MEASUREMENT_PROP0,

};

#define MEASUREMENT_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), MEASUREMENT_TYPE_PROCESS, MeasurementProcessPrivate))

G_DEFINE_TYPE(MeasurementProcess, measurement_process, ULTIMATE_TYPE_PROCESS_OBJECT)

static void measurement_process_set_analyze(MeasurementProcess *process, GList *channels, gboolean analyze) {
    GList *chl = NULL;
    for (chl = channels; chl != NULL; chl = chl->next) {
        ChannelsMeasurement *measurement = channels_object_get_measurement(CHANNELS_OBJECT(chl->data));
        if (measurement) channels_measurement_set_measure(measurement, analyze);
    }
}

static void measurement_process_break(MeasurementProcess *process){
    measurement_process_set_analyze(process, process->priv->measurement_channels, FALSE);
}

static void measurement_start_measurement(MeasurementProcess *process) {
    GList *chl = NULL;
    for (chl = process->priv->measurement_channels; chl != NULL; chl = chl->next) {
        ultimate_channel_start_measurement(ULTIMATE_CHANNEL(chl->data));
    }
}

static void measurement_next_measurement(MeasurementProcess *process) {
    GList *chl = NULL;
    for (chl = process->priv->measurement_channels; chl != NULL; chl = chl->next) {
        ultimate_channel_next_measurement(ULTIMATE_CHANNEL(chl->data));
        if (!process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(process)))) {
            ChannelsSingle *single = channels_object_get_single(CHANNELS_OBJECT(chl->data));
            channels_single_set_last_measurement(single, channels_simple_get_measurement(channels_object_get_simple(CHANNELS_OBJECT(chl->data))));
            channels_single_emit_update(single);
        }
    }
}

static void measurement_transmit_value(MeasurementProcess *process) {
    ProcessSimple *psimple = process_object_get_simple(PROCESS_OBJECT(process));
    GList *        chl     = NULL;
    for (chl = process->priv->measurement_channels; chl != NULL; chl = chl->next) {
        ChannelsSimple *simple = NULL;
        simple                 = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
        channels_simple_set_measure_kind(simple, process_simple_get_kind_type(process_object_get_simple(PROCESS_OBJECT(process))));
        if ((process_simple_get_amount_counter(psimple) > 0 && process_simple_get_amount_percentage(psimple) > 0.01) &&
            process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(process))) && !process_simple_get_check_process(process_object_get_simple(PROCESS_OBJECT(process))))
            ultimate_channel_transmit_amount(ULTIMATE_CHANNEL(chl->data), MKT_PROCESS_OBJECT(process));
        else
            ultimate_channel_transmit_M_result(ULTIMATE_CHANNEL(chl->data), MKT_PROCESS_OBJECT(process));
        if (process_simple_get_check_process(process_object_get_simple(PROCESS_OBJECT(process)))) {
            channels_check_set_result(channels_object_get_check(CHANNELS_OBJECT(chl->data)), channels_simple_get_result(simple));
            channels_check_set_last_analyse(channels_object_get_check(CHANNELS_OBJECT(chl->data)), market_db_time_now());
            ultimate_channel_check_limit_check(ULTIMATE_CHANNEL(chl->data));
            ultimate_channel_transmit_analog_check(ULTIMATE_CHANNEL(chl->data));
        } else if (process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(process)))) {
            channels_simple_set_online_result(simple, channels_simple_get_result(simple));
            channels_measurement_set_last_analyse(channels_object_get_measurement(CHANNELS_OBJECT(chl->data)), market_db_time_now());
            ultimate_channel_check_limit(ULTIMATE_CHANNEL(chl->data));
            ultimate_channel_transmit_analog(ULTIMATE_CHANNEL(chl->data));
            channels_simple_emit_update(simple);
        } else {
            ChannelsSingle *single = channels_object_get_single(CHANNELS_OBJECT(chl->data));
            channels_single_emit_update(single);
        }
    }
    guint trigger = process_simple_get_transmit_trigger(process_object_get_simple(PROCESS_OBJECT(process)));
    process_simple_set_transmit_trigger(process_object_get_simple(PROCESS_OBJECT(process)), trigger + 1);
}

static void measurement_transmit_integration_value(MeasurementProcess *process) {
    GList *channels = NULL;
    for (channels = process->priv->measurement_channels; channels != NULL; channels = channels->next) {
        if (channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channels->data)))) {
            ChannelsSimple *simple = NULL;
            simple                 = channels_object_get_simple(CHANNELS_OBJECT(channels->data));
            channels_simple_set_measure_kind(simple, process_simple_get_kind_type(process_object_get_simple(PROCESS_OBJECT(process))));
            ultimate_channel_transmit_M_replicate(ULTIMATE_CHANNEL(channels->data), MKT_PROCESS_OBJECT(process));
            if (!process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(process)))) {
                ChannelsSingle *single = channels_object_get_single(CHANNELS_OBJECT(channels->data));
                channels_single_emit_update(single);
            }
        }
    }
    for (channels = process->priv->measurement_channels; channels != NULL; channels = channels->next) {
        if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channels->data)))) {
            ChannelsSimple *simple = NULL;
            simple                 = channels_object_get_simple(CHANNELS_OBJECT(channels->data));
            channels_simple_set_measure_kind(simple, process_simple_get_kind_type(process_object_get_simple(PROCESS_OBJECT(process))));
            ultimate_channel_transmit_M_replicate(ULTIMATE_CHANNEL(channels->data), MKT_PROCESS_OBJECT(process));
            if (!process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(process)))) {
                ChannelsSingle *single = channels_object_get_single(CHANNELS_OBJECT(channels->data));
                channels_single_emit_update(single);
            }
        }
    }
}

static void measurement_recalculate_statistic_value(MeasurementProcess *process) {
    GList *channels = NULL;
    for (channels = process->priv->measurement_channels; channels != NULL; channels = channels->next) {
        mkt_process_calculate_measurement_statistic(MKT_PROCESS_OBJECT(process), CHANNELS_OBJECT(channels->data));
    }
}

static gboolean measurement_runned_measurement_values(MeasurementProcess *measurement) {
    gboolean runned_value = FALSE;
    GList *  chl          = NULL;
    GList *  channels     = measurement->priv->measurement_channels;
    for (chl = channels; chl != NULL; chl = chl->next) {
        ChannelsSimple *simple = NULL;
        simple                 = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
        if ((!channels_simple_get_statistic_done(simple)) && (!channels_simple_get_measure_error(simple))) {
            runned_value = TRUE;
        }
    }
    return runned_value;
}

static void measurement_run_analyse(GTask *subTask);

static void waite_3_sec_DONE(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    g_task_return_boolean(subTask, TRUE);
    g_object_unref(subTask);
}

static void measurement_done_project(GTask *subTask) {
    MeasurementProcess *measurement_process = MEASUREMENT_PROCESS(g_task_get_source_object(subTask));
    if (!process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(measurement_process)))) {
        g_task_return_boolean(subTask, TRUE);
        g_object_unref(subTask);
        return;
    }
    mkt_status_activate(measurement_process->priv->wait_status, TRUE);
    lar_timer_default_run(g_task_get_cancellable(subTask), waite_3_sec_DONE, 3.0, subTask);
}

static void clean_dilution_pump_DONE(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    gboolean is_done = FALSE;
    pumps_pump_call_stop_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, NULL, NULL);
    measurement_done_project(subTask);
}
static void prepare_approve(MeasurementProcess *mprocess) {
    mprocess->priv->preparation_done = FALSE;
    if (mprocess->priv->prepare_timer) {
        g_timer_destroy(mprocess->priv->prepare_timer);
    }
    mprocess->priv->prepare_timer = NULL;
}
static void prepare_approve_callback(GObject *source_object, gint res_id, gpointer user_data) {
    MeasurementProcess *mprocess = MEASUREMENT_PROCESS(user_data);
    prepare_approve(mprocess);
}

static void measurement_process_approve_prepare(MeasurementProcess *mprocess, gboolean is_tic, MktTaskObject *task) {
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(mprocess));
    if (process_simple_get_replicates(simple) == 1 && (mprocess->priv->values_TIC == NULL || is_tic ==TRUE)) {
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(mprocess));
        StreamsUltra * ultra  = streams_object_get_ultra(stream);
        if(task!=NULL ){
            g_signal_connect(task,"free-resources",G_CALLBACK(prepare_approve_callback),mprocess);
        }else{
            prepare_approve(mprocess);
        }
        if (ultra) g_object_unref(ultra);
    }
    if (simple) g_object_unref(simple);
}

static void measurement_process_analyse_done(GTask *subTask) {
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    MeasurementProcess *measurement_process = MEASUREMENT_PROCESS(g_task_get_source_object(subTask));
    measurement_process_set_analyze(measurement_process, measurement_process->priv->measurement_channels, FALSE);
    measurement_transmit_integration_value(measurement_process);
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(measurement_process));

    measurement_recalculate_statistic_value(measurement_process);

    if (measurement_runned_measurement_values(measurement_process)) {
        process_simple_set_current_replicate(simple, (process_simple_get_current_replicate(simple) + 1));
        measurement_run_analyse(subTask);
    } else {
        if (process_simple_get_current_replicate(simple) > 1) {
            prepare_approve(measurement_process);
        }
        measurement_transmit_value(measurement_process);
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(measurement_process));
        if (streams_ultra_get_is_dilution(streams_object_get_ultra(stream))) {
            gboolean is_done = FALSE;
            pumps_pump_call_start_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, g_task_get_cancellable(subTask), NULL);
            lar_timer_default_run(g_task_get_cancellable(subTask), clean_dilution_pump_DONE, 3.0, subTask);
        } else {
            measurement_done_project(subTask);
        }
    }
}

static void measurement_process_realize_channels(MeasurementProcess *measurement) {

    if (measurement->priv->values_TC) g_list_free(measurement->priv->values_TC);
    if (measurement->priv->values_TIC) g_list_free(measurement->priv->values_TIC);
    if (measurement->priv->values_TOC) g_list_free(measurement->priv->values_TOC);
    if (measurement->priv->measurement_channels) g_list_free(measurement->priv->measurement_channels);
    measurement->priv->values_TC            = NULL;
    measurement->priv->values_TIC           = NULL;
    measurement->priv->values_TOC           = NULL;
    measurement->priv->measurement_channels = NULL;
    GList *chl                              = NULL;
    GList *channels                         = ultra_stream_channels(ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(measurement))));
    for (chl = channels; chl != NULL; chl = chl->next) {
        ChannelsSimple *simple = NULL;
        simple                 = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
        if (channels_simple_get_is_activate(simple) && channels_simple_get_is_allow(simple) &&
            (!process_simple_get_check_process(process_object_get_simple(PROCESS_OBJECT(measurement))) || channels_simple_get_is_check(simple))) {
            measurement->priv->measurement_channels = g_list_append(measurement->priv->measurement_channels, chl->data);
            channels_simple_set_measure_kind(simple, process_simple_get_kind_type(process_object_get_simple(PROCESS_OBJECT(measurement))));
            if (channels_simple_get_is_measurement(simple)) {
                if (!channels_simple_get_tic(simple))
                    measurement->priv->values_TC = g_list_append(measurement->priv->values_TC, chl->data);
                else
                    measurement->priv->values_TIC = g_list_append(measurement->priv->values_TIC, chl->data);
            } else if (channels_simple_get_is_calculated(simple))
                measurement->priv->values_TOC = g_list_append(measurement->priv->values_TOC, chl->data);

            if (!process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(measurement)))) {
                ChannelsSingle *single = channels_object_get_single(CHANNELS_OBJECT(chl->data));
                channels_single_set_last_measurement(single, (channels_single_get_last_measurement(single) + 1));
                channels_single_emit_update(single);
            }
        }
    }
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
    MeasurementProcess *measurement_process = MEASUREMENT_PROCESS(g_task_get_source_object(subTask));
    measurement_process_set_analyze(measurement_process, measurement_process->priv->values_TIC, FALSE);
    measurement_process_analyse_done(subTask);
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
    MeasurementProcess *measurement_process = MEASUREMENT_PROCESS(g_task_get_source_object(subTask));
    measurement_process_set_analyze(measurement_process, measurement_process->priv->values_TC, FALSE);
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(measurement_process)));
    if (measurement_process->priv->values_TIC != NULL) {
        MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(ANALYZE_TYPE_TASK, "name", "TC-Analyze", "analyze-is-tic", TRUE, "analyze-stream", stream, "analyze-timeout", 600., NULL));
        measurement_process_set_analyze(measurement_process, measurement_process->priv->values_TIC, TRUE);
        analyze_task_set_channels(ANALYZE_TASK(analyze), measurement_process->priv->values_TIC);
        mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TIC_task_done, subTask);
        measurement_process_approve_prepare(measurement_process,TRUE,analyze);
        g_object_unref(analyze);
    } else {
        measurement_process_analyse_done(subTask);
    }
}

static void measurement_run_analyse(GTask *subTask) {

    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    MeasurementProcess *measurement_process = MEASUREMENT_PROCESS(g_task_get_source_object(subTask));
    UltraStreamObject * stream              = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(measurement_process)));
    if (streams_ultra_get_process_rinsing(streams_object_get_ultra(STREAMS_OBJECT(stream))) && streams_ultra_get_rinsing_replicate(streams_object_get_ultra(STREAMS_OBJECT(stream))) > 0) {
        if (measurement_process->priv->values_TC != NULL) {
            MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(PSANALYSE_TYPE_TASK, "name", "TCPS-Analyze", "analyze-stream", stream, "analyze-timeout", 1200., NULL));
            measurement_process_set_analyze(measurement_process, measurement_process->priv->values_TC, TRUE);
            psanalyse_task_set_channels(PSANALYSE_TASK(analyze), measurement_process->priv->values_TC);
            mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TC_task_done, subTask);
            measurement_process_approve_prepare(measurement_process,FALSE,analyze);
            g_object_unref(analyze);
            return;
        }
    }
    if (measurement_process->priv->values_TC != NULL) {
        MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(ANALYZE_TYPE_TASK, "name", "TC-Analyze", "analyze-is-tic", FALSE, "analyze-stream", stream, "analyze-timeout", 600., NULL));
        measurement_process_set_analyze(measurement_process, measurement_process->priv->values_TC, TRUE);
        analyze_task_set_channels(ANALYZE_TASK(analyze), measurement_process->priv->values_TC);
        mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TC_task_done, subTask);
        measurement_process_approve_prepare(measurement_process,FALSE,analyze);
        g_object_unref(analyze);
    } else if (measurement_process->priv->values_TIC != NULL) {
        MktTaskObject *analyze = MKT_TASK_OBJECT(g_object_new(ANALYZE_TYPE_TASK, "name", "TC-Analyze", "analyze-is-tic", TRUE, "analyze-stream", stream, "analyze-timeout", 600., NULL));
        measurement_process_set_analyze(measurement_process, measurement_process->priv->values_TIC, TRUE);
        analyze_task_set_channels(ANALYZE_TASK(analyze), measurement_process->priv->values_TIC);
        mkt_task_object_run(analyze, g_task_get_cancellable(subTask), analyze_TIC_task_done, subTask);
        measurement_process_approve_prepare(measurement_process,TRUE,analyze);
        g_object_unref(analyze);
    } else {
        g_task_return_error(subTask, g_error_new(control_error_quark(), 1001, _("Measurement process channels not found")));
        g_object_unref(subTask);
        return;
    }
}

static void measurement_prepare_task_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    // g_print("prepare done...%p\n",source_object);
    GTask *subTask = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(subTask)) {
        g_object_unref(subTask);
        return;
    }
    if (source_object == NULL && !PREPARE_IS_TASK(source_object)) {
        mkt_log_error_message("Measurement process prepare error.");
    }
    MeasurementProcess *measurement_process = MEASUREMENT_PROCESS(g_task_get_source_object(subTask));
    GError *            error               = NULL;
    if (!mkt_task_object_finish(MKT_TASK_OBJECT(source_object), res, &error)) {
        g_task_return_error(subTask, error);
        g_object_unref(subTask);
        return;
    }
    // g_print("run analyse..\n");
    measurement_process_realize_channels(measurement_process);
    measurement_start_measurement(measurement_process);
    measurement_next_measurement(measurement_process);
    measurement_run_analyse(subTask);
}
static gboolean is_remote(MeasurementProcess *process ){
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
    if (simple == NULL) return FALSE;
    gboolean online = process_simple_get_online_process(simple);
    gboolean remote_control = process_simple_get_remote_control(simple); 
    g_object_unref(simple);
    return  online && remote_control;
}

static gboolean need_sampling(MeasurementProcess *process) {
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
    if (simple == NULL) return FALSE;
    gboolean online = process_simple_get_online_process(simple);
    g_object_unref(simple);
    if (!online) return FALSE;
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    if (stream == NULL) return FALSE;
    gboolean ret = ultra_stream_get_sampling(stream);
    return ret;
}
static gdouble sampling_time(MeasurementProcess *process) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    if (stream == NULL) return 0.0;
    StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(stream));
    gdouble       t     = 0.0;
    if (ultra) {
        t = streams_ultra_get_sample_filling_time(ultra);
        g_object_unref(ultra);
    }
    return t;
}
static void stop_timer(MeasurementProcess *process){
    if (process->priv->sampling_timer) {
        g_timer_destroy(process->priv->sampling_timer);
        process->priv->sampling_timer = NULL;
    }
}
static gdouble stripping_time(MeasurementProcess *process) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    if (stream == NULL) return 0.0;
    StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(stream));
    gdouble       t     = 0.0;
    if (ultra) {
        t = streams_ultra_get_stripping_time(ultra);
        if (!streams_ultra_get_need_stripping(ultra)) t = 0.0;
        g_object_unref(ultra);
    }
    return ;
}
static void start_timer(MeasurementProcess *process) {
    if (process->priv->sampling_timer) {
        g_timer_destroy(process->priv->sampling_timer);
    }
    process->priv->sampling_timer = g_timer_new();
    g_timer_start(process->priv->sampling_timer);
}
static void stop_stream_pump(MeasurementProcess *process) {
    gboolean           is_done = FALSE;
    UltraStreamObject *stream  = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    if (stream) {
        PumpsObject *pump = ultra_stream_get_pump(stream);
        if (pump) {
            PumpsPump *p = pumps_object_get_pump(pump);
            if (p) {
                pumps_pump_call_stop_sync(p, &is_done, NULL, NULL);
                g_object_unref(p);
                stop_timer(process);
            }
        }
    }
}

static void start_stream_pump(MeasurementProcess *process) {
    if (!is_remote(process)&&!need_sampling(process)) {
        return;
    }
    gboolean           is_done = FALSE;
    UltraStreamObject *stream  = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    PumpsObject *      pump    = ultra_stream_get_pump(stream);
    if (pump) {
        PumpsPump *p = pumps_object_get_pump(pump);
        if (p) {
            pumps_pump_call_start_sync(p, &is_done, NULL, NULL);
            g_object_unref(p);
            start_timer(process);
        }
    }
}

static void prepare_intervals(MeasurementProcess *process) {
    process->priv->sampling_time  = sampling_time(process);
    process->priv->stripping_time = stripping_time(process);
    process->priv->prepare_time   = process->priv->sampling_time + process->priv->stripping_time;
}
static gdouble measurement_preprare_timer(MeasurementProcess *process) {
    if (process->priv->prepare_timer == NULL) {
        process->priv->prepare_timer = g_timer_new();
        g_timer_start(process->priv->prepare_timer);
        mkt_process_object_trace(process, "start timer");
    }
    return g_timer_elapsed(process->priv->prepare_timer, NULL);
}

static void finish_process(MeasurementProcess *measurement_process) {
    gboolean is_done = FALSE;
    measurement_process_break(measurement_process);
    if (pumps_pump_get_is_on(pumps_object_get_pump(TERA_PUMP_6()))) pumps_pump_call_stop_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, NULL, NULL);
}

static void waitStripping_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *sub_task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(sub_task)) {
        g_object_unref(sub_task);
        return;
    }
    MeasurementProcess *process = MEASUREMENT_PROCESS(g_task_get_source_object(sub_task));
    MktTaskObject *     prepare = MKT_TASK_OBJECT(g_object_new(PREPARE_TYPE_TASK, "name", "Prepare", "prepare-stream", ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)), NULL));
    // g_print("prepare type %s\n",G_OBJECT_TYPE_NAME(prepare));
    mkt_task_object_run(prepare, g_task_get_cancellable(sub_task), measurement_prepare_task_done, sub_task);
    g_object_unref(prepare);
    process->priv->preparation_done = TRUE;
}

static void waitSampling_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *             sub_task = G_TASK(user_data);
    MeasurementProcess *process  = MEASUREMENT_PROCESS(g_task_get_source_object(sub_task));
    stop_stream_pump(process);
    if (g_task_return_error_if_cancelled(sub_task)) {
        g_object_unref(sub_task);
        return;
    }
    gdouble elapsed = process->priv->prepare_time - measurement_preprare_timer(process);
    lar_timer_default_run(g_task_get_cancellable(sub_task), waitStripping_callback, elapsed, sub_task);
    return;
}

static void measurement_process_run(MktProcessObject *process, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {

    // g_print("run measurement %s\n",process_simple_get_full_name(process_object_get_simple(PROCESS_OBJECT(process))));

    GTask *sub_task = g_task_new(process, cancellable, callback, user_data);
    if (!MEASUREMENT_PROCESS(process)) {
        mkt_log_error_message("Measurement process wrong object type - stop measurement");
        g_task_return_error(sub_task, g_error_new(control_error_quark(), 999, "Measurement process wrong object type - stop measurement"));
        return;
    }
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    if (!process_simple_get_online_process(process_object_get_simple(PROCESS_OBJECT(process)))) {
        ultra_stream_no_sampling(stream);
    }
    ultra_stream_set_state(stream, mkt_process_object_get_state(process));
    MeasurementProcess *mprocess = MEASUREMENT_PROCESS(process);
    prepare_intervals(mprocess);
    if (measurement_preprare_timer(mprocess) > 20.0 * 60.0) {
        prepare_approve(mprocess);
    }
    gdouble wait = mprocess->priv->sampling_time - measurement_preprare_timer(mprocess);
    if (mprocess->priv->prepare) {
        g_cancellable_cancel(mprocess->priv->prepare);
        g_object_unref(mprocess->priv->prepare);
        mprocess->priv->prepare = NULL;
    }
    if (wait > 0.0) start_stream_pump(mprocess);
    lar_timer_default_run(g_task_get_cancellable(sub_task), waitSampling_callback, wait, sub_task);
}

static gboolean measurement_process_finish(MktProcessObject *process, GTask *task, GError **error) {
    MeasurementProcess *mprocess = MEASUREMENT_PROCESS(process);
    finish_process(mprocess);
    if (g_cancellable_is_cancelled(g_task_get_cancellable(task))) {
        if (mprocess->priv->prepare) {
            g_cancellable_cancel(mprocess->priv->prepare);
            g_object_unref(mprocess->priv->prepare);
        }
        prepare_approve(mprocess);
        mprocess->priv->prepare = NULL;
    }
    stop_stream_pump(mprocess);
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process)));
    ultra_stream_on_sampling(stream);
    return g_task_propagate_boolean(task, error);
}

static void measurement_process_init(MeasurementProcess *measurement_process) {
    measurement_process->priv                       = MEASUREMENT_PROCESS_PRIVATE(measurement_process);
    measurement_process->priv->measurement_channels = NULL;
    measurement_process->priv->values_TC            = NULL;
    measurement_process->priv->values_TIC           = NULL;
    measurement_process->priv->wait_status          = mkt_status_new("W", "Wait process");
    measurement_process->priv->prepare_timer        = NULL;
    measurement_process->priv->sampling_timer       = NULL;
}

static void measurement_process_finalize(GObject *object) {
    MeasurementProcess *process = MEASUREMENT_PROCESS(object);
    if (process->priv->values_TC) g_list_free(process->priv->values_TC);
    if (process->priv->values_TIC) g_list_free(process->priv->values_TIC);
    if (process->priv->measurement_channels) g_list_free(process->priv->measurement_channels);
    G_OBJECT_CLASS(measurement_process_parent_class)->finalize(object);
}

static void measurement_process_online(MeasurementProcess *mprocess, gpointer data) {
    if (mprocess->priv->amount) g_array_free(mprocess->priv->amount, TRUE);
    mprocess->priv->amount = g_array_new(TRUE, TRUE, sizeof(gdouble));
    if (mprocess->priv->prepare) {
        g_cancellable_cancel(mprocess->priv->prepare);
        g_object_unref(mprocess->priv->prepare);
    }
    mprocess->priv->prepare = NULL;
    prepare_intervals(mprocess);
}

static void waitStrippingPrepare_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    MeasurementProcess *process    = MEASUREMENT_PROCESS(user_data);
    process->priv->preparation_run = FALSE;
    if (res != NULL && G_IS_TASK(res)) {
        if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res))) || !lar_timer_default_finish(res, NULL)) {
            return;
        }
    }
    process->priv->preparation_done = TRUE;
}
static void waitSamplingPrepare_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    MeasurementProcess *process = MEASUREMENT_PROCESS(user_data);
    stop_stream_pump(process);
    if (res != NULL && G_IS_TASK(res)) {
        if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res))) || !lar_timer_default_finish(res, NULL)) {
            process->priv->preparation_run = FALSE;
            return;
        }
    }
    gdouble elapsed = process->priv->prepare_time - measurement_preprare_timer(process);
    lar_timer_default_run(g_task_get_cancellable(G_TASK(res)), waitStrippingPrepare_callback, elapsed, process);
}

static void measurement_process_interval_triger(MktProcessObject *process, gdouble sec) {
    MeasurementProcess *mprocess = MEASUREMENT_PROCESS(process);
    if (mprocess->priv->prepare_time >= sec && !is_remote(mprocess)) {
        if (mprocess->priv->prepare_timer == NULL) {
            mprocess->priv->preparation_done = TRUE;

            if (mprocess->priv->prepare) {
                g_cancellable_cancel(mprocess->priv->prepare);
                g_object_unref(mprocess->priv->prepare);
            }
            mprocess->priv->prepare = g_cancellable_new();
            gdouble elapsed         = mprocess->priv->sampling_time - measurement_preprare_timer(mprocess);
            start_stream_pump(mprocess);
            mprocess->priv->preparation_run = TRUE;
            lar_timer_default_run(mprocess->priv->prepare, waitSamplingPrepare_callback, elapsed, mprocess);
        }
    }
}

static void measurement_process_offline(MeasurementProcess *process, gpointer data) {
    MeasurementProcess *mprocess = MEASUREMENT_PROCESS(process);
    finish_process(mprocess);
    prepare_approve(mprocess);
    stop_stream_pump(mprocess);
}

static void measurement_process_constructed(GObject *object) {
    MeasurementProcess *process = MEASUREMENT_PROCESS(object);
    if (G_OBJECT_CLASS(measurement_process_parent_class)->constructed) G_OBJECT_CLASS(measurement_process_parent_class)->constructed(object);
    g_signal_connect(process, "process-online", G_CALLBACK(measurement_process_online), NULL);
    g_signal_connect(process, "process-offline", G_CALLBACK(measurement_process_offline), NULL);
}

static void measurement_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    // MeasurementProcess *process = MEASUREMENT_PROCESS( object );
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void measurement_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    // MeasurementProcess *process = MEASUREMENT_PROCESS( object );
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}


static void measurement_process_class_init(MeasurementProcessClass *klass) {
    GObjectClass *         object_class = G_OBJECT_CLASS(klass);
    MktProcessObjectClass *pclass       = MKT_PROCESS_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(MeasurementProcessClass));
    object_class->finalize     = measurement_process_finalize;
    object_class->set_property = measurement_process_set_property;
    object_class->get_property = measurement_process_get_property;
    object_class->constructed  = measurement_process_constructed;
    pclass->finish             = measurement_process_finish;
    pclass->run                = measurement_process_run;
    pclass->interval_trigger   = measurement_process_interval_triger;
}

gboolean measurement_process_check_range(MeasurementProcess *process, gdouble limit, const gchar *channel, gboolean up) {
    gboolean is_ok        = TRUE;
    GList *  chl          = NULL;
    gboolean find_channel = FALSE;
    if (process->priv->measurement_channels == NULL) is_ok = FALSE;
    for (chl = process->priv->measurement_channels; chl != NULL; chl = chl->next) {
        if (0 == g_strcmp0(channel, g_dbus_object_get_object_path(G_DBUS_OBJECT(chl->data)))) {
            ChannelsSimple *simple = NULL;
            simple                 = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
            gdouble res            = channels_simple_get_online_result(simple);
            if (is_ok) is_ok = up ? res < limit : res > limit;
            find_channel = TRUE;
        }
    }
    if (!find_channel) mkt_log_error_message("Measurement check %s range fail channel %s not found", up ? "low" : "hight", channel);
    return is_ok;
}


