/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @file  ultra-integration-object.c	Pump object
 * @brief This is Pump control object description.
 *  Copyright (C) LAR  2015
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>
#include "../../config.h"
#include "ultra-stream-object.h"

#include "measurement-application-object.h"
#include "ultimate-integration.h"
#include "ultra-channel-object.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"

#include <math.h>

#include <glib/gi18n-lib.h>
// Gerhard: warning: implicit declaration of function ‘ConfigureDevice’ [-W
// mplicit-function-declaration]
#include <ultraconfig.h>

enum
{
  PROP_0,
  PROP_INTEGRATION_OBJECT_SENSOR,
  PROP_INTEGRATION_OBJECT_HAVE_TIC,
  PROP_INTEGRATION_OBJECT_IS_TIC,

};

static GDBusObjectManagerServer *INTEGRATION_SERVER = NULL;

struct _UltraIntegrationObjectPrivate
{
  guint type;
  SignalObject *sensor;
  gboolean is_tic;
  gboolean have_tic;
  guint number;
  SignalObject *old_signal;
  gulong singnal_connect;
  GCancellable *cancel;
  GSList *integrations_data;
  GSList *transmit_data;
  MktSensorData *pre;
};

#define ULTRA_INTEGRATION_OBJECT_PRIVATE(o)                        \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_INTEGRATION_OBJECT, \
                               UltraIntegrationObjectPrivate))

static void
ULTRA_INTEGRATION_SIGNAL_TRIGGER(UltraIntegrationObject *integration)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  guint triger = integration_simple_get_signal_trigger(simple);
  integration_simple_set_signal_trigger(simple, triger + 1);
}

static gdouble START_MIN_TIME(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_start_min_time(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_start_min_time(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}

static gdouble STOP_MIN_TIME(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_stop_min_time(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_stop_min_time(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}
static gdouble STOP_MAX_TIME(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_stop_max_time(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_stop_max_time(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}
static gdouble START_THRESHOLD(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_start_threshold(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_start_threshold(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}
static gdouble STOP_THRESHOLD(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_stop_threshold(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_stop_threshold(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}

static void SET_LAST_ZERO(UltraIntegrationObject *integration, gdouble value)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    integration_tc_set_last_zero(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)), value);
  else
    integration_tic_set_last_zero(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)), value);
}
static gdouble GET_LAST_ZERO(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_last_zero(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_last_zero(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}
static void SET_LAST_ZERO_CV(UltraIntegrationObject *integration,
                             gdouble value)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    integration_tc_set_last_zero_cv(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)), value);
  else
    integration_tic_set_last_zero_cv(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)), value);
}
static void SET_INTEGRAL(UltraIntegrationObject *integration, gdouble value)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    integration_tc_set_integral(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)), value);
  else
    integration_tic_set_integral(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)), value);
}

static void SET_ROW_INTEGRAL(UltraIntegrationObject *integration,
                             gdouble value)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    integration_tc_set_row_integral(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)), value);
  else
    integration_tic_set_row_integral(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)), value);
}

static gdouble GET_ROW_INTEGRAL(UltraIntegrationObject *integration)
{
  if (!integration->priv->is_tic || !integration->priv->have_tic)
    return integration_tc_get_row_integral(
        integration_object_get_tc(INTEGRATION_OBJECT(integration)));
  else
    return integration_tic_get_row_integral(
        integration_object_get_tic(INTEGRATION_OBJECT(integration)));
}

static void
ultra_integration_transmit_history(UltraIntegrationObject *integration)
{
  if (integration->priv->integrations_data == NULL)
    return;
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  g_return_if_fail(simple != NULL);
  mkt_sensor_data_append_history(integration_simple_get_creator(simple),
                                 integration->priv->integrations_data, NULL,
                                 NULL, NULL);
  if(integration->priv->pre)g_object_unref(integration->priv->pre);
  integration->priv->pre = NULL;
   g_slist_free_full(integration->priv->integrations_data, g_object_unref);
  integration->priv->integrations_data = NULL;
}

static void ultra_integration_add_data(UltraIntegrationObject *integration,
                                       gdouble value, guint state)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  if (simple)
  {
    guint trigger = integration_simple_get_trigger(simple);
    MktSensorData *data = mkt_sensor_data_new(value, trigger, state);
    integration->priv->integrations_data = g_slist_append(
        integration->priv->integrations_data, g_object_ref(data));
    // mkt_sensor_data_new
    GSList *l = NULL;
    l = g_slist_append(l, data);
    mkt_sensor_data_append_sync(integration_simple_get_creator(simple), l);
    g_slist_free(l);
    g_object_unref(data);
    g_object_unref(simple);
  }
}

static void
ultra_integration_calculate_last_zerro(UltraIntegrationObject *integration)
{
  g_return_if_fail(integration != NULL);
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  g_return_if_fail(simple != NULL);
  GSList *datas = integration->priv->integrations_data;
  if (datas == NULL)
  {
    integration_simple_set_justifying_error(simple, TRUE);
    mkt_log_error_message("Measurement: justification data is not found");
    mkt_errors_come(E2010);
    return;
  }
  gdouble yi = 0.0;
  gdouble Sy = 0.;
  gdouble Ay = 0.;
  gdouble standardDeviation = 0.0;
  gdouble sum_d_sqr = 0.0;
  gdouble N = 0.0;
  gdouble CV = 100.0;
  GSList *l = NULL;
  gint length = 0;
  for (l = datas; l != NULL; l = l->next)
  {
    length++;
    if (l->data && mkt_sensor_data_state(MKT_SENSOR_DATA(l->data)) == ULTIMATE_SIGNAL_JUSTIFICATION_DATA)
    {
      yi = mkt_sensor_data_value(MKT_SENSOR_DATA(l->data));
      Sy += yi;
      N += 1;
      Ay = Sy / N;
      sum_d_sqr += (yi - Ay) * (yi - Ay);
      if (N > 1.0)
      {
        standardDeviation = sqrt(sum_d_sqr / (N - 1));
        CV = 100. * standardDeviation / (Ay != 0.0 ? fabs(Ay) : 1.0);
      }
    }
  }
  ultra_integration_add_data(integration, Ay, ULTIMATE_SIGNAL_JUSTIFICATED);
  SET_LAST_ZERO(integration, Ay);
  SET_LAST_ZERO_CV(integration, CV);
  integration_simple_set_justifying(simple, FALSE);
  integration_simple_set_justifyed(simple, TRUE);
}

static void
ultra_integration_calc_integration(UltraIntegrationObject *integration)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  GSList *datas = integration->priv->integrations_data;
  if (datas == NULL || integration_simple_get_justifying_error(simple))
  {
    integration_simple_set_integrating_error(simple, FALSE);
  }
  else
  {
    gdouble last_zerro = GET_LAST_ZERO(integration);
    gdouble valley = 0.0;
    gdouble mountain = 0.0;
    MktSensorData *pre = NULL;
    for (; datas != NULL; datas = datas->next)
    {
      if (mkt_sensor_data_state(MKT_SENSOR_DATA(datas->data)) == ULTIMATE_SIGNAL_INTEGRATION_DATA)
      {
        gdouble new_data = mkt_sensor_data_value(MKT_SENSOR_DATA(datas->data));
        gdouble diff = mkt_sensor_data_past_from(MKT_SENSOR_DATA(datas->data), pre);
        pre = MKT_SENSOR_DATA(datas->data);
        gdouble area = 0.0;
        area = (new_data - last_zerro) * diff;
        if (area >= 0.0)
          mountain = mountain + area;
        else
          valley = valley + area;
      }
    }

    SET_INTEGRAL(integration, mountain);
    SET_ROW_INTEGRAL(integration, mountain);
  }
  integration_simple_set_integrated(simple, TRUE);
  integration_simple_set_integrated_time(simple, market_db_time_now());
  integration_simple_set_integrating(simple, FALSE);
  integration_simple_set_integration_start_time(simple, 0.0);
}

static void stop_sensor_signal(UltraIntegrationObject *integration)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  g_return_if_fail(simple != NULL);
  SignalObject *sig =
      SIGNAL_OBJECT(sensors_signal_manager_client_get_signal_object(
          integration_simple_get_sensor(simple)));

  g_object_unref(simple);
  if (sig)
  {
    SignalIfc *ifc = signal_object_get_ifc(sig);

    if (integration->priv->singnal_connect)
      if (g_signal_handler_is_connected(ifc, integration->priv->singnal_connect))
        g_signal_handler_disconnect(ifc,
                                    integration->priv->singnal_connect);
    integration->priv->singnal_connect = 0;
    signal_ifc_set_run(ifc, FALSE);
    g_object_unref(sig);
    g_object_unref(ifc);
  }
}
static gint
ultra_integration_set_integration(UltraIntegrationObject *integration,
                                  gdouble value)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  gdouble rtime = market_db_time_now();
  gboolean done = FALSE;
  gdouble wtime = START_MIN_TIME(integration);
  gdouble minstop = STOP_MIN_TIME(integration);
  gdouble maxstop = STOP_MAX_TIME(integration);
  gdouble last_zerro = GET_LAST_ZERO(integration);
  gdouble startthr = START_THRESHOLD(integration);
  gdouble stopthr = STOP_THRESHOLD(integration);

  gboolean calculating_runs = integration_simple_get_calculating(simple);
  gint ret = 0;
  if (rtime > integration_simple_get_integration_start_time(simple) + wtime)
  {
    if (calculating_runs)
    {
      ultra_integration_add_data(integration, value,
                                 ULTIMATE_SIGNAL_INTEGRATION_DATA);
      if (((value - last_zerro) < stopthr && (rtime > integration_simple_get_integration_start_time(simple) + minstop)) || (rtime > integration_simple_get_integration_start_time(simple) + maxstop))
      {
        ultra_integration_calc_integration(integration);
        ultra_integration_add_data(integration, value,
                                   ULTIMATE_SIGNAL_INTEGRATION_STOP);
        integration_simple_set_calculating(simple, FALSE);
        integration_simple_set_integrating(simple, FALSE);
        integration_simple_set_integrated(simple, TRUE);
        done = TRUE;
      }
      gdouble row_value = GET_ROW_INTEGRAL(integration);
      gdouble next_iter = (value - last_zerro);
      MktSensorData *curr = MKT_SENSOR_DATA(integration->priv->integrations_data->data);
      row_value = row_value + (next_iter * mkt_sensor_data_past_from(curr, integration->priv->pre));
      SET_ROW_INTEGRAL(integration, row_value);
      if(integration->priv->pre)g_object_unref(integration->priv->pre);
      integration->priv->pre = g_object_ref(curr);
      ret = 1;
    }
    else
    {
      if (value - last_zerro > startthr)
      {
        integration_simple_set_calculating(simple, TRUE);
        ultra_integration_add_data(integration, value,
                                   ULTIMATE_SIGNAL_INTEGRATION_START);
        integration_simple_set_calculate_time(simple, market_db_time_now());
        SET_ROW_INTEGRAL(integration, 0.0);
        integration_simple_set_integrated(simple, FALSE);
        ret = 1;
      }
      else if (rtime > integration_simple_get_integration_start_time(simple) +
                           maxstop)
      {
        ultra_integration_calc_integration(integration);
        ultra_integration_add_data(integration, value,
                                   ULTIMATE_SIGNAL_INTEGRATION_STOP);
        integration_simple_set_calculating(simple, FALSE);
        integration_simple_set_integrating(simple, FALSE);
      }
    }
  }
  g_object_unref(simple);
  if (done)
  {
    stop_sensor_signal(integration);
  }
  return ret;
}

static gboolean
ultra_integration_set_signal_value(UltraIntegrationObject *integration,
                                   gdouble value)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));

  integration_simple_set_signal(simple, value);
  if (integration_simple_get_justifying(simple))
  {
    ultra_integration_add_data(integration, value,
                               ULTIMATE_SIGNAL_JUSTIFICATION_DATA);
  }
  else if (integration_simple_get_integrating(simple))
  {
    if (0 == ultra_integration_set_integration(integration, value))
    {
      ultra_integration_add_data(integration, value,
                                 ULTIMATE_SIGNAL_ANALYSE_DATA);
    }
  }
  else
  {
    ultra_integration_add_data(integration, value,
                               ULTIMATE_SIGNAL_ANALYSE_DATA);
  }
  integration_simple_set_signal(simple, value);
  ULTRA_INTEGRATION_SIGNAL_TRIGGER(integration);
  return TRUE;
}

static void
ultra_integration_calculate_signal(UltraIntegrationObject *integration,
                                   gdouble value)
{
  ultra_integration_set_signal_value(integration, value);
}

void ultra_integration_object_analyse_break(
    UltraIntegrationObject *integration)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  integration_simple_set_calculating(simple, FALSE);
  integration_simple_set_justifying(simple, FALSE);
  integration_simple_set_integrating(simple, FALSE);
  SignalObject *signal =
      SIGNAL_OBJECT(sensors_signal_manager_client_get_signal_object(
          integration_simple_get_sensor(simple)));
  if (signal)
  {
    if (integration->priv->singnal_connect)
      if (g_signal_handler_is_connected(signal_object_get_ifc(signal),
                                        integration->priv->singnal_connect))
        g_signal_handler_disconnect(signal_object_get_ifc(signal),
                                    integration->priv->singnal_connect);
    integration->priv->singnal_connect = 0;
  }
  if (integration_simple_get_measure(simple))
  {
    signal_ifc_set_run(signal_object_get_ifc(signal), FALSE);
    integration_simple_set_measure(simple, FALSE);
  }
  ULTRA_INTEGRATION_SIGNAL_TRIGGER(integration);
  ultra_integration_transmit_history(integration);
}

static void ultra_integration_object_change_sensor_signal_callback(
    SignalIfc *signal_ifc, gdouble value, UltraIntegrationObject *integration)
{
  if (integration_simple_get_measure(
          integration_object_get_simple(INTEGRATION_OBJECT(integration))))
  {
    ultra_integration_calculate_signal(integration, value);
  }
}

gboolean ultra_integration_object_calculate_justification(
    UltraIntegrationObject *integration)
{
  ultra_integration_calculate_last_zerro(ULTRA_INTEGRATION_OBJECT(integration));
  return TRUE;
}

gboolean ultra_integration_object_calculate_integration(
    UltraIntegrationObject *integration)
{
  ultra_integration_calc_integration(ULTRA_INTEGRATION_OBJECT(integration));
  return TRUE;
}

gboolean ultra_integration_object_analyze(UltraIntegrationObject *integration,
                                          guint64 creator, guint trigger,
                                          gboolean is_tic)
{
  static GCancellable *cancelable = NULL;
  if (cancelable == NULL)
    cancelable = g_cancellable_new();

  ultra_integration_object_analyse_break(integration);
  integration->priv->is_tic = integration->priv->have_tic ? is_tic : FALSE;
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
 integration_simple_set_justifying_error(simple, FALSE);
  integration_simple_set_integrating_error(simple, FALSE);
  integration_simple_set_justifyed(simple, FALSE);
  integration_simple_set_integrated(simple, FALSE);
  integration_simple_set_creator(simple, creator);
  integration_simple_set_trigger(simple, trigger);
  integration_simple_set_measure(simple, TRUE);
  g_cancellable_cancel(cancelable);
  g_cancellable_reset(cancelable);
  MktSqlite *conn = mkt_sensor_data_conn(creator);
  mkt_sensor_data_clean(conn);
  g_object_unref(conn);
  ultra_integration_transmit_history(integration);
  mkt_sensor_data_clean_history(creator, 500, NULL, NULL, NULL);
  // if (trigger > 300) {
  //     mkt_model_delete_async(MKT_TYPE_SENSOR_DATA, cancelable, NULL, NULL,
  //     "delete from MktSensorData where data_creator=%" G_GUINT64_FORMAT " and
  //     data_trigger < %u", creator, trigger - 300);
  // }

  SignalObject *signal =
      SIGNAL_OBJECT(sensors_signal_manager_client_get_signal_object(
          integration_simple_get_sensor(simple)));
  if (signal)
  {
    if (integration->priv->singnal_connect)
      if (g_signal_handler_is_connected(signal_object_get_ifc(signal),
                                        integration->priv->singnal_connect))
        g_signal_handler_disconnect(signal_object_get_ifc(signal),
                                    integration->priv->singnal_connect);
    integration->priv->singnal_connect = g_signal_connect(
        signal_object_get_ifc(signal), "sensor-signal",
        G_CALLBACK(ultra_integration_object_change_sensor_signal_callback),
        integration);
    signal_ifc_set_run(signal_object_get_ifc(signal), TRUE);
  }
  ULTRA_INTEGRATION_SIGNAL_TRIGGER(integration);
  return TRUE;
}

gboolean
ultra_integration_object_justifying_run(UltraIntegrationObject *integration)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  integration_simple_set_justifyed(simple, FALSE);
  integration_simple_set_justificatein_start_time(simple, market_db_time_now());
  integration_simple_set_justifying(simple, TRUE);
  integration_simple_set_justifying_error(simple, FALSE);

  return TRUE;
}

gboolean
ultra_integration_object_integrating_run(UltraIntegrationObject *integration)
{
  IntegrationSimple *simple =
      integration_object_get_simple(INTEGRATION_OBJECT(integration));
  integration_simple_set_calculating(simple, FALSE);
  integration_simple_set_integrated(simple, FALSE);
  SET_ROW_INTEGRAL(integration, 0.0);
  SET_INTEGRAL(integration, 0.0);
  integration_simple_set_integrating_error(simple, FALSE);

  if (integration->priv->integrations_data)
    integration_simple_set_integrating(simple, TRUE);
  else
    integration_simple_set_integrating(simple, FALSE);

  ultra_integration_add_data(integration, integration_simple_get_signal(simple),
                             ULTIMATE_SIGNAL_INTEGRATION_RUN);
  integration_simple_set_integration_start_time(simple, market_db_time_now());
  return TRUE;
}

G_DEFINE_TYPE(UltraIntegrationObject, ultra_integration_object,
              INTEGRATION_TYPE_OBJECT_SKELETON)

#define NDIR1_PREFIX SENSORS_SIGNAL_MANAGE_PATH "/inst_1"

#define NDIR1_1_SUFFIX "_1_1"
#define NDIR1_2_SUFFIX "_1_2"
#define NDIR1_3_SUFFIX "_1_3"
#define NDIR2_PREFIX SENSORS_SIGNAL_MANAGE_PATH "/inst_2"
#define NDIR2_1_SUFFIX "_2_1"
#define NDIR2_2_SUFFIX "_2_2"
#define NDIR2_3_SUFFIX "_2_3"
#define TNb_PREFIX SENSORS_SIGNAL_MANAGE_PATH "/inst_3"
#define TNb__SUFFIX "_3_1"
#define O2_PREFIX SENSORS_SIGNAL_MANAGE_PATH "/inst_4"
#define O2__SUFFIX "_4_1"

static const gchar *
ultra_integration_signal_kind_full_name(SignalObject *signal_object)
{
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_1_SUFFIX))
    return "NDIR1_1";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_2_SUFFIX))
    return "NDIR1_2";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_3_SUFFIX))
    return "NDIR1_3";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_1_SUFFIX))
    return "NDIR2_1";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_2_SUFFIX))
    return "NDIR2_2";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_3_SUFFIX))
    return "NDIR2_3";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          TNb__SUFFIX))
    return "TNb";
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          O2__SUFFIX))
    return "CODo";
  return "Unknown";
}

static LarIntegration *
ultra_integration_signal_kind_tc_parameters(SignalObject *signal_object,
                                            LarIntegrations *integrations)
{
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_1_SUFFIX))
    return m_LarIntegrationsGetNDir1TC1(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_2_SUFFIX))
    return m_LarIntegrationsGetNDir1TC2(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_3_SUFFIX))
    return m_LarIntegrationsGetNDir1TC3(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_1_SUFFIX))
    return m_LarIntegrationsGetNDir2TC1(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_2_SUFFIX))
    return m_LarIntegrationsGetNDir2TC2(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_3_SUFFIX))
    return m_LarIntegrationsGetNDir2TC3(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          TNb__SUFFIX))
    return m_LarIntegrationsGetTNbTC(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          O2__SUFFIX))
    return m_LarIntegrationsGetCODo(integrations);
  return m_LarIntegrationsGetNDir1TC1(integrations);
}
static LarIntegration *
ultra_integration_signal_kind_tic_parameters(SignalObject *signal_object,
                                             LarIntegrations *integrations)
{
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_1_SUFFIX))
    return m_LarIntegrationsGetNDir1TIC1(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_2_SUFFIX))
    return m_LarIntegrationsGetNDir1TIC2(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_3_SUFFIX))
    return m_LarIntegrationsGetNDir1TIC3(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_1_SUFFIX))
    return m_LarIntegrationsGetNDir2TIC1(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_2_SUFFIX))
    return m_LarIntegrationsGetNDir2TIC2(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_3_SUFFIX))
    return m_LarIntegrationsGetNDir2TIC3(integrations);
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          TNb__SUFFIX))
    return m_LarIntegrationsGetTNbTIC(integrations);
  return m_LarIntegrationsGetNDir1TC1(integrations);
}

static void
ultra_integration_connect_model(UltraIntegrationObject *integration)
{
  gchar *patch = NULL;
  IntegrationTc *tc =
      integration_object_get_tc(INTEGRATION_OBJECT(integration));
  IntegrationTic *tic =
      integration_object_get_tic(INTEGRATION_OBJECT(integration));
  Ultradevice *udev = ConfigureDevice();
  LarIntegrations *integrations = UltradeviceGetIntegrations(udev);
  // Gerhard: warning: unused variable ‘initial’ [-Wunused-variable]
  //    gboolean           initial     = device_get_run_counter(DEVICE(udev)) ==
  //    1;
  if (tc == NULL)
    return;
  g_object_bind_property(ultra_integration_signal_kind_tc_parameters(
                             integration->priv->sensor, integrations),
                         "startThreshold", tc, "start-threshold",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tc_parameters(
                             integration->priv->sensor, integrations),
                         "stopThreshold", tc, "stop-threshold",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tc_parameters(
                             integration->priv->sensor, integrations),
                         "startMin", tc, "start-min-time",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tc_parameters(
                             integration->priv->sensor, integrations),
                         "stopMax", tc, "stop-max-time",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tc_parameters(
                             integration->priv->sensor, integrations),
                         "stopMin", tc, "stop-min-time",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

  patch = g_strdup_printf(
      "%s.TC", g_dbus_object_get_object_path(G_DBUS_OBJECT(integration)));
  UltimateIntegration *integrationTC =
      ULTIMATE_INTEGRATION(mkt_model_select_one(
          ULTIMATE_TYPE_INTEGRATION_OBJECT,
          "select * from %s where param_object_path = '%s' and param_type = "
          "'TC'",
          g_type_name(ULTIMATE_TYPE_INTEGRATION_OBJECT), patch));
  if (integrationTC != NULL)
  {
    g_object_bind_property(integrationTC, "start-threshold", tc,
                           "start-threshold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTC, "stop-threshold", tc,
                           "stop-threshold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTC, "start-min-time", tc,
                           "start-min-time",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTC, "stop-min-time", tc, "stop-min-time",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTC, "stop-max-time", tc, "stop-max-time",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    mkt_model_delete(MKT_MODEL(integrationTC));
    g_object_unref(integrationTC);
  }
  g_free(patch);
  if (tic == NULL)
  {
    return;
  }
  g_object_bind_property(ultra_integration_signal_kind_tic_parameters(
                             integration->priv->sensor, integrations),
                         "startThreshold", tic, "start-threshold",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tic_parameters(
                             integration->priv->sensor, integrations),
                         "stopThreshold", tic, "stop-threshold",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tic_parameters(
                             integration->priv->sensor, integrations),
                         "startMin", tic, "start-min-time",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tic_parameters(
                             integration->priv->sensor, integrations),
                         "stopMax", tic, "stop-max-time",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  g_object_bind_property(ultra_integration_signal_kind_tic_parameters(
                             integration->priv->sensor, integrations),
                         "stopMin", tic, "stop-min-time",
                         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  patch = g_strdup_printf(
      "%s.TIC", g_dbus_object_get_object_path(G_DBUS_OBJECT(integration)));
  UltimateIntegration *integrationTIC =
      ULTIMATE_INTEGRATION(mkt_model_select_one(
          ULTIMATE_TYPE_INTEGRATION_OBJECT,
          "select * from %s where param_object_path = '%s' and param_type = "
          "'TIC'",
          g_type_name(ULTIMATE_TYPE_INTEGRATION_OBJECT), patch));
  if (integrationTIC != NULL)
  {
    g_object_bind_property(integrationTIC, "start-threshold", tic,
                           "start-threshold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTIC, "stop-threshold", tic,
                           "stop-threshold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTIC, "start-min-time", tic,
                           "start-min-time",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTIC, "stop-min-time", tic,
                           "stop-min-time",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(integrationTIC, "stop-max-time", tic,
                           "stop-max-time",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    mkt_model_delete(MKT_MODEL(integrationTIC));
    g_object_unref(integrationTIC);
  }
  g_free(patch);
}

static void ultra_integration_object_init(
    UltraIntegrationObject *ultra_integration_object)
{
  UltraIntegrationObjectPrivate *priv =
      ULTRA_INTEGRATION_OBJECT_PRIVATE(ultra_integration_object);
  priv->sensor = NULL;
  priv->old_signal = NULL;
  priv->singnal_connect = 0;
  priv->cancel = g_cancellable_new();
  ultra_integration_object->priv = priv;
}

static void ultra_integration_object_finalize(GObject *object)
{
  UltraIntegrationObject *integration = ULTRA_INTEGRATION_OBJECT(object);
  if (integration->priv->sensor)
    g_object_unref(integration->priv->sensor);
  G_OBJECT_CLASS(ultra_integration_object_parent_class)->finalize(object);
}

static guint ultra_integration_signal_plug(SignalObject *signal_object)
{
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_1_SUFFIX))
    return 1;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_2_SUFFIX))
    return 2;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_3_SUFFIX))
    return 3;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_1_SUFFIX))
    return 4;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_2_SUFFIX))
    return 5;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_3_SUFFIX))
    return 6;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          O2__SUFFIX))
    return 7;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          TNb__SUFFIX))
    return 8;

  return ULTIMATE_SENSOR_KIND_UNKNOWN;
}

static guint ultra_integration_signal_kind(SignalObject *signal_object)
{
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_1_SUFFIX))
    return ULTIMATE_SENSOR_KIND_NDIR;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_2_SUFFIX))
    return ULTIMATE_SENSOR_KIND_NDIR;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_3_SUFFIX))
    return ULTIMATE_SENSOR_KIND_NDIR;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_1_SUFFIX))
    return ULTIMATE_SENSOR_KIND_NDIR;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_2_SUFFIX))
    return ULTIMATE_SENSOR_KIND_NDIR;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_3_SUFFIX))
    return ULTIMATE_SENSOR_KIND_NDIR;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          TNb__SUFFIX))
    return ULTIMATE_SENSOR_KIND_TNb;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          O2__SUFFIX))
    return ULTIMATE_SENSOR_KIND_CODo;
  return ULTIMATE_SENSOR_KIND_UNKNOWN;
}

static guint ultra_integration_signal_real_kind(SignalObject *signal_object)
{
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_1_SUFFIX))
    return 1;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_2_SUFFIX))
    return 1;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR1_3_SUFFIX))
    return 1;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_1_SUFFIX))
    return 2;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_2_SUFFIX))
    return 2;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          NDIR2_3_SUFFIX))
    return 2;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          TNb__SUFFIX))
    return 3;
  if (g_str_has_suffix(
          g_dbus_object_get_object_path(G_DBUS_OBJECT(signal_object)),
          O2__SUFFIX))
    return 4;
  return 0;
}

static void ultra_integration_object_system_stop_all_callback(
    SecurityDevice *security, gboolean is_done,
    UltraIntegrationObject *integration)
{
  ultra_integration_object_analyse_break(integration);
}

static IntegrationObject *_ndir1 = NULL;
static IntegrationObject *_ndir2 = NULL;
static IntegrationObject *_tnb = NULL;
static IntegrationObject *_codo = NULL;

static void ultra_integration_object_constructed(GObject *object)
{
  UltraIntegrationObject *integration = ULTRA_INTEGRATION_OBJECT(object);
  IntegrationSimple *simple = integration_simple_skeleton_new();
  integration->priv->integrations_data = NULL;
  integration->priv->pre = NULL;
  integration->priv->transmit_data = NULL;
  integration_object_skeleton_set_simple(INTEGRATION_OBJECT_SKELETON(object),
                                         simple);
  integration_simple_set_sensor(
      simple,
      g_dbus_object_get_object_path(G_DBUS_OBJECT(integration->priv->sensor)));
  integration_simple_set_plug(
      simple, ultra_integration_signal_plug(integration->priv->sensor));
  integration_simple_set_kind(
      simple, ultra_integration_signal_kind(integration->priv->sensor));
  integration_simple_set_activated(simple, TRUE);

  integration->priv->have_tic =
      integration_simple_get_kind(simple) == ULTIMATE_SENSOR_KIND_NDIR ||
      integration_simple_get_kind(simple) == ULTIMATE_SENSOR_KIND_TNb;
  integration_simple_set_name(simple, signal_ifc_get_name(signal_object_get_ifc(
                                          integration->priv->sensor)));
  integration_simple_set_unit(simple, _("fsr"));

  IntegrationTc *tc = integration_tc_skeleton_new();
  integration_object_skeleton_set_tc(INTEGRATION_OBJECT_SKELETON(object), tc);
  integration_tc_set_start_threshold(tc, 0.002);
  integration_tc_set_stop_threshold(tc, 0.003);
  integration_tc_set_start_min_time(tc, 10.00);
  integration_tc_set_stop_min_time(tc, 80.00);
  integration_tc_set_stop_max_time(tc, 120.00);

  if (integration->priv->have_tic)
  {
    IntegrationTic *tic = integration_tic_skeleton_new();
    integration_object_skeleton_set_tic(INTEGRATION_OBJECT_SKELETON(object),
                                        tic);
    integration_tic_set_start_threshold(tic, 0.002);
    integration_tic_set_stop_threshold(tic, 0.003);
    integration_tic_set_start_min_time(tic, 10.00);
    integration_tic_set_stop_min_time(tic, 80.00);
    integration_tic_set_stop_max_time(tic, 120.00);
  }
  ultra_integration_connect_model(integration);

  g_object_unref(simple);

  g_signal_connect(
      TERA_GUARD(), "stop-all",
      G_CALLBACK(ultra_integration_object_system_stop_all_callback),
      integration);
  switch (ultra_integration_signal_real_kind(integration->priv->sensor))
  {
  case 1:
    _ndir1 = INTEGRATION_OBJECT(g_object_ref(integration));
    break;
  case 2:
    _ndir2 = INTEGRATION_OBJECT(g_object_ref(integration));
    break;
  case 3:
    _tnb = INTEGRATION_OBJECT(g_object_ref(integration));
    break;
  case 4:
    _codo = INTEGRATION_OBJECT(g_object_ref(integration));
    break;
  default:
    g_warning("integration sensor unknown type %s %s",
              g_dbus_object_get_object_path(G_DBUS_OBJECT(integration)),
              g_dbus_object_get_object_path(
                  G_DBUS_OBJECT(integration->priv->sensor)));
    break;
  }
  G_OBJECT_CLASS(ultra_integration_object_parent_class)->constructed(object);
}

static void ultra_integration_object_set_property(GObject *object,
                                                  guint prop_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec)
{
  g_return_if_fail(ULTRA_IS_INTEGRATION_OBJECT(object));
  UltraIntegrationObject *integration = ULTRA_INTEGRATION_OBJECT(object);
  switch (prop_id)
  {
  case PROP_INTEGRATION_OBJECT_SENSOR:
    if (integration->priv->sensor)
      g_object_unref(integration->priv->sensor);
    integration->priv->sensor = g_value_dup_object(value);
    break;
  case PROP_INTEGRATION_OBJECT_HAVE_TIC:
    integration->priv->have_tic = g_value_get_boolean(value);
    break;
  case PROP_INTEGRATION_OBJECT_IS_TIC:
    integration->priv->is_tic = g_value_get_boolean(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void ultra_integration_object_get_property(GObject *object,
                                                  guint prop_id, GValue *value,
                                                  GParamSpec *pspec)
{
  g_return_if_fail(ULTRA_IS_INTEGRATION_OBJECT(object));
  UltraIntegrationObject *integration = ULTRA_INTEGRATION_OBJECT(object);
  switch (prop_id)
  {
  case PROP_INTEGRATION_OBJECT_SENSOR:
    g_value_set_object(value, integration->priv->sensor);
    break;
  case PROP_INTEGRATION_OBJECT_HAVE_TIC:
    g_value_set_boolean(value, integration->priv->have_tic);
    break;
  case PROP_INTEGRATION_OBJECT_IS_TIC:
    g_value_set_boolean(value, integration->priv->is_tic);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
ultra_integration_object_class_init(UltraIntegrationObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(UltraIntegrationObjectClass));
  object_class->finalize = ultra_integration_object_finalize;
  object_class->set_property = ultra_integration_object_set_property;
  object_class->get_property = ultra_integration_object_get_property;
  object_class->constructed = ultra_integration_object_constructed;
  // parent_class->initialize     = NULL;
  // parent_class->emergency_stop = NULL;

  g_object_class_install_property(
      object_class, PROP_INTEGRATION_OBJECT_SENSOR,
      g_param_spec_object("integration-sensor", "Integration sensor signal",
                          "Integration sensor signal", SIGNAL_TYPE_OBJECT,
                          G_PARAM_WRITABLE | G_PARAM_READABLE |
                              G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
      object_class, PROP_INTEGRATION_OBJECT_HAVE_TIC,
      g_param_spec_boolean("have-tic", "Integration have value tic",
                           "Integration have value tic", FALSE,
                           G_PARAM_WRITABLE | G_PARAM_READABLE |
                               G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
      object_class, PROP_INTEGRATION_OBJECT_IS_TIC,
      g_param_spec_boolean("is-tic", "Integration value tic",
                           "Integration value tic", FALSE,
                           G_PARAM_WRITABLE | G_PARAM_READABLE));
}

static void
ultra_integration_create_integration_object(SignalObject *signal_object)
{
  gchar *skeleton_path =
      g_strdup_printf("%s/%s", TERA_INTEGRATION_MANAGER,
                      ultra_integration_signal_kind_full_name(signal_object));
  IntegrationObject *integration = INTEGRATION_OBJECT(
      g_object_new(ULTRA_TYPE_INTEGRATION_OBJECT, "g-object-path",
                   skeleton_path, "integration-sensor", signal_object, NULL));
  g_dbus_object_manager_server_export(INTEGRATION_SERVER,
                                      G_DBUS_OBJECT_SKELETON(integration));
  g_object_unref(integration);
  g_free(skeleton_path);
}

static IntegrationObject *
ultra_integration_search_integration_for_signal(SignalObject *signal_object)
{
  GList *integrations = g_dbus_object_manager_get_objects(
      G_DBUS_OBJECT_MANAGER(INTEGRATION_SERVER));
  GList *chl = NULL;
  IntegrationObject *result_object = NULL;
  for (chl = integrations; chl != NULL; chl = chl->next)
  {
    IntegrationSimple *simple = NULL;
    simple = integration_object_get_simple(INTEGRATION_OBJECT(chl->data));
    if (simple)
    {
      if (0 == g_strcmp0(integration_simple_get_sensor(simple),
                         g_dbus_object_get_object_path(
                             G_DBUS_OBJECT(signal_object))))
      {
        result_object = INTEGRATION_OBJECT(chl->data);
        break;
      }
    }
  }
  g_list_free(integrations);
  return result_object;
}

static void
ultra_integration_create_new_integrations(SignalObject *signal_object)
{
  if (NULL != ultra_integration_search_integration_for_signal(signal_object))
    return;
  ultra_integration_create_integration_object(signal_object);
}

#undef NDIR1_PREFIX
#undef NDIR1_1_SUFFIX
#undef NDIR1_2_SUFFIX
#undef NDIR1_3_SUFFIX
#undef NDIR2_PREFIX
#undef NDIR2_1_SUFFIX
#undef NDIR2_2_SUFFIX
#undef NDIR2_3_SUFFIX
#undef TNb_SIGNAL
#undef TNb__SUFFIX
#undef O2_SIGNAL
#undef O2__SUFFIX

static void ultra_integration_signal_manager_on_object_added(
    GDBusObjectManager *manager, GDBusObject *object, gpointer user_data)
{
  ultra_integration_create_new_integrations(SIGNAL_OBJECT(object));
}

static void ultra_integration_signal_manager_on_object_removed(
    GDBusObjectManager *manager, GDBusObject *object, gpointer user_data) {}

void ultra_integration_server_run(GDBusConnection *connection)
{
  if (INTEGRATION_SERVER != NULL)
    return;
  INTEGRATION_SERVER =
      g_dbus_object_manager_server_new(TERA_INTEGRATION_MANAGER);
  g_dbus_object_manager_server_set_connection(INTEGRATION_SERVER, connection);

  GDBusObjectManager *sensors_manager =
      sensors_signal_manager_client_get_manager();
  GList *signal_list = g_dbus_object_manager_get_objects(sensors_manager);
  GList *l = NULL;
  for (l = signal_list; l != NULL; l = l->next)
  {
    ultra_integration_create_new_integrations(SIGNAL_OBJECT(l->data));
  }
  g_list_free(signal_list);
  g_signal_connect(sensors_manager, "object-added",
                   G_CALLBACK(ultra_integration_signal_manager_on_object_added),
                   NULL);
  g_signal_connect(
      sensors_manager, "object-removed",
      G_CALLBACK(ultra_integration_signal_manager_on_object_removed), NULL);
  // Load all signals ..
}

IntegrationObject *ultra_integration(const gchar *patch)
{
  IntegrationObject *integration =
      INTEGRATION_OBJECT(g_dbus_object_manager_get_object(
          G_DBUS_OBJECT_MANAGER(INTEGRATION_SERVER), patch));
  return integration;
}

GDBusObjectManager *ultra_integration_server(void)
{
  g_return_val_if_fail(INTEGRATION_SERVER != NULL, NULL);
  return G_DBUS_OBJECT_MANAGER(INTEGRATION_SERVER);
}

void ultra_integration_stop()
{
  GList *integrations = g_dbus_object_manager_get_objects(
      G_DBUS_OBJECT_MANAGER(INTEGRATION_SERVER));
  GList *ri = NULL;
  for (ri = integrations; ri != NULL; ri = ri->next)
  {
    ultra_integration_object_analyse_break(
        ULTRA_INTEGRATION_OBJECT(integrations->data));
  }
  if (integrations)
    g_list_free(integrations);
}

IntegrationObject *ultra_integration_get_ndir1() { return _ndir1; }

IntegrationObject *ultra_integration_get_ndir2() { return _ndir2; }

IntegrationObject *ultra_integration_get_tnb() { return _tnb; }

IntegrationObject *ultra_integration_get_codo() { return _codo; }

/*
GList *
ultra_integration_list                          ( StreamsObject *stream )
{
        static GList *ready_list = NULL;
        if(ready_list) g_list_free(ready_list);
        ready_list = NULL;
        GList *integrations =
g_dbus_object_manager_get_objects(G_DBUS_OBJECT_MANAGER(INTEGRATION_SERVER));
        GList *ri = NULL;
        if(ready_list != NULL ) g_list_free(ready_list);
        for(ri =integrations;ri!=NULL;ri=ri->next)
        {
                if(integration_simple_get_activated(integration_object_get_simple(INTEGRATION_OBJECT(ri->data))))
                {
                        if(ultra_stream_check_integration_pointer(ULTRA_STREAM_OBJECT(stream),INTEGRATION_OBJECT(ri->data)))
                                ready_list = g_list_append(ready_list,ri->data);
                }
        }
        if(integrations)g_list_free(integrations);
        return ready_list;
}






gboolean
ultra_integration_is_runned                  ( StreamsObject *stream  )
{
        GList *integrations = ultra_integration_list(stream);
        gboolean is_runned = FALSE;
        for(;integrations!=NULL;integrations=integrations->next)
        {
                if(integration_simple_get_measure(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data))))
                {
                        is_runned = TRUE;
                }
        }
        return is_runned;
}



gboolean
ultra_integration_is_integrating                  ( StreamsObject *stream )
{
        GList *integrations = ultra_integration_list(stream);
        gboolean is_integrating = FALSE;
        for(;integrations!=NULL;integrations=integrations->next)
        {
                if(integration_simple_get_measure(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data)))
                        &&integration_simple_get_integrating(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data))))
                {
                        is_integrating = TRUE;
                }
        }
        return is_integrating;
}



void
ultra_integration_analyse                          ( StreamsObject *stream
,const gchar *value_type )
{
        GList *integrations = ultra_integration_list(stream);
        for(;integrations!=NULL;integrations=integrations->next)
        {
                integration_simple_set_value_type(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data)),value_type);
                integration_simple_set_last_stream(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data)),streams_simple_get_name(streams_object_get_simple(stream)));
                ultra_integration_object_analyze(ULTRA_INTEGRATION_OBJECT(integrations->data));
        }
}

void
ultra_integration_justification                    ( StreamsObject *stream)
{
        GList *integrations = ultra_integration_list(stream);
        for(;integrations!=NULL;integrations=integrations->next)
        {
                ultra_integration_object_justifying_run(ULTRA_INTEGRATION_OBJECT(integrations->data));
        }
}

void
ultra_integration_integration                      ( StreamsObject *stream)
{
        GList *integrations = ultra_integration_list(stream);
        for(;integrations!=NULL;integrations=integrations->next)
        {
                ultra_integration_object_integrating_run(ULTRA_INTEGRATION_OBJECT(integrations->data));
        }
}


void
ultra_integration_calculate_justification          ( StreamsObject *stream)
{
        GList *integrations = ultra_integration_list(stream);
        for(;integrations!=NULL;integrations=integrations->next)
        {
                ultra_integration_object_calculate_justification(ULTRA_INTEGRATION_OBJECT(integrations->data));
        }
}

void
ultra_integration_calculate_integration            (  StreamsObject *stream )
{
        GList *integrations = ultra_integration_list(stream);
        for(;integrations!=NULL;integrations=integrations->next)
        {
                if(integration_simple_get_integrating(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data)))
                        &&!integration_simple_get_integrated(integration_object_get_simple(INTEGRATION_OBJECT(integrations->data))))
                        ultra_integration_object_calculate_integration(ULTRA_INTEGRATION_OBJECT(integrations->data));
        }
}
*/
