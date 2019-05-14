/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultra-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
ultra-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ultra-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "analyze-task.h"
#include "measurement-application-object.h"
#include "prepare-task.h"
#include "ultra-channel-diff.h"
#include "ultra-channel-object.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

#include "calibration-process.h"
#include "measurement-process.h"

#include "ultra-control-process.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum {
    PROP_0,
    PROP_STREAM_NUMBER,
    PROP_STREAM_SINGLE_VESSEL,
    PROP_STREAM_SAMPLE_VESSEL,
    PROP_STREAM_CHECK_VESSEL,
    PROP_STREAM_DRAIN_VESSEL,
    PROP_STREAM_CAL_VESSEL,
    PROP_STREAM_DILUTION_VESSEL,
    PROP_SOLUTION
};

static GDBusObjectManagerServer *ULTRA_STREAM_SERVER = NULL;

struct _UltraStreamObjectPrivate {
    gboolean  brocked;
    guint     stream_number;
    gboolean  is_online;
    MktModel *online_category;
    MktModel *cal_category;
    MktModel *check_category;
    MktModel *online_statistic;
    MktModel *single_statistic;
    MktModel *cal_statistic;
    MktModel *measparam;
    MktModel *posparam;

    MktParamboolean *is_dilution;
    MktParamboolean *on_replicate;

    ChannelsObject *Diff;
    ChannelsObject *TC;
    ChannelsObject *TIC;
    ChannelsObject *CODo;
    ChannelsObject *TNb;

    GList *channels;
    GList *measurement_channels;
    GList *calibration_channels;
    GList *activated_channels;
    GList *check_channels;

    MktProcessObject *single;
    MktProcessObject *online;
    MktProcessObject *check;
    MktProcessObject *calibration;
    PumpsObject *     sample_pump;

    NodesDigital16 *remote_node;
    guint           remote_in;
    guint           state;
    gboolean        no_sampling;
    guint           range;
};

/* signals */
/*
enum {
        ULTRA_STREAM_PREPARE_DONE,
        ULTRA_STREAM_ANALYSE_DONE,
        ULTRA_STREAM_LAST_SIGNAL
};

static guint ultra_stream_signals[ULTRA_STREAM_LAST_SIGNAL];
*/

#define ULTRA_STREAM_OBJECT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_STREAM_OBJECT, UltraStreamObjectPrivate))

/*static gboolean
ULTRA_STREAM_IS_CHECK ( )
{
        return security_device_get_expiry_type(TERA_GUARD())==CHECK_EXPIRY;
}*/

G_DEFINE_TYPE(UltraStreamObject, ultra_stream_object, STREAMS_TYPE_OBJECT_SKELETON)

// ---------------------------------------------------STREAM VALUE ------------------------------------

// static PumpsObject *STREAM_INITIAL_PUMP(UltraStreamObject *stream) {
//     switch (stream->priv->stream_number) {
//     case 1:
//         return TERA_PUMP_1();
//     case 2:
//         return TERA_PUMP_2();
//     case 3:
//         return TERA_PUMP_3();
//     case 4:
//         return TERA_PUMP_4();
//     case 5:
//         return TERA_PUMP_5();
//     case 6:
//         return TERA_PUMP_6();
//     default:
//         return TERA_PUMP_1();
//     }
//     return TERA_PUMP_1();
// }

void ultra_check_process_models(ProcessSimple *process, MktStatistic *statistic, MktCategory *cathegory) {
    g_return_if_fail(process != NULL);
    if (cathegory) {
        g_object_bind_property(cathegory, "category-interval", process, "interval", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(cathegory, "remote-control", process, "remote-control", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(cathegory, "category-online", process, "online", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(cathegory, "category-runs", process, "was-runs", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    }
    if (statistic) {
        process_simple_set_replicates(process,mkt_statistic_replicates(statistic));
        process_simple_set_outliers(process,mkt_statistic_outliers(statistic));
        process_simple_set_max_cv(process,mkt_statistic_max_cv(statistic));
		g_object_bind_property(statistic, "statistic-jump", process, "jump", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
		g_object_bind_property(statistic, "statistic-amount-counter", process, "amount-counter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
		g_object_bind_property(statistic, "statistic-amount-percentage", process, "amount-percentage", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    }
}

void ultra_stream_check_models(UltraStreamObject *stream) {
    MktModel *stream_model =
        mkt_model_select_one(MKT_TYPE_STREAM_MODEL, "select * from %s where param_object_path = '%s'", g_type_name(MKT_TYPE_STREAM_MODEL), g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
    if (stream_model) {
        g_object_bind_property(stream_model, "stream-name", streams_object_get_simple(STREAMS_OBJECT(stream)), "name", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_unref(stream_model);
    }

    stream->priv->online_category = mkt_model_select_one(MKT_TYPE_CATEGORY_MODEL, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(MKT_TYPE_CATEGORY_MODEL),
                                                         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "online");
    if (stream->priv->online_category == NULL) {
        // REVIEW: check parameter name
        g_warning("STREAM %s ONLINE CATHEGORY NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *pname                  = g_strdup_printf("(#0%d) online interval", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->online_category = mkt_model_new(MKT_TYPE_CATEGORY_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "online", "category-interval",
                                                      720.0, "category-online", TRUE, "param-name", pname, NULL);
        g_free(pname);
    }
    // g_debug("Online catchegory %d",mkt_cat)
    stream->priv->cal_category = mkt_model_select_one(MKT_TYPE_CATEGORY_MODEL, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(MKT_TYPE_CATEGORY_MODEL),
                                                      g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "calibration");
    if (stream->priv->cal_category == NULL) {
        // REVIEW: check parameter name
        g_warning("STREAM %s CAL CATHEGORY NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *pname               = g_strdup_printf("(#0%d) auto calibration interval", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->cal_category = mkt_model_new(MKT_TYPE_CATEGORY_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "calibration", "category-interval",
                                                   43200.0, "category-online", FALSE, "param-name", pname, NULL);
        g_free(pname);
    }

    stream->priv->check_category = mkt_model_select_one(MKT_TYPE_CATEGORY_MODEL, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(MKT_TYPE_CATEGORY_MODEL),
                                                        g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "check");
    if (stream->priv->check_category == NULL) {
        // REVIEW: check parameter name
        g_warning("STREAM %s CHECK CATHEGORY NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *pname                 = g_strdup_printf("(#0%d) check interval", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->check_category = mkt_model_new(MKT_TYPE_CATEGORY_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "check", "category-interval",
                                                     0.0, "category-online", FALSE, "param-name", pname, NULL);
        g_free(pname);
    }

    stream->priv->online_statistic = mkt_model_select_one(MKT_TYPE_STATISTIC_MODEL, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(MKT_TYPE_STATISTIC_MODEL),
                                                          g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "online");
    if (stream->priv->online_statistic == NULL) {
        // REVIEW: check parameter name
        g_warning("STREAM %s ONLINE STATISTIC NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));

        gchar *pname = g_strdup_printf("(#0%d) online multiple determination", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->online_statistic =
            mkt_model_new(MKT_TYPE_STATISTIC_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "online", "param-name", pname, NULL);
        g_free(pname);
    }

    stream->priv->single_statistic = mkt_model_select_one(MKT_TYPE_STATISTIC_MODEL, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(MKT_TYPE_STATISTIC_MODEL),
                                                          g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "single");
    if (stream->priv->single_statistic == NULL) {
        // REVIEW: check parameter "Stream %d single multiple determination"

        g_warning("STREAM %s SINGLE STATISTIC NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *pname = g_strdup_printf("(#0%d) single multiple determination", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->single_statistic =
            mkt_model_new(MKT_TYPE_STATISTIC_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "single", "param-name", pname, NULL);
        g_free(pname);
    }

    stream->priv->cal_statistic = mkt_model_select_one(MKT_TYPE_STATISTIC_MODEL, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(MKT_TYPE_STATISTIC_MODEL),
                                                       g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "calibration");
    if (stream->priv->cal_statistic == NULL) {
        // REVIEW: check parameter "Stream %d calibration multiple determination"
        g_warning("STREAM %s CAL STATISTIC NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *pname                = g_strdup_printf("(#0%d) calibration multiple determination", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->cal_statistic = mkt_model_new(MKT_TYPE_STATISTIC_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "calibration", "param-name",
                                                    pname, "statistic-replicates", 5, "statistic-outliers", 2, "statistic-max-cv", 2.0, NULL);
        g_free(pname);
    }
}

static GList *dilution_check_streams = NULL;
static void   ultra_stream_change_is_dilution_callback(StreamsUltra *ultra, GParamSpec *pspec, UltraStreamObject *stream) {
    if (streams_ultra_get_is_dilution(ultra)) {
        GList *st = NULL;
        for (st = dilution_check_streams; st != NULL; st = st->next) {
            if (st->data != (gpointer)ultra) streams_ultra_set_is_dilution(STREAMS_ULTRA(st->data), FALSE);
        }
    }
}

static void ultra_stream_init_measparam_data_model(UltraStreamObject *stream) {
    StreamsUltra *ultra = streams_object_get_ultra(STREAMS_OBJECT(stream));
    if (stream->priv->measparam != NULL) g_object_unref(stream->priv->measparam);
    stream->priv->measparam = mkt_model_select_one(ULTIMATE_TYPE_MESSPARM_OBJECT, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(ULTIMATE_TYPE_MESSPARM_OBJECT),
                                                   g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "main");
    if (stream->priv->measparam == NULL) {
        g_warning("STREAM %s MEASUREMENT PARAMETER NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *name = g_strdup_printf("(#0%d) measurement", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));
        stream->priv->measparam =
            mkt_model_new(ULTIMATE_TYPE_MESSPARM_OBJECT, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "main", "param-name", name, NULL);
        g_free(name);
    }

    g_object_bind_property(stream->priv->measparam, "ultimate-sample-volume", ultra, "sample-volume", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-sample-filling-time", ultra, "sample-filling-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-injection-volume", ultra, "injection-volume", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-injection-volume-tic", ultra, "injection-volume-tic", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-delay", ultra, "delay", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-delay-tic", ultra, "delay-tic", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-is-pre-rinsing", ultra, "is-pre-rinsing", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-rinsing-count", ultra, "rinsing-count", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-is-after-rinsing", ultra, "is-after-rinsing", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-after-rinsing-count", ultra, "after-rinsing-count", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-codo-injection", ultra, "codo-injection", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-need-stripping", ultra, "need-stripping", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-stripping-time", ultra, "stripping-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-dilution-type", ultra, "dilution-type", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-dilution-factor", ultra, "dilution-factor", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-dilution-pump-time", ultra, "dilution-pump-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-dilution-wait-time", ultra, "dilution-wait-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-allowed-deviation", ultra, "allowed-deviation", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "ultimate-autocal-deviation", ultra, "autocal-deviation", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property(stream->priv->measparam, "process-rinsing", ultra, "process-rinsing", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "prinsing-y1-pos", ultra, "rinsing-pos-y1", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "prinsing-injection-volume", ultra, "rinsing-volume", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "prinsing-injection-replicate", ultra, "rinsing-replicate", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "prinsing-wait-time", ultra, "rinsing-wait-inj", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "prinsing-y2-pos", ultra, "rinsing-pos-y2", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->measparam, "prinsing-wait-after", ultra, "rinsing-wait-after", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    stream->priv->is_dilution = mkt_paramboolean_get(tera_service_id(), g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "is-dilution");
    if (stream->priv->is_dilution == NULL) {
        stream->priv->is_dilution = MKT_PARAMBOOLEAN(mkt_model_new(MKT_TYPE_PARAMBOOLEAN_MODEL, "param-object-id", tera_service_id(), "param-object-path",
                                                                   g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-name", "is-dilution", "param-activated", TRUE, "value", FALSE, NULL));
    }
    g_object_bind_property(stream->priv->is_dilution, "value", ultra, "is-dilution", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    stream->priv->on_replicate = mkt_paramboolean_get(tera_service_id(), g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "on-replicate");
    if (stream->priv->on_replicate == NULL) {
        stream->priv->on_replicate = MKT_PARAMBOOLEAN(mkt_model_new(MKT_TYPE_PARAMBOOLEAN_MODEL, "param-object-id", tera_service_id(), "param-object-path",
                                                                    g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-name", "on-replicate", "param-activated", TRUE, "value", FALSE, NULL));
    }
    // g_object_bind_property(stream->priv->is_dilution,"value",ultra,"on-replicate",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);

    dilution_check_streams = g_list_append(dilution_check_streams, ultra);
    g_signal_connect(ultra, "notify::is-dilution", G_CALLBACK(ultra_stream_change_is_dilution_callback), stream);
}

static void ultra_stream_init_posparam_data_model(UltraStreamObject *stream) {
    StreamsUltra *positions = streams_object_get_ultra(STREAMS_OBJECT(stream));
    if (stream->priv->posparam != NULL) g_object_unref(stream->priv->posparam);
    stream->priv->posparam = mkt_model_select_one(ULTIMATE_TYPE_POSPARM_OBJECT, "select * from %s where param_object_path = '%s' and param_type = '%s'", g_type_name(ULTIMATE_TYPE_POSPARM_OBJECT),
                                                  g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "main");

    if (stream->priv->posparam == NULL) {
        g_warning("STREAM %s POSITIONS PARAMETER NOT FOUND", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));
        gchar *pname = g_strdup_printf("(#0%d) positions", streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream))));

        stream->priv->posparam = mkt_model_new(ULTIMATE_TYPE_POSPARM_OBJECT, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type", "main", "param-name", pname,
                                               "calibration-vessel", g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_VESSEL1())), NULL);
        g_free(pname);
    }
    g_object_bind_property(stream->priv->posparam, "online-vessel", positions, "online-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->posparam, "single-vessel", positions, "single-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->posparam, "calibration-vessel", positions, "calibration-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->posparam, "check-vessel", positions, "check-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->posparam, "drain-vessel", positions, "drain-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(stream->priv->posparam, "dilution-vessel", positions, "dilution-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
}



static void ultra_stream_change_is_dilution(GObject *object, GParamSpec *pspec, UltraStreamObject *stream) { ultra_control_init_stirrers(); }

static gboolean ultra_stream_object_reload_channels_counter(gpointer data) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(data);
    guint              chM    = stream->priv->measurement_channels != NULL ? g_list_length(stream->priv->measurement_channels) : 0;
    guint              chC    = stream->priv->calibration_channels != NULL ? g_list_length(stream->priv->calibration_channels) : 0;
    guint              chA    = stream->priv->activated_channels != NULL ? g_list_length(stream->priv->activated_channels) : 0;
    guint              chCH   = stream->priv->check_channels != NULL ? g_list_length(stream->priv->check_channels) : 0;

    if (stream->priv->calibration) process_simple_set_channels(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), chC);
    if (stream->priv->single) process_simple_set_channels(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), chA);
    if (stream->priv->online) process_simple_set_channels(process_object_get_simple(PROCESS_OBJECT(stream->priv->online)), chA);
    if (stream->priv->check) process_simple_set_channels(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), chCH);
    streams_simple_set_measurement_channels(streams_object_get_simple(STREAMS_OBJECT(stream)), chM);
    streams_simple_set_calibration_channels(streams_object_get_simple(STREAMS_OBJECT(stream)), chC);
    streams_simple_set_activated_channels(streams_object_get_simple(STREAMS_OBJECT(stream)), chA);
    return FALSE;
}

static void ultra_stream_object_reload_channels(UltraStreamObject *stream) {
    if (stream->priv->measurement_channels) g_list_free(stream->priv->measurement_channels);
    if (stream->priv->calibration_channels) g_list_free(stream->priv->calibration_channels);
    if (stream->priv->activated_channels) g_list_free(stream->priv->activated_channels);
    if (stream->priv->check_channels) g_list_free(stream->priv->check_channels);
    stream->priv->activated_channels   = NULL;
    stream->priv->measurement_channels = NULL;
    stream->priv->calibration_channels = NULL;
    stream->priv->check_channels       = NULL;

    GList *chl = NULL;
    for (chl = stream->priv->channels; chl != NULL; chl = chl->next) {
        ChannelsSimple *simple = NULL;
        simple                 = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
        if (channels_simple_get_is_activate(simple)) {
            stream->priv->activated_channels = g_list_append(stream->priv->activated_channels, chl->data);
            if (channels_simple_get_is_measurement(simple)) {
                stream->priv->measurement_channels = g_list_append(stream->priv->measurement_channels, chl->data);
                if (channels_simple_get_is_calibration(simple)) stream->priv->calibration_channels = g_list_append(stream->priv->calibration_channels, chl->data);
            }
            if (channels_simple_get_is_check(simple)) stream->priv->check_channels = g_list_append(stream->priv->check_channels, chl->data);
        }
    }
    g_timeout_add(20, ultra_stream_object_reload_channels_counter, stream);
}

/*
static void
ultra_stream_object_reload_calibration_waite_activated_channels ( UltraStreamObject *stream )
{
        GList   *chl                  = NULL;
        gboolean waite_activation     = FALSE;
        for(chl=stream->priv->channels;chl!=NULL;chl=chl->next)
        {
                if(channels_simple_get_is_calibration(channels_object_get_simple(CHANNELS_OBJECT(chl->data))))
                {
                        if(channels_calibration_get_calibration_done(channels_object_get_calibration(CHANNELS_OBJECT(chl->data))))
                                waite_activation = TRUE;
                }
        }
        process_simple_set_wait_activation(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)),waite_activation);
}
*/

static void ultra_stream_object_change_activated_property(ChannelsSimple *simple, GParamSpec *pspec, UltraStreamObject *stream) {
    ultra_stream_object_reload_channels(stream);
    // ultra_stream_object_reload_calibration_waite_activated_channels(stream);
}

static void ultra_stream_add_new_channel(UltraStreamObject *stream, ChannelsObject *channel) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    g_return_if_fail(channel != NULL);
    g_return_if_fail(CHANNELS_IS_OBJECT(channel));
    stream->priv->channels = g_list_append(stream->priv->channels, channel);
    g_signal_connect(channels_object_get_simple(channel), "notify::is-activate", G_CALLBACK(ultra_stream_object_change_activated_property), stream);
    g_signal_connect(channels_object_get_simple(channel), "notify::is-check", G_CALLBACK(ultra_stream_object_change_activated_property), stream);
    g_signal_connect(channels_object_get_simple(channel), "notify::is-calibration", G_CALLBACK(ultra_stream_object_change_activated_property), stream);
    ultra_stream_object_reload_channels(stream);
}

static void ultra_stream_channel_init_start_sensor(UltraStreamObject *stream, ChannelsObject *channel) {
    gboolean           automatic_change = FALSE;
    IntegrationObject *integration      = ultimate_channel_get_integration(ULTIMATE_CHANNEL(channel));
    if (integration == NULL) {
        GList *integrations = g_dbus_object_manager_get_objects(ultra_integration_server());
        integrations        = g_list_sort(integrations, tera_measurement_manager_sort_integration_on_number_callback);
        GList *l            = NULL;
        for (l = integrations; l != NULL; l = l->next) {
            if (channels_simple_get_sensor_kind(channels_object_get_simple(CHANNELS_OBJECT(channel))) == integration_simple_get_kind(integration_object_get_simple(INTEGRATION_OBJECT(l->data)))) {
                channels_simple_set_sensor(channels_object_get_simple(CHANNELS_OBJECT(channel)), g_dbus_object_get_object_path(G_DBUS_OBJECT(l->data)));
                automatic_change = TRUE;
                break;
            }
        }
        if (integrations) g_list_free(integrations);
    }
    if (automatic_change) streams_simple_set_auto_changed(streams_object_get_simple(STREAMS_OBJECT(stream)), automatic_change);
}

static void ultra_stream_create_channels(UltraStreamObject *stream) {
    const gchar *paht = g_dbus_object_get_object_path(G_DBUS_OBJECT(stream));
    gchar *      id   = g_strdup_printf("%s/channels", paht);

    guint stream_number  = streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream)));
    guint channel_number = ((streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream)))) * 10);

    // gboolean is_ndir = FALSE;
    // gboolean is_codo = FALSE;
    // gboolean is_tnb = FALSE;

    // GList *integrations = g_dbus_object_manager_get_objects(ultra_integration_server());
    // GList *ri = NULL;
    // for (ri = integrations; ri != NULL; ri = ri->next)
    // {
    //     guint plug = integration_simple_get_kind(integration_object_get_simple(INTEGRATION_OBJECT(ri->data)));
    //     if (plug == ULTIMATE_SENSOR_KIND_NDIR)
    //         is_ndir = TRUE;
    //     else if (plug == ULTIMATE_SENSOR_KIND_CODo)
    //         is_codo = TRUE;
    //     else if (plug == ULTIMATE_SENSOR_KIND_TNb)
    //         is_tnb = TRUE;
    // }
    // if (integrations)
    //     g_list_free(integrations);
    ChannelsSimple *simple         = NULL;
    gchar *         skeleton_patch = NULL;
    skeleton_patch                 = g_strdup_printf(TERA_CHANNELS_MANAGER_OBJECT_FORMAT, stream_number, "TC");
    stream->priv->TC               = CHANNELS_OBJECT(g_object_new(ULTRA_TYPE_CHANNEL_OBJECT, "g-object-path", skeleton_patch, "channel-stream", stream, "channel-number", channel_number + 2, NULL));
    g_dbus_object_manager_server_export(ultimate_channel_get_manager(), G_DBUS_OBJECT_SKELETON(stream->priv->TC));
    simple = channels_object_get_simple(stream->priv->TC);
    channels_simple_set_stream_number(simple, stream_number);
    channels_simple_set_sensor_kind(simple, ULTIMATE_SENSOR_KIND_NDIR);
    channels_simple_set_signatur(simple, "TC");
    g_free(skeleton_patch);
    ultra_stream_channel_init_start_sensor(stream, stream->priv->TC);
    ultra_stream_add_new_channel(stream, stream->priv->TC);

    skeleton_patch    = g_strdup_printf(TERA_CHANNELS_MANAGER_OBJECT_FORMAT, stream_number, "TIC");
    stream->priv->TIC = CHANNELS_OBJECT(g_object_new(ULTRA_TYPE_CHANNEL_OBJECT, "g-object-path", skeleton_patch, "channel-stream", stream, "channel-number", channel_number + 3, NULL));
    g_dbus_object_manager_server_export(ultimate_channel_get_manager(), G_DBUS_OBJECT_SKELETON(stream->priv->TIC));
    simple = channels_object_get_simple(stream->priv->TIC);
    channels_simple_set_stream_number(simple, stream_number);
    channels_simple_set_sensor_kind(channels_object_get_simple(stream->priv->TIC), ULTIMATE_SENSOR_KIND_NDIR);
    channels_simple_set_tic(channels_object_get_simple(stream->priv->TIC), TRUE);
    channels_simple_set_signatur(channels_object_get_simple(stream->priv->TIC), "TIC");
    g_free(skeleton_patch);
    ultra_stream_channel_init_start_sensor(stream, stream->priv->TIC);
    ultra_stream_add_new_channel(stream, stream->priv->TIC);

    skeleton_patch     = g_strdup_printf(TERA_CHANNELS_MANAGER_OBJECT_FORMAT, stream_number, "TOCdiff");
    stream->priv->Diff = CHANNELS_OBJECT(g_object_new(ULTRA_TYPE_CHANNEL_DIFF, "g-object-path", skeleton_patch, "channel-stream", stream, "channel-number", channel_number + 1, "channel-tc",
                                                      stream->priv->TC, "channel-tic", stream->priv->TIC, NULL));
    g_dbus_object_manager_server_export(ultimate_channel_get_manager(), G_DBUS_OBJECT_SKELETON(stream->priv->Diff));
    simple = channels_object_get_simple(stream->priv->Diff);
    channels_simple_set_stream_number(simple, stream_number);
    channels_simple_set_signatur(channels_object_get_simple(stream->priv->Diff), "TOCdiff");
    ultra_stream_add_new_channel(stream, stream->priv->Diff);
    // channels_simple_set_sensor_kind(channels_object_get_simple(stream->priv->Diff),ULTIMATE_SENSOR_KIND_NDIR);
    g_free(skeleton_patch);
    skeleton_patch     = g_strdup_printf(TERA_CHANNELS_MANAGER_OBJECT_FORMAT, stream_number, "CODo");
    stream->priv->CODo = CHANNELS_OBJECT(g_object_new(ULTRA_TYPE_CHANNEL_OBJECT, "g-object-path", skeleton_patch, "channel-stream", stream, "channel-number", channel_number + 4, NULL));
    g_dbus_object_manager_server_export(ultimate_channel_get_manager(), G_DBUS_OBJECT_SKELETON(stream->priv->CODo));
    simple = channels_object_get_simple(stream->priv->CODo);
    channels_simple_set_stream_number(simple, stream_number);
    channels_simple_set_sensor_kind(channels_object_get_simple(stream->priv->CODo), ULTIMATE_SENSOR_KIND_CODo);
    channels_simple_set_signatur(channels_object_get_simple(stream->priv->CODo), "CODo");
    g_free(skeleton_patch);
    ultra_stream_channel_init_start_sensor(stream, stream->priv->CODo);
    ultra_stream_add_new_channel(stream, stream->priv->CODo);
    skeleton_patch    = g_strdup_printf(TERA_CHANNELS_MANAGER_OBJECT_FORMAT, stream_number, "TNb");
    stream->priv->TNb = CHANNELS_OBJECT(g_object_new(ULTRA_TYPE_CHANNEL_OBJECT, "g-object-path", skeleton_patch, "channel-stream", stream, "channel-number", channel_number + 5, NULL));
    g_dbus_object_manager_server_export(ultimate_channel_get_manager(), G_DBUS_OBJECT_SKELETON(stream->priv->TNb));
    simple = channels_object_get_simple(stream->priv->TNb);
    channels_simple_set_stream_number(simple, stream_number);
    channels_simple_set_sensor_kind(channels_object_get_simple(stream->priv->TNb), ULTIMATE_SENSOR_KIND_TNb);
    channels_simple_set_signatur(channels_object_get_simple(stream->priv->TNb), "TNb");
    g_free(skeleton_patch);
    ultra_stream_channel_init_start_sensor(stream, stream->priv->TNb);
    ultra_stream_add_new_channel(stream, stream->priv->TNb);
    g_free(id);
}

static void ultra_stream_create_measurement_process(UltraStreamObject *stream) {
    const gchar *paht          = g_dbus_object_get_object_path(G_DBUS_OBJECT(stream));
    gchar *      id            = g_strdup_printf("%s/process", paht);
    guint        stream_number = streams_simple_get_number(streams_object_get_simple(STREAMS_OBJECT(stream)));

    gchar *    skeleton_patch = NULL;
    MktStatus *status         = NULL;
    gchar *    stream_status  = NULL;
    skeleton_patch            = g_strdup_printf("%s/%d_%s", TERA_PROCESS_MANAGER, stream_number, TERA_PROCESS_SINGLE);
    StreamsSimple *simple     = streams_object_get_simple(STREAMS_OBJECT(stream));

    stream->priv->single = MKT_PROCESS_OBJECT(g_object_new(MEASUREMENT_TYPE_PROCESS, "g-object-path", skeleton_patch, "default-category", stream->priv->online_category, "default-statistic",
                                                           stream->priv->single_statistic, "process-identification", 100 + stream_number, "process-stream", stream, NULL));

    process_simple_set_number(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), STATE_SINGLE);
    process_simple_set_name(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), _("Einzelmessung"));
    process_simple_set_can_start(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), TRUE);
    process_simple_set_kind_type(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), "S");
    process_simple_set_description(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), "single measurement");
    process_simple_set_stream_id(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), streams_simple_get_id(simple));
    process_simple_set_is_measurement(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), TRUE);
    g_free(skeleton_patch);
    control_add_process(stream->priv->single);
    stream_status = g_strdup_printf("S%d", stream_number);
    status        = mkt_status_new(stream_status, _("Single measurement"));
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), "run", status, "status-active", G_BINDING_DEFAULT);
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->single)), "run", streams_object_get_simple(STREAMS_OBJECT(stream)), "analyse", G_BINDING_DEFAULT);

    g_free(stream_status);

    skeleton_patch         = g_strdup_printf("%s/%d_%s", TERA_PROCESS_MANAGER, stream_number, TERA_PROCESS_ONLINE);
    stream->priv->online   = MKT_PROCESS_OBJECT(g_object_new(MEASUREMENT_TYPE_PROCESS, "g-object-path", skeleton_patch, "default-category", stream->priv->online_category, "default-statistic",
                                                           stream->priv->online_statistic, "process-identification", ULTRA_PROCESS_ONLINE + stream_number, "process-stream", stream, NULL));
    ProcessSimple *psimple = process_object_get_simple(PROCESS_OBJECT(stream->priv->online));
    process_simple_set_number(psimple, STATE_ONLINE);
    process_simple_set_name(psimple, _("Messung"));
    if (!process_simple_get_online(psimple)) process_simple_set_online(psimple, TRUE);
    // g_debug("Process is online %d",process_simple_get_online(psimple));
    process_simple_set_online_process(psimple, TRUE);
    process_simple_set_can_start(psimple, TRUE);
    process_simple_set_kind_type(psimple, "M");
    process_simple_set_description(psimple, "online measurement");
    process_simple_set_stream_id(psimple, streams_simple_get_id(simple));
    process_simple_set_is_measurement(psimple, TRUE);
    g_object_unref(psimple);
    g_free(skeleton_patch);
    control_add_process(stream->priv->online);
    switch (streams_simple_get_number(simple)) {
        case 1:
            control_add_range1(stream->priv->online);
            break;
        case 2:
            control_add_range2(stream->priv->online);
            break;
    }

    stream_status = g_strdup_printf("M%d", stream_number);
    status        = mkt_status_new(stream_status, _("Online measurement"));
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->online)), "run", status, "status-active", G_BINDING_DEFAULT);
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->online)), "run", streams_object_get_simple(STREAMS_OBJECT(stream)), "analyse", G_BINDING_DEFAULT);

    g_free(stream_status);

    skeleton_patch            = g_strdup_printf("%s/%d_%s", TERA_PROCESS_MANAGER, stream_number, TERA_PROCESS_CALIBRATION);
    stream->priv->calibration = MKT_PROCESS_OBJECT(g_object_new(CALIBRATION_TYPE_PROCESS, "g-object-path", skeleton_patch, "default-category", stream->priv->cal_category, "default-statistic",
                                                                stream->priv->cal_statistic, "process-identification", 300 + stream_number, "process-stream", stream, NULL));
    if (stream->priv->TC != NULL) mkt_process_object_add_channel(stream->priv->calibration, CHANNELS_OBJECT(stream->priv->TC));
    if (stream->priv->TIC != NULL) mkt_process_object_add_channel(stream->priv->calibration, CHANNELS_OBJECT(stream->priv->TIC));
    if (stream->priv->CODo != NULL) mkt_process_object_add_channel(stream->priv->calibration, CHANNELS_OBJECT(stream->priv->CODo));
    if (stream->priv->TNb != NULL) mkt_process_object_add_channel(stream->priv->calibration, CHANNELS_OBJECT(stream->priv->TNb));

    process_simple_set_number(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), STATE_CALIBRATION);
    process_simple_set_name(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), _("Kalibrierung"));
    process_simple_set_online_process(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), TRUE);
    process_simple_set_can_start(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), TRUE);
    process_simple_set_kind_type(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), "C");
    process_simple_set_description(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), "calibration");
    process_simple_set_stream_id(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), streams_simple_get_id(simple));
    process_simple_set_is_measurement(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), TRUE);
    g_free(skeleton_patch);
    control_add_process(stream->priv->calibration);
    stream_status = g_strdup_printf("C%d", stream_number);
    status        = mkt_status_new(stream_status, _("Calibration"));
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), "run", status, "status-active", G_BINDING_DEFAULT);
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->calibration)), "run", streams_object_get_simple(STREAMS_OBJECT(stream)), "analyse", G_BINDING_DEFAULT);

    g_free(stream_status);

    skeleton_patch      = g_strdup_printf("%s/%d_%s", TERA_PROCESS_MANAGER, stream_number, TERA_PROCESS_CHECK);
    stream->priv->check = MKT_PROCESS_OBJECT(g_object_new(MEASUREMENT_TYPE_PROCESS, "g-object-path", skeleton_patch, "default-category", stream->priv->check_category, "default-statistic",
                                                          stream->priv->online_statistic, "process-identification", ULTRA_PROCESS_CHECK + stream_number, "process-stream", stream, NULL));

    process_simple_set_number(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), STATE_CHECK);
    process_simple_set_name(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), _("Check-Function"));
    process_simple_set_online_process(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), TRUE);
    process_simple_set_check_process(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), TRUE);
    process_simple_set_can_start(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), TRUE);
    process_simple_set_kind_type(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), "V");
    process_simple_set_description(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), "check measurement");
    process_simple_set_stream_id(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), streams_simple_get_id(simple));
    process_simple_set_is_measurement(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), TRUE);

    g_free(skeleton_patch);
    control_add_process(stream->priv->check);
    stream_status = g_strdup_printf("V%d", stream_number);
    status        = mkt_status_new(stream_status, _("Check"));
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), "run", status, "status-active", G_BINDING_DEFAULT);
    g_object_bind_property(process_object_get_simple(PROCESS_OBJECT(stream->priv->check)), "run", streams_object_get_simple(STREAMS_OBJECT(stream)), "analyse", G_BINDING_DEFAULT);

    g_free(stream_status);
    g_free(id);
}

static gboolean check_remote_control_callback(gpointer data);

static void remote_in_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(user_data);
    gboolean           result = FALSE;
    GError *           error  = NULL;
    if (nodes_digital16_call_get_digital_in_finish(stream->priv->remote_node, &result, res, &error)) {
        ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(stream->priv->online));
        if (process_simple_get_is_online(simple) && mkt_process_object_is_activate(MKT_PROCESS_OBJECT(stream->priv->online))) {
            if (!process_simple_get_remote_signal(simple))
                process_simple_set_remote_signal(process_object_get_simple(PROCESS_OBJECT(stream->priv->online)), result);
        } else {
            process_simple_set_remote_signal(process_object_get_simple(PROCESS_OBJECT(stream->priv->online)), result);
        }
    }
    g_timeout_add_seconds(1, check_remote_control_callback, stream);
}
gboolean check_remote_control_callback(gpointer data) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(data);
    nodes_digital16_call_get_digital_in(stream->priv->remote_node, stream->priv->remote_in, NULL, remote_in_async_callback, stream);
    return FALSE;
}
static void ultra_stream_remote_control_check(UltraStreamObject *stream) { g_timeout_add_seconds(2, check_remote_control_callback, stream); }

static void init_remote_node(UltraStreamObject *stream) {
    NodesObject *node = NULL;
    switch (stream->priv->stream_number) {
        case 1:
            node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
            if (node == NULL) {
                mkt_log_error_message_sync("Stream1 node Digital1:/com/lar/nodes/Digital1 not found");
                // g_error("Node Digital1:/com/lar/nodes/Digital1 not found");
            } else {
                if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 24) {
                    mkt_log_error_message("Stream1 /com/lar/nodes/Digital1 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                }
                if (stream->priv->remote_in == 0) stream->priv->remote_in = 9;
                stream->priv->remote_node = nodes_object_get_digital16(node);
            }
            break;
        case 2:
            node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
            if (node == NULL) {
                mkt_log_error_message_sync("Stream2 node Digital1:/com/lar/nodes/Digital1 not found");
                // g_error("Node Digital1:/com/lar/nodes/Digital1 not found");
            } else {
                if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 24) {
                    mkt_log_error_message("Stream2 /com/lar/nodes/Digital1 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                }
                if (stream->priv->remote_in == 0) stream->priv->remote_in = 10;
                stream->priv->remote_node = nodes_object_get_digital16(node);
            }
            break;
        case 3:
            node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
            if (node == NULL) {
                mkt_log_error_message_sync("Node Digital1:/com/lar/nodes/Digital1 not found");
                // g_error("Node Digital1:/com/lar/nodes/Digital1 not found");
            } else {
                if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 24) {
                    mkt_log_error_message("Stream3 /com/lar/nodes/Digital1 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                    // g_error("Stream3 /com/lar/nodes/Digital1 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                }
                if (stream->priv->remote_in == 0) stream->priv->remote_in = 12;
                stream->priv->remote_node = nodes_object_get_digital16(node);
            }
            break;
        case 4:
            node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
            if (node == NULL) {
                mkt_log_error_message_sync("Stream4 node Digital2:/com/lar/nodes/Digital2 not found");
                // g_error("Stream4 node Digital2:/com/lar/nodes/Digital2 not found");
            } else {
                if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 25) {
                    mkt_log_error_message("Stream4 /com/lar/nodes/Digital2 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                    // g_error("Stream4 /com/lar/nodes/Digital2 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                }
                stream->priv->remote_in   = 4;
                stream->priv->remote_node = nodes_object_get_digital16(node);
            }
            break;
        case 5:
            node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
            if (node == NULL) {
                mkt_log_error_message_sync("Stream5 node Digital2:/com/lar/nodes/Digital2 not found");
                // g_error("Stream5 node Digital2:/com/lar/nodes/Digital2 not found");
            } else {
                if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 25) {
                    mkt_log_error_message("Stream5 /com/lar/nodes/Digital2 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                    // g_error("Stream5 /com/lar/nodes/Digital2 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                }
                stream->priv->remote_in   = 5;
                stream->priv->remote_node = nodes_object_get_digital16(node);
            }
            break;
        case 6:
            node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
            if (node == NULL) {
                mkt_log_error_message_sync("Stream6 node Digital2:/com/lar/nodes/Digital2 not found");
                // g_error("Stream6 node Digital2:/com/lar/nodes/Digital2 not found");
            } else {
                if (nodes_simple_get_node_id(nodes_object_get_simple(node)) != 25) {
                    mkt_log_error_message("Stream6 /com/lar/nodes/Digital2 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                    // g_error("Stream6 /com/lar/nodes/Digital2 hat falsche ID %x (Empfohlen worden ist %x)", nodes_simple_get_node_id(nodes_object_get_simple(node)), 24);
                }
                stream->priv->remote_in   = 6;
                stream->priv->remote_node = nodes_object_get_digital16(node);
            }
            break;
        default:
            break;
    }
}

static void ultra_stream_object_constructed(GObject *object) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(object);

    // g_timeout_add(300,test_idle_notify_test_change_intercal,stream);
    StreamsSimple *simple = streams_simple_skeleton_new();
    streams_object_skeleton_set_simple(STREAMS_OBJECT_SKELETON(object), simple);
    streams_simple_set_number(simple, stream->priv->stream_number);
    streams_simple_set_id(simple, stream->priv->stream_number);
    gchar *name = g_strdup_printf("S%d", streams_simple_get_number(simple));
    streams_simple_set_name(simple, name);
    streams_simple_set_process(streams_object_get_simple(STREAMS_OBJECT(stream)), _("Pause"));
    streams_simple_set_status(streams_object_get_simple(STREAMS_OBJECT(stream)), _("Stop"));
    ultra_stream_check_models(stream);
    // g_signal_connect (simple,"handle::start",G_CALLBACK(ultra_stream_object_start_callback),stream);
    // g_signal_connect (simple,"handle::stop",G_CALLBACK(ultra_stream_object_stop_callback),stream);
    g_object_unref(simple);

    StreamsUltra *ultra = streams_ultra_skeleton_new();
    streams_object_skeleton_set_ultra(STREAMS_OBJECT_SKELETON(object), ultra);

    g_signal_connect(ultra, "notify::is-dilution", G_CALLBACK(ultra_stream_change_is_dilution), stream);

    ultra_stream_init_measparam_data_model(stream);

    g_object_unref(ultra);
    ultra_stream_create_channels(stream);
    ultra_stream_create_measurement_process(stream);
    mkt_error_createv(1840 + stream->priv->stream_number, MKT_ERROR_WARNING, _("No sample / no injection stream %d"), stream->priv->stream_number);
    mkt_error_gone(1840 + stream->priv->stream_number);

    ultra_stream_init_posparam_data_model(stream);
    init_remote_node(stream);
    ultra_stream_remote_control_check(stream);
    G_OBJECT_CLASS(ultra_stream_object_parent_class)->constructed(object);
    // UltraStreamObject *stream = ULTRA_STREAM_OBJECT(object);
}

static void ultra_stream_object_init(UltraStreamObject *ultra_stream_object) {
    UltraStreamObjectPrivate *priv = ULTRA_STREAM_OBJECT_PRIVATE(ultra_stream_object);
    priv->measurement_channels     = NULL;
    priv->calibration_channels     = NULL;
    priv->check_channels           = NULL;
    priv->channels                 = NULL;
    priv->sample_pump              = NULL;
    priv->online                   = 0;
    priv->range                    = 0;
    ultra_stream_object->priv      = priv;
    // Settings property connection ...
    /* TODO: Add initialization code here */
}

static void ultra_stream_object_finalize(GObject *object) {
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(object);
    if (stream->priv->cal_category) g_object_unref(stream->priv->cal_category);
    if (stream->priv->cal_statistic) g_object_unref(stream->priv->cal_statistic);
    if (stream->priv->check_category) g_object_unref(stream->priv->check_category);
    if (stream->priv->measparam) g_object_unref(stream->priv->measparam);
    if (stream->priv->online_category) g_object_unref(stream->priv->online_category);
    if (stream->priv->posparam) g_object_unref(stream->priv->posparam);
    if (stream->priv->single_statistic) g_object_unref(stream->priv->single_statistic);
    if (stream->priv->measurement_channels) g_list_free(stream->priv->measurement_channels);
    if (stream->priv->calibration_channels) g_list_free(stream->priv->calibration_channels);
    if (stream->priv->check_channels) g_list_free(stream->priv->check_channels);

    G_OBJECT_CLASS(ultra_stream_object_parent_class)->finalize(object);
}

static void ultra_stream_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(object));
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(object);
    switch (prop_id) {
        case PROP_STREAM_NUMBER:
            stream->priv->stream_number = g_value_get_uint(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_stream_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(object));
    // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.UltraStreamInterface",value,pspec)) return;
    UltraStreamObject *stream = ULTRA_STREAM_OBJECT(object);
    switch (prop_id) {
        case PROP_STREAM_NUMBER:
            g_value_set_uint(value, stream->priv->stream_number);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_stream_object_class_init(UltraStreamObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraStreamObjectPrivate));

    object_class->finalize     = ultra_stream_object_finalize;
    object_class->set_property = ultra_stream_object_set_property;
    object_class->get_property = ultra_stream_object_get_property;
    object_class->constructed  = ultra_stream_object_constructed;

    g_object_class_install_property(
        object_class, PROP_STREAM_NUMBER,
        g_param_spec_uint("stream-number", "Stream definitions number", "Stream definitions number", 0, G_MAXUINT32, 0, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void ultra_stream_server_create_stream(const gchar *path, guint stream_number) {
    UltraStreamObject *object = ULTRA_STREAM_OBJECT(g_object_new(ULTRA_TYPE_STREAM_OBJECT, "g-object-path", path, "stream-number", stream_number, NULL));
    g_dbus_object_manager_server_export(ULTRA_STREAM_SERVER, G_DBUS_OBJECT_SKELETON(object));
}

void ultra_stream_server_run(GDBusConnection *connection) {
    if (ULTRA_STREAM_SERVER != NULL) return;
    ULTRA_STREAM_SERVER = g_dbus_object_manager_server_new(TERA_STREAMS_MANAGER);
    g_dbus_object_manager_server_set_connection(ULTRA_STREAM_SERVER, connection);
    LarpcDevice *device = mkt_pc_manager_client_get_device();
    guint streams_lizence = 1;
    larpc_device_call_check_stream_license_sync(device,&streams_lizence,NULL,NULL);
    ultimate_channel_create_manager(connection);
    ultra_stream_server_create_stream(TERA_STREAMS_MANAGER_STREAM1_PATH, 1);
    guint stream_n        = 0;
    for (stream_n = 2; stream_n <= streams_lizence; stream_n++) {
        gchar *skeleton_patch = g_strdup_printf(TERA_STREAMS_MANAGER_OBJECT_FORMAT, stream_n);
        ultra_stream_server_create_stream(skeleton_patch, stream_n);
        g_free(skeleton_patch);
    }
}

GDBusObjectManager *ultra_stream_server_object_manager() {
    g_return_val_if_fail(ULTRA_STREAM_SERVER != NULL, NULL);
    return G_DBUS_OBJECT_MANAGER(ULTRA_STREAM_SERVER);
}

void ultra_stream_change_status(UltraStreamObject *stream, const gchar *format, ...) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    StreamsSimple *simple         = streams_object_get_simple(STREAMS_OBJECT(stream));
    gchar *        process_status = g_strdup_printf("%s - %s", streams_simple_get_process(simple), new_status);
    streams_simple_set_status(simple, process_status);
    g_free(new_status);
    g_free(process_status);
}

MktProcessObject *ultra_stream_calibration_process(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return stream->priv->calibration;
}

GList *ultra_stream_channels(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return stream->priv->channels;
}

void ultra_stream_set_state(UltraStreamObject *stream, guint state) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    stream->priv->state = state;
}
void ultra_stream_set_default_pump(UltraStreamObject *stream) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    switch (stream->priv->stream_number) {
        case 1:
            stream->priv->sample_pump = TERA_PUMP_1();
            break;
        case 2:
            stream->priv->sample_pump = TERA_PUMP_2();
            break;
        case 3:
            stream->priv->sample_pump = TERA_PUMP_3();
            break;
        case 4:
            stream->priv->sample_pump = TERA_PUMP_4();
            break;
        case 5:
            stream->priv->sample_pump = TERA_PUMP_5();
            break;
        case 6:
            stream->priv->sample_pump = TERA_PUMP_6();
            break;
        default:
            stream->priv->sample_pump = TERA_PUMP_1();
            break;
    }
}
void ultra_stream_no_sampling(UltraStreamObject *stream) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    stream->priv->no_sampling = TRUE;
}
void ultra_stream_on_sampling(UltraStreamObject *stream) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    stream->priv->no_sampling = FALSE;
}

gboolean ultra_stream_get_sampling(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, TRUE);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), TRUE);
    return !stream->priv->no_sampling;
}

void ultra_stream_set_pump(UltraStreamObject *stream, PumpsObject *pump) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    if (pump != NULL) stream->priv->sample_pump = pump;
}

PumpsObject *ultra_stream_get_pump(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    if (stream->priv->sample_pump == NULL) {
        ultra_stream_set_default_pump(stream);
    }
    return stream->priv->sample_pump;
}

VesselsObject *ultra_stream_get_drain(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return ultra_vessels_manager_client_get_vessel(streams_ultra_get_drain_vessel(streams_object_get_ultra(STREAMS_OBJECT(stream))));
}

VesselsObject *ultra_stream_get_sample(UltraStreamObject *stream) {
    VesselsObject *vessel = NULL;
    switch (stream->priv->state) {
        case STATE_ONLINE:
            vessel = ultra_vessels_manager_client_get_vessel(streams_ultra_get_online_vessel(streams_object_get_ultra(STREAMS_OBJECT(stream))));
            break;
        case STATE_SINGLE:
            vessel = ultra_vessels_manager_client_get_vessel(streams_ultra_get_single_vessel(streams_object_get_ultra(STREAMS_OBJECT(stream))));
            break;
        case STATE_CALIBRATION:
            vessel = ultra_vessels_manager_client_get_vessel(streams_ultra_get_calibration_vessel(streams_object_get_ultra(STREAMS_OBJECT(stream))));
            break;
        case STATE_CHECK:
            vessel = ultra_vessels_manager_client_get_vessel(streams_ultra_get_check_vessel(streams_object_get_ultra(STREAMS_OBJECT(stream))));
            break;
    }
    return vessel;
}

GList *ultra_stream_process_channels(UltraStreamObject *stream) {
    GList *channels = NULL;
    switch (stream->priv->state) {
        case STATE_ONLINE:
            if (stream->priv->measurement_channels && g_list_length(stream->priv->measurement_channels) > 0) channels = g_list_copy(stream->priv->measurement_channels);
            break;
        case STATE_SINGLE:
            if (stream->priv->measurement_channels && g_list_length(stream->priv->measurement_channels) > 0) channels = g_list_copy(stream->priv->measurement_channels);
            break;
        case STATE_CALIBRATION:
            if (stream->priv->calibration_channels && g_list_length(stream->priv->calibration_channels) > 0) channels = g_list_copy(stream->priv->calibration_channels);
            break;
        case STATE_CHECK:
            if (stream->priv->check_channels && g_list_length(stream->priv->check_channels) > 0) channels = g_list_copy(stream->priv->check_channels);
            break;
        default:
            break;
    }
    return channels;
}

ChannelsObject *UltraDBusStreamChannelTC(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return CHANNELS_OBJECT(stream->priv->TC);
}
ChannelsObject *UltraDBusStreamChannelTIC(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return CHANNELS_OBJECT(stream->priv->TIC);
}
ChannelsObject *UltraDBusStreamChannelTOC(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return CHANNELS_OBJECT(stream->priv->Diff);
}
ChannelsObject *UltraDBusStreamChannelTNb(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return CHANNELS_OBJECT(stream->priv->TNb);
}
ChannelsObject *UltraDBusStreamChannelCODo(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return CHANNELS_OBJECT(stream->priv->CODo);
}

ProcessObject *UltraDBusStreamGetOnlineProcess(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return PROCESS_OBJECT(stream->priv->online);
}
ProcessObject *UltraDBusStreamGetSingleProcess(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return PROCESS_OBJECT(stream->priv->single);
}
ProcessObject *UltraDBusStreamGetCalibrationProcess(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return PROCESS_OBJECT(stream->priv->calibration);
}
ProcessObject *UltraDBusStreamGetCheckProcess(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), NULL);
    return PROCESS_OBJECT(stream->priv->check);
}

guint ultra_stream_object_get_number(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, 0);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), 0);
    return stream->priv->stream_number;
}
void ultra_stream_object_online(UltraStreamObject *stream) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    if (stream->priv->is_online) return;
    stream->priv->is_online = TRUE;
}
void ultra_stream_object_offline(UltraStreamObject *stream) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    if (!stream->priv->is_online) return;
    stream->priv->is_online = FALSE;
}

void ultra_stream_object_set_remote_address(UltraStreamObject *stream, guint din) {
    g_return_if_fail(stream != NULL);
    g_return_if_fail(ULTRA_IS_STREAM_OBJECT(stream));
    if (stream->priv->stream_number <= 3) stream->priv->remote_in = din;
}

guint ultra_stream_object_get_remote_address(UltraStreamObject *stream) {
    g_return_val_if_fail(stream != NULL, 0);
    g_return_val_if_fail(ULTRA_IS_STREAM_OBJECT(stream), 0);
    return stream->priv->remote_in;
}