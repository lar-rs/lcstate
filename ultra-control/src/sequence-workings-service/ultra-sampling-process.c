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
#include "ultra-sampling-process.h"
enum {
    PROP_0,
};

struct _UltraSamplingProcessPrivate {
    GDBusMethodInvocation *invocation;
    VesselsObject *        sampling_vessel;
    guint                  repeat;
};

#define ULTRA_SAMPLING_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_SAMPLING_PROCESS, UltraSamplingProcessPrivate))

G_DEFINE_TYPE(UltraSamplingProcess, ultra_sampling_process, ULTRA_TYPE_SEQUENCE_PROCESS)

static VesselsObject *SAMPLING_VESSEL(UltraSamplingProcess *sampling_process) {
    if (sampling_process->priv->sampling_vessel) g_object_unref(sampling_process->priv->sampling_vessel);
    const gchar *vessels_path = sequence_workers_sample_get_sample_main(sequence_object_get_workers_sample(SEQUENCE_OBJECT(sampling_process)));

    if (vessels_path != NULL && g_variant_is_object_path(vessels_path)) {
        VesselsObject *object = VESSELS_OBJECT(g_dbus_object_manager_get_object(ultra_vessels_manager_client_get_manager(), vessels_path));
        if (object != NULL)
            sampling_process->priv->sampling_vessel = object;
        else
            sampling_process->priv->sampling_vessel = g_object_ref(ULTRA_VESSEL3());
    } else
        sampling_process->priv->sampling_vessel = g_object_ref(ULTRA_VESSEL3());
    return sampling_process->priv->sampling_vessel;
}

static void sampling_ptp_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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
    g_task_return_boolean(task, TRUE);
}

static void sampling_run(GTask *task) {
    UltraSamplingProcess *sampling_process = ULTRA_SAMPLING_PROCESS(g_task_get_source_object(task));
    GString *             commands         = g_string_new("");

    if (achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()))) != achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))) {
        g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
    }
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()));
    guint            volume    = sequence_workers_sample_get_volume(sequence_object_get_workers_sample(SEQUENCE_OBJECT(sampling_process)));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + volume) * 2.5;
    guint            max       = achsen_achse_get_max(achse);
    if (position > max) {
        mkt_errors_report(E1740,_("sampling maximal volum exedded volume:%d position:%d changed to %d"), volume, position, max-10);
        position = max - 10;
    }
    guint air           = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) * 2.5);
    guint vessels_pos   = vessels_simple_get_pos_xachse(vessels_object_get_simple(SAMPLING_VESSEL(sampling_process)));
    guint injection_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(SAMPLING_VESSEL(sampling_process)));
    // g_debug("HOLD:%d AIR:%d-%d VOLUME:%d POSITION:%d MAX%d", achsen_achse_get_hold(achse), achsen_injection_get_air(injection), air, volume, position, max);

    g_string_append_printf(commands, "MoveX(%d,1,1);MoveInj(%d,2,1);MoveY(%d,1,1);", vessels_pos, air, injection_pos);
    g_string_append_printf(commands, "MoveInj(%d,2,1);", position);
    g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 60000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), sampling_ptp_done_async_callback, task);
    g_string_free(commands, TRUE);
}

static void ultra_sampling_process_run(UltraSequenceProcess *sequence, GTask *task) {
    // UltraSamplingProcess *sampling_process = ULTRA_SAMPLING_PROCESS(sequence);
    sampling_run(task);
}

static void ultra_sampling_process_cancel(UltraSequenceProcess *sequence) {
    // UltraSamplingProcess *sampling_process = ULTRA_SAMPLING_PROCESS(sequence);
    // gboolean is_done = FALSE;
    // valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()), &is_done, NULL, NULL);
}

static void ultra_sampling_process_init(UltraSamplingProcess *ultra_sampling_process) {
    UltraSamplingProcessPrivate *priv = ULTRA_SAMPLING_PROCESS_PRIVATE(ultra_sampling_process);
    ultra_sampling_process->priv      = priv;
}

static void ultra_sampling_process_constructed(GObject *object) {
    UltraSamplingProcess *sampling_process = ULTRA_SAMPLING_PROCESS(object);
    G_OBJECT_CLASS(ultra_sampling_process_parent_class)->constructed(object);
    SequenceWorkersSample *sample = sequence_workers_sample_skeleton_new();
    sequence_object_skeleton_set_workers_sample(SEQUENCE_OBJECT_SKELETON(sampling_process), sample);
    sequence_workers_sample_set_repeat(sample, 1);
    sequence_workers_sample_set_volume(sample, 400);
    sequence_workers_sample_set_sample_main(sample, g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_VESSEL3())));
    sequence_workers_sample_set_sample_second(sample, g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_VESSEL1())));
    g_object_unref(sample);
}

static void ultra_sampling_process_finalize(GObject *object) {
    // UltraSamplingProcess *ultra_sensors = ULTRA_SAMPLING_PROCESS(object);
    G_OBJECT_CLASS(ultra_sampling_process_parent_class)->finalize(object);
}

static void ultra_sampling_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_SAMPLING_PROCESS(object));
    // UltraSamplingProcess *data = ULTRA_SAMPLING_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_sampling_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_SAMPLING_PROCESS(object));
    // UltraSamplingProcess *data = ULTRA_SAMPLING_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_sampling_process_class_init(UltraSamplingProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);

    g_type_class_add_private(klass, sizeof(UltraSamplingProcessPrivate));

    object_class->finalize     = ultra_sampling_process_finalize;
    object_class->set_property = ultra_sampling_process_set_property;
    object_class->get_property = ultra_sampling_process_get_property;
    object_class->constructed  = ultra_sampling_process_constructed;
    sclass->run                = ultra_sampling_process_run;
    sclass->cancel             = ultra_sampling_process_cancel;
}
