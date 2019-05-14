/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
   mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "sequence-workers-application-object.h"
#include "ultra-dilution-process.h"

enum {
    PROP_0,
};

struct _UltraDilutionProcessPrivate {
    VesselsObject * sampling_vessel;
    VesselsObject * rinsing_vessel;
    gdouble         fill_time;
    gdouble         dilution_time;
    gdouble         proportion;
    gdouble         dilution_vessel_vol;
    gboolean        is_dilution_done;
    gboolean        is_rinsing_done;
    guint           repeat;
    guint           dilution_repeat;
    GTimer *        sample_dilution;
    GArray *        fluid_values;
    NodesObject *   node;
    NodesDigital16 *digital16;
};

#define ULTRA_DILUTION_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_DILUTION_PROCESS, UltraDilutionProcessPrivate))

G_DEFINE_TYPE(UltraDilutionProcess, ultra_dilution_process, ULTRA_TYPE_SEQUENCE_PROCESS)

static void set_default_stirrer(UltraDilutionProcess *dilution_process) { stirrers_simple_call_run_mod1(stirrers_object_get_simple(ULTRA_STIRRER_1()), NULL, NULL, NULL); }
static void set_dilution_stirrer(UltraDilutionProcess *dilution_process) { stirrers_simple_call_run_mod2(stirrers_object_get_simple(ULTRA_STIRRER_1()), NULL, NULL, NULL); }

static VesselsObject *DILUTION_SAMPLE_VESSEL(UltraDilutionProcess *dilution_process) {
    if (dilution_process->priv->sampling_vessel) g_object_unref(dilution_process->priv->sampling_vessel);
    const gchar *vessels_path = sequence_workers_sample_get_sample_main(sequence_object_get_workers_sample(SEQUENCE_OBJECT(dilution_process)));

    if (vessels_path != NULL && g_variant_is_object_path(vessels_path)) {
        VesselsObject *object = VESSELS_OBJECT(g_dbus_object_manager_get_object(ultra_vessels_manager_client_get_manager(), vessels_path));
        if (object != NULL)
            dilution_process->priv->sampling_vessel = object;
        else
            dilution_process->priv->sampling_vessel = g_object_ref(ULTRA_VESSEL3());
    } else
        dilution_process->priv->sampling_vessel = g_object_ref(ULTRA_VESSEL3());
    return dilution_process->priv->sampling_vessel;
}

static VesselsObject *DILUTION_RINSE_VESSEL(UltraDilutionProcess *dilution_process) {
    if (dilution_process->priv->rinsing_vessel) g_object_unref(dilution_process->priv->rinsing_vessel);
    const gchar *vessels_path = sequence_workers_sample_get_sample_second(sequence_object_get_workers_sample(SEQUENCE_OBJECT(dilution_process)));

    if (vessels_path != NULL && g_variant_is_object_path(vessels_path)) {
        VesselsObject *object = VESSELS_OBJECT(g_dbus_object_manager_get_object(ultra_vessels_manager_client_get_manager(), vessels_path));
        if (object != NULL)
            dilution_process->priv->rinsing_vessel = object;
        else
            dilution_process->priv->rinsing_vessel = g_object_ref(ULTRA_VESSEL3());
    } else
        dilution_process->priv->rinsing_vessel = g_object_ref(ULTRA_VESSEL3());
    return dilution_process->priv->rinsing_vessel;
}

///

static void WAITE_dilution_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    GError *error = NULL;
    if (!lar_timer_default_finish(res, &error)) {
        g_task_return_error(task, error);
    } else {
        UltraDilutionProcess *dilution_process   = ULTRA_DILUTION_PROCESS(g_task_get_source_object(task));
        dilution_process->priv->is_dilution_done = TRUE;
        set_default_stirrer(dilution_process);
        if (dilution_process->priv->is_rinsing_done) {
            sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(dilution_process)), _("Process done"));
            g_task_return_boolean(task, TRUE);
        }
    }
    g_object_unref(task);
}

static void rinsing_ptp_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        // TODO:Add movement error
        return;
    }
    UltraDilutionProcess *dilution_process  = ULTRA_DILUTION_PROCESS(g_task_get_source_object(task));
    dilution_process->priv->is_rinsing_done = TRUE;
    if (dilution_process->priv->is_dilution_done) {
        g_task_return_boolean(task, TRUE);
    }
}

static void rinsing_after_dilution(GTask *task) {
    UltraDilutionProcess *dilution_process = ULTRA_DILUTION_PROCESS(g_task_get_source_object(task));
    GString *             commands         = g_string_new("");
    guint                 vessels_pos      = vessels_simple_get_pos_xachse(vessels_object_get_simple(DILUTION_RINSE_VESSEL(dilution_process)));
    guint                 injection_pos    = vessels_simple_get_injection_pos(vessels_object_get_simple(DILUTION_RINSE_VESSEL(dilution_process)));
    g_string_append_printf(commands, "MoveX(%d,1,1);MoveY(%d,1,1);V3C();HoldInj(1,1);SensorInj();V3O();HoldInj(1,1);V3C();", vessels_pos, injection_pos);
    int              repeat    = 0;
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rinsing(injection)) * 2.5;
    if (position > achsen_achse_get_max(achse)) position = achsen_achse_get_max(achse) - 15;
    for (; repeat < sequence_workers_sample_get_repeat(sequence_object_get_workers_sample(SEQUENCE_OBJECT(dilution_process))); repeat++) {
        g_string_append_printf(commands, "V3O();MoveInj(%d,2,0);V3C();HoldInj(1,1);", position);
    }
    g_string_append_printf(commands, "HoldY(1,1);SensorY();HoldY(1,1);");

    set_dilution_stirrer(dilution_process);
    dilution_process->priv->is_dilution_done = FALSE;
    dilution_process->priv->is_rinsing_done  = FALSE;

    lar_timer_default_run(g_task_get_cancellable(task), WAITE_dilution_callback, dilution_process->priv->dilution_time, g_object_ref(task));
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 60000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), rinsing_ptp_done_async_callback, task);
    g_string_free(commands, TRUE);
}

static void dilution_ptp_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        // TODO:Add movement error
        return;
    }
    rinsing_after_dilution(task);
}

static void start_dilution_process(GTask *task) {
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    UltraDilutionProcess *dilution_process = ULTRA_DILUTION_PROCESS(g_task_get_source_object(task));
    sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(dilution_process)), _("Y - go to hold position"));
    GString *commands = g_string_new("");
    if (achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()))) != achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))) {
        g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
    }
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()));
    guint            air       = (guint)(achsen_achse_get_hold(achse) + (((gdouble)achsen_injection_get_air(injection)) * 2.5));

    guint vessels_pos   = vessels_simple_get_pos_xachse(vessels_object_get_simple(DILUTION_SAMPLE_VESSEL(dilution_process)));
    guint injection_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(DILUTION_SAMPLE_VESSEL(dilution_process)));
    g_string_append_printf(commands, "MoveX(%d,1,0);MoveInj(%d,2,0);MoveY(%d,1,1);", vessels_pos, air, injection_pos);
    guint max = achsen_achse_get_max(achse);

    gdouble proportion = dilution_process->priv->dilution_vessel_vol / dilution_process->priv->proportion;
    guint   volume     = (guint)achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + (guint)proportion) * 2.5;

    if (volume > max) {
        mkt_errors_report(E1740, _("dilution maximal volum exedded - volume %.0f proportion 1:%.0f"), dilution_process->priv->dilution_vessel_vol, dilution_process->priv->proportion);
        volume = max - 10;
    }
    g_string_append_printf(commands, "MoveInj(%d,2,0);", volume);
    g_string_append_printf(commands, "HoldY(1,1);SensorY();HoldY(1,1);");
    guint            dilution_vessel = vessels_simple_get_pos_xachse(vessels_object_get_simple(ULTRA_VESSEL6()));
    VesselsDilution *dilution        = vessels_object_get_dilution(ULTRA_VESSEL6());
    guint            dilution_pos    = 750;
    if (dilution) dilution_pos = vessels_dilution_get_dilution_pos(dilution);
    g_string_append_printf(commands, "V3C();MoveX(%d,1,1);MoveY(%d,1,1);MoveInj(%d,1,1);", dilution_vessel, dilution_pos, air);
    g_string_append_printf(commands, "HoldY(1,1);SensorY();HoldY(1,1);");
    guint push_pos = sequence_workers_dilution_get_push_pos(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process)));
    guint take_pos = sequence_workers_dilution_get_take_pos(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process)));
    guint inj_take = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + achsen_injection_get_take_volume(injection)) * 2.5;
    if (inj_take > max) inj_take = max - 5;
    guint inj_push = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection)) * 2.5;
    if (inj_push > max) inj_push = max - 5;

    int repeat = 0;
    for (; repeat < sequence_workers_dilution_get_repeat(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process))); repeat++) {
        g_string_append_printf(commands, "MoveY(%d,1,1);MoveInj(%d,2,0);MoveY(%d,1,1);MoveInj(%d,1,0);", take_pos, inj_take, push_pos, inj_push);
    }
    g_string_append_printf(commands, "HoldY(1,1);SensorY();HoldY(1,1);");
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 60000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), dilution_ptp_done_async_callback, task);
    g_string_free(commands, TRUE);
}

static void assessment_of_fluid_result(UltraDilutionProcess *dilution_process) {
    guint err = 0;
    guint ok  = 0;
    if (dilution_process->priv->fluid_values) {
        guint i = 0;
        for (i = 0; i < dilution_process->priv->fluid_values->len; i++) {
            if (g_array_index(dilution_process->priv->fluid_values, gboolean, i))
                err++;
            else
                ok++;
        }
    }
    gboolean error = FALSE;
    if (err > ok)
        error = TRUE;
    else if (err > 0 && dilution_process->priv->fluid_values && dilution_process->priv->fluid_values->len > 10)
        error = ok / err < 4;
    if (error) {
        mkt_errors_come(E2107);
    } else {
        mkt_errors_clean(E2107);
    }
}

static void WAITE_dilution_sampling_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    UltraDilutionProcess *dilution_process = ULTRA_DILUTION_PROCESS(g_task_get_source_object(task));
    if (g_timer_elapsed(dilution_process->priv->sample_dilution, NULL) > dilution_process->priv->fill_time) {
        if (TERA_PUMP_6()) pumps_pump_call_stop_sync(pumps_object_get_pump(TERA_PUMP_6()), NULL, NULL, NULL);
        assessment_of_fluid_result(dilution_process);

        start_dilution_process(task);
    } else {
        GError * error  = NULL;
        gboolean result = FALSE;
        if (!nodes_digital16_call_get_digital_in_sync(dilution_process->priv->digital16, 5, &result, g_task_get_cancellable(task), &error)) {
            g_task_return_error(task, g_error_new(ERROR_QUARK, E1750, "Read fluid sensor value from digital16 in fail - %s", error ? error->message : "unknown"));
            if (error) g_error_free(error);
            return;
        }
        g_array_append_val(dilution_process->priv->fluid_values, result);
        lar_timer_default_run(g_task_get_cancellable(task), WAITE_dilution_sampling_callback, 0.2, task);
    }
}

static void ultra_dilution_process_run(UltraSequenceProcess *sequence, GTask *task) {
    UltraDilutionProcess *dilution_process      = ULTRA_DILUTION_PROCESS(sequence);
    gboolean              is_done               = FALSE;
    GError *              error                 = NULL;
    dilution_process->priv->repeat              = 0;
    dilution_process->priv->fill_time           = sequence_workers_dilution_get_fill_time(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process)));
    dilution_process->priv->dilution_time       = sequence_workers_dilution_get_dilution_time(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process)));
    dilution_process->priv->proportion          = sequence_workers_dilution_get_proportion(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process)));
    dilution_process->priv->dilution_vessel_vol = sequence_workers_dilution_get_volume(sequence_object_get_workers_dilution(SEQUENCE_OBJECT(dilution_process)));

    if (!valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()), &is_done, g_task_get_cancellable(task), &error)) {
        sequence_workers_process_change_status_error(sequence_object_get_workers_process(SEQUENCE_OBJECT(dilution_process)), _("Close injection valve fail"));
        g_task_return_error(task, error);
        return;
    }
    if (TERA_PUMP_6() == NULL) {
        g_task_return_error(task, g_error_new(ERROR_QUARK, E1750, _("pump 6 is not found")));
        return;
    }
    if (!pumps_pump_call_start_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, g_task_get_cancellable(task), &error)) {
        sequence_workers_process_change_status_error(sequence_object_get_workers_process(SEQUENCE_OBJECT(dilution_process)),
                                                     _("Pump6 will not start. The starting component of the pump's motor has failed"));
        g_task_return_error(task, error);
        return;
    }
    gboolean init_parameter = FALSE;
    achsen_achse_call_init_parameter_sync(achsen_object_get_achse(X_AXIS()), &init_parameter, g_task_get_cancellable(task), NULL);
    achsen_achse_call_init_parameter_sync(achsen_object_get_achse(Y_AXIS()), &init_parameter, g_task_get_cancellable(task), NULL);
    achsen_achse_call_init_parameter_sync(achsen_object_get_achse(Z_AXIS()), &init_parameter, g_task_get_cancellable(task), NULL);
    if (dilution_process->priv->sample_dilution) g_timer_destroy(dilution_process->priv->sample_dilution);
    if (dilution_process->priv->fluid_values) g_array_free(dilution_process->priv->fluid_values, TRUE);
    dilution_process->priv->fluid_values = g_array_new(FALSE, FALSE, sizeof(gboolean));

    dilution_process->priv->sample_dilution = g_timer_new();
    g_timer_start(dilution_process->priv->sample_dilution);
    lar_timer_default_run(g_task_get_cancellable(task), WAITE_dilution_sampling_callback, 0.2, task);
}

static void ultra_dilution_process_cancel(UltraSequenceProcess *sequence) {
    UltraDilutionProcess *dilution_process = ULTRA_DILUTION_PROCESS(sequence);
    set_default_stirrer(dilution_process);
    if (TERA_PUMP_6()) pumps_pump_call_stop_sync(pumps_object_get_pump(TERA_PUMP_6()), NULL, NULL, NULL);
    gboolean is_done = FALSE;
    valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()), &is_done, NULL, NULL);
    //	stirrers_simple_set_default_delay(stirrers_object_get_simple(ULTRA_STIRRER_1),50);
    //		stirrers_simple_set_default_current(stirrers_object_get_simple(ULTRA_STIRRER_1),70);
}

static void ultra_dilution_process_init(UltraDilutionProcess *ultra_dilution_process) {
    UltraDilutionProcessPrivate *priv             = ULTRA_DILUTION_PROCESS_PRIVATE(ultra_dilution_process);
    ultra_dilution_process->priv                  = priv;
    ultra_dilution_process->priv->sample_dilution = NULL;
    ultra_dilution_process->priv->fluid_values    = NULL;
    ultra_dilution_process->priv->node            = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
    if (ultra_dilution_process->priv->node == NULL) {
        mkt_log_error_message_sync("node Digital1:/com/lar/nodes/Digital1 not found");
        // g_error("node Digital1:/com/lar/nodes/Digital1 not found");
        return;
    }
    ultra_dilution_process->priv->digital16 = nodes_object_get_digital16(ultra_dilution_process->priv->node);
}

static void ultra_dilution_process_constructed(GObject *object) {
    UltraDilutionProcess *dilution_process = ULTRA_DILUTION_PROCESS(object);

    G_OBJECT_CLASS(ultra_dilution_process_parent_class)->constructed(object);
    SequenceWorkersSample *sample = sequence_workers_sample_skeleton_new();
    sequence_object_skeleton_set_workers_sample(SEQUENCE_OBJECT_SKELETON(dilution_process), sample);
    sequence_workers_sample_set_repeat(sample, 1);
    sequence_workers_sample_set_volume(sample, 150);
    sequence_workers_sample_set_sample_main(sample, g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_VESSEL3())));
    sequence_workers_sample_set_sample_second(sample, g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_VESSEL1())));
    g_object_unref(sample);

    SequenceWorkersDilution *dilution = sequence_workers_dilution_skeleton_new();
    sequence_object_skeleton_set_workers_dilution(SEQUENCE_OBJECT_SKELETON(dilution_process), dilution);

    sequence_workers_dilution_set_fill_time(dilution, 1.0);
    sequence_workers_dilution_set_dilution_time(dilution, 40.0);
    sequence_workers_dilution_set_proportion(dilution, 5.0);
    sequence_workers_dilution_set_volume(dilution, 1700.0);
    sequence_workers_dilution_set_take_pos(dilution, 1420);
    sequence_workers_dilution_set_push_pos(dilution, 1020);
    sequence_workers_dilution_set_repeat(dilution, 3);
    mkt_errors_clean(E2107);
    g_object_unref(dilution);

    MktParamint32 *int32 = NULL;
    int32                = mkt_paramint32_get(ULTRA_SEQUENCE_WORKERS_NAME, g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "take-pos");
    if (int32 != NULL) {
        sequence_workers_dilution_set_take_pos(dilution, mkt_paramint32_value(int32));
        g_object_unref(int32);
    }

    int32 = mkt_paramint32_get(ULTRA_SEQUENCE_WORKERS_NAME, g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "push-pos");
    if (int32 != NULL) {
        sequence_workers_dilution_set_push_pos(dilution, mkt_paramint32_value(int32));
        g_object_unref(int32);
    }
    int32 = mkt_paramint32_get(ULTRA_SEQUENCE_WORKERS_NAME, g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "repeat");
    if (int32 != NULL) {
        sequence_workers_dilution_set_repeat(dilution, mkt_paramint32_value(int32));
        g_object_unref(int32);
    }
}

static void ultra_dilution_process_finalize(GObject *object) {
    // UltraDilutionProcess *dilution = ULTRA_DILUTION_PROCESS(object);
    G_OBJECT_CLASS(ultra_dilution_process_parent_class)->finalize(object);
}

static void ultra_dilution_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_DILUTION_PROCESS(object));
    // UltraDilutionProcess *data = ULTRA_DILUTION_PROCESS(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_dilution_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_DILUTION_PROCESS(object));
    // UltraDilutionProcess *data = ULTRA_DILUTION_PROCESS(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_dilution_process_class_init(UltraDilutionProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);

    g_type_class_add_private(klass, sizeof(UltraDilutionProcessPrivate));

    object_class->finalize     = ultra_dilution_process_finalize;
    object_class->set_property = ultra_dilution_process_set_property;
    object_class->get_property = ultra_dilution_process_get_property;
    object_class->constructed  = ultra_dilution_process_constructed;
    sclass->run                = ultra_dilution_process_run;
    sclass->cancel             = ultra_dilution_process_cancel;
}
