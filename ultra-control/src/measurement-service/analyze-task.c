/* @ingroup AnalyzeTask
 * @{
 * @file  analyze-task.c	Task object
 * @brief This is Task object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include <ultimate-library.h>

#include "analyze-task.h"
#include "ultimate-channel.h"
#include "ultra-control-process.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"
#include "ultraconfig.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _AnalyzeTaskPrivate
{
  UltraStreamObject *stream;
  gboolean is_tic;
  gdouble analyse;
  GList *channels;
  gboolean sampling_done;
  gboolean stabilization_done;
  gboolean after_rinsing;
  GCancellable *cancellable;
  gulong handler_id;

  gdouble airflow_current;
  GTimer *analyse_timeout;
  gboolean stoped;
  gboolean complited;
};

/* signals */

enum
{
  TASK_PROP0,
  ANALYZE_IS_TIC,
  ANALYZE_INJECTION_VOLUME,
  ANALYZE_STREAM,
  ANALYZE_SAMPLE_VESSEL,
  ANALYZE_DRAIN_VESSEL,
  ANALYZE_SOLUTION,
  ANALYZE_JUSTIFICATION_TIME,
  ANALYZE_TIME,
  ANALYZE_STABILIZATION_TIME
};

G_DEFINE_TYPE_WITH_PRIVATE(AnalyzeTask, analyze_task, MKT_TYPE_TASK_OBJECT);

static void ANALYSE_wait_check_callback(GObject *source_object,
                                        GAsyncResult *res, gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  GError *error = NULL;
  if (!lar_timer_default_finish(res, &error))
  {
    g_task_return_error(subTask, error);
    g_object_unref(subTask);
    return;
  }
  gboolean is_integrating = FALSE;
  GList *ch = NULL;
  for (ch = analyze_task->priv->channels; ch != NULL; ch = ch->next)
  {
    if (ultimate_channel_integration_is_runned(ULTIMATE_CHANNEL(ch->data)))
    {
      is_integrating = TRUE;
    }
    else
    {
    }
  }
  if (!is_integrating && analyze_task->priv->after_rinsing)
  {
    g_task_return_boolean(subTask, TRUE);
    g_object_unref(subTask);
    return;
  }
  if (g_timer_elapsed(analyze_task->priv->analyse_timeout, NULL) >
      analyze_task->priv->analyse)
  {
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("Analyse time out "));
    g_task_return_error(
        subTask,
        g_error_new(control_error_quark(), 1001, _("analyse stream %d timeout"),
                    streams_simple_get_number(streams_object_get_simple(
                        STREAMS_OBJECT(analyze_task->priv->stream)))));
    return;
  }
  lar_timer_default_run(g_task_get_cancellable(subTask),
                        ANALYSE_wait_check_callback, 2.0, subTask);
}
// XXXXXX
static void ANALYSE_task_AFTER_HOLD_DONE(GObject *source_object,
                                         GAsyncResult *res,
                                         gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }

  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  gboolean result = FALSE;
  GError *error = NULL;
  if (!sequence_workers_process_call_run_finish(
          SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error))
  {
    if (error)
      g_dbus_error_strip_remote_error(error);

    mkt_task_object_set_status(
        MKT_TASK_OBJECT(analyze_task), _("sequence %s failed - %s"),
        g_dbus_object_get_object_path(
            G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
        sequence_workers_process_get_status(sequence_object_get_workers_process(
            ULTRA_RINSING_SEQUENCE_WORKER())));
    g_task_return_error(subTask, error);
    g_object_unref(subTask);
    return;
  }
  else
  {

    analyze_task->priv->after_rinsing = TRUE;
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("wait integration"));
    lar_timer_default_run(g_task_get_cancellable(subTask),
                          ANALYSE_wait_check_callback, 2.0, subTask);
  }
}
// XXXXXX
static void ANALYSE_task_AFTER_RINSING_DONE(GObject *source_object,
                                            GAsyncResult *res,
                                            gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }

  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  gboolean result = FALSE;
  GError *error = NULL;
  if (!sequence_workers_process_call_run_finish(
          SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error))
  {
    if (error)
      g_dbus_error_strip_remote_error(error);

    mkt_task_object_set_status(
        MKT_TASK_OBJECT(analyze_task), _("sequence %s failed - %s"),
        g_dbus_object_get_object_path(
            G_DBUS_OBJECT(ULTRA_RINSING_SEQUENCE_WORKER())),
        sequence_workers_process_get_status(sequence_object_get_workers_process(
            ULTRA_RINSING_SEQUENCE_WORKER())));
    g_task_return_error(subTask, error);
    g_object_unref(subTask);
    return;
  }
  else
  {
    g_dbus_proxy_set_default_timeout(
        G_DBUS_PROXY(sequence_object_get_workers_process(
            ULTRA_AXISHOLD_SEQUENCE_WORKER())),
        20000);
    sequence_workers_process_call_run(
        sequence_object_get_workers_process(ULTRA_AXISHOLD_SEQUENCE_WORKER()),
        g_task_get_cancellable(subTask), ANALYSE_task_AFTER_HOLD_DONE, subTask);
    control_was_rinsed();
  }
}

static void ANALYSE_task_AFTER_RINSING(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  analyze_task->priv->after_rinsing = FALSE;
  VesselsObject *drain = ultra_stream_get_drain(analyze_task->priv->stream);

  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                             _("run sequence %s"),
                             g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                 ULTRA_RINSING_SEQUENCE_WORKER())));
  sequence_workers_sample_set_sample_main(
      sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()),
      g_dbus_object_get_object_path(G_DBUS_OBJECT(drain)));
  g_object_unref(drain);
  guint rinsing_repeat = streams_ultra_get_rinsing_count(
      streams_object_get_ultra(STREAMS_OBJECT(analyze_task->priv->stream)));
  sequence_workers_sample_set_repeat(
      sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()),
      rinsing_repeat);
  g_dbus_proxy_set_default_timeout(
      G_DBUS_PROXY(
          sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER())),
      100000);
  sequence_workers_process_call_run(
      sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER()),
      g_task_get_cancellable(subTask), ANALYSE_task_AFTER_RINSING_DONE,
      subTask);
}

static void ANALYSE_ultra_stream_AIRFLOW_analyze_done_callback(
    GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  if (analyze_task->priv->complited ||
      g_cancellable_is_cancelled(g_task_get_cancellable(subTask)))
  {
    g_object_unref(subTask);
    return;
  }
  GError *error = NULL;
  gboolean out_value = FALSE;
  if (!airflow_sensor_call_injection_analyse_out_finish(
          airflow_object_get_sensor(ULTRA_AIRFLOW()), &out_value, res,
          &error))
  {
    if (error)
      g_error_free(error);
  }
  else
  {
    gint stream = streams_simple_get_number(streams_object_get_simple(
                      STREAMS_OBJECT(analyze_task->priv->stream))) -
                  1;
    if (!out_value)
      mkt_errors_come(E1841 + stream);
    else
      mkt_errors_clean(E1841 + stream);
  }
  g_object_unref(subTask);
}

static void ANALYSE_task_AIRFLOW_ANALYSE(GTask *subTask)
{
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  mkt_task_object_set_status(
      MKT_TASK_OBJECT(analyze_task), _("run airflow analyse %s"),
      g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_AIRFLOW())));
  // mkt_error_gone(1840+streams_simple_get_number(streams_object_get_simple(task->priv->stream)));
  airflow_sensor_set_inj_analyse_timeout(
      airflow_object_get_sensor(ULTRA_AIRFLOW()), 20.0);
  airflow_sensor_call_injection_analyse_out(
      airflow_object_get_sensor(ULTRA_AIRFLOW()),
      g_task_get_cancellable(subTask),
      ANALYSE_ultra_stream_AIRFLOW_analyze_done_callback,
      g_object_ref(subTask));
}

static void ANALYSE_ultra_stream_run_INTEGRATION(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));

  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                             _("run integration"));
  analyze_task->priv->after_rinsing = FALSE;
  GList *ch = NULL;
  for (ch = analyze_task->priv->channels; ch != NULL; ch = ch->next)
  {
    ultimate_channel_integration(ULTIMATE_CHANNEL(ch->data));
  }
  ANALYSE_task_AFTER_RINSING(subTask);
}

static void ANALYSE_INJECTION_DONE(GObject *source_object, GAsyncResult *res,
                                   gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  gboolean result = FALSE;
  GError *error = NULL;
  if (!sequence_workers_process_call_run_finish(
          SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error))
  {
    if (error)
      g_dbus_error_strip_remote_error(error);

    mkt_task_object_set_status(
        MKT_TASK_OBJECT(analyze_task), _("sequence %s failed - %s"),
        g_dbus_object_get_object_path(
            G_DBUS_OBJECT(ULTRA_INJECTIONTIC_SEQUENCE_WORKER())),
        sequence_workers_process_get_status(sequence_object_get_workers_process(
            ULTRA_INJECTIONTIC_SEQUENCE_WORKER())));
    g_task_return_error(subTask, error);
    g_object_unref(subTask);
    return;
  }
  guint rinsing_repeat = streams_ultra_get_rinsing_count(
      streams_object_get_ultra(STREAMS_OBJECT(analyze_task->priv->stream)));
  sequence_workers_sample_set_repeat(
      sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()),
      rinsing_repeat);
  ANALYSE_ultra_stream_run_INTEGRATION(subTask);
}

static void ANALYSE_ultra_stream_run_INJECTION_TIC(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                             _("run sequence %s"),
                             g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                 ULTRA_INJECTIONTIC_SEQUENCE_WORKER())));
  sequence_workers_process_call_run(
      sequence_object_get_workers_process(ULTRA_INJECTIONTIC_SEQUENCE_WORKER()),
      g_task_get_cancellable(subTask), ANALYSE_INJECTION_DONE, subTask);
}

static void ANALYSE_ultra_stream_run_INJECTION_TC(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  if (streams_ultra_get_codo_injection(streams_object_get_ultra(
          STREAMS_OBJECT(analyze_task->priv->stream))))
  {
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("run sequence %s"),
                               g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                   ULTRA_INJECTIONCOD_SEQUENCE_WORKER())));
    sequence_workers_process_call_run(sequence_object_get_workers_process(
                                          ULTRA_INJECTIONCOD_SEQUENCE_WORKER()),
                                      g_task_get_cancellable(subTask),
                                      ANALYSE_INJECTION_DONE, subTask);
  }
  else
  {
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("run sequence %s"),
                               g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                   ULTRA_INJECTION_SEQUENCE_WORKER())));
    sequence_workers_process_call_run(
        sequence_object_get_workers_process(ULTRA_INJECTION_SEQUENCE_WORKER()),
        g_task_get_cancellable(subTask), ANALYSE_INJECTION_DONE, subTask);
  }
  ANALYSE_task_AIRFLOW_ANALYSE(subTask);
}

static void ANALYSE_justification_done(GObject *source_object,
                                       GAsyncResult *res, gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));

  airflow_sensor_set_smooth(airflow_object_get_sensor(ULTRA_AIRFLOW()), FALSE);
  GList *ch = NULL;
  analyze_task->priv->airflow_current =
      airflow_sensor_get_air_out(airflow_object_get_sensor(ULTRA_AIRFLOW()));
  for (ch = analyze_task->priv->channels; ch != NULL; ch = ch->next)
  {
    ultimate_channel_calculate_justification(ULTIMATE_CHANNEL(ch->data));
  }
  if (analyze_task->priv->is_tic)
    ANALYSE_ultra_stream_run_INJECTION_TIC(subTask);
  else
    ANALYSE_ultra_stream_run_INJECTION_TC(subTask);
}

static void ANALYSE_ultra_run_JUSTIFICATION(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                             _("run justification"));
  GList *ch = NULL;
  for (ch = analyze_task->priv->channels; ch != NULL; ch = ch->next)
  {
    ultimate_channel_justification(ULTIMATE_CHANNEL(ch->data));
  }
  gdouble just_time = UltradeviceGetJustificationTime(ConfigureDevice());
  lar_timer_default_run(g_task_get_cancellable(subTask),
                        ANALYSE_justification_done, just_time, subTask);
  airflow_sensor_set_smooth(airflow_object_get_sensor(ULTRA_AIRFLOW()), TRUE);
}

static void ANALYSE_ultra_start(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));

  GList *ch = NULL;
  for (ch = analyze_task->priv->channels; ch != NULL; ch = ch->next)
  {
    ultimate_channel_start_analyse(ULTIMATE_CHANNEL(ch->data));
  }
  analyze_task->priv->analyse_timeout = g_timer_new();
  g_timer_start(analyze_task->priv->analyse_timeout);
  ANALYSE_ultra_run_JUSTIFICATION(subTask);
}

static void analyze_task_SAMPLING_DONE(GObject *source_object,
                                       GAsyncResult *res, gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  gboolean result = FALSE;
  GError *error = NULL;
  if (!sequence_workers_process_call_run_finish(
          SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error))
  {
    if (error)
      g_dbus_error_strip_remote_error(error);

    mkt_task_object_set_status(
        MKT_TASK_OBJECT(analyze_task), _("sequence %s failed - %s"),
        g_dbus_object_get_object_path(
            G_DBUS_OBJECT(ULTRA_SAMPLING_SEQUENCE_WORKER())),
        sequence_workers_process_get_status(sequence_object_get_workers_process(
            ULTRA_SAMPLING_SEQUENCE_WORKER())));
    g_task_return_error(subTask, error);
    g_object_unref(subTask);
    return;
  }
  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task), _("done"));
  analyze_task->priv->sampling_done = TRUE;
  mkt_task_object_free_res(MKT_TASK_OBJECT(analyze_task), 1);
  ANALYSE_ultra_start(subTask);
  /*if (task->priv->stabilization_done)
      ANALYSE_ultra_start(task);
     else
      mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task), _("wait %s gas
     stabilization"), task->priv->is_tic ? "TIC" : "TC");
   */
}

static void ANALYSE_task_run_SAMPLING(GTask *subTask)
{
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  gint injection_volume =
      analyze_task->priv->is_tic
          ? streams_ultra_get_injection_volume_tic(streams_object_get_ultra(
                STREAMS_OBJECT(analyze_task->priv->stream)))
          : streams_ultra_get_injection_volume(streams_object_get_ultra(
                STREAMS_OBJECT(analyze_task->priv->stream)));
  VesselsObject *sample = NULL;
  VesselsObject *drain = ultra_stream_get_drain(analyze_task->priv->stream);

  if (streams_ultra_get_is_dilution(streams_object_get_ultra(
          STREAMS_OBJECT(analyze_task->priv->stream))))
  {
    sample = g_object_ref(ULTRA_VESSEL6());
  }
  else
  {
    sample = ultra_stream_get_sample(analyze_task->priv->stream);
  }

  if (streams_ultra_get_codo_injection(streams_object_get_ultra(
          STREAMS_OBJECT(analyze_task->priv->stream))) &&
      !analyze_task->priv->is_tic)
  {
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("run sequence %s"),
                               g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                   ULTRA_SAMPLINGCOD_SEQUENCE_WORKER())));
    sequence_workers_sample_set_sample_main(
        sequence_object_get_workers_sample(ULTRA_SAMPLINGCOD_SEQUENCE_WORKER()),
        g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
    sequence_workers_sample_set_volume(
        sequence_object_get_workers_sample(ULTRA_SAMPLINGCOD_SEQUENCE_WORKER()),
        injection_volume);
    sequence_workers_process_call_run(sequence_object_get_workers_process(
                                          ULTRA_SAMPLINGCOD_SEQUENCE_WORKER()),
                                      g_task_get_cancellable(subTask),
                                      analyze_task_SAMPLING_DONE, subTask);
  }
  else
  {
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("run sequence %s"),
                               g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                   ULTRA_SAMPLING_SEQUENCE_WORKER())));
    sequence_workers_sample_set_sample_main(
        sequence_object_get_workers_sample(ULTRA_SAMPLING_SEQUENCE_WORKER()),
        g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
    sequence_workers_sample_set_volume(
        sequence_object_get_workers_sample(ULTRA_SAMPLING_SEQUENCE_WORKER()),
        injection_volume);
    sequence_workers_process_call_run(
        sequence_object_get_workers_process(ULTRA_SAMPLING_SEQUENCE_WORKER()),
        g_task_get_cancellable(subTask), analyze_task_SAMPLING_DONE, subTask);
  }

  control_need_hold();
  control_need_rinsing(drain);
  g_object_unref(sample);
  g_object_unref(drain);
}

static void analyze_task_DILUTION_DONE(GObject *source_object,
                                       GAsyncResult *res, gpointer user_data)
{
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  gboolean result = FALSE;
  GError *error = NULL;
  if (!sequence_workers_process_call_run_finish(
          SEQUENCE_WORKERS_PROCESS(source_object), &result, res, &error))
  {
    if (error)
      g_dbus_error_strip_remote_error(error);

    mkt_task_object_set_status(
        MKT_TASK_OBJECT(analyze_task), _("sequence %s failed - %s"),
        g_dbus_object_get_object_path(
            G_DBUS_OBJECT(ULTRA_DILUTION_SEQUENCE_WORKER())),
        sequence_workers_process_get_status(sequence_object_get_workers_process(
            ULTRA_DILUTION_SEQUENCE_WORKER())));
    g_task_return_error(subTask, error);
    g_object_unref(subTask);
    return;
  }
  ANALYSE_task_run_SAMPLING(subTask);
  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task), _("wait sampling"));
}
static void ANALYSE_stabilization_done(GObject *source_object,
                                       GAsyncResult *res, gpointer user_data)
{
  // MktModel *data = mkt_model_new(MKT_TYPE_SENSOR_DATA, NULL);
  GTask *subTask = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(subTask))
  {
    g_object_unref(subTask);
    return;
  }
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  analyze_task->priv->stabilization_done = TRUE;

  if (streams_ultra_get_is_dilution(streams_object_get_ultra(
          STREAMS_OBJECT(analyze_task->priv->stream))) &&
      streams_ultra_get_on_replicte(streams_object_get_ultra(
          STREAMS_OBJECT(analyze_task->priv->stream))))
  {
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("run sequence %s"),
                               g_dbus_object_get_object_path(G_DBUS_OBJECT(
                                   ULTRA_DILUTION_SEQUENCE_WORKER())));
    VesselsObject *sample = ultra_stream_get_sample(analyze_task->priv->stream);
    VesselsObject *drain = ultra_stream_get_drain(analyze_task->priv->stream);
    sequence_workers_sample_set_sample_main(
        sequence_object_get_workers_sample(ULTRA_DILUTION_SEQUENCE_WORKER()),
        g_dbus_object_get_object_path(G_DBUS_OBJECT(sample)));
    sequence_workers_sample_set_sample_second(
        sequence_object_get_workers_sample(ULTRA_DILUTION_SEQUENCE_WORKER()),
        g_dbus_object_get_object_path(G_DBUS_OBJECT(drain)));
    g_object_unref(sample);
    g_object_unref(drain);
    StreamsUltra *ultra = streams_object_get_ultra(
        STREAMS_OBJECT(STREAMS_OBJECT(analyze_task->priv->stream)));
    sequence_workers_dilution_set_fill_time(
        sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()),
        (gdouble)streams_ultra_get_dilution_pump_time(ultra));
    sequence_workers_dilution_set_dilution_time(
        sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()),
        streams_ultra_get_dilution_wait_time(ultra));
    sequence_workers_dilution_set_proportion(
        sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER()),
        streams_ultra_get_dilution_factor(ultra));
    sequence_workers_process_call_run(
        sequence_object_get_workers_process(ULTRA_DILUTION_SEQUENCE_WORKER()),
        g_task_get_cancellable(subTask), analyze_task_DILUTION_DONE, subTask);

    control_need_hold();
    control_need_rinsing(drain);
  }
  else
  {
    ANALYSE_task_run_SAMPLING(subTask);
    mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                               _("wait sampling"));
  }
}

static gboolean analyze_task_start(GTask *subTask)
{

  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));
  analyze_task->priv->sampling_done = FALSE;
  analyze_task->priv->stabilization_done = FALSE;
  gdouble stabilization_time =
      analyze_task->priv->is_tic
          ? streams_ultra_get_delay_tic(streams_object_get_ultra(
                STREAMS_OBJECT(analyze_task->priv->stream)))
          : streams_ultra_get_delay(streams_object_get_ultra(
                STREAMS_OBJECT(analyze_task->priv->stream)));
  mkt_task_object_set_status(MKT_TASK_OBJECT(analyze_task),
                             _("gas stabilization"));
  airflow_sensor_set_furnace_way(airflow_object_get_sensor(ULTRA_AIRFLOW()),
                                 !analyze_task->priv->is_tic);
  lar_timer_default_run(g_task_get_cancellable(subTask),
                        ANALYSE_stabilization_done, stabilization_time,
                        subTask);
  return TRUE;
}

static gboolean check_task_parameter(GTask *subTask)
{
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(subTask));

  if (analyze_task->priv->stream == NULL ||
      !ULTRA_IS_STREAM_OBJECT(analyze_task->priv->stream))
  {
    g_task_return_error(subTask, g_error_new(control_error_quark(), 1001,
                                             _("stream object not found")));
    g_object_unref(subTask);
    return FALSE;
  }
  StreamsUltra *ultra =
      streams_object_get_ultra(STREAMS_OBJECT(analyze_task->priv->stream));
  if (ultra == NULL)
  {
    g_task_return_error(
        subTask, g_error_new(control_error_quark(), 1001,
                             _("task stream ultra interface not found")));
    g_object_unref(subTask);
    return FALSE;
  }
  VesselsObject *drain = ultra_stream_get_drain(analyze_task->priv->stream);
  if (drain == NULL)
  {
    g_task_return_error(subTask,
                        g_error_new(control_error_quark(), 1001,
                                    _("drain vessel %s not found"),
                                    streams_ultra_get_drain_vessel(ultra)));
    g_object_unref(subTask);
    return FALSE;
  }
  g_object_unref(drain);
  VesselsObject *sample = ultra_stream_get_sample(analyze_task->priv->stream);
  if (sample == NULL)
  {
    g_task_return_error(subTask, g_error_new(control_error_quark(), 1001,
                                             _("sample vessel not found")));
    g_object_unref(subTask);
    return FALSE;
  }
  g_object_unref(sample);
  if (analyze_task->priv->channels == NULL)
  {
    g_task_return_error(subTask,
                        g_error_new(control_error_quark(), 1001,
                                    _("No less that one channel must be activated")));
    // TODO: Message durch eine Fehler ersetzen
    mkt_log_error_message(
        "No less that one channel, must be activated on stream %s",
        streams_simple_get_name(streams_object_get_simple(
            STREAMS_OBJECT(analyze_task->priv->stream))));
    g_object_unref(subTask);
    return FALSE;
  }
  return TRUE;
}
static void analyse_stop_intern(AnalyzeTask *analyze_task)
{
  if (analyze_task->priv->stoped)
    return;
  analyze_task->priv->stoped = TRUE;
  airflow_sensor_set_smooth(airflow_object_get_sensor(ULTRA_AIRFLOW()), FALSE);

  if (analyze_task && ANALYZE_IS_TASK(analyze_task))
  {
    GList *ch = NULL;
    for (ch = analyze_task->priv->channels; ch != NULL; ch = ch->next)
    {
      ultimate_channel_analyse_stop(ULTIMATE_CHANNEL(ch->data));
      channels_simple_set_measure(
          channels_object_get_simple(CHANNELS_OBJECT(ch->data)), FALSE);
    }
  }
  airflow_sensor_set_furnace_way(airflow_object_get_sensor(ULTRA_AIRFLOW()),
                                 FALSE);
}

static void analyze_cancelled(GCancellable *cancel, GTask *task)
{
  AnalyzeTask *analyze_task = ANALYZE_TASK(g_task_get_source_object(task));
  analyse_stop_intern(analyze_task);
}

static void analyze_task_run(MktTaskObject *task, GCancellable *cancellable,
                             GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *subTask = g_task_new(task, cancellable, callback, user_data);
  AnalyzeTask *analyze_task = ANALYZE_TASK(task);
  analyze_task->priv->cancellable = g_object_ref(cancellable);
  analyze_task->priv->complited = FALSE;
  analyze_task->priv->handler_id =
      g_cancellable_connect(g_task_get_cancellable(subTask),
                            G_CALLBACK(analyze_cancelled), subTask, NULL);
  if (check_task_parameter(subTask))
    analyze_task_start(subTask);
}

static gboolean analyze_task_finish(MktTaskObject *task, GTask *res,
                                    GError **error)
{
  g_return_val_if_fail(task != NULL, FALSE);
  g_return_val_if_fail(res != NULL, FALSE);
  g_return_val_if_fail(G_IS_TASK(res), FALSE);
  AnalyzeTask *analyze_task = ANALYZE_TASK(task);
  analyze_task->priv->complited = TRUE;
  analyse_stop_intern(analyze_task);
  gboolean result = g_task_propagate_boolean(G_TASK(res), error);
  return result;
}

static void analyze_task_init(AnalyzeTask *analyze_task)
{
  analyze_task->priv = analyze_task_get_instance_private(analyze_task);
  tera_pumps_manager_client_new();
  ultra_vessels_manager_client_new();

  analyze_task->priv->is_tic = FALSE;
  analyze_task->priv->channels = NULL;
  analyze_task->priv->stream = NULL;
  analyze_task->priv->analyse_timeout = NULL;
  analyze_task->priv->stoped = FALSE;

  /* TODO: Add initialization code here */
}

static void analyze_task_finalize(GObject *object)
{
  /* TODO: Add deinitalization code here */
  AnalyzeTask *task = ANALYZE_TASK(object);
  if (task->priv->cancellable)
  {
    g_cancellable_disconnect(task->priv->cancellable, task->priv->handler_id);
    g_object_unref(task->priv->cancellable);
  }
  if (task->priv->channels)
    g_list_free(task->priv->channels);
  if (task->priv->stream)
    g_object_unref(task->priv->stream);
  if (task->priv->analyse_timeout)
    g_timer_destroy(task->priv->analyse_timeout);
  G_OBJECT_CLASS(analyze_task_parent_class)->finalize(object);
}

static void analyze_task_set_property(GObject *object, guint prop_id,
                                      const GValue *value, GParamSpec *pspec)
{
  AnalyzeTask *task = ANALYZE_TASK(object);
  switch (prop_id)
  {
  case ANALYZE_IS_TIC:
    task->priv->is_tic = g_value_get_boolean(value);
    break;
  case ANALYZE_STREAM:
    if (task->priv->stream)
      g_object_unref(task->priv->stream);
    task->priv->stream = g_value_dup_object(value);
    break;
  case ANALYZE_TIME:
    task->priv->analyse = g_value_get_double(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void analyze_task_get_property(GObject *object, guint prop_id,
                                      GValue *value, GParamSpec *pspec)
{
  AnalyzeTask *task = ANALYZE_TASK(object);
  switch (prop_id)
  {
  case ANALYZE_IS_TIC:
    g_value_set_boolean(value, task->priv->is_tic);
    break;
  case ANALYZE_STREAM:
    g_value_set_object(value, task->priv->stream);
    break;
  case ANALYZE_TIME:
    g_value_set_double(value, task->priv->analyse);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void analyze_task_class_init(AnalyzeTaskClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  // object_class->dispose           = analyze_atom_dispose;
  object_class->finalize = analyze_task_finalize;
  object_class->set_property = analyze_task_set_property;
  object_class->get_property = analyze_task_get_property;
  MKT_TASK_OBJECT_CLASS(klass)->run = analyze_task_run;
  MKT_TASK_OBJECT_CLASS(klass)->finish = analyze_task_finish;

  g_object_class_install_property(
      object_class, ANALYZE_IS_TIC,
      g_param_spec_boolean(
          "analyze-is-tic", "Analyze is TIC", "Analyze is TIC", FALSE,
          G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
      object_class, ANALYZE_STREAM,
      g_param_spec_object("analyze-stream", "Analyze stream object",
                          "Analyze stream object", STREAMS_TYPE_OBJECT,
                          G_PARAM_WRITABLE | G_PARAM_READABLE));
  g_object_class_install_property(
      object_class, ANALYZE_TIME,
      g_param_spec_double("analyze-timeout", "Analyze timeout",
                          "Analyze timeout", 250.0, 2000.0, 700.0,
                          G_PARAM_WRITABLE | G_PARAM_READABLE));
}

void analyze_task_set_channels(AnalyzeTask *task, GList *channels)
{
  g_return_if_fail(task != NULL);
  g_return_if_fail(channels != NULL);
  if (task->priv->channels)
    g_list_free(task->priv->channels);
  task->priv->channels = g_list_copy(channels);
}

gboolean analyze_task_is_tic(AnalyzeTask *task)
{
  g_return_val_if_fail(task != NULL, FALSE);
  return task->priv->is_tic;
}

gdouble analyse_justification_airflow(AnalyzeTask *task)
{
  g_return_val_if_fail(task != NULL, FALSE);
  return task->priv->airflow_current;
}

/** @} */
