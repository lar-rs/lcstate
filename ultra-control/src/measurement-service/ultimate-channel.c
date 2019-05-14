/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup UltimateChannelObject
 * @{
 * @file  ultimate-channel-object.c
 * @brief This is CHANNEL model object description.
 *
 *  Copyright (C) LAR  2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include <gio/gio.h>

#include <math.h>

#include "../../config.h"
#include "ultimate-channel.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"
#include <glib/gi18n-lib.h>
#include <ultimate-library.h>

#if GLIB_CHECK_VERSION(2, 31, 7)
static GRecMutex init_rmutex;
#define MUTEX_LOCK() g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)
#else
static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
#define MUTEX_LOCK() g_static_rec_mutex_lock(&init_mutex)
#define MUTEX_UNLOCK() g_static_rec_mutex_unlock(&init_mutex)
#endif

static void ultimate_channel_base_init(gpointer g_iface) {
    static gboolean is_channel_initialized = FALSE;
    MUTEX_LOCK();
    if (!is_channel_initialized) {

        is_channel_initialized = TRUE;
    }
    MUTEX_UNLOCK();
}

GType ultimate_channel_get_type(void) {
    static GType iface_type = 0;
    if (iface_type == 0) {
        static const GTypeInfo info = {sizeof(UltimateChannelInterface), (GBaseInitFunc)ultimate_channel_base_init, (GBaseFinalizeFunc)NULL,
            (GClassInitFunc)NULL, NULL, NULL, 0, 0, (GInstanceInitFunc)NULL, 0};
        MUTEX_LOCK();
        if (iface_type == 0) {
            iface_type = g_type_register_static(G_TYPE_INTERFACE, "UltimateChannelInterface", &info, 0);
        }
        MUTEX_UNLOCK();
    }
    return iface_type;
}

GDBusObjectManagerServer *_channels_object_manager_server = NULL;

void ultimate_channel_create_manager(GDBusConnection *connection) {
    if (_channels_object_manager_server != NULL) return;
    _channels_object_manager_server = g_dbus_object_manager_server_new(TERA_CHANNELS_MANAGER);
    g_dbus_object_manager_server_set_connection(_channels_object_manager_server, connection);
}

GDBusObjectManagerServer *ultimate_channel_get_manager(void) { return _channels_object_manager_server; }

MktChannel *ultimate_channel_get_channel_model(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, NULL);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), NULL);
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->channel_model) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->channel_model(channel);
    return NULL;
}

MktCalibration *ultimate_channel_get_calibration_model(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, NULL);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), NULL);
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->calibration_model) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->calibration_model(channel);
    return NULL;
}

gboolean ultimate_channel_start_measurement(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    channels_measurement_set_curr_replicate(channels_object_get_measurement(CHANNELS_OBJECT(channel)), 0);
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->start_measurement) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->start_measurement(channel);
    return TRUE;
}

gboolean ultimate_channel_start_calibration(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_calibration(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->start_calibration) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->start_calibration(channel);
    return FALSE;
}

gboolean ultimate_channel_start_solution_point(UltimateChannel *channel, guint solution) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->start_solution_point) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->start_solution_point(channel, solution);
    return FALSE;
}

gboolean ultimate_channel_have_solution_value(UltimateChannel *channel, guint solution) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->have_solution_value) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->have_solution_value(channel, solution);
    return FALSE;
}

gboolean ultimate_channel_clean(UltimateChannel *channel) {
    ChannelsSimple *simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
    channels_simple_set_statistic_done(simple, FALSE);
    channels_simple_set_transmit_done(simple, FALSE);
    channels_simple_set_measure_error(simple, FALSE);
    ChannelsMeasurement *measurement = channels_object_get_measurement(CHANNELS_OBJECT(channel));
    if (measurement) {
        channels_measurement_emit_update(measurement);
    }
    ChannelsCalibration *calibration = channels_object_get_calibration(CHANNELS_OBJECT(channel));
    if (calibration) {
        channels_calibration_emit_update(calibration);
    }

    return TRUE;
}

gboolean ultimate_channel_next_measurement(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    ChannelsSimple *simple            = channels_object_get_simple(CHANNELS_OBJECT(channel));
    guint           measurement_count = channels_simple_get_measurement(simple);
    channels_simple_set_measurement(simple, measurement_count + 1);
    channels_simple_set_statistic_done(simple, FALSE);
    channels_simple_set_transmit_done(simple, FALSE);
    channels_simple_set_measure_error(simple, FALSE);
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->next_measurement) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->next_measurement(channel);
    return TRUE;
}

gboolean ultimate_channel_transmit_M_replicate(UltimateChannel *channel, MktProcessObject *process) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->transmit_M_replicate) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->transmit_M_replicate(channel, process);
    return FALSE;
}

gboolean ultimate_channel_transmit_M_result(UltimateChannel *channel, MktProcessObject *process) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->transmit_M_result) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->transmit_M_result(channel, process);
    return FALSE;
}

gboolean ultimate_channel_reset_measurement(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->reset_measurement) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->reset_measurement(channel);
    return FALSE;
}

void ultimate_channel_amount_init(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->amount_init) ULTIMATE_CHANNEL_GET_INTERFACE(channel)->amount_init(channel);
}

void ultimate_channel_transmit_amount(UltimateChannel *channel, MktProcessObject *process) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->amount_transmit)
        ULTIMATE_CHANNEL_GET_INTERFACE(channel)->amount_transmit(channel, process);
    else
        ultimate_channel_transmit_M_result(channel, process);
}

void ultimate_channel_change_status(UltimateChannel *channel, const gchar *format, ...) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    ChannelsSimple *simple      = channels_object_get_simple(CHANNELS_OBJECT(channel));
    gchar *         full_status = g_strdup_printf("%s - %s", channels_simple_get_measure_kind(simple), new_status);
    channels_simple_set_status(simple, full_status);
    g_free(new_status);
    g_free(full_status);
}

IntegrationObject *ultimate_channel_get_integration(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, NULL);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), NULL);
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    return integration;
}

gboolean ultimate_channel_start_analyse(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return FALSE;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        guint trigger = channels_simple_get_trigger(channels_object_get_simple(CHANNELS_OBJECT(channel))) + 1;
        channels_simple_set_trigger(channels_object_get_simple(CHANNELS_OBJECT(channel)), trigger);
        if (ultra_integration_object_analyze(ULTRA_INTEGRATION_OBJECT(integration),
                channels_simple_get_link(channels_object_get_simple(CHANNELS_OBJECT(channel))), trigger,
                channels_simple_get_tic(channels_object_get_simple(CHANNELS_OBJECT(channel))))) {
            if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->analyze_start) return ULTIMATE_CHANNEL_GET_INTERFACE(channel)->analyze_start(channel);
            return TRUE;
        }
    }
    return FALSE;
}

gboolean ultimate_channel_integration_is_runned(UltimateChannel *channel) {

    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return FALSE;
    gboolean           is_runned   = FALSE;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        is_runned = integration_simple_get_measure(integration_object_get_simple(integration)) &&
                    integration_simple_get_integrating(integration_object_get_simple(integration));
    }
    return is_runned;
}

void ultimate_channel_analyse_stop(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    ChannelsSimple *simple =  channels_object_get_simple(CHANNELS_OBJECT(channel));
    gboolean is_meas = channels_simple_get_is_measurement(simple);
    g_object_unref(simple);
    g_return_if_fail(simple!=NULL);
    if (!is_meas) return;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        ultra_integration_object_analyse_break(ULTRA_INTEGRATION_OBJECT(integration));
    }
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->analyze_break) ULTIMATE_CHANNEL_GET_INTERFACE(channel)->analyze_break(channel);
}

gboolean ultimate_channel_is_integrating(UltimateChannel *channel) {

    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return FALSE;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return FALSE;
    gboolean           is_integrating = FALSE;
    IntegrationObject *integration    = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        if (integration_simple_get_measure(integration_object_get_simple(INTEGRATION_OBJECT(integration))) &&
            integration_simple_get_integrating(integration_object_get_simple(INTEGRATION_OBJECT(integration)))) {
            is_integrating = TRUE;
        }
    }
    return is_integrating;
}

void ultimate_channel_justification(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        ultra_integration_object_justifying_run(ULTRA_INTEGRATION_OBJECT(integration));
    }
}

void ultimate_channel_integration(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        ultra_integration_object_integrating_run(ULTRA_INTEGRATION_OBJECT(integration));
    }
}

void ultimate_channel_calculate_justification(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        ultra_integration_object_calculate_justification(ULTRA_INTEGRATION_OBJECT(integration));
        channels_simple_set_measure_error(
            channels_object_get_simple(CHANNELS_OBJECT(channel)), integration_simple_get_justifying_error(integration_object_get_simple(integration)));
    }
}

void ultimate_channel_calculate_integration(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (channels_object_get_measurement(CHANNELS_OBJECT(channel)) == NULL) return;
    if (!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channel)))) return;
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        if (integration_simple_get_integrating(integration_object_get_simple(integration)) &&
            !integration_simple_get_integrated(integration_object_get_simple(integration))) {
            ultra_integration_object_calculate_integration(ULTRA_INTEGRATION_OBJECT(integration));
            channels_simple_set_measure_error(
                channels_object_get_simple(CHANNELS_OBJECT(channel)), integration_simple_get_integrating_error(integration_object_get_simple(integration)));
        }
    }
}

void ultimate_channel_check_limit(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    ChannelsSimple *     simple      = channels_object_get_simple(CHANNELS_OBJECT(channel));
    ChannelsMeasurement *measurement = channels_object_get_measurement(CHANNELS_OBJECT(channel));
    gboolean activated = channels_simple_get_limit_activated(simple);

    if(!activated){
      if( channels_simple_get_limit_pending(simple) != 0){
            mkt_log_limit_message("Channel %s (#0%d) limit gone", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
      }
      return;
    }
    gdouble min       = channels_simple_get_limit_min(simple);
    gdouble max       = channels_simple_get_limit_max(simple);
    gdouble value     = channels_simple_get_result(simple);
    gint    chpending = channels_simple_get_limit_pending(simple);
    gint    pending   = 0;
    if (value < min)
        pending = -1;
    else if (value > max)
        pending = 1;
    if (chpending != pending) {
        if (pending == -1) {
            mkt_log_limit_message("Channel %s (#0%d) limit min came", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
        } else if (pending == 1) {
            mkt_log_limit_message("Channel %s (#0%d) limit max came", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
        } else {
            mkt_log_limit_message("Channel %s (#0%d) limit gone", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
        }
    }
    gchar *info = NULL;
    if (pending != 0)
        info = g_strdup_printf("%s%s", channels_simple_get_limit_name(simple), pending > 0 ? "max" : "min");
    else
        info = g_strdup(_("OK"));
    if (measurement) channels_measurement_set_limit_info(measurement, info);
    g_free(info);
    channels_simple_set_limit_pending(simple, pending);
}

void ultimate_channel_check_limit_check(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    ChannelsSimple *simple    = channels_object_get_simple(CHANNELS_OBJECT(channel));
    ChannelsCheck * check     = channels_object_get_check(CHANNELS_OBJECT(channel));
    gboolean activated        = channels_check_get_limit_activated(check);
    if(!activated){
      if( channels_simple_get_limit_pending(simple) != 0){
        mkt_log_limit_message("Channel %s (#0%d) check limit gone", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
      }
      return;
    }
    gdouble         min       = channels_check_get_limit_min(check);
    gdouble         max       = channels_check_get_limit_max(check);
    gdouble         value     = channels_check_get_result(check);
    gint            chpending = channels_check_get_limit_pending(check);
    gint            pending   = 0;
    if (value < min)
        pending = -1;
    else if (value > max)
        pending = 1;
    if (chpending != pending) {
        if (pending == -1) {
            mkt_log_limit_message("Channel %s (#0%d) check limit min came", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
        } else if (pending == 1) {
            mkt_log_limit_message("Channel %s (#0%d) check limit max came", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
        } else {
            mkt_log_limit_message("Channel %s (#0%d) check limit gone", channels_simple_get_name(simple),channels_simple_get_stream_number(simple));
        }
    }
    gchar *info = NULL;
    if (pending != 0)
        info = g_strdup_printf("%s%s", channels_check_get_limit_name(check), pending > 0 ? "max" : "min");
    else
        info = g_strdup(_("OK"));
    channels_check_set_limit_info(check, info);
    channels_check_set_limit_pending(check, pending);
    g_free(info);
}

// #include <stdio.h>
// #include <stdarg.h>


// static void print_debug(const gchar *format, ...) {
//     gchar *path = g_build_path("/", g_get_home_dir(), "set-channels.log", NULL);
//     FILE *f = fopen(path, "a+");
//     if (f != NULL)
//     {
//         gchar *test = NULL;
//         va_list args;

//         va_start(args, format);
//         test = g_strdup_vprintf(format, args);
//         va_end(args);
//         fprintf(f,"%s\n",test);
//         g_free(test);
//         fflush(f);
//         fclose(f);
//     }
//     g_free(path);
// }


void ultimate_channel_transmit_last(UltimateChannel *channel ){
    g_return_if_fail(channel!=NULL);
    g_return_if_fail(ULTIMATE_CHANNEL(channel));
    // print_debug("tetst 1");
	ChannelsSimple *simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
	if(simple == NULL) return;

    // print_debug("tetst is-active:%d | is-alow:%d",channels_simple_get_is_activate(simple),channels_simple_get_is_allow(simple));
	if (channels_simple_get_is_activate(simple)){

        // print_debug("tetst 3");
	    guint identification = ULTRA_PROCESS_ONLINE+ channels_simple_get_stream_number(simple);
		MktMeasurement *measurement =MKT_MEASUREMENT(mkt_model_find_one(MKT_TYPE_MEASUREMENT_DATA,"select * from MktMeasurementData where measurement_channel=%"G_GUINT64_FORMAT" and measurement_identification=%d and measurement_type=1 ORDER BY measurement_trigger DESC LIMIT 1;",
				channels_simple_get_link(simple),identification));
        // print_debug("SQL:%s");
		if ( measurement ){
            // print_debug("send to analog value %f",mkt_measurement_value(measurement));
			channels_simple_set_result(simple,mkt_measurement_value(measurement));
			channels_simple_set_result(simple,mkt_measurement_value(measurement));
			channels_simple_set_online_result(simple, mkt_measurement_value(measurement));
			channels_simple_set_last_changed(simple,mkt_measurement_changed(measurement));
			g_object_unref(measurement);
            ultimate_channel_transmit_analog(channel);
		}
        ChannelsCheck *check = channels_object_get_check(CHANNELS_OBJECT(channel));
        if(check != NULL){
            identification = ULTRA_PROCESS_CHECK + channels_simple_get_stream_number(simple);
            MktMeasurement *measurement =MKT_MEASUREMENT(mkt_model_find_one(MKT_TYPE_MEASUREMENT_DATA,"select * from MktMeasurementData where measurement_channel=%"G_GUINT64_FORMAT" and measurement_identification = %d and measurement_type = 1 ORDER BY measurement_trigger DESC LIMIT 1;",
                    channels_simple_get_link(simple),identification));
            if (measurement&&channels_simple_get_is_check(simple)){
                channels_check_set_result(check,mkt_measurement_value(measurement));
                ultimate_channel_transmit_analog_check(channel);
            }
            if(measurement) g_object_unref(measurement);
            g_object_unref(check);
        }
	}
	g_object_unref(simple);
}


void ultimate_channel_transmit_analog(UltimateChannel *channel) {

    ChannelsSimple *simple = channels_object_get_simple(CHANNELS_OBJECT(channel));

    GDBusObjectManager *manager = tera_client_get_manager(tera_client_lookup("com.lar.analogs.out"), "/analogs");
    if (manager) {
        GDBusObject *analogout = g_dbus_object_manager_get_object(manager, channels_simple_get_analog_out(simple));
        if (analogout) {
            gdouble result = channels_simple_get_online_result(simple);
            if(result < channels_simple_get_min(simple))result = channels_simple_get_min(simple);
            if(result > channels_simple_get_max(simple))result = channels_simple_get_max(simple);
            analogs_out_call_set_value(analogs_object_get_out(ANALOGS_OBJECT(analogout)), result,
                channels_simple_get_min(simple), channels_simple_get_max(simple), NULL, NULL, NULL);
            g_object_unref(analogout);
        }
    }
}

void ultimate_channel_transmit_analog_check(UltimateChannel *channel) {
    ChannelsSimple *    simple  = channels_object_get_simple(CHANNELS_OBJECT(channel));
    ChannelsCheck *     check   = channels_object_get_check(CHANNELS_OBJECT(channel));
    GDBusObjectManager *manager = tera_client_get_manager(tera_client_lookup("com.lar.analogs.out"), "/analogs");
    if (manager) {
        GDBusObject *analogout = g_dbus_object_manager_get_object(manager, channels_check_get_analog_out(check));
        if (analogout) {
            analogs_out_call_set_value(analogs_object_get_out(ANALOGS_OBJECT(analogout)), channels_check_get_result(check), channels_simple_get_min(simple),
                channels_simple_get_max(simple), NULL, NULL, NULL);
            g_object_unref(analogout);
        }
    }
}

// Channel calculation errors
gboolean ultimate_channel_is_error(UltimateChannel *channel) {
    g_return_val_if_fail(channel != NULL, FALSE);
    g_return_val_if_fail(ULTIMATE_IS_CHANNEL(channel), FALSE);
    return FALSE;
}

void ultimate_channel_calculate_error(UltimateChannel *channel) {
    g_return_if_fail(channel != NULL);
    g_return_if_fail(ULTIMATE_IS_CHANNEL(channel));
    if (ULTIMATE_CHANNEL_GET_INTERFACE(channel)->calculate_error) ULTIMATE_CHANNEL_GET_INTERFACE(channel)->calculate_error(channel);
}

/** @} */
