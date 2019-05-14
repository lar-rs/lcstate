/*
 * @ingroup UltraChannelObject
 * @{
 * @file  ultra-channel-object.c	Pump object
 * @brief This is Pump control object description.
 *
 *
 *  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "ultra-stream-object.h"
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

#include "ultimate-process-object.h"
#include "ultra-channel-object.h"
#include "ultra-integration-object.h"

#include <math.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum {
    PROP_0,
    PROP_CHANNEL_OBJECT_STREAM,
    PROP_CHANNEL_OBJECT_NUMBER

};

struct _UltraChannelObjectPrivate {
    guint              type;
    UltraStreamObject *stream;
    guint              number;
    MktChannel *       channel;
    MktLimit *         limit;
    MktLimit *         limit_ch;
    IntegrationObject *active_sensor;
    GArray *           amount_array;
    MktCalibration *   current_calibration;
    MktCalPoint *      current_point;
};

#define ULTRA_CHANNEL_OBJECT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_CHANNEL_OBJECT, UltraChannelObjectPrivate))

static void ULTRA_CHANNEL_MEASUREMENT_TRIGGER(UltraChannelObject *channel) {
    ChannelsSimple *simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
    guint           triger = channels_simple_get_measurement_trigger(simple);
    channels_simple_set_measurement_trigger(simple, triger + 1);
}

static ChannelsSimple *ULTRA_CHANNEL_SIMPLE(UltraChannelObject *channel) {
    ChannelsSimple *simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
    g_assert_nonnull(simple);
    return simple;
}

static ChannelsMeasurement *ULTRA_CHANNEL_MEASUREMENT(UltraChannelObject *channel) {
    ChannelsMeasurement *meas = channels_object_get_measurement(CHANNELS_OBJECT(channel));
    g_assert_nonnull(meas);
    return meas;
}

/// Ultra channel interface functions
static MktChannel *ultra_channel_object_get_channel_model(UltimateChannel *channel) {
    UltraChannelObject *channel_object = ULTRA_CHANNEL_OBJECT(channel);
    return channel_object->priv->channel;
}

static gboolean ultra_channel_object_start_measurement(UltimateChannel *channel) {
    // UltraChannelObject *channel_object = ULTRA_CHANNEL_OBJECT(channel);
    return TRUE;
}

static gboolean ultra_channel_object_transmit_M_replicate(UltimateChannel *channel, MktProcessObject *process) {
    UltraChannelObject *channel_object    = ULTRA_CHANNEL_OBJECT(channel);
    gdouble             value             = 0.0;
    guint               measurement_count = 0;
    guint               trigger           = 0;
    MktProcess *        process_model     = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
    if (process_model == NULL) {
        mkt_errors_report(E2010, "Process %s origin model not found", process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))));
        return FALSE;
    }
    ChannelsSimple *   simple      = channels_object_get_simple(CHANNELS_OBJECT(channel));
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel))));
    if (integration) {
        gdouble integral = channels_simple_get_tic(channels_object_get_simple(CHANNELS_OBJECT(channel))) != TRUE ? integration_tc_get_integral(integration_object_get_tc(integration))
                                                                                                                 : integration_tic_get_integral(integration_object_get_tic(integration));
        measurement_count = channels_simple_get_measurement(simple);
        trigger           = channels_simple_get_trigger(simple);

        if (integration_simple_get_justifying_error(integration_object_get_simple(integration)))
            mkt_log_error_message("Channel %s justification %s failed", channels_simple_get_name(simple), integration_simple_get_name(integration_object_get_simple(integration)));
        else if (integration_simple_get_integrating_error(integration_object_get_simple(integration)))
            mkt_log_error_message("Channel %s integration %s failed", channels_simple_get_name(simple), integration_simple_get_name(integration_object_get_simple(integration)));
        // value = (raw - intercept) / slope
        gdouble         slope       = 1.0;
        gdouble         interceopt  = 0.0;
        MktCalibration *calibration = mkt_calibration_activated_for_channel(mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)));
        if (calibration) {
            slope      = mkt_calibration_slope(calibration);
            interceopt = mkt_calibration_intercept(calibration);
        }
        value = (integral - interceopt) / slope;
        value = value * channels_simple_get_factor(simple);
        channels_simple_set_curr_value(simple, value);
        gint replicate = process_simple_get_current_replicate(process_object_get_simple(PROCESS_OBJECT(process)));

        g_return_val_if_fail(process_model != NULL, FALSE);
        // MktModel *model = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA, NULL);
        MktModel *model = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA, "measurement-identification", mkt_process_identification(process_model), "measurement-channel",
            mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)), "measurement-process", mkt_model_ref_id(MKT_IMODEL(process_model)), "measurement-stream", mkt_process_stream(process_model),
            "measurement-type", 0, "measurement-changed", market_db_time_now(), "measurement-value", value, "measurement-value-row", integral, "measurement-trigger", measurement_count,
            "measurement-signal", trigger, "measurement-replicate", replicate, "measurement-name", channels_simple_get_name(simple), "measurement-unit", channels_simple_get_unit(simple), NULL);
            
        g_object_unref(model);
        ULTRA_CHANNEL_MEASUREMENT_TRIGGER(channel_object);
        return TRUE;
    } else {
        mkt_log_error_message("Channel %s integration failed - sensor object %s not found", channels_simple_get_name(simple), channels_simple_get_sensor_name(simple));
        channels_simple_set_measure_error(simple, TRUE);
   
    }
    return FALSE;
}

static gboolean ultra_channel_object_transmit_M_result(UltimateChannel *channel, MktProcessObject *process) {
    UltraChannelObject *channel_object    = ULTRA_CHANNEL_OBJECT(channel);
    guint               measurement_count = 0;
    guint               trigger           = 0;
    gdouble             cv                = 0.0;
    gdouble             result            = 0.0;
    MktProcess *        process_model     = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
    if (process_model == NULL) {
        mkt_errors_report(E2010, _("Process %s origin model not found"), process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))));
        return FALSE;
    }

    ChannelsSimple *simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
    if (channels_simple_get_statistic_done(simple) && !channels_simple_get_transmit_done(simple)) {
        measurement_count = channels_simple_get_measurement(simple);
        trigger           = channels_simple_get_trigger(simple);
        cv                = channels_measurement_get_last_cv(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
        result            = channels_measurement_get_last_round(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
        MktModel *model   = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA, "measurement-identification", mkt_process_identification(process_model), "measurement-channel",
            mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)), "measurement-process", mkt_model_ref_id(MKT_IMODEL(process_model)), "measurement-stream", mkt_process_stream(process_model),
            "measurement-type", 1, "measurement-changed", market_db_time_now(), "measurement-value", result, "measurement-cv", cv, "measurement-trigger", measurement_count, "measurement-signal",
            trigger, "measurement-replicate", 0, "measurement-name", channels_simple_get_name(simple), "measurement-unit", channels_simple_get_unit(simple), NULL);
        g_object_unref(model);
        channels_simple_set_result(simple, result);
        channels_simple_set_last_changed(simple, market_db_time_now());
        /*if(security_device_get_expiry_type(TERA_GUARD()) == ONLINE_EXPIRY)
                ultra_channel_transmit_to_analog_out(channel);*/
        channels_simple_set_transmit_done(simple, TRUE);
        ULTRA_CHANNEL_MEASUREMENT_TRIGGER(channel_object);

        return TRUE;
    }
    return FALSE;
}

static void ultra_channel_object_calculation_error(UltimateChannel *channel) { ultimate_channel_change_status(channel, _("Calculate error")); }

static void ultra_channel_integration_change_signal(IntegrationSimple *simple, GParamSpec *pspec, UltraChannelObject *channel) {
    channels_simple_set_signal(channels_object_get_simple(CHANNELS_OBJECT(channel)), integration_simple_get_signal(simple));
}

static void ultra_channel_integration_change_row_integral(GObject *object, GParamSpec *pspec, UltraChannelObject *channel) {

    gdouble row_integral = 0.0;
    g_object_get(object, "row-integral", &row_integral, NULL);
    channels_simple_set_row_result(channels_object_get_simple(CHANNELS_OBJECT(channel)), row_integral);
}

static gboolean ultra_channel_object_analyze_start(UltimateChannel *channel) {
    IntegrationObject *object = ultimate_channel_get_integration(channel);
    g_signal_connect(integration_object_get_simple(object), "notify::signal", G_CALLBACK(ultra_channel_integration_change_signal), channel);
    if (channels_simple_get_tic(channels_object_get_simple(CHANNELS_OBJECT(channel))) && integration_object_get_tic(object))
        g_signal_connect(integration_object_get_tic(object), "notify::row-integral", G_CALLBACK(ultra_channel_integration_change_row_integral), channel);
    else
        g_signal_connect(integration_object_get_tc(object), "notify::row-integral", G_CALLBACK(ultra_channel_integration_change_row_integral), channel);
    return TRUE;
}

static void ultra_channel_object_analyze_break(UltimateChannel *channel) {
    IntegrationObject *object = ultimate_channel_get_integration(channel);
    g_signal_handlers_disconnect_by_func(integration_object_get_simple(object), ultra_channel_integration_change_signal, channel);
    g_signal_handlers_disconnect_by_func(integration_object_get_tc(object), ultra_channel_integration_change_row_integral, channel);
    if (integration_object_get_tic(object)) g_signal_handlers_disconnect_by_func(integration_object_get_tic(object), ultra_channel_integration_change_row_integral, channel);
}

static void ultra_channel_object_amount_init(UltimateChannel *channel) {
    UltraChannelObject *channel_object = ULTRA_CHANNEL_OBJECT(channel);
    if (channel_object->priv->amount_array) g_array_free(channel_object->priv->amount_array, TRUE);
    channel_object->priv->amount_array = g_array_new(TRUE, TRUE, sizeof(gdouble));
}

static void ultra_channel_object_amount_transmit(UltimateChannel *channel, MktProcessObject *process) {
    UltraChannelObject *channel_object   = ULTRA_CHANNEL_OBJECT(channel);
    gdouble             measValue        = channels_measurement_get_last_round(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
    gdouble             cv               = channels_measurement_get_last_cv(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
    guint               amount_counter   = process_simple_get_amount_counter(process_object_get_simple(PROCESS_OBJECT(process)));
    gdouble             av_mv_percentage = process_simple_get_amount_percentage(process_object_get_simple(PROCESS_OBJECT(process)));
    MktProcess *        process_model    = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
    if (process_model == NULL) {
        mkt_errors_report(E2010, _("Process %s origin model not found"), process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))));
        return;
    }
    if (channel_object->priv->amount_array == NULL) {
        channel_object->priv->amount_array = g_array_new(TRUE, TRUE, sizeof(gdouble));
    }
    if (amount_counter > 0 && av_mv_percentage > 0.0001) {

        gdouble currentAverage = measValue;

        if (channel_object->priv->amount_array->len > 0) {
            currentAverage = 0.0;
            guint i        = 0;
            for (i = 0; i < channel_object->priv->amount_array->len; i++) currentAverage += g_array_index(channel_object->priv->amount_array, gdouble, i);

            currentAverage /= channel_object->priv->amount_array->len;
        }
        if (measValue > currentAverage * (1 + (av_mv_percentage / 100.0)) || measValue < currentAverage * (1 - (av_mv_percentage / 100.0))) {
            if (channel_object->priv->amount_array) g_array_free(channel_object->priv->amount_array, TRUE);
            channel_object->priv->amount_array = g_array_new(TRUE, TRUE, sizeof(gdouble));
        }
        guint i                            = 0;
        currentAverage                     = 0.0;
        channel_object->priv->amount_array = g_array_append_val(channel_object->priv->amount_array, measValue);
        for (i = 0; i < channel_object->priv->amount_array->len; i++) currentAverage += g_array_index(channel_object->priv->amount_array, gdouble, i);

        currentAverage /= channel_object->priv->amount_array->len;
        if (i >= amount_counter) g_array_remove_index(channel_object->priv->amount_array, 0);
        measValue = currentAverage;
    }
    guint           measurement_count = 0;
    guint           trigger           = 0;
    ChannelsSimple *simple            = channels_object_get_simple(CHANNELS_OBJECT(channel));
    if (channels_simple_get_statistic_done(simple) && !channels_simple_get_transmit_done(simple)) {
        measurement_count = channels_simple_get_measurement(simple);
        trigger           = channels_simple_get_trigger(simple);
        cv                = channels_measurement_get_last_cv(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
        MktModel *model   = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA, "measurement-identification", mkt_process_identification(process_model), "measurement-channel",
            mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)), "measurement-process", mkt_model_ref_id(MKT_IMODEL(process_model)), "measurement-stream", mkt_process_stream(process_model),
            "measurement-type", 1, "measurement-changed", market_db_time_now(), "measurement-value", measValue, "measurement-cv", cv, "measurement-trigger", measurement_count, "measurement-signal",
            trigger, "measurement-replicate", 0, "measurement-name", channels_simple_get_name(simple), "measurement-unit", channels_simple_get_unit(simple), NULL);
        g_object_unref(model);
        channels_simple_set_result(simple, measValue);
        channels_simple_set_last_changed(simple, market_db_time_now());
        channels_simple_set_transmit_done(simple, TRUE);
        ULTRA_CHANNEL_MEASUREMENT_TRIGGER(channel_object);
    }
}

static void ultra_channel_object_init_interface(UltimateChannelInterface *iface) {
    iface->channel_model        = ultra_channel_object_get_channel_model;
    iface->start_measurement    = ultra_channel_object_start_measurement;
    iface->transmit_M_replicate = ultra_channel_object_transmit_M_replicate;
    iface->transmit_M_result    = ultra_channel_object_transmit_M_result;
    iface->calculate_error      = ultra_channel_object_calculation_error;
    iface->amount_init          = ultra_channel_object_amount_init;
    iface->amount_transmit      = ultra_channel_object_amount_transmit;
    iface->analyze_start        = ultra_channel_object_analyze_start;
    iface->analyze_break        = ultra_channel_object_analyze_break;
}

G_DEFINE_TYPE_WITH_CODE(UltraChannelObject, ultra_channel_object, CHANNELS_TYPE_OBJECT_SKELETON, G_IMPLEMENT_INTERFACE(ULTIMATE_TYPE_CHANNEL, ultra_channel_object_init_interface))

// Watch property calback function

static void ultra_channel_load_main_channel(UltraChannelObject *channel) {
    const gchar *object_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(channel));
    if (channel->priv->channel) g_object_unref(channel->priv->channel);
    channel->priv->channel = MKT_CHANNEL(mkt_model_select_one(MKT_TYPE_CHANNEL_MODEL, "select * from  %s where param_object_path='%s'", g_type_name(MKT_TYPE_CHANNEL_MODEL),object_path));
    if (channel->priv->channel == NULL) {
        guint64 stream_id      = streams_simple_get_id(streams_object_get_simple(STREAMS_OBJECT(channel->priv->stream)));
        gchar * name           = g_strdup_printf("Channel %d measurement", channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel)));
        gchar * analog_path    = g_strdup_printf("/analogs/%d", channel->priv->number);
        gchar * channel_name   = g_strdup_printf("CH%d", channel->priv->number);
        channel->priv->channel = MKT_CHANNEL(mkt_model_new(MKT_TYPE_CHANNEL_MODEL, "param-object-path", object_path, "param-name", name, "channel-stream", stream_id, "channel-type", "main",
            "channel-analog-out", analog_path, "channel-max", 100.0, "channel-name", channel_name, NULL));
        mkt_model_delete_async(MKT_TYPE_SENSOR_DATA, NULL, NULL, NULL, "delete from MktSensorData where data_creator=%" G_GUINT64_FORMAT, mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
        mkt_model_delete_async(
            MKT_TYPE_MEASUREMENT_DATA, NULL, NULL, NULL, "delete from MktMeasurementData where measurement_channel=%" G_GUINT64_FORMAT, mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
        g_free(name);
        g_free(analog_path);
        g_free(channel_name);
    }
    mkt_param_activate(MKT_PARAM(channel->priv->channel));

    // g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"stream",channel->priv->channel,"channel-stream",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    // g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"sensor",channel->priv->channel,"channel-sensor",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

    g_object_bind_property(channel->priv->channel, "channel-result", ULTRA_CHANNEL_SIMPLE(channel), "result", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-changed", ULTRA_CHANNEL_SIMPLE(channel), "last-changed", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-sensor", ULTRA_CHANNEL_SIMPLE(channel), "sensor", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-min", ULTRA_CHANNEL_SIMPLE(channel), "min", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-max", ULTRA_CHANNEL_SIMPLE(channel), "max", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-factor", ULTRA_CHANNEL_SIMPLE(channel), "factor", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-activated", ULTRA_CHANNEL_SIMPLE(channel), "activated", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-check", ULTRA_CHANNEL_SIMPLE(channel), "is-check", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-measurement", ULTRA_CHANNEL_SIMPLE(channel), "measurement", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-trigger", ULTRA_CHANNEL_SIMPLE(channel), "trigger", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-analog-out", ULTRA_CHANNEL_SIMPLE(channel), "analog-out", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-name", ULTRA_CHANNEL_SIMPLE(channel), "name", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-unit", ULTRA_CHANNEL_SIMPLE(channel), "unit", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-raw", ULTRA_CHANNEL_MEASUREMENT(channel), "last-round", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-cv", ULTRA_CHANNEL_MEASUREMENT(channel), "last-cv", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel), "is-allow", channel->priv->channel, "channel-allow", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->channel, "channel-activated-cal", channels_object_get_calibration(CHANNELS_OBJECT(channel)), "activated", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    channels_simple_set_link(channels_object_get_simple(CHANNELS_OBJECT(channel)), mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));

    if (channel->priv->limit) g_object_unref(channel->priv->limit);
    channel->priv->limit = MKT_LIMIT(mkt_model_select_one(MKT_TYPE_LIMIT_MESSAGE, "select * from %s where param_object_path='%s' and param_type='Measurement'", g_type_name(MKT_TYPE_LIMIT_MESSAGE),object_path));
    guint limit_number   = channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel));
    if (channel->priv->limit == NULL) {
        gchar *limit_name    = g_strdup_printf("L%d_", limit_number);
        gchar *name          = g_strdup_printf("Channel %d limit", channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel)));
        channel->priv->limit = MKT_LIMIT(
            mkt_model_new(MKT_TYPE_LIMIT_MESSAGE, "param-object-path", object_path, "param-name", name, "param-type", "Measurement", "limit-number", limit_number, "limit-name", limit_name, NULL));
        g_free(name);
        g_free(limit_name);
    }
    g_object_set(channel->priv->limit, "limit-pending", 0, NULL);
    g_object_bind_property(channel->priv->limit, "limit-name", ULTRA_CHANNEL_SIMPLE(channel), "limit-name", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->limit, "limit-min", ULTRA_CHANNEL_SIMPLE(channel), "limit-min", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->limit, "limit-max", ULTRA_CHANNEL_SIMPLE(channel), "limit-max", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->limit, "limit-pending", ULTRA_CHANNEL_SIMPLE(channel), "limit-pending", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(channel->priv->limit, "limit-activated", ULTRA_CHANNEL_SIMPLE(channel), "limit-activated", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    ChannelsCheck *check = channels_object_get_check(CHANNELS_OBJECT(channel));
    if (check) {
        if (channel->priv->limit_ch) g_object_unref(channel->priv->limit_ch);

        channel->priv->limit_ch = MKT_LIMIT(mkt_model_select_one(MKT_TYPE_LIMIT_MESSAGE, "select * from %s where param_object_path='%s' and param_type='Check'", g_type_name(MKT_TYPE_LIMIT_MESSAGE),object_path));
        if (channel->priv->limit_ch == NULL) {
            gchar *limit_name       = g_strdup_printf("LV%d_", limit_number);
            gchar *name             = g_strdup_printf("Channel %d check limit", channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel)));
            channel->priv->limit_ch = MKT_LIMIT(mkt_model_new(MKT_TYPE_LIMIT_MESSAGE, "param-object-path", object_path, "param-type", "Check", "param-name", name, "limit-number", limit_number,
                "limit-name", limit_name, "limit-activated", TRUE, NULL));
            g_free(name);
            g_free(limit_name);
        }
        g_object_set(channel->priv->limit_ch, "limit-pending", FALSE, NULL);
        if (!mkt_limit_activated(channel->priv->limit_ch)) g_object_set(channel->priv->limit_ch, "limit-activated", TRUE, NULL);

        g_object_bind_property(channel->priv->limit_ch, "limit-name", check, "limit-name", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(channel->priv->limit_ch, "limit-min", check, "limit-min", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(channel->priv->limit_ch, "limit-max", check, "limit-max", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(channel->priv->limit_ch, "limit-pending", check, "limit-pending", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(channel->priv->limit_ch, "limit-activated", check, "limit-activated", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(channel->priv->channel, "channel-check-analog-out", check, "analog-out", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    }

    GSList *points = mkt_model_select(MKT_TYPE_POINT_MODEL, "select * from $tablename where point_ref = %" G_GUINT64_FORMAT, mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));

    if (points == NULL) {

        MktModel *point =
            mkt_model_new(MKT_TYPE_POINT_MODEL, "point-ref", mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)), NULL); //"point-ref",mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)),
        g_object_unref(point);
    }
    if (points) g_slist_free_full(points, g_object_unref);
}

static void ultra_channel_object_init(UltraChannelObject *ultra_channel_object) {

    UltraChannelObjectPrivate *priv = ULTRA_CHANNEL_OBJECT_PRIVATE(ultra_channel_object);
    priv->channel                   = NULL;
    priv->stream                    = NULL;
    priv->limit                     = NULL;
    priv->limit_ch                  = NULL;
    priv->current_calibration       = NULL;
    priv->current_point             = NULL;
    ultra_channel_object->priv      = priv;
}

static void ultra_channel_object_finalize(GObject *object) {
    UltraChannelObject *channel = ULTRA_CHANNEL_OBJECT(object);
    if (channel->priv->channel) g_object_unref(channel->priv->channel);
    if (channel->priv->stream) g_object_unref(channel->priv->stream);
    if (channel->priv->limit) g_object_unref(channel->priv->limit);
    if (channel->priv->limit_ch) g_object_unref(channel->priv->limit_ch);
    G_OBJECT_CLASS(ultra_channel_object_parent_class)->finalize(object);
}

static void ultra_channel_change_channel_status(GObject *object, GParamSpec *pspec, UltraChannelObject *channel) {
    ChannelsSimple *simple = NULL;
    simple                 = channels_object_get_simple(CHANNELS_OBJECT(channel));
    gboolean activated     = (channels_simple_get_is_allow(simple) && channels_simple_get_activated(simple));
    channels_simple_set_is_activate(simple, activated);
    channels_simple_set_is_calibration(simple, activated && channels_calibration_get_activated(channels_object_get_calibration(CHANNELS_OBJECT(channel))));
}

static void ultra_channel_change_channel_sensor(ChannelsSimple *simple, GParamSpec *pspec, UltraChannelObject *channel) {
    if (channel->priv->active_sensor != NULL) {
        g_signal_handlers_disconnect_by_data(channel->priv->active_sensor, channel);
        g_signal_handlers_disconnect_by_data(integration_object_get_simple(channel->priv->active_sensor), channel);
        g_signal_handlers_disconnect_by_data(integration_object_get_tc(channel->priv->active_sensor), channel);
        g_signal_handlers_disconnect_by_data(integration_object_get_tic(channel->priv->active_sensor), channel);
    }
    channel->priv->active_sensor = NULL;
    IntegrationObject *object    = ultra_integration(channels_simple_get_sensor(simple));
    if (object) {
        channel->priv->active_sensor = object;
        channels_simple_set_sensor_name(simple, integration_simple_get_name(integration_object_get_simple(object)));
        channels_simple_set_is_allow(simple, TRUE);
    } else {
        channels_simple_set_sensor_name(simple, _("sensor fail"));
        channels_simple_set_is_allow(simple, FALSE);
    }
}

static void ultra_channel_object_constructed(GObject *object) {
    UltraChannelObject *channel = ULTRA_CHANNEL_OBJECT(object);
    ChannelsSimple *    simple  = channels_simple_skeleton_new();

    channels_object_skeleton_set_simple(CHANNELS_OBJECT_SKELETON(object), simple);
    channels_simple_set_stream(simple, g_dbus_object_get_object_path(G_DBUS_OBJECT(channel->priv->stream)));
    channels_simple_set_number(simple, channel->priv->number);
    channels_simple_set_is_measurement(simple, TRUE);
    channels_simple_set_is_calculated(simple, FALSE);

    g_object_unref(simple);
    ChannelsMeasurement *meas = channels_measurement_skeleton_new();
    channels_object_skeleton_set_measurement(CHANNELS_OBJECT_SKELETON(object), meas);
    g_object_unref(meas);

    channel->priv->active_sensor = NULL;

    ChannelsCalibration *cal = channels_calibration_skeleton_new();
    channels_object_skeleton_set_calibration(CHANNELS_OBJECT_SKELETON(object), cal);

    g_object_unref(cal);

    g_signal_connect(channels_object_get_simple(CHANNELS_OBJECT(channel)), "notify::is-allow", G_CALLBACK(ultra_channel_change_channel_status), channel);
    g_signal_connect(channels_object_get_simple(CHANNELS_OBJECT(channel)), "notify::activated", G_CALLBACK(ultra_channel_change_channel_status), channel);
    g_signal_connect(channels_object_get_calibration(CHANNELS_OBJECT(channel)), "notify::activated", G_CALLBACK(ultra_channel_change_channel_status), channel);
    g_signal_connect(channels_object_get_simple(CHANNELS_OBJECT(channel)), "notify::sensor", G_CALLBACK(ultra_channel_change_channel_sensor), channel);

    ChannelsSingle *single = channels_single_skeleton_new();
    channels_object_skeleton_set_single(CHANNELS_OBJECT_SKELETON(object), single);
    g_object_unref(single);

    ChannelsCheck *check = channels_check_skeleton_new();
    channels_object_skeleton_set_check(CHANNELS_OBJECT_SKELETON(object), check);
    g_object_unref(check);

    ultra_channel_load_main_channel(channel);
    // g_signal_connect (integration_object_get_simple(INTEGRATION_OBJECT(channel->priv->integration)),"integrated",G_CALLBACK (ultra_channel_object_add_calibration_point_callback),object);
    // g_signal_connect(TERA_GUARD(),"stop-all",G_CALLBACK(ultra_channel_object_system_stop_all_callback), channel);

    G_OBJECT_CLASS(ultra_channel_object_parent_class)->constructed(object);
}

static void ultra_channel_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_CHANNEL_OBJECT(object));
    UltraChannelObject *channel = ULTRA_CHANNEL_OBJECT(object);
    switch (prop_id) {
    case PROP_CHANNEL_OBJECT_STREAM:
        if (channel->priv->stream) g_object_unref(channel->priv->stream);
        channel->priv->stream = g_value_dup_object(value);
        break;
    case PROP_CHANNEL_OBJECT_NUMBER:
        channel->priv->number = g_value_get_uint(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_channel_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_CHANNEL_OBJECT(object));
    UltraChannelObject *channel = ULTRA_CHANNEL_OBJECT(object);
    switch (prop_id) {
    case PROP_CHANNEL_OBJECT_STREAM:
        g_value_set_object(value, channel->priv->stream);
        break;
    case PROP_CHANNEL_OBJECT_NUMBER:
        g_value_set_uint(value, channel->priv->number);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_channel_object_class_init(UltraChannelObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraChannelObjectClass));
    object_class->finalize     = ultra_channel_object_finalize;
    object_class->set_property = ultra_channel_object_set_property;
    object_class->get_property = ultra_channel_object_get_property;
    object_class->constructed  = ultra_channel_object_constructed;
    // parent_class->initialize     = NULL;
    // parent_class->emergency_stop = NULL;
    g_object_class_install_property(object_class, PROP_CHANNEL_OBJECT_STREAM,
        g_param_spec_object("channel-stream", "Channel stream number", "Channel stream number", ULTRA_TYPE_STREAM_OBJECT, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(
        object_class, PROP_CHANNEL_OBJECT_NUMBER, g_param_spec_uint("channel-number", "Channel number", "Channel number", 0, 200, 0, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

gboolean ultra_channel_calibration_calculate_statistic(UltraChannelObject *channel_object, MktProcessObject *process) {
    g_return_val_if_fail(channel_object != NULL, FALSE);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), FALSE);
    g_return_val_if_fail(channel_object->priv->current_calibration != NULL, FALSE);
    g_return_val_if_fail(channel_object->priv->current_point != NULL, FALSE);

    guint          max_replicates;
    guint          outlier;
    gdouble        max_cv;
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));

    max_replicates            = process_simple_get_replicates(simple);
    outlier                   = process_simple_get_outliers(simple);
    max_cv                    = process_simple_get_max_cv(simple);
    gboolean enoughReplicates = FALSE;
    GSList * data             = mkt_calibration_point_data(channel_object->priv->current_point);
    if (data == NULL) {
        mkt_process_object_critical(
            process, "Calibration %s point %.0f have not data", mkt_param_name(MKT_PARAM(channel_object->priv->current_calibration)), mkt_cal_point_solution(channel_object->priv->current_point));
        return FALSE;
    }
    guint   curr_replicate = g_slist_length(data);
    gint    curr_outlier   = 0;
    GSList *l              = NULL;
    for (l = data; l != NULL; l = l->next) {
        //
        mkt_model_print_stdout(MKT_MODEL(l->data));
        g_object_set(l->data, "cal-data-outlier", FALSE, NULL);
    }
    gdouble raw = 0.;
    // mkt_debug("New value %d = %f", mchannel->priv->curr_replicate, );
    gdouble standardDeviation = 0.;
    gdouble max_d_sqr = 0., d_sqr = 0.;
    gdouble sum_d_sqr = 0.;
    gdouble cv        = 0.0;
    guint   interrupt = 0;

    while (!enoughReplicates) {
        curr_replicate = 0;
        curr_outlier   = 0;
        interrupt++;
        raw = 0.0;

        for (l = data; l != NULL; l = l->next) {
            if (!mkt_cal_data_outlier(MKT_CAL_DATA(l->data))) {
                raw += mkt_cal_data_value(MKT_CAL_DATA(l->data));
                curr_replicate++;
            } else {
                curr_outlier++;
            }
        }
        raw /= curr_replicate;
        if (max_replicates == 1) {
            enoughReplicates = TRUE;
            break;
        }
        if (1 == curr_replicate) {
            break;
        }
        sum_d_sqr = 0.;
        for (l = data; l != NULL; l = l->next)
            if (!mkt_cal_data_outlier(MKT_CAL_DATA(l->data))) sum_d_sqr += (mkt_cal_data_value(MKT_CAL_DATA(l->data)) - raw) * (mkt_cal_data_value(MKT_CAL_DATA(l->data)) - raw);

        standardDeviation = sqrt(sum_d_sqr / (curr_replicate - 1));
        if (raw > 0.000000) cv = 100. * standardDeviation / fabs(raw);

        if (max_replicates > curr_replicate) {
            break;
        }
        if (max_replicates <= curr_replicate && (cv < max_cv || cv < 0.0099)) {
            enoughReplicates = TRUE;
            break;
        }
        if (max_replicates == curr_replicate && (curr_outlier >= outlier)) {
            enoughReplicates = TRUE;
            break;
        }
        max_d_sqr              = G_MINDOUBLE;
        GSList *outlier_object = NULL;
        for (l = data; l != NULL; l = l->next) {
            if (!mkt_cal_data_outlier(MKT_CAL_DATA(l->data))) {

                d_sqr = (mkt_cal_data_value(MKT_CAL_DATA(l->data)) - raw) * (mkt_cal_data_value(MKT_CAL_DATA(l->data)) - raw);
                // TRACE:mkt_trace ("val %f d_sqr=%f",channel->values[i].value,d_sqr);
                if (d_sqr > max_d_sqr || isnan(mkt_cal_data_value(MKT_CAL_DATA(l->data))) || isinf(mkt_cal_data_value(MKT_CAL_DATA(l->data)))) {
                    outlier_object = l;
                    // TRACE:mkt_trace ("%d d_sqr=%f>%f",iv,d_sqr,max_d_sqr);
                    max_d_sqr = d_sqr;
                }
            }
        }
        if (outlier_object) {
            g_object_set(outlier_object->data, "cal-data-outlier", TRUE, NULL);
        }
        if (interrupt > (max_replicates + outlier) * 2) {
            enoughReplicates = TRUE;
            break;
        }
    }
    g_slist_free_full(data, g_object_unref);
    if (cv > 100.0) cv = 100.;
    g_object_set(channel_object->priv->current_point, "cal-point-average", raw, "cal-point-cv", cv, "cal-point-done", enoughReplicates, NULL);
    if (enoughReplicates) {
        mkt_calibration_calculate(channel_object->priv->current_calibration);
    }
    return enoughReplicates;
}

MktCalibration *ultra_channel_calibration_model(UltraChannelObject *channel_object) {
    g_return_val_if_fail(channel_object != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), NULL);
    return channel_object->priv->current_calibration;
}

MktCalPoint *ultra_channel_calibration_current_point(UltraChannelObject *channel_object) {
    g_return_val_if_fail(channel_object != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), NULL);
    return channel_object->priv->current_point;
}
gboolean ultra_channel_calibration_next_point(UltraChannelObject *channel_object) {
    g_return_val_if_fail(channel_object != NULL, FALSE);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), FALSE);
    g_return_val_if_fail(channel_object->priv->current_calibration != NULL, FALSE);
    if (channel_object->priv->current_point) g_object_unref(channel_object->priv->current_point);
    channel_object->priv->current_point = NULL;
    GSList *points                      = mkt_calibration_next_points(channel_object->priv->current_calibration);
    if (points) {
        channel_object->priv->current_point = MKT_CAL_POINT(g_object_ref(points->data));
        g_slist_free_full(points, g_object_unref);
        return TRUE;
    }
    return FALSE;
}

gint ultra_channel_start_calibration(UltraChannelObject *channel_object, MktProcessObject *process) {
    g_return_val_if_fail(channel_object != NULL, FALSE);
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), FALSE);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
    if (!channels_simple_get_is_activate(channels_object_get_simple(CHANNELS_OBJECT(channel_object))) ||
        !channels_simple_get_is_calibration(channels_object_get_simple(CHANNELS_OBJECT(channel_object))))
        return 0;
    MktProcess *process_model = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
    g_return_val_if_fail(process_model != NULL, FALSE);
    if (channel_object->priv->current_point) g_object_unref(channel_object->priv->current_point);
    channel_object->priv->current_point = NULL;
    if (channel_object->priv->current_calibration) g_object_unref(channel_object->priv->current_calibration);
    channel_object->priv->current_calibration = NULL;
    MktCalibration *grund_cal                 = NULL;
    MktCalibration *activ_cal                 = NULL;
    GSList *        points                    = NULL;
    guint           start_status              = -1;
    gboolean        noob                      = security_device_get_level(TERA_GUARD()) < 3;
    gboolean        online                    = process_simple_get_is_online(process_object_get_simple(PROCESS_OBJECT(process)));
    ChannelsSimple *simple                    = NULL;
    simple                                    = channels_object_get_simple(CHANNELS_OBJECT(channel_object));
    points                                    = mkt_calibration_default_points(channels_simple_get_link(simple));
    if (points == NULL) {
        mkt_log_error_message("channel %s %s can not start - calibration points not found", channels_simple_get_name(channels_object_get_simple(CHANNELS_OBJECT(channel_object))),
            online ? "Autocalibration" : "Calibration");
        goto failed;
    }
    activ_cal = mkt_calibration_activated_for_channel(channels_simple_get_link(simple));

    if (activ_cal != NULL) {
        gboolean afail = FALSE;
        GSList * ap    = mkt_calibration_points(activ_cal);
        if (ap) {
            guint plen = g_slist_length(ap);
            if (plen > 1 && (noob || online)) {
                mkt_log_error_message("channel %s %s can not start - multipoints calibration active ", channels_simple_get_name(channels_object_get_simple(CHANNELS_OBJECT(channel_object))),
                    online ? "Autocalibration" : "Calibration");
                afail = TRUE;
            }
            if (ap) g_slist_free_full(ap, g_object_unref);
        } else {
            mkt_log_error_message("channel %s %s can not start -  active calibration points not found", channels_simple_get_name(channels_object_get_simple(CHANNELS_OBJECT(channel_object))),
                online ? "Autocalibration" : "Calibration");
            afail = TRUE;
        }
        if (afail) goto failed;
    } else {
        if (online) {
            mkt_log_error_message("channel %s autocalibration can not start - not finished calibrations found ", channels_simple_get_name(channels_object_get_simple(CHANNELS_OBJECT(channel_object))));
            goto failed;
        }
        if (noob) {
            mkt_log_error_message("channel %s autocalibration can not start - not finished calibrations found ", channels_simple_get_name(channels_object_get_simple(CHANNELS_OBJECT(channel_object))));
            goto failed;
        }
    }
    grund_cal = mkt_calibration_main_for_channel(channels_simple_get_link(simple));
    if (grund_cal == NULL) {
        if (noob) {
            mkt_log_error_message("channel %s %s can not start - parent calibration no found", channels_simple_get_name(channels_object_get_simple(CHANNELS_OBJECT(channel_object))),
                online ? "Autocalibration" : "Calibration");
            goto failed;
        }
    }
    StreamsObject *stream    = ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(process));
    gdouble        deviation = 0.0;
    if (stream) deviation    = streams_ultra_get_allowed_deviation(streams_object_get_ultra(stream));
    gdouble max_cv           = process_simple_get_max_cv(process_object_get_simple(PROCESS_OBJECT(process)));
    // gchar *cal_name = g_strdup_printf("Calibration %s",channels_simple_get_name(simple));
    // gchar *cal_desc = g_strdup_printf("%s channel %s",process_simple_get_full_name(process_object_get_simple(PROCESS_OBJECT(process))),channels_simple_get_name(simple));
    channel_object->priv->current_calibration = MKT_CALIBRATION(mkt_model_new(MKT_TYPE_CALIBRATION_MODEL, "calibration-identification", mkt_process_identification(process_model),
        "calibration-channel", channels_simple_get_link(simple), "calibration-process", mkt_model_ref_id(MKT_IMODEL(process_model)), "calibration-stream", mkt_process_stream(process_model),
        "calibration-deviation", deviation, "calibration-cv", max_cv, NULL));
    mkt_calibration_update_time(channel_object->priv->current_calibration);
    GSList *pl = NULL;
    for (pl = points; pl != NULL; pl = pl->next) {

        MktCalPoint *point = MKT_CAL_POINT(mkt_model_new(MKT_TYPE_CAL_POINT_MODEL, "cal-point-calibration", mkt_model_ref_id(MKT_IMODEL(channel_object->priv->current_calibration)),
            "cal-point-solution", mkt_point_solution(MKT_POINT(pl->data)), NULL));
        start_status                                                                         = 1;
        if (channel_object->priv->current_point == NULL) channel_object->priv->current_point = MKT_CAL_POINT(g_object_ref(point));
        g_object_unref(point);
    }
// g_free(cal_name);
// g_free(cal_desc);

failed:
    if (points) g_slist_free_full(points, g_object_unref);
    if (activ_cal != NULL) g_object_unref(activ_cal);
    if (grund_cal != NULL) g_object_unref(grund_cal);

    return start_status;
}

gboolean ultra_channel_transmit_integration(UltraChannelObject *channel_object, MktProcessObject *process) {
    g_return_val_if_fail(channel_object != NULL, FALSE);
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), FALSE);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
    g_return_val_if_fail(channel_object->priv->current_calibration != NULL, FALSE);
    g_return_val_if_fail(channel_object->priv->current_point != NULL, FALSE);
    ProcessSimple *    simple      = process_object_get_simple(PROCESS_OBJECT(process));
    IntegrationObject *integration = ultra_integration(channels_simple_get_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel_object))));
    if (integration) {
        gdouble integral = channels_simple_get_tic(channels_object_get_simple(CHANNELS_OBJECT(channel_object))) != TRUE ? integration_tc_get_integral(integration_object_get_tc(integration))
                                                                                                                        : integration_tic_get_integral(integration_object_get_tic(integration));
        guint       trigger = channels_simple_get_trigger(channels_object_get_simple(CHANNELS_OBJECT(channel_object)));
        MktCalData *data =
            MKT_CAL_DATA(mkt_model_new(MKT_TYPE_CAL_DATA_MODEL, "cal-data-value", integral, "cal-data-trigger", trigger, "cal-data-replicate", process_simple_get_current_replicate(simple), NULL));
        mkt_calibration_point_add_data(channel_object->priv->current_point, data);
        mkt_calibration_update_time(channel_object->priv->current_calibration);
        g_object_unref(data);
        return TRUE;
    }
    return FALSE;
}

gboolean ultra_channel_activate_calibration(UltraChannelObject *channel_object, gboolean main) {
    g_return_val_if_fail(channel_object != NULL, FALSE);
    g_return_val_if_fail(ULTRA_IS_CHANNEL_OBJECT(channel_object), FALSE);
    g_return_val_if_fail(channel_object->priv->current_calibration != NULL, FALSE);
    g_return_val_if_fail(mkt_calibration_done(channel_object->priv->current_calibration), FALSE);
    if (!main) {
        mkt_model_exec(MKT_TYPE_CALIBRATION_MODEL, "UPDATE %s SET calibration_activated=0 WHERE calibration_channel = %" G_GUINT64_FORMAT, g_type_name(MKT_TYPE_CALIBRATION_MODEL),
            channels_simple_get_link(channels_object_get_simple(CHANNELS_OBJECT(channel_object))));
        g_object_set(channel_object->priv->current_calibration, "calibration-activated", TRUE, NULL);
    } else {
        mkt_model_exec(MKT_TYPE_CALIBRATION_MODEL, "UPDATE %s SET calibration_activated=0, calibration_main = 0 WHERE calibration_channel = %" G_GUINT64_FORMAT,
            g_type_name(MKT_TYPE_CALIBRATION_MODEL), channels_simple_get_link(channels_object_get_simple(CHANNELS_OBJECT(channel_object))));
        g_object_set(channel_object->priv->current_calibration, "calibration-activated", TRUE, "calibration-main", TRUE, NULL);
    }
    return TRUE;
}

/** @} */
