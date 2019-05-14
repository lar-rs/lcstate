/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup UltraAirflowObject
 * @{
 * @file  ultra-airflow-object.c
 * @brief This is AXIS X model object description.
 *
 *  Copyright (C) LAR  2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "ultra-airflow-object.h"
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include <mktlib.h>
#include <ultimate-library.h>

enum {
  PROP_0,
};

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraAirflowObjectPrivate {
  GCancellable *activity;
  GCancellable *check;
  GTimer *timer;
  NodesObject *nodes_object;
  NodesAnalog1 *analog_node1;
  GSettings *airflow_settings;
  GArray *in_row;
  GArray *out_row;
  GArray *in_array;
  GArray *out_array;
  GArray *in_monitoring;
  GArray *out_monitoring;
  gboolean is_1835;
  gboolean is_1836;
  gboolean is_1837;

  gboolean is_1830;

  guint tag;
  gboolean last_state;
  gboolean tc_error;
  gboolean tic_error;
  gboolean start;
  // GSetting   *settings;
};

static void E1830_check(UltraAirflowObject *airflow) {
  gboolean error = mkt_error_pending_number(E1831) ||
                   mkt_error_pending_number(E1832) ||
                   mkt_error_pending_number(E1833);
  error = error || airflow->priv->is_1835;
  if (error)
    mkt_errors_come(E1830);
  else
    mkt_errors_clean(E1830);
}

#define ULTRA_AIRFLOW_OBJECT_PRIVATE(o)                                        \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_AIRFLOW_OBJECT,                 \
                               UltraAirflowObjectPrivate))

static gboolean _valves_set_digital(UltraAirflowObject *airflow,
                                    gboolean value) {
  NodesObject *object = NODES_OBJECT(g_dbus_object_manager_get_object(
      mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
  g_return_val_if_fail(object != NULL, FALSE);
  NodesDigital16 *digital16 = nodes_object_get_digital16(object);
  g_return_val_if_fail(digital16 != NULL, FALSE);
  GError *error = NULL;
  gboolean result = FALSE;
  if (!nodes_digital16_call_set_digital_out_sync(digital16, 4, value, &result,
                                                 NULL, &error)) {
    mkt_log_error_message_sync(
        "Airflow: connection to 'digital node 1' is fail");
    if (error)
      g_error_free(error);
  }
  return result;
}

static void airflow_in_error(UltraAirflowObject *airflow, gboolean error) {
  if (airflow->priv->is_1835 != error) {
    airflow->priv->is_1835 = error;
    if (error) {
      mkt_errors_come(E1835);
    } else {
      MktError *model = mkt_error_find(E1835);
      if (model && (mkt_error_type(model) == MKT_ERROR_WARNING ||
                    airflow->priv->start)) {
        airflow->priv->start = FALSE;
        mkt_errors_clean(E1835);
      }
    }
  }
  if (airflow_sensor_get_furnace_way(
          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)))) {
    if (airflow->priv->is_1836 != error) {
      airflow->priv->is_1836 = error;
      if (error)
        mkt_errors_come(E1836);
      else
        mkt_errors_clean(E1836);
    }
  } else {
    if (airflow->priv->is_1837 != error) {
      airflow->priv->is_1837 = error;
      if (error)
        mkt_errors_come(E1837);
      else
        mkt_errors_clean(E1837);
    }
  }
}

static void airflow_brocked_sensor(UltraAirflowObject *airflow) {
  AirflowSensor *sensor = airflow_object_get_sensor(AIRFLOW_OBJECT(airflow));
  airflow_sensor_set_air_in(sensor, 0.0);
  airflow_sensor_set_air_out(sensor, 0.0);
  airflow_sensor_set_round_value_in(sensor, 0.0);
  airflow_sensor_set_round_value_out(sensor, 0.0);
  airflow_in_error(airflow, TRUE);
  mkt_errors_come(E1830);
}

static NodesAnalog1 *
ultra_airflow_get_NodesObject(UltraAirflowObject *airflow) {
  g_return_val_if_fail(mkt_can_manager_client_nodes() != NULL, NULL);
  airflow->priv->nodes_object = NODES_OBJECT(g_dbus_object_manager_get_object(
      mkt_can_manager_client_nodes(), "/com/lar/nodes/Analog1"));
  if (airflow->priv->nodes_object == NULL) {
    return NULL;
  }
  return nodes_object_get_analog1(airflow->priv->nodes_object);
}

static gdouble ultra_airflow_signal(UltraAirflowObject *airflow, gdouble value,
                                    gboolean IN) {
  static const double a6 = 0.003836617; // für 0..60:   0.230197;
  static const double a5 = -0.06027397; // für 0..60:  -3.616438;
  static const double a4 = 0.3727283;   // für 0..60:  22.36370;
  static const double a3 = -1.1430475;  // für 0..60: -68.58285;
  static const double a2 = 1.83842;     // für 0..60: 110.3052;
  static const double a1 = -1.4032;     // für 0..60: -84.19201;
  static const double a0 = 0.39159;     // für 0..60:  23.49542;
  gdouble adj = airflow_sensor_get_adjustment_factor(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
  ;
  if (IN)
    adj = 1. - adj;
  else
    adj = 1. + adj;

  gdouble signal = value / 4096. * 5.;
  signal =
      signal < 1.
          ? 0.
          : ((((((a6 * signal + a5) * signal + a4) * signal + a3) * signal +
               a2) *
                  signal +
              a1) *
                 signal +
             a0);
  signal = signal < 0. ? 0. : signal * adj;
  return signal;
}

G_DEFINE_TYPE(UltraAirflowObject, ultra_airflow_object,
              AIRFLOW_TYPE_OBJECT_SKELETON)

static void ultra_airflow_set_row_in(UltraAirflowObject *airflow,
                                     gdouble in_value) {
  guint index = 0;
  gdouble summ = 0.0;
  gdouble round = 0.0;
  airflow->priv->in_row = g_array_append_val(airflow->priv->in_row, in_value);
  for (index = 0; index < airflow->priv->in_row->len; index++) {
    gdouble value = g_array_index(airflow->priv->in_row, gdouble, index);
    summ += value;
  }
  round = summ / ((gdouble)airflow->priv->in_row->len);
  airflow_sensor_set_round_row_in(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), round);
  if (airflow->priv->in_row->len > 10)
    airflow->priv->in_row = g_array_remove_index(airflow->priv->in_row, 0);
}

static void ultra_airflow_set_row_out(UltraAirflowObject *airflow,
                                      gdouble out_value) {
  guint index = 0;
  gdouble summ = 0.0;
  gdouble round = 0.0;
  airflow->priv->out_row =
      g_array_append_val(airflow->priv->out_row, out_value);
  for (index = 0; index < airflow->priv->out_row->len; index++) {
    gdouble value = g_array_index(airflow->priv->out_row, gdouble, index);
    summ += value;
  }
  round = summ / ((gdouble)airflow->priv->out_row->len);
  airflow_sensor_set_round_row_out(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), round);
  if (airflow->priv->out_row->len > 10)
    airflow->priv->out_row = g_array_remove_index(airflow->priv->out_row, 0);
}

// -------------------------------------------- Help functions
// --------------------------------------------------------

static gboolean ultra_airflow_object_waite_read(gpointer user_data);
static void ultra_airflow_object_waite_read_destroy(gpointer user_data);

static void ultra_airflow_out_monitoring(UltraAirflowObject *airflow) {
  gdouble summ = 0.0;
  gdouble round = 0.0;
  guint index = 0;
  gdouble val = airflow_sensor_get_current_value_out(
                    airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))) *
                60.0;
  airflow->priv->out_monitoring =
      g_array_append_val(airflow->priv->out_monitoring, val);

  for (index = 0; index < airflow->priv->out_monitoring->len; index++) {
    summ += g_array_index(airflow->priv->out_monitoring, gdouble, index);
  }
  round = summ / ((gdouble)airflow->priv->out_monitoring->len);
  if (airflow->priv->out_monitoring->len > 60) {

    airflow->priv->out_monitoring =
        g_array_remove_index(airflow->priv->out_monitoring, 0);
    gdouble correction = airflow_sensor_get_correction(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    gdouble soll_value = airflow_sensor_get_soll_value(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    gdouble div = airflow_sensor_get_deviation(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));

    gboolean isError = ((round * (1. + (1. / 100. * correction))) <
                        (((100.0 - div) / 100) * soll_value)) ||
                       ((round * (1. - (1. / 100. * correction))) >
                        (((100.0 + div) / 100) * soll_value));
    if (airflow_sensor_get_out_of_range(
            airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))) != isError)
      airflow_sensor_set_out_of_range(
          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), isError);
    airflow_in_error(airflow, isError);
  }
}

static void
ultra_airflow_object_change_current_out(UltraAirflowObject *airflow) {

  gdouble summ = 0.0;
  gdouble round = 0.0;
  guint index = 0;
  gdouble val = airflow_sensor_get_current_value_out(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
  airflow->priv->out_array = g_array_append_val(airflow->priv->out_array, val);

  for (index = 0; index < airflow->priv->out_array->len; index++) {
    summ += g_array_index(airflow->priv->out_array, gdouble, index);
  }
  round = summ / ((gdouble)airflow->priv->out_array->len);
  airflow_sensor_set_round_value_out(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), round);
  airflow_sensor_set_air_out(airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)),
                             round * 60.);
  if (airflow->priv->out_array->len > 10)
    airflow->priv->out_array =
        g_array_remove_index(airflow->priv->out_array, 0);
}

static void ultra_airflow_AIRFLOWOUT_async_callback(GObject *source_object,
                                                    GAsyncResult *res,
                                                    gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  GError *error = NULL;
  gdouble out_value = 0.0;
  if (!nodes_analog1_call_get_in2_finish(airflow->priv->analog_node1,
                                         &out_value, res, &error)) {
    mkt_log_error_message_sync(
        "Airflow: connection to 'analog node 1' is failed");
    if (error) {
      g_error_free(error);
    }
    airflow_sensor_set_air_out(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), 0.0);
    airflow_brocked_sensor(airflow);
  } else {
    gdouble signal = ultra_airflow_signal(airflow, out_value, FALSE);
    airflow_sensor_set_current_value_out(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), signal);
    if (airflow_sensor_get_smooth(
            airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))))
      ultra_airflow_set_row_out(airflow, signal);
    ultra_airflow_object_change_current_out(airflow);
    ultra_airflow_out_monitoring(airflow);
    airflow->priv->tag = g_timeout_add_seconds_full(
        G_PRIORITY_DEFAULT, 1, ultra_airflow_object_waite_read, airflow,
        ultra_airflow_object_waite_read_destroy);
  }
}

static void ultra_airflow_in_monitoring(UltraAirflowObject *airflow) {
  gdouble summ = 0.0;
  gdouble round = 0.0;
  guint index = 0;
  gdouble val = airflow_sensor_get_current_value_in(
                    airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))) *
                60.0;
  airflow->priv->in_monitoring =
      g_array_append_val(airflow->priv->in_monitoring, val);

  for (index = 0; index < airflow->priv->in_monitoring->len; index++) {
    summ += g_array_index(airflow->priv->in_monitoring, gdouble, index);
  }
  round = summ / ((gdouble)airflow->priv->in_monitoring->len);
  if (airflow->priv->in_monitoring->len > 60) {
    airflow->priv->in_monitoring =
        g_array_remove_index(airflow->priv->in_monitoring, 0);
    gdouble correction = airflow_sensor_get_correction(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    gdouble soll_value = airflow_sensor_get_soll_value(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    gdouble div = airflow_sensor_get_deviation(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));

    gboolean isError = ((round * (1. + (1. / 100. * correction))) <
                        (((100.0 - div) / 100) * soll_value)) ||
                       ((round * (1. - (1. / 100. * correction))) >
                        (((100.0 + div) / 100) * soll_value));
    if (airflow_sensor_get_out_of_range(
            airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))) != isError)
      airflow_sensor_set_out_of_range(
          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), isError);
  }
}

static void
ultra_airflow_object_change_current_in(UltraAirflowObject *airflow) {
  gdouble summ = 0.0;
  gdouble round = 0.0;
  guint index = 0;
  gdouble val = airflow_sensor_get_current_value_in(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
  airflow->priv->in_array = g_array_append_val(airflow->priv->in_array, val);

  for (index = 0; index < airflow->priv->in_array->len; index++) {
    summ += g_array_index(airflow->priv->in_array, gdouble, index);
  }
  round = summ / ((gdouble)airflow->priv->in_array->len);
  airflow_sensor_set_round_value_in(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), round);
  airflow_sensor_set_air_in(airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)),
                            round * 60.);
  if (airflow->priv->in_array->len > 10)
    airflow->priv->in_array = g_array_remove_index(airflow->priv->in_array, 0);
}

static void ultra_airflow_AIRFLOWIN_async_callback(GObject *source_object,
                                                   GAsyncResult *res,
                                                   gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  GError *error = NULL;
  gdouble out_value = 0.0;
  if (!nodes_analog1_call_get_in1_finish(airflow->priv->analog_node1,
                                         &out_value, res, &error)) {
    mkt_log_error_message_sync(
        "Airflow: connection to analog node 1 is failed");
    if (error)
      g_error_free(error);
    airflow_brocked_sensor(airflow);
  } else {
    gdouble signal = ultra_airflow_signal(airflow, out_value, TRUE);
    airflow_sensor_set_current_value_in(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), signal);
    if (airflow_sensor_get_smooth(
            airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))))
      ultra_airflow_set_row_in(airflow, signal);
    ultra_airflow_object_change_current_in(airflow);
    ultra_airflow_in_monitoring(airflow);
    nodes_analog1_call_get_in2(
        airflow->priv->analog_node1, airflow->priv->check,
        ultra_airflow_AIRFLOWOUT_async_callback, airflow);
  }
}

gboolean ultra_airflow_object_waite_read(gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  g_cancellable_cancel(airflow->priv->check);
  g_cancellable_reset(airflow->priv->check);
  if (airflow->priv->analog_node1)
    nodes_analog1_call_get_in1(airflow->priv->analog_node1,
                               airflow->priv->check,
                               ultra_airflow_AIRFLOWIN_async_callback, airflow);
  else
    return TRUE;
  return FALSE;
}

void ultra_airflow_object_waite_read_destroy(gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  airflow->priv->tag = 0;
}

// static void add_injection_analyze_line(gdouble signal, gdouble treshold,
// gdouble airflow_row)
// {
// 	gchar *path = g_build_path("/", g_get_home_dir(),
// "airflow-in-analyze.log", NULL); 	FILE *f = NULL; 	f = fopen(path,
// "a"); 	if (f
// != NULL)
// 	{
// 		gboolean isError = signal <= (1.0 + treshold / 100.0) *
// airflow_row; 		fprintf(f, "%s
// isError=%d;signal=%f;threshold=%f;airflow_row_id=%f(res=%f)\n",
// market_db_get_date_sqlite_format(market_db_time_now()), isError, signal,
// treshold, airflow_row, 				(1.0 + treshold / 100.0)
// * airflow_row); 		fflush(f); 		fclose(f);
// 	}
// 	g_free(path);
// }

// static void clean_injection_analyze_log()
// {
// 	gchar *path = g_build_path("/", g_get_home_dir(),
// "airflow-in-analyze.log", NULL); 	g_remove(path); 	g_free(path);
// }
static void ultra_airflow_AIRFLOW_injection_anayse_in_async_callback(
    GObject *source_object, GAsyncResult *res, gpointer user_data);
/*static GQuark airflow_error_quark(void) {
    static GQuark error;
    if (!error) error = g_quark_from_static_string("airflow-error");
    return error;
   }
 */
// static GError *NODES_REQUEST_ERROR() { return
// g_error_new(airflow_error_quark(), 1, "Node request error"); }

static void WAIT_BUS_IN_timeout_callback(GObject *source_object,
                                         GAsyncResult *res,
                                         gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  UltraAirflowObject *airflow =
      ULTRA_AIRFLOW_OBJECT(g_task_get_source_object(task));
  nodes_analog1_call_get_in1(
      airflow->priv->analog_node1, g_task_get_cancellable(task),
      ultra_airflow_AIRFLOW_injection_anayse_in_async_callback, task);
}

static void ultra_airflow_AIRFLOW_injection_anayse_in_async_callback(
    GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GError *error = NULL;
  gdouble out_value = 0.0;
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  UltraAirflowObject *airflow =
      ULTRA_AIRFLOW_OBJECT(g_task_get_source_object(task));
  nodes_analog1_call_get_in1_finish(NODES_ANALOG1(source_object), &out_value,
                                    res, &error);
  if (error) {
    airflow_brocked_sensor(airflow);
    g_task_return_error(task, error);
    g_object_unref(task);
    return;
  }
  GTimer *timer = (GTimer *)g_task_get_task_data(task);
  if (g_timer_elapsed(timer, NULL) <
      airflow_sensor_get_inj_analyse_timeout(
          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)))) {

    gdouble signal = ultra_airflow_signal(airflow, out_value, TRUE);
    gdouble treshold = airflow_sensor_get_injection_error_threshold(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    if (signal < airflow_sensor_get_inj_flow_row_in(
                     airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))))
      airflow_sensor_set_inj_flow_row_in(
          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), signal);
    //
    gboolean isError =
        signal >= (1.0 + treshold / 100.0) *
                      airflow_sensor_get_inj_flow_row_in(
                          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    // add_injection_analyze_line(signal,treshold,airflow_sensor_get_inj_flow_row_in(airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))));
    //    airflow_sensor_get_inj_flow_row_in(airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))),airflow_sensor_get_inj_flow_row_in(airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))),treshold);
    if (!isError) {
      g_task_return_boolean(task, TRUE);
      g_object_unref(task);
    } else {
      lar_timer_default_run(g_task_get_cancellable(task),
                            WAIT_BUS_IN_timeout_callback, 0.20, task);
    }
  } else {
    g_task_return_boolean(task, FALSE);
    g_object_unref(task);
  }
}

static void ultra_airflow_IN_object_injection_analyse_done(
    GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GError *error = NULL;
  GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
  if (g_dbus_connection_is_closed(
          g_dbus_method_invocation_get_connection(invocation)))
    return;
  gboolean result = g_task_propagate_boolean(G_TASK(res), &error);
  if (error) {
    g_dbus_method_invocation_return_dbus_error(
        invocation, "com.lar.airflow.Error.InjectionAnalyse",
        error != NULL ? error->message : "unknown error");
  } else {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(b)", result));
  }
  g_object_unref(invocation);
}

static gboolean ultra_airflow_object_injection_analyse_in_callback(
    AchsenAchse *interface, GDBusMethodInvocation *invocation,
    gpointer user_data) {
  static GCancellable *cancelable = NULL;
  UltraAirflowObject *airflow_object = ULTRA_AIRFLOW_OBJECT(user_data);
  if (cancelable) {
    g_cancellable_cancel(airflow_object->priv->activity);
    g_object_unref(airflow_object->priv->activity);
  }
  cancelable = g_cancellable_new();
  if (airflow_object->priv->analog_node1 == NULL) {
    g_dbus_method_invocation_return_dbus_error(
        invocation, "com.lar.airflow.Error.ConnectionFailed",
        "Node /com/lar/nodes/Analog1 connection is fail");
    return TRUE;
  }
  // clean_injection_analyze_log();
  GTask *task = g_task_new(airflow_object, cancelable,
                           ultra_airflow_IN_object_injection_analyse_done,
                           g_object_ref(invocation));
  GTimer *timer = g_timer_new();
  g_timer_start(timer);
  g_task_set_task_data(task, timer, (GDestroyNotify)g_timer_destroy);
  gdouble round_in = airflow_sensor_get_round_row_in(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow_object)));
  airflow_sensor_set_inj_flow_row_in(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow_object)), round_in);
  nodes_analog1_call_get_in1(
      airflow_object->priv->analog_node1, g_task_get_cancellable(task),
      ultra_airflow_AIRFLOW_injection_anayse_in_async_callback, task);

  return TRUE;
}
static void ultra_airflow_AIRFLOW_injection_anayse_out_async_callback(
    GObject *source_object, GAsyncResult *res, gpointer user_data);

static void WAIT_BUS_OUT_timeout_callback(GObject *source_object,
                                          GAsyncResult *res,
                                          gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  UltraAirflowObject *airflow =
      ULTRA_AIRFLOW_OBJECT(g_task_get_source_object(task));
  nodes_analog1_call_get_in2(
      airflow->priv->analog_node1, g_task_get_cancellable(task),
      ultra_airflow_AIRFLOW_injection_anayse_out_async_callback, task);
}

void ultra_airflow_AIRFLOW_injection_anayse_out_async_callback(
    GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GError *error = NULL;
  gdouble out_value = 0.0;
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  UltraAirflowObject *airflow =
      ULTRA_AIRFLOW_OBJECT(g_task_get_source_object(task));
  nodes_analog1_call_get_in2_finish(NODES_ANALOG1(source_object), &out_value,
                                    res, &error);
  if (error) {
    g_task_return_error(task, error);
    g_object_unref(task);
    airflow_brocked_sensor(airflow);
    return;
  }

  GTimer *timer = (GTimer *)g_task_get_task_data(task);
  if (g_timer_elapsed(timer, NULL) <
      airflow_sensor_get_inj_analyse_timeout(
          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)))) {

    gdouble signal = ultra_airflow_signal(airflow, out_value, FALSE);
    gdouble treshold = airflow_sensor_get_injection_error_threshold(
        airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    // add_injection_analyze_line(signal, treshold,
    // airflow_sensor_get_inj_flow_row_out(airflow_object_get_sensor(AIRFLOW_OBJECT(airflow))));

    gboolean isError =
        signal <= (1.0 + treshold / 100.0) *
                      airflow_sensor_get_inj_flow_row_out(
                          airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)));
    if (!isError) {
      g_task_return_boolean(task, TRUE);
      g_object_unref(task);
    } else {
      lar_timer_default_run(g_task_get_cancellable(task),
                            WAIT_BUS_OUT_timeout_callback, 0.20, task);
    }
  } else {
    g_task_return_boolean(task, FALSE);
    g_object_unref(task);
  }
}
static void ultra_airflow_OUT_object_injection_analyse_done(
    GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GError *error = NULL;
  GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
  if (g_dbus_connection_is_closed(
          g_dbus_method_invocation_get_connection(invocation)))
    return;
  gboolean result = g_task_propagate_boolean(G_TASK(res), &error);
  if (error) {
    g_dbus_method_invocation_return_dbus_error(
        invocation, "com.lar.airflow.Error.OUT.InjectionAnalyse",
        error != NULL ? error->message : "unknown error");
    return;
  } else {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(b)", result));
  }
  g_object_unref(invocation);
}

static gboolean ultra_airflow_object_injection_analyse_out_callback(
    AchsenAchse *interface, GDBusMethodInvocation *invocation,
    gpointer user_data) {
  static GCancellable *cancelable = NULL;
  UltraAirflowObject *airflow_object = ULTRA_AIRFLOW_OBJECT(user_data);
  if (cancelable) {
    g_cancellable_cancel(cancelable);
    g_object_unref(cancelable);
  }
  cancelable = g_cancellable_new();
  if (airflow_object->priv->analog_node1 == NULL) {
    g_dbus_method_invocation_return_dbus_error(
        invocation, "com.lar.airflow.Error.ConnectionFailed",
        "Node /com/lar/nodes/Analog1 connection is fail");
    return TRUE;
  }
  // clean_injection_analyze_log();
  GTask *task = g_task_new(airflow_object, cancelable,
                           ultra_airflow_OUT_object_injection_analyse_done,
                           g_object_ref(invocation));
  GTimer *timer = g_timer_new();
  g_timer_start(timer);
  g_task_set_task_data(task, timer, (GDestroyNotify)g_timer_destroy);
  gdouble round_out = airflow_sensor_get_round_row_out(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow_object)));
  airflow_sensor_set_inj_flow_row_out(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow_object)), round_out);

  nodes_analog1_call_get_in2(
      airflow_object->priv->analog_node1, cancelable,
      ultra_airflow_AIRFLOW_injection_anayse_out_async_callback, task);
  return TRUE;
}

static gboolean
ultra_airflow_object_calibration_callback(AchsenAchse *interface,
                                          GDBusMethodInvocation *invocation,
                                          gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  AirflowSensor *sensor = airflow_object_get_sensor(AIRFLOW_OBJECT(airflow));
  if (sensor == NULL) {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(b)", FALSE));
    return TRUE;
  }
  if (airflow_sensor_get_adjustment_factor(sensor) != 0.0) {
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(b)", FALSE));
    g_object_unref(sensor);
    return TRUE;
  }
  airflow_sensor_set_adjustment_factor(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), 0.0);
  gdouble in = airflow_sensor_get_round_value_in(sensor);
  gdouble out = airflow_sensor_get_round_value_out(sensor);
  gdouble adjustmentFactor = (in - out) / (in + out);
  /*if (adjustmentFactor > 0.1)
                  adjustmentFactor = 0.1;
          if (adjustmentFactor < -0.1)
                  adjustmentFactor = -0.1;*/
  airflow_sensor_set_adjustment_factor(sensor, adjustmentFactor);
  UltraAirflowObject *airflow_object = ULTRA_AIRFLOW_OBJECT(airflow);
  g_array_free(airflow_object->priv->in_array, TRUE);
  g_array_free(airflow_object->priv->out_array, TRUE);
  airflow_object->priv->in_array = g_array_new(FALSE, FALSE, sizeof(gdouble));
  airflow_object->priv->out_array = g_array_new(FALSE, FALSE, sizeof(gdouble));
  g_object_unref(sensor);

  g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
  return TRUE;
}
static gboolean
ultra_airflow_object_reset_callback(AchsenAchse *interface,
                                    GDBusMethodInvocation *invocation,
                                    gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  g_array_free(airflow->priv->in_array, TRUE);
  g_array_free(airflow->priv->out_array, TRUE);
  airflow->priv->in_array = g_array_new(FALSE, FALSE, sizeof(gdouble));
  airflow->priv->out_array = g_array_new(FALSE, FALSE, sizeof(gdouble));
  airflow_sensor_set_adjustment_factor(
      airflow_object_get_sensor(AIRFLOW_OBJECT(airflow)), 0.0);
  g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
  return TRUE;
}

static void ultra_airflow_object_furnace_way_callback(
    AirflowSensor *sensor, GParamSpec *pspec, UltraAirflowObject *airflow) {
  if (airflow->priv->last_state != airflow_sensor_get_furnace_way(sensor)) {
    airflow->priv->last_state = airflow_sensor_get_furnace_way(sensor);
    airflow_sensor_set_switch_way(sensor, market_db_time_now());
  }
  _valves_set_digital(airflow, airflow->priv->last_state);
}
static void ultra_airflow_object_smooth_callback(AirflowSensor *sensor,
                                                 GParamSpec *pspec,
                                                 UltraAirflowObject *airflow) {
  if (airflow_sensor_get_smooth(sensor)) {
    if (airflow->priv->out_row)
      g_array_free(airflow->priv->out_row, TRUE);
    airflow->priv->out_row = g_array_new(FALSE, FALSE, sizeof(gdouble));
  }
}

static gboolean airflow_init_default_way(gpointer user_data) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(user_data);
  _valves_set_digital(airflow, airflow->priv->last_state);
  return FALSE;
}

gboolean airflow_check_1830_error_callback(gpointer user_data) {
  E1830_check(ULTRA_AIRFLOW_OBJECT(user_data));
  return TRUE;
}

static void
ultra_airflow_object_critical_callback(AirflowSensor *sensor, GParamSpec *pspec,
                                       UltraAirflowObject *airflow) {
  MktError *error = mkt_error_find(E1835);
  if (error) {
    gchar type = airflow_sensor_get_critical_error(sensor) &&
                         airflow_sensor_get_is_online(sensor)
                     ? MKT_ERROR_CRITICAL
                     : MKT_ERROR_WARNING;
    mkt_error_set_type(error, type);
  }
}
static void ultra_airflow_object_online_callback(AirflowSensor *sensor,
                                                 GParamSpec *pspec,
                                                 UltraAirflowObject *airflow) {
  MktError *error = mkt_error_find(E1835);
  if (error) {
    if (!mkt_error_pending(error) && airflow_sensor_get_is_online(sensor)) {
      airflow->priv->start = FALSE;
    }
    if (!mkt_error_pending(error) || airflow_sensor_get_is_online(sensor)) {
      gchar type = airflow_sensor_get_critical_error(sensor) &&
                           airflow_sensor_get_is_online(sensor)
                       ? MKT_ERROR_CRITICAL
                       : MKT_ERROR_WARNING;
      mkt_error_set_type(error, type);
    }
  }
}

static void ultra_airflow_object_constructed(GObject *object) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(object);
  mkt_errors_clean(E1830);
  mkt_errors_clean(E1835);
  mkt_errors_clean(E1836);
  mkt_errors_clean(E1837);

  AirflowSensor *sensor = airflow_sensor_skeleton_new();
  airflow_object_skeleton_set_sensor(AIRFLOW_OBJECT_SKELETON(airflow), sensor);

  g_settings_bind(airflow->priv->airflow_settings, "critical-error", sensor,
                  "critical-error", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(airflow->priv->airflow_settings, "soll-value", sensor,
                  "soll-value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(airflow->priv->airflow_settings, "correction-value", sensor,
                  "correction", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(airflow->priv->airflow_settings, "deviation-value", sensor,
                  "deviation", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(airflow->priv->airflow_settings, "adjustment-factor", sensor,
                  "adjustment-factor", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(airflow->priv->airflow_settings, "inj-analyse-time", sensor,
                  "inj-analyse-timeout", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(airflow->priv->airflow_settings, "injection-error-threshold",
                  sensor, "injection-error-threshold", G_SETTINGS_BIND_DEFAULT);

  airflow->priv->tc_error = FALSE;
  airflow->priv->tic_error = FALSE;
  airflow->priv->is_1830 = FALSE;
  airflow->priv->is_1835 = FALSE;
  airflow->priv->is_1836 = FALSE;
  airflow->priv->is_1837 = FALSE;

  // g_signal_connect (sensor,"handle-")

  g_signal_connect(
      sensor, "handle-injection-analyse-in",
      G_CALLBACK(ultra_airflow_object_injection_analyse_in_callback), airflow);
  g_signal_connect(
      sensor, "handle-injection-analyse-out",
      G_CALLBACK(ultra_airflow_object_injection_analyse_out_callback), airflow);
  g_signal_connect(sensor, "handle-calibration",
                   G_CALLBACK(ultra_airflow_object_calibration_callback),
                   airflow);
  g_signal_connect(sensor, "handle-reset",
                   G_CALLBACK(ultra_airflow_object_reset_callback), airflow);
  g_signal_connect(sensor, "notify::furnace-way",
                   G_CALLBACK(ultra_airflow_object_furnace_way_callback),
                   airflow);
  g_signal_connect(sensor, "notify::smooth",
                   G_CALLBACK(ultra_airflow_object_smooth_callback), airflow);
  g_signal_connect(sensor, "notify::critical-error",
                   G_CALLBACK(ultra_airflow_object_critical_callback), airflow);
  g_signal_connect(sensor, "notify::is-online",
                   G_CALLBACK(ultra_airflow_object_online_callback), airflow);

  airflow->priv->last_state = FALSE;
  g_timeout_add(300, airflow_init_default_way, airflow);
  g_object_unref(sensor);
  airflow->priv->analog_node1 = ultra_airflow_get_NodesObject(airflow);
  airflow->priv->tag = g_timeout_add_seconds_full(
      G_PRIORITY_DEFAULT, 2, ultra_airflow_object_waite_read, airflow,
      ultra_airflow_object_waite_read_destroy);
  airflow->priv->start = TRUE;
  g_timeout_add_seconds(2, airflow_check_1830_error_callback, airflow);
  if (G_OBJECT_CLASS(ultra_airflow_object_parent_class)->constructed)
    G_OBJECT_CLASS(ultra_airflow_object_parent_class)->constructed(object);
}

// static void ultra_airflow_settings_changed(GSettings *settings, gchar *key,
// gpointer user_data) { 	GSettingsSchema *   schema;
// GSettingsSchemaKey *skey; 	g_object_get(settings, "settings-schema",
// &schema, NULL); 	skey = g_settings_schema_get_key(schema, key); 	if (0 ==
// g_strcmp0("soll-value", key)) { 		GVariant *value =
// g_settings_get_value(settings, key); 		gdouble val   =
// g_variant_get_double(value); g_settings_schema_key_unref(skey);
// 		g_variant_unref(value);
// 	}
// 	g_settings_schema_unref(schema);
// }

static void
ultra_airflow_object_init(UltraAirflowObject *ultra_airflow_object) {
  UltraAirflowObjectPrivate *priv = ULTRA_AIRFLOW_OBJECT_PRIVATE(ultra_airflow_object);
  priv->timer = NULL;
  priv->nodes_object = NULL;
  priv->analog_node1 = NULL;
  priv->airflow_settings = g_settings_new("com.lar.tera.airflow");
  // g_signal_connect(priv->airflow_settings, "changed",
  // G_CALLBACK(ultra_airflow_settings_changed), ultra_airflow_object);

  priv->in_array = g_array_new(FALSE, FALSE, sizeof(gdouble));
  priv->in_row = g_array_new(FALSE, FALSE, sizeof(gdouble));
  priv->out_row = g_array_new(FALSE, FALSE, sizeof(gdouble));
  priv->out_array = g_array_new(FALSE, FALSE, sizeof(gdouble));
  priv->in_monitoring = g_array_new(FALSE, FALSE, sizeof(gdouble));
  priv->out_monitoring = g_array_new(FALSE, FALSE, sizeof(gdouble));
  priv->activity = g_cancellable_new();
  priv->check = g_cancellable_new();
  ultra_airflow_object->priv = priv;

  // Settings property connection ...
  /* TODO: Add initialization code here */
}

static void ultra_airflow_object_finalize(GObject *object) {
  UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(object);
  g_cancellable_cancel(airflow->priv->activity);
  g_object_unref(airflow->priv->activity);
  g_cancellable_cancel(airflow->priv->check);
  g_object_unref(airflow->priv->check);
  if (airflow->priv->tag)
    g_source_remove(airflow->priv->tag);
  if (airflow->priv->timer)
    g_timer_destroy(airflow->priv->timer);
  if (airflow->priv->airflow_settings)
    g_object_unref(airflow->priv->airflow_settings);
  G_OBJECT_CLASS(ultra_airflow_object_parent_class)->finalize(object);
}

static void ultra_airflow_object_set_property(GObject *object, guint prop_id,
                                              const GValue *value,
                                              GParamSpec *pspec) {
  g_return_if_fail(ULTRA_IS_AIRFLOW_OBJECT(object));
  // UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(object);
  switch (prop_id) {

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void ultra_airflow_object_get_property(GObject *object, guint prop_id,
                                              GValue *value,
                                              GParamSpec *pspec) {
  g_return_if_fail(ULTRA_IS_AIRFLOW_OBJECT(object));
  // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.UltraAirflowInterface",value,pspec))
  // return; UltraAirflowObject *airflow = ULTRA_AIRFLOW_OBJECT(object);
  switch (prop_id) {

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void ultra_airflow_object_class_init(UltraAirflowObjectClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(UltraAirflowObjectPrivate));
  object_class->finalize = ultra_airflow_object_finalize;
  object_class->set_property = ultra_airflow_object_set_property;
  object_class->get_property = ultra_airflow_object_get_property;
  object_class->constructed = ultra_airflow_object_constructed;

  /*	klass->check_airflow        = NULL;
     klass->raw_value           = NULL;*/
}

/** @} */
