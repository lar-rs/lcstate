/*
 * Copyright (C)
 * sascha.smolkov@gmail.com
 *
 */

#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>
#include <stdio.h>
#include <ultimate-library.h>

//#define G_SETTINGS_ENABLE_BACKEND
//#include <gio/gsettingsbackend.h>

#include "../../config.h"

#include <glib/gi18n-lib.h>
//#include "online-task.h"
#include "calibration-process.h"
#include "measurement-process.h"
#include "ultimate-channel.h"
#include "ultimate-process-hold.h"
#include "ultimate-process-object.h"
#include "ultra-control-process.h"
#include "ultra-stream-object.h"
#include "ultraconfig.h"

// static       GDBusObjectManagerServer  *process_manager;

enum
{
    PROP_0,
    PROP_CONTROL_AI_WORKED,
    PROP_CONTROL_ONLINE,
    PROP_CONTROL_NEED_RINSING,
    PROP_CONTROL_NEED_HOLD,
    PROP_CONTROL_RINSING_VESSEL,

    PROP_CONTROL_ANALYZE_PROCESS,

};

enum
{
    ULTRA_STATE_PAUSE,
    ULTRA_STATE_USER_ACTION,
    ULTRA_STATE_ONLINE,
};

struct _UltraControlProcessPrivate
{
    TeraControl *control;
    GCancellable *cancellable;
    gint error_retry;
    VesselsObject *rinsing;
    GDBusObjectManagerServer *process_server;
    GSettings *settings;
    gboolean first_autostart;
    GList *all_process;
    gboolean online_task;
    GList *wait_process;
    MktProcessObject *HOLD;
    MktProcessObject *runned;
    MktStatus *pause_status;
    MktStatus *wait_status;
    MktProcessObject *range1;
    MktProcessObject *range2;
    gboolean IGNORE_CRITICAL;
    guint switched;
    gboolean node_has_reseted;
    GCancellable *online;
    NodesDigital16 *remote_node;
    gboolean stop_signal;
    gboolean mbu_is_legal;
};

static UltraControlProcess *CONTROL = NULL;

#define ULTRA_CONTROL_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_CONTROL_PROCESS, UltraControlProcessPrivate))

G_DEFINE_TYPE(UltraControlProcess, ultra_control_process, G_TYPE_OBJECT)

static void ultra_control_process_will_be_break(MktProcessObject *process, gpointer user_data);
static void ultra_control_process_will_be_run(MktProcessObject *process, gpointer user_data);
static void update_status()
{
    if (CONTROL->priv->runned != NULL)
    {
        if (mkt_status_is_active(CONTROL->priv->wait_status))
            mkt_status_activate(CONTROL->priv->wait_status, FALSE);
        if (mkt_status_is_active(CONTROL->priv->pause_status))
            mkt_status_activate(CONTROL->priv->pause_status, FALSE);
    }
    else if (tera_control_get_autostart(CONTROL->priv->control))
    {
        if (!mkt_status_is_active(CONTROL->priv->wait_status))
            mkt_status_activate(CONTROL->priv->wait_status, TRUE);
        if (mkt_status_is_active(CONTROL->priv->pause_status))
            mkt_status_activate(CONTROL->priv->pause_status, FALSE);
    }
    else
    {
        if (!mkt_status_is_active(CONTROL->priv->pause_status))
            mkt_status_activate(CONTROL->priv->pause_status, TRUE);
        if (mkt_status_is_active(CONTROL->priv->wait_status))
            mkt_status_activate(CONTROL->priv->wait_status, FALSE);
    }
}

static void ultra_control_process_all_check_status()
{
    gboolean red_button = FALSE;
    gboolean green_button = FALSE;
    gboolean busy = FALSE;
    gboolean online = tera_control_get_autostart(CONTROL->priv->control);
    gboolean interruptible = TRUE;
    GList *lp = NULL;
    if (CONTROL->priv->runned)
    {
        ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(CONTROL->priv->runned));
        control_status("%s", process_simple_get_full_name(simple));
        busy = TRUE;
        interruptible = process_simple_get_interruptible(simple);
        g_object_unref(simple);
    }
    else if (online)
    {
        busy = TRUE;
        if (CONTROL->priv->wait_process && CONTROL->priv->wait_process->data)
        {
            control_status(_("Wartet auf %s"), process_simple_get_full_name(process_object_get_simple(CONTROL->priv->wait_process->data)));
        }
        else
        {
            ProcessObject *next_run = NULL;
            for (lp = CONTROL->priv->all_process; lp != NULL; lp = lp->next)
            {
                ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(lp->data));
                if (process_simple_get_is_online(simple))
                {
                    if (next_run == NULL)
                        next_run = PROCESS_OBJECT(lp->data);
                    ProcessSimple *ns = process_object_get_simple(next_run);
                    if (process_simple_get_start_time(simple) < process_simple_get_start_time(ns))
                    {
                        next_run = PROCESS_OBJECT(lp->data);
                    }
                }
            }
            if (next_run)
            {
                ProcessSimple *simple = process_object_get_simple(next_run);
                control_status(_("Wartet auf %s"), process_simple_get_full_name(simple));
                g_object_unref(simple);
            }
            else
            {
                busy = FALSE;
            }
        }
    }
    else
    {
        control_status(_("Offline"));
    }
    if (busy == TRUE)
        red_button = TRUE;
    else
        green_button = TRUE;

    if (!interruptible)
        red_button = FALSE;
    tera_control_set_busy(CONTROL->priv->control, busy);
    tera_control_set_online(CONTROL->priv->control, online);
    tera_control_set_interruptible(CONTROL->priv->control, interruptible);
    tera_control_set_red_button(CONTROL->priv->control, red_button);
    tera_control_set_green_button(CONTROL->priv->control, green_button);
    update_status();
}

static void show_waited_process()
{
    return;
    gchar *path = g_build_path("/", g_get_home_dir(), "ultra-waited-process.log", NULL);
    FILE *f = fopen(path, "w");
    if (f != NULL)
    {
        fprintf(f, "Protokoll erstellt:%s\n", market_db_get_date_lar_format(market_db_time_now()));
        GList *l = NULL;
        for (l = CONTROL->priv->wait_process; l != NULL; l = l->next)
        {
            ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(l->data));
            fprintf(f, "%s:%s %s %s %s START %.0f<%.0f\n", process_simple_get_full_name(simple), process_simple_get_is_online(simple) ? "online" : "offline",
                    process_simple_get_busy(simple) ? "busy" : "free", process_simple_get_run(simple) ? "run" : "-", process_simple_get_wait(simple) ? "wait" : "-",
                    process_simple_get_start_time(simple), market_db_time_now());
        }
        fflush(f);
        fclose(f);
    }
    g_free(path);
}

static void show_all_process()
{
    return; // test info process status.
    gchar *path = g_build_path("/", g_get_home_dir(), "ultra-all-process.log", NULL);
    FILE *f = fopen(path, "w");
    if (f != NULL)
    {
        fprintf(f, "Protokoll erstellt:%s\n", market_db_get_date_lar_format(market_db_time_now()));
        GList *l = NULL;
        for (l = CONTROL->priv->all_process; l != NULL; l = l->next)
        {
            ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(l->data));
            fprintf(f, "%s:%s %s %s %s START %.0f<%.0f \n", process_simple_get_full_name(simple), process_simple_get_is_online(simple) ? "online" : "offline",
                    process_simple_get_busy(simple) ? "busy" : "free", process_simple_get_run(simple) ? "run" : "-", process_simple_get_wait(simple) ? "wait" : "-",
                    process_simple_get_start_time(simple), market_db_time_now());
        }
        fflush(f);
        fclose(f);
    }
    g_free(path);
}

static gboolean ultra_control_check_wait_process(MktProcessObject *process)
{
    if (CONTROL->priv->wait_process == NULL)
        return FALSE;
    gpointer result = g_list_find(CONTROL->priv->wait_process, process);
    return result != NULL;
}

static gboolean critical_break() { return (security_device_get_criticals(TERA_GUARD()) > 0 && !CONTROL->priv->IGNORE_CRITICAL); }

static void ultra_control_start_process(MktProcessObject *process);

static gboolean run_next_process()
{
    if (CONTROL->priv->stop_signal)
    {
        ultra_control_process_all_check_status();
        return FALSE;
    }
    if (!critical_break())
    {
        if (CONTROL->priv->runned == NULL)
        {
            if (CONTROL->priv->wait_process && MKT_IS_PROCESS_OBJECT(CONTROL->priv->wait_process->data))
            {
                MktProcessObject *process = MKT_PROCESS_OBJECT(CONTROL->priv->wait_process->data);
                CONTROL->priv->wait_process = g_list_remove(CONTROL->priv->wait_process, process);
                if (mkt_process_object_is_activate(process))
                {
                    ultra_control_start_process(process);
                    mkt_errors_clean(E1715);
                }
            }
        }
    }
    ultra_control_process_all_check_status();
    return FALSE;
}
static void control_process_set_default_stream_pump(MktProcessObject *process)
{
    if (ULTIMATE_IS_PROCESS_OBJECT(process))
    {
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
        if (stream && ULTRA_IS_STREAM_OBJECT(stream))
        {
            ultra_stream_set_default_pump(ULTRA_STREAM_OBJECT(stream));
        }
    }
}
static void control_process_set_no_sampling(MktProcessObject *process)
{
    if (ULTIMATE_IS_PROCESS_OBJECT(process))
    {
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
        if (stream && ULTRA_IS_STREAM_OBJECT(stream))
        {
            ultra_stream_no_sampling(ULTRA_STREAM_OBJECT(stream));
        }
    }
}
static void control_process_set_on_sampling(MktProcessObject *process)
{
    if (ULTIMATE_IS_PROCESS_OBJECT(process))
    {
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
        if (stream && ULTRA_IS_STREAM_OBJECT(stream))
        {
            ultra_stream_on_sampling(ULTRA_STREAM_OBJECT(stream));
        }
    }
}
static void control_process_set_new_stream_pump(MktProcessObject *process, PumpsObject *pump)
{
    if (ULTIMATE_IS_PROCESS_OBJECT(process))
    {
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
        if (stream && ULTRA_IS_STREAM_OBJECT(stream))
        {
            ultra_stream_set_pump(ULTRA_STREAM_OBJECT(stream), pump);
        }
    }
}

static PumpsObject *control_process_get_stream_pump(MktProcessObject *process)
{
    if (ULTIMATE_IS_PROCESS_OBJECT(process))
    {
        StreamsObject *stream = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
        if (stream && ULTRA_IS_STREAM_OBJECT(stream))
        {
            return ultra_stream_get_pump(ULTRA_STREAM_OBJECT(stream));
        }
    }
    return NULL;
}

static void control_activate_range(guint range)
{
    StreamsSimple *simple_range1 = NULL;
    StreamsSimple *simple_range2 = NULL;
    StreamsObject *stream1 = NULL;
    StreamsObject *stream2 = NULL;

    stream1 = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(CONTROL->priv->range1));
    if (stream1)
    {
        simple_range1 = streams_object_get_simple(stream1);
    }
    stream2 = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(CONTROL->priv->range2));
    if (stream2)
    {
        simple_range2 = streams_object_get_simple(stream2);
    }
    switch (range)
    {
    case 1:
        if (simple_range1)
            streams_simple_set_out_of_range(simple_range1, FALSE);
        if (simple_range2)
            streams_simple_set_out_of_range(simple_range2, TRUE);
        break;
    case 2:
        if (simple_range1)
            streams_simple_set_out_of_range(simple_range1, TRUE);
        if (simple_range2)
            streams_simple_set_out_of_range(simple_range2, FALSE);
        break;
    default:
        if (simple_range1)
            streams_simple_set_out_of_range(simple_range1, FALSE);
        if (simple_range2)
            streams_simple_set_out_of_range(simple_range2, FALSE);
        break;
    }
}
static void control_switch_range(guint range)
{
    switch (range)
    {
    case 1:
        mkt_process_object_deactivate(CONTROL->priv->range2);
        mkt_process_object_activate(CONTROL->priv->range1);
        if (CONTROL->priv->wait_process)
        {
            CONTROL->priv->wait_process = g_list_remove(CONTROL->priv->wait_process, CONTROL->priv->range2);
        }
        break;
    case 2:
        mkt_process_object_deactivate(CONTROL->priv->range1);
        mkt_process_object_activate(CONTROL->priv->range2);
        if (CONTROL->priv->wait_process)
        {
            CONTROL->priv->wait_process = g_list_remove(CONTROL->priv->wait_process, CONTROL->priv->range1);
        }
        break;
    default:
        mkt_process_object_activate(CONTROL->priv->range1);
        break;
    }
}

static void control_process_done(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    //
    MktProcessObject *process = MKT_PROCESS_OBJECT(source_object);
    CONTROL->priv->runned = NULL;
    GError *error = NULL;
    gboolean result = mkt_process_object_finish(process, res, &error);
    g_message("Process %s (%s) %s", process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))), g_dbus_object_get_object_path(G_DBUS_OBJECT(process)),
              result ? "done" : (error ? error->message : "unknown error"));
    control_process_set_default_stream_pump(process);

    if (!result)
    {
        if (CONTROL->priv->error_retry < 2)
            ultra_control_start_process(CONTROL->priv->HOLD);
        CONTROL->priv->error_retry++;
        ultra_control_process_all_check_status();
        show_waited_process();
        show_all_process();
        update_status();
        return;
    }
    else
    {
        if (CONTROL->priv->node_has_reseted)
        {
            mkt_errors_clean(E1890);
        }
        CONTROL->priv->node_has_reseted = FALSE;
        CONTROL->priv->error_retry = 0;
    }
    if (CONTROL->priv->mbu_is_legal && g_settings_get_boolean(CONTROL->priv->settings, "range-switching"))
    {
        gdouble max = g_settings_get_double(CONTROL->priv->settings, "range-max");
        gdouble min = g_settings_get_double(CONTROL->priv->settings, "range-min");
        gchar *channel_max = g_settings_get_string(CONTROL->priv->settings, "range-max-channel");
        gchar *channel_min = g_settings_get_string(CONTROL->priv->settings, "range-min-channel");
        control_process_set_on_sampling(CONTROL->priv->range1);
        control_process_set_new_stream_pump(CONTROL->priv->range2, control_process_get_stream_pump(CONTROL->priv->range1));
        if ((gpointer)process == (gpointer)CONTROL->priv->range1)
        {
            if (!measurement_process_check_range(MEASUREMENT_PROCESS(CONTROL->priv->range1), max, channel_max, TRUE))
            {
                control_switch_range(2);
                control_process_set_no_sampling(CONTROL->priv->range2);
                ultra_control_start_process(CONTROL->priv->range2);
            }
            else
            {
                CONTROL->priv->switched = 0;
                control_activate_range(1);
                run_next_process();
            }
        }
        else if ((gpointer)process == (gpointer)CONTROL->priv->range2)
        {
            if (!measurement_process_check_range(MEASUREMENT_PROCESS(CONTROL->priv->range2), min, channel_min, FALSE))
            {
                CONTROL->priv->switched++;
                if (CONTROL->priv->switched < 3)
                {
                    control_switch_range(1);
                    control_process_set_no_sampling(CONTROL->priv->range1);
                    ultra_control_start_process(CONTROL->priv->range1);
                }
                else
                {
                    CONTROL->priv->switched = 0;
                    run_next_process();
                }
            }
            else
            {
                CONTROL->priv->switched = 0;
                control_activate_range(2);
                run_next_process();
            }
        }
        else
        {
            run_next_process();
        }
    }
    else
    {
        run_next_process();
    }

    ultra_control_process_all_check_status();
    show_waited_process();
    show_all_process();
    update_status();
}

static void ultra_control_cancel()
{
    if (CONTROL->priv->cancellable)
    {
        g_cancellable_cancel(CONTROL->priv->cancellable);
        g_object_unref(CONTROL->priv->cancellable);
    }
    CONTROL->priv->cancellable = NULL;
}

void ultra_control_start_process(MktProcessObject *process)
{
    if (CONTROL->priv->cancellable)
    {
        g_cancellable_cancel(CONTROL->priv->cancellable);
        g_object_unref(CONTROL->priv->cancellable);
    }
    CONTROL->priv->cancellable = g_cancellable_new();
    CONTROL->priv->runned = process;
    mkt_process_object_run(MKT_PROCESS_OBJECT(process), CONTROL->priv->cancellable, control_process_done, CONTROL);
    ultra_control_process_all_check_status();
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
    if (simple)
    {
        mkt_process_object_online(MKT_PROCESS_OBJECT(process));
        g_object_unref(simple);
    }
    update_status();
}

static gboolean check_stop_signal_callback(gpointer data);

static void check_stop_signal_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    gboolean result = FALSE;
    GError *error = NULL;
    if (nodes_digital16_call_get_digital_in_finish(CONTROL->priv->remote_node, &result, res, &error))
    {
        gboolean change_status = CONTROL->priv->stop_signal != result;
        CONTROL->priv->stop_signal = result;
        if (change_status)
            run_next_process();
        g_timeout_add_seconds(2, check_stop_signal_callback, NULL);
    }
    if (error)
        g_error_free(error);
}
gboolean check_stop_signal_callback(gpointer data)
{
    if (CONTROL->priv->remote_node == NULL)
        return FALSE;
    static guint count = 0;
    static guint sec20 = 0;
    count++;
    if (count > sec20 + 10)
    {
        run_next_process();
        sec20 = count;
    }
    nodes_digital16_call_get_digital_in(CONTROL->priv->remote_node, 11, NULL, check_stop_signal_async_callback, NULL);
    return FALSE;
}
static void ultra_control_watch_stop_signal_run()
{
    if (CONTROL->priv->remote_node)
    {
        CONTROL->priv->stop_signal = FALSE;
        nodes_digital16_call_get_digital_in_sync(CONTROL->priv->remote_node, 11, &CONTROL->priv->stop_signal, NULL, NULL);
    }
    g_timeout_add_seconds(2, check_stop_signal_callback, NULL);
}

static void ultra_control_break()
{
    AirflowSensor *sensor = airflow_object_get_sensor(ULTRA_AIRFLOW());
    if (sensor)
    {
        airflow_sensor_set_is_online(sensor, FALSE);
        g_object_unref(sensor);
    }
}
static void ultra_control_critical_berak()
{
    if (CONTROL->priv->wait_process)
        g_list_free(CONTROL->priv->wait_process);
    CONTROL->priv->wait_process = NULL;
    if (tera_control_get_autostart(CONTROL->priv->control))
        mkt_log_message(MKT_LOG_STATE_MEASUREMENT, "Go offline (Critical error)");
    tera_control_set_autostart(CONTROL->priv->control, FALSE);
    ultra_control_break();
    ultra_control_cancel();
    GList *lpo = NULL;
    for (lpo = CONTROL->priv->all_process; lpo != NULL; lpo = lpo->next)
    {
        mkt_process_object_offline(MKT_PROCESS_OBJECT(lpo->data));
        mkt_process_object_break(MKT_PROCESS_OBJECT(lpo->data));
    }
    if (!tera_control_get_maintenance(CONTROL->priv->control))
    {
        ultra_control_start_process(MKT_PROCESS_OBJECT(CONTROL->priv->HOLD));
    }
    ultra_control_process_all_check_status();
    update_status();
}

static gboolean ultra_control_add_wait_process(MktProcessObject *process)
{
    tera_control_set_maintenance(CONTROL->priv->control, FALSE);
    if (ultra_control_check_wait_process(process))
        return FALSE;
    CONTROL->priv->wait_process = g_list_append(CONTROL->priv->wait_process, process);
    run_next_process();
    update_status();
    return TRUE;
}

static void ultra_control_online()
{
    tera_control_set_autostart(CONTROL->priv->control, TRUE);
    CONTROL->priv->first_autostart = FALSE;
    AirflowSensor *sensor = airflow_object_get_sensor(ULTRA_AIRFLOW());
    if (sensor)
    {
        airflow_sensor_set_is_online(sensor, TRUE);
        g_object_unref(sensor);
    }
    GList *pl = NULL;
    for (pl = CONTROL->priv->all_process; pl != NULL; pl = pl->next)
    {
        ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(pl->data));
        mkt_process_object_activate(MKT_PROCESS_OBJECT(pl->data));
        process_simple_set_start_time(simple, 0.0);
    }
    if (CONTROL->priv->wait_process)
        g_list_free(CONTROL->priv->wait_process);
    CONTROL->priv->wait_process = NULL;
    ultra_control_watch_stop_signal_run();
    tera_control_set_maintenance(CONTROL->priv->control, FALSE);
    for (pl = CONTROL->priv->all_process; pl != NULL; pl = pl->next)
    {
        ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(pl->data));
        if (process_simple_get_online_process(simple) && process_simple_get_online(simple))
        {
            if (process_simple_get_can_start(simple))
            {
                process_simple_set_is_online(simple, TRUE);
                if (!CALIBRATION_IS_PROCESS(pl->data) && !process_simple_get_check_process(simple))
                {
                    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(pl->data)));
                    GList *channels = ultra_stream_channels(stream);
                    GList *l = NULL;
                    for (l = channels; l != NULL; l = l->next)
                    {
                        ultimate_channel_transmit_last(ULTIMATE_CHANNEL(l->data));
                    }
                    if (CONTROL->priv->mbu_is_legal && g_settings_get_boolean(CONTROL->priv->settings, "range-switching") &&
                        (((gpointer)pl->data == (gpointer)CONTROL->priv->range1) || ((gpointer)pl->data == (gpointer)CONTROL->priv->range2)))
                    {
                        if ((gpointer)pl->data == (gpointer)CONTROL->priv->range1)
                        {
                            UltraStreamObject *stream1 = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(CONTROL->priv->range1)));
                            UltraStreamObject *stream2 = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(CONTROL->priv->range2)));
                            ultra_stream_object_set_remote_address(stream2, ultra_stream_object_get_remote_address(stream1));
                            control_switch_range(1);
                            if (!process_simple_get_remote_control(simple) || process_simple_get_remote_signal(simple))
                            {
                                mkt_process_object_start(MKT_PROCESS_OBJECT(pl->data));
                            }
                            else
                            {
                                mkt_process_object_online(MKT_PROCESS_OBJECT(pl->data));
                            }
                        }
                        else
                        {
                        }
                    }
                    else
                    {
                        if (!process_simple_get_remote_control(simple) || process_simple_get_remote_signal(simple))
                        {
                            mkt_process_object_start(MKT_PROCESS_OBJECT(pl->data));
                        }
                        else
                        {
                            mkt_process_object_online(MKT_PROCESS_OBJECT(pl->data));
                        }
                    }
                }
                else
                {
                    mkt_process_object_online(MKT_PROCESS_OBJECT(pl->data));
                }
            }
            else
            {
                mkt_process_object_offline(MKT_PROCESS_OBJECT(pl->data));
            }
        }
        g_object_unref(simple);
    }
    ultra_control_process_all_check_status();
    show_all_process();
}

static gboolean ultra_control_security_online_callback(TeraControl *control, GDBusMethodInvocation *invocation, UltraControlProcess *ultra_control_process)
{
    if (!tera_control_get_busy(CONTROL->priv->control))
    {
        if (CONTROL->priv->first_autostart || !critical_break())
        {
            mkt_log_message(MKT_LOG_STATE_MEASUREMENT, "Go online (User action)");
            ultra_control_online();
        }
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

static gboolean ultra_control_security_offline_callback(TeraControl *control, GDBusMethodInvocation *invocation, UltraControlProcess *ultra_control_process)
{
    if (tera_control_get_autostart(CONTROL->priv->control))
        mkt_log_message(MKT_LOG_STATE_MEASUREMENT, "Go offline (User action)");
    tera_control_set_autostart(CONTROL->priv->control, FALSE);
    if (CONTROL->priv->wait_process)
        g_list_free(CONTROL->priv->wait_process);
    CONTROL->priv->wait_process = NULL;
    ultra_control_break();
    control_need_hold();
    ultra_control_cancel();
    GList *lpo = NULL;
    for (lpo = CONTROL->priv->all_process; lpo != NULL; lpo = lpo->next)
    {
        mkt_process_object_offline(MKT_PROCESS_OBJECT(lpo->data));
        mkt_process_object_break(MKT_PROCESS_OBJECT(lpo->data));
    }
    if (CONTROL->priv->wait_process)
        g_list_free(CONTROL->priv->wait_process);
    CONTROL->priv->wait_process = NULL;
    ultra_control_start_process(CONTROL->priv->HOLD);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    ultra_control_process_all_check_status();
    update_status();
    return TRUE;
    // mkt_process_object_start(CONTROL->priv->HOLD);
}

static gboolean ultra_control_ignore_critical_callback(TeraControl *control, GDBusMethodInvocation *invocation, gboolean in, UltraControlProcess *ultra_control_process)
{
    ultra_control_process->priv->IGNORE_CRITICAL = in;
    if (critical_break())
    {
        ultra_control_critical_berak();
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    ultra_control_process_all_check_status();
    return TRUE;
    // mkt_process_object_start(CONTROL->priv->HOLD);
}

static void ultra_control_security_critical_error_callback(SecurityDevice *device, GParamSpec *pspec, UltraControlProcess *ultra_control_process)
{
    if (security_device_get_critical(TERA_GUARD()) && !CONTROL->priv->IGNORE_CRITICAL)
    {
        ultra_control_critical_berak();
    }
    else
    {
        run_next_process();
    }
    ultra_control_process_all_check_status();
}
static void ultra_control_node_reseted_callback(CandeviceSimple *can, guint nodeid, gpointer user_data)
{
    if (!CONTROL->priv->node_has_reseted)
    {
        mkt_errors_come(E1890);
    }
    if (CONTROL->priv->runned)
    {
        CONTROL->priv->wait_process = g_list_prepend(CONTROL->priv->wait_process, CONTROL->priv->runned);
    }
    CONTROL->priv->node_has_reseted = TRUE;
    ultimate_process_hold_reinit(ULTIMATE_PROCESS_HOLD(CONTROL->priv->HOLD));
    ultra_control_cancel();
    control_need_hold();
    ultra_control_start_process(CONTROL->priv->HOLD);
    control_was_rinsed();
    ultra_control_process_all_check_status();
}

static void ultra_control_process_init(UltraControlProcess *ultra_control_process)
{
    UltraControlProcessPrivate *priv = ULTRA_CONTROL_PROCESS_PRIVATE(ultra_control_process);
    ultra_control_process->priv = priv;
    ultra_control_process->priv->cancellable = NULL;
    ultra_control_process->priv->settings = g_settings_new("ultra.measurement");
    // g_signal_connect(ultra_control_process->priv->settings,"changed",G_CALLBACK(ultra_control_range_switch_changed),ultra_control_process);
    tera_security_manager_client_new();
    ultra_control_process->priv->pause_status = mkt_status_new("P", "Pause");
    ultra_control_process->priv->wait_status = mkt_status_new("W", "Wait process");
    MktModel *model = mkt_model_select_one(MKT_TYPE_STATUS_MODEL, "select * from %s where status_signification = '%s'", g_type_name(MKT_TYPE_STATUS_MODEL), "E");
    if (model)
        mkt_model_delete(model);

    ultra_control_process->priv->wait_process = NULL;
    ultra_control_process->priv->IGNORE_CRITICAL = FALSE;
    ultra_control_process->priv->first_autostart = TRUE;
    mkt_errors_clean(E1890);
    mkt_errors_clean(E1841);
    mkt_errors_clean(E1842);
    mkt_errors_clean(E1843);
    mkt_errors_clean(E1844);
    mkt_errors_clean(E1845);
    mkt_errors_clean(E1846);
    mkt_errors_clean(E2010);
    mkt_errors_clean(E2107);
}

static void ultra_control_process_change_busy(ProcessSimple *psimple, GParamSpec *pspec, gpointer data) { ultra_control_process_all_check_status(); }

static void ultra_control_process_change_is_online(ProcessSimple *psimple, GParamSpec *pspec, gpointer data) { ultra_control_process_all_check_status(); }

static void ultra_control_process_change_start_time(ProcessSimple *psimple, GParamSpec *pspec, gpointer data) { ultra_control_process_all_check_status(); }

static gboolean control_waite_main_loop_autostart(gpointer user_data)
{
    ultra_control_start_process(CONTROL->priv->HOLD);
    if (tera_control_get_autostart(CONTROL->priv->control))
    {
        mkt_log_message(MKT_LOG_STATE_MEASUREMENT, "Go online (Autostart)");
        ultra_control_online();
    }
    return FALSE;
}
static void ultra_control_process_constructed(GObject *object)
{
    UltraControlProcess *control_process = ULTRA_CONTROL_PROCESS(object);
    CONTROL = control_process;
    if (G_OBJECT_CLASS(ultra_control_process_parent_class)->constructed)
        G_OBJECT_CLASS(ultra_control_process_parent_class)->constructed(object);

    control_process->priv->control = tera_control_skeleton_new();

    GError *error = NULL;
    if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(control_process->priv->control), tera_service_dbus_connection(), TERA_CONTROL_PATH, &error))
    {
        // g_error("Service interface create fail");
    }
    g_signal_connect(control_process->priv->control, "handle-online", G_CALLBACK(ultra_control_security_online_callback), control_process);
    g_signal_connect(control_process->priv->control, "handle-offline", G_CALLBACK(ultra_control_security_offline_callback), control_process);

    g_signal_connect(control_process->priv->control, "handle-ignore-critical", G_CALLBACK(ultra_control_ignore_critical_callback), control_process);

    GDBusObjectManager *can_device_manager = mkt_can_manager_client_devices();
    if (can_device_manager)
    {
        CandeviceObject *object = CANDEVICE_OBJECT(g_dbus_object_manager_get_object(can_device_manager, CAN_DEVICE_CAN0));
        if (object)
        {
            g_signal_connect(candevice_object_get_simple(object), "node-reseted", G_CALLBACK(ultra_control_node_reseted_callback), CONTROL);
        }
    }
    g_signal_connect(TERA_GUARD(), "notify::critical", G_CALLBACK(ultra_control_security_critical_error_callback), control_process);
    Ultradevice *device = ConfigureDevice();
    g_object_bind_property(device, "autostart", control_process->priv->control, "autostart", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(device, "stirrers", control_process->priv->control, "stirrers", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(device, "justificationTime", control_process->priv->control, "justification", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_timeout_add_seconds(1, ultra_control_propagete_timeout_callback, CONTROL);
    control_process->priv->process_server = g_dbus_object_manager_server_new(TERA_PROCESS_MANAGER);
    g_dbus_object_manager_server_set_connection(control_process->priv->process_server, tera_service_dbus_connection());
    gchar *id = g_strdup_printf("%s/Hold", TERA_PROCESS_MANAGER);
    control_process->priv->HOLD = MKT_PROCESS_OBJECT(g_object_new(ULTIMATE_TYPE_PROCESS_HOLD, "g-object-path", id, "need-hold", TRUE, "process-identification", 1, NULL));
    process_simple_set_number(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), 1);
    process_simple_set_name(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), _("Hold parameter initialization"));
    process_simple_set_full_name(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), _("Hold parameter initialization"));
    process_simple_set_can_start(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), TRUE);
    g_signal_connect(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), "notify::busy", G_CALLBACK(ultra_control_process_change_busy), CONTROL);
    g_signal_connect(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), "notify::run", G_CALLBACK(ultra_control_process_change_busy), CONTROL);
    g_signal_connect(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), "notify::is-online", G_CALLBACK(ultra_control_process_change_is_online), CONTROL);
    g_signal_connect(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), "notify::start-time", G_CALLBACK(ultra_control_process_change_start_time), CONTROL);
    g_signal_connect(process_object_get_simple(PROCESS_OBJECT(control_process->priv->HOLD)), "notify::is-online", G_CALLBACK(ultra_control_process_change_start_time), CONTROL);

    g_dbus_object_manager_server_export(CONTROL->priv->process_server, G_DBUS_OBJECT_SKELETON(control_process->priv->HOLD));

    NodesObject *node = NULL;
    node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
    if (node == NULL)
    {
        mkt_log_error_message("Remote control stop signal node Digital1:/com/lar/nodes/Digital1 not found");
    }
    else
    {
        if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 24)
        {
            mkt_log_error_message("Remote control stop signal Stop singlal /com/lar/nodes/Digital1 hat falsche ID %x (Empfohlen worden ist %x)",
                                  nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
        }
        control_process->priv->remote_node = nodes_object_get_digital16(node);
    }

    g_timeout_add(900, control_waite_main_loop_autostart, control_process);
}

static void ultra_control_process_finalize(GObject *object)
{
    // UltraControlProcess *ultra_sensors = ULTRA_CONTROL_PROCESS(object);

    G_OBJECT_CLASS(ultra_control_process_parent_class)->finalize(object);
}

/*
   static const gchar*
   _date_hmydm ( gdouble tval  )
   {
        mktTime_t time  =  mktNow();
        time.tv_sec = (long int )tval;
        struct tm *timeinfo;
        static char buf[20];
        timeinfo = localtime(&time.tv_sec);
        strftime(buf,sizeof(buf),"%Y.%d.%m %H:%M",timeinfo);
        return( buf );
   }
 */

static void ultra_control_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    g_return_if_fail(ULTRA_IS_CONTROL_PROCESS(object));
    //	UltraControlProcess *process = ULTRA_CONTROL_PROCESS(object);
    switch (prop_id)
    {
    /* case PROP_CONTROL_AI_WORKED:
                            process->priv->AI_WORKED = g_value_get_boolean(value);
                            ultra_control_process_all_check_status();
                            break;
                    case PROP_CONTROL_ONLINE:
                            process->priv->ONLINE = g_value_get_boolean(value);
                            ultra_control_process_all_check_status();
                            break;
                    case PROP_CONTROL_NEED_HOLD:
                            process->priv->NEED_HOLD = g_value_get_boolean(value);
                            break;
                    case PROP_CONTROL_NEED_RINSING:
                            process->priv->NEED_RINSING = g_value_get_boolean(value);
                            break;
                    case PROP_CONTROL_RINSING_VESSEL:
                            if(process->priv->rinsing)g_object_unref(process->priv->rinsing);
                            process->priv->rinsing = g_value_dup_object(value);
                            break;*/
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_control_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    g_return_if_fail(ULTRA_IS_CONTROL_PROCESS(object));
    //	UltraControlProcess *process = ULTRA_CONTROL_PROCESS(object);
    switch (prop_id)
    {
        /*	case PROP_CONTROL_AI_WORKED:
                        g_value_set_boolean(value,process->priv->AI_WORKED);
                        break;
                   case PROP_CONTROL_ONLINE:
                        security_device_set_autostart(TERA_GUARD(),CONTROL->priv->ONLINE);
                        security_device_set_online(TERA_GUARD(),CONTROL->priv->ONLINE);
                        g_value_set_boolean(value,process->priv->ONLINE);
                        break;
                   case PROP_CONTROL_NEED_HOLD:
                        g_value_set_boolean(value,process->priv->NEED_HOLD);
                        break;
                   case PROP_CONTROL_NEED_RINSING:
                        g_value_set_boolean(value,process->priv->NEED_RINSING);
                        break;
                   case PROP_CONTROL_RINSING_VESSEL:
                        g_value_set_object(value,process->priv->rinsing);
                        break;*/

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_control_process_class_init(UltraControlProcessClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraControlProcessPrivate));

    object_class->finalize = ultra_control_process_finalize;
    object_class->set_property = ultra_control_process_set_property;
    object_class->get_property = ultra_control_process_get_property;
    object_class->constructed = ultra_control_process_constructed;

    /*	g_object_class_install_property (object_class,PROP_CONTROL_AI_WORKED,
                    g_param_spec_boolean ("ai-worked",
                                    _("Control ai worked mode"),
                                    _("Control ai worked mode"),
                                    FALSE,
                                    G_PARAM_WRITABLE | G_PARAM_READABLE ));
       g_object_class_install_property (object_class,PROP_CONTROL_ONLINE,
                    g_param_spec_boolean ("online",
                                    _("Control online mode"),
                                    _("Control online mode"),
                                    FALSE,
                                    G_PARAM_WRITABLE | G_PARAM_READABLE ));
       g_object_class_install_property (object_class,PROP_CONTROL_NEED_HOLD,
                    g_param_spec_boolean ("need-hold",
                                    _("Control need hold"),
                                    _("Control need hold"),
                                    FALSE,
                                    G_PARAM_WRITABLE | G_PARAM_READABLE ));
       g_object_class_install_property (object_class,PROP_CONTROL_NEED_RINSING,
                    g_param_spec_boolean ("need-rinsing",
                                    _("Control need rinsing"),
                                    _("Control need rinsing"),
                                    FALSE,
                                    G_PARAM_WRITABLE | G_PARAM_READABLE ));

       g_object_class_install_property (object_class,PROP_CONTROL_RINSING_VESSEL,
                    g_param_spec_object ("rinsing-vessel",
                                    _("Control rinsing vessel"),
                                    _("Control rinsing vessel"),
                                    VESSELS_TYPE_OBJECT,
                                    G_PARAM_WRITABLE | G_PARAM_READABLE ));

       g_object_class_install_property (object_class,PROP_CONTROL_ANALYZE_PROCESS,
                    g_param_spec_object ("analyze-process",
                                    _("Control analyze process"),
                                    _("Control analyze process"),
                                    ULTIMATE_TYPE_PROCESS_OBJECT,
                                    G_PARAM_WRITABLE | G_PARAM_READABLE ));*/
}

void ultra_control_new()
{
    if (CONTROL == NULL)
    {
        CONTROL = ULTRA_CONTROL_PROCESS(g_object_new(ULTRA_TYPE_CONTROL_PROCESS, NULL));
    }
}

UltraControlProcess *ULTRA_CONTROL() { return ULTRA_CONTROL_PROCESS(CONTROL); }

void ultra_control_init_stirrers()
{
    ultra_control_new();
    GList *streams = g_dbus_object_manager_get_objects(ultra_stream_server_object_manager());
    GList *l = NULL;
    guint count = 0;
    for (l = streams; l != NULL; l = l->next)
    {
        count++;
        if (streams_ultra_get_is_dilution(streams_object_get_ultra(STREAMS_OBJECT(l->data))))
            count++;
    }
    if (streams)
        g_list_free_full(streams, g_object_unref);
    if (tera_control_get_stirrers(CONTROL->priv->control) == 0)
    {
        tera_control_set_stirrers(CONTROL->priv->control, count);
    }
    StirrersSimple *simple = stirrers_object_get_simple(ULTRA_STIRRER_1());
    g_object_bind_property(CONTROL->priv->control, "stirrers", simple, "count", G_BINDING_SYNC_CREATE | G_BINDING_DEFAULT);
    g_object_unref(simple);
}

void ultra_control_process_will_be_run(MktProcessObject *process, gpointer user_data) { ultra_control_add_wait_process(process); }

void ultra_control_process_will_be_break(MktProcessObject *process, gpointer user_data)
{
    if (CONTROL->priv->wait_process != NULL)
    {
        CONTROL->priv->wait_process = g_list_remove(CONTROL->priv->wait_process, process);
    }
    if (process == CONTROL->priv->runned)
    {
        ultra_control_cancel();
    }
}

gboolean control_add_process(MktProcessObject *process)
{
    g_return_val_if_fail(CONTROL != NULL, FALSE);
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
    // g_debug("CONTROL add process:%s", process_simple_get_full_name(simple));

    CONTROL->priv->all_process = g_list_append(CONTROL->priv->all_process, process);
    g_signal_connect(process, "process-will-be-run", G_CALLBACK(ultra_control_process_will_be_run), CONTROL);
    g_signal_connect(process, "process-will-be-break", G_CALLBACK(ultra_control_process_will_be_break), CONTROL);
    g_signal_connect(simple, "notify::busy", G_CALLBACK(ultra_control_process_change_busy), CONTROL);
    g_signal_connect(simple, "notify::run", G_CALLBACK(ultra_control_process_change_busy), CONTROL);
    g_signal_connect(simple, "notify::is-online", G_CALLBACK(ultra_control_process_change_is_online), CONTROL);
    g_signal_connect(simple, "notify::start-time", G_CALLBACK(ultra_control_process_change_start_time), CONTROL);
    g_signal_connect(simple, "notify::is-online", G_CALLBACK(ultra_control_process_change_start_time), CONTROL);

    g_dbus_object_manager_server_export(CONTROL->priv->process_server, G_DBUS_OBJECT_SKELETON(process));
    g_object_unref(simple);
    return TRUE;
}

gboolean control_add_range1(MktProcessObject *process)
{
    g_return_val_if_fail(CONTROL != NULL, FALSE);
    CONTROL->priv->range1 = g_object_ref(process);
    return TRUE;
}
gboolean control_add_range2(MktProcessObject *process)
{
    g_return_val_if_fail(CONTROL != NULL, FALSE);
    if (CONTROL->priv->range1 != NULL && CONTROL->priv->range1 != process)
    {
        CONTROL->priv->range2 = g_object_ref(process);
        CONTROL->priv->mbu_is_legal = TRUE;
        if (g_settings_get_boolean(CONTROL->priv->settings, "range-switching"))
        {
            UltraStreamObject *stream = ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(CONTROL->priv->range2)));
            ultra_stream_object_set_remote_address(stream, 9);
        }
    }
    return TRUE;
}

void control_status(const gchar *format, ...)
{
    g_return_if_fail(CONTROL != NULL);
    va_list args;
    gchar *new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    tera_control_set_status(CONTROL->priv->control, new_status);
    g_free(new_status);
}

void control_was_rinsed()
{
    g_return_if_fail(CONTROL != NULL);
    g_object_set(CONTROL->priv->HOLD, "need-rinsing", FALSE, NULL);
}

void control_need_rinsing(VesselsObject *object)
{
    g_return_if_fail(CONTROL != NULL);
    if (object == NULL)
        g_object_set(CONTROL->priv->HOLD, "need-rinsing", FALSE, NULL);
    else
        g_object_set(CONTROL->priv->HOLD, "need-rinsing", TRUE, "drain-vessel", object, NULL);
}

void control_need_hold()
{
    g_return_if_fail(CONTROL != NULL);
    g_object_set(CONTROL->priv->HOLD, "need-hold", TRUE, NULL);
}
void control_need_nothig()
{
    g_return_if_fail(CONTROL != NULL);
    g_object_set(CONTROL->priv->HOLD, "need-hold", FALSE, "need-rinsing", FALSE, NULL);
}

void contron_maintenance_off(void)
{
    g_return_if_fail(CONTROL != NULL);
    tera_control_set_maintenance(CONTROL->priv->control, FALSE);
}

GQuark control_error_quark(void)
{
    static GQuark error;
    if (!error)
        error = g_quark_from_static_string("ultra-control-error");
    return error;
}
