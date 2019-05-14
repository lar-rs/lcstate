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
#include "ultra-rinsing-process.h"
enum {
    PROP_0,
};

struct _UltraRinsingProcessPrivate {
    VesselsObject *rinsing_vessel;
    guint          repeat;
};

#define ULTRA_RINSING_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_RINSING_PROCESS, UltraRinsingProcessPrivate))

G_DEFINE_TYPE(UltraRinsingProcess, ultra_rinsing_process, ULTRA_TYPE_SEQUENCE_PROCESS)

static VesselsObject *RINSING_VESSEL(UltraRinsingProcess *rinsing_process) {
    if (rinsing_process->priv->rinsing_vessel) g_object_unref(rinsing_process->priv->rinsing_vessel);
    const gchar *vessels_path = sequence_workers_sample_get_sample_main(sequence_object_get_workers_sample(SEQUENCE_OBJECT(rinsing_process)));

    if (vessels_path != NULL && g_variant_is_object_path(vessels_path)) {
        VesselsObject *object = VESSELS_OBJECT(g_dbus_object_manager_get_object(ultra_vessels_manager_client_get_manager(), vessels_path));
        if (object != NULL)
            rinsing_process->priv->rinsing_vessel = object;
        else
            rinsing_process->priv->rinsing_vessel = g_object_ref(ULTRA_VESSEL3());
    } else
        rinsing_process->priv->rinsing_vessel = g_object_ref(ULTRA_VESSEL3());
    return rinsing_process->priv->rinsing_vessel;
}

static void rinsing_ptp_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
      g_task_return_boolean(task,TRUE);
}
static guint get_rinsing_position ( ){
  AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
  AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()));
  guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rinsing(injection)) * 2.5;
  guint            max       = achsen_achse_get_max(achse);
  if (position > max)   position = max-10;
  return position;
}



static void TIC_CLOSED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) return;
  UltraRinsingProcess *rinsing_process = ULTRA_RINSING_PROCESS(g_task_get_source_object(task));
    gboolean             result          = FALSE;
    GError *             error           = NULL;
    if (!vessels_ticport_call_close_finish(vessels_object_get_ticport(ULTRA_TICPORT()), &result, res, &error)) {
      g_task_return_error(task, error);
    } else {
        sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(rinsing_process)), _("Go to rinsing"));
        GString *             commands          = g_string_new("");
        guint vessels_pos = vessels_simple_get_pos_xachse(vessels_object_get_simple(RINSING_VESSEL(rinsing_process)));
        guint injection_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(RINSING_VESSEL(rinsing_process)));
        g_string_append_printf(commands, "MoveX(%d,1,1);MoveY(%d,1,1);V3C();HoldInj(1,1);SensorInj();V3O();HoldInj(1,1);", vessels_pos, injection_pos);
        int repeat = 1;
        guint            position  = get_rinsing_position();
        g_string_append_printf(commands, "V3O();MoveInj(%d,2,0);V3C();HoldInj(1,1);", position);
        for(;repeat<sequence_workers_sample_get_repeat(sequence_object_get_workers_sample(SEQUENCE_OBJECT(rinsing_process)));repeat++){
          g_string_append_printf(commands, "V3O();MoveInj(%d,2,0);V3C();HoldInj(1,1);", position);
        }
        g_string_append_printf(commands, HOLD_Y_AXIS_TASK);

        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 50000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), rinsing_ptp_done_async_callback,g_object_ref(task));
        g_string_free(commands,TRUE);
    }
}

static void FURNACE_CLOSED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) return;
  UltraRinsingProcess *rinsing_process = ULTRA_RINSING_PROCESS(g_task_get_source_object(task));
    gboolean             result          = FALSE;
    GError *             error           = NULL;
    if (!vessels_furnace_call_close_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
      g_task_return_error(task, error);

    } else {
        sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(rinsing_process)), _("Close TIC-Port"));
        vessels_ticport_call_close(vessels_object_get_ticport(ULTRA_TICPORT()), g_task_get_cancellable(task), TIC_CLOSED_async_callback, task);
    }
}
static void rinsing_ptp_hold_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        return;
    }
    vessels_furnace_call_close(vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), FURNACE_CLOSED_async_callback, task);
}
static void rinsing_run(GTask *task) {
    // UltraRinsingProcess *rinsing_process = ULTRA_RINSING_PROCESS(g_task_get_source_object(task));
    GString *             commands          = g_string_new("");
    AchsenAchse *    achse             = achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()));
    if(achse){
      if(achsen_achse_get_hold(achse)!= achsen_achse_get_position(achse)){
        g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 10000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), rinsing_ptp_hold_async_callback,task);
        g_string_free(commands,TRUE);
        return ;
      }
    }
    vessels_furnace_call_close(vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), FURNACE_CLOSED_async_callback, task);
}

static void ultra_rinsing_process_run(UltraSequenceProcess *sequence,GTask *task) {

    rinsing_run(task);
}

static void ultra_rinsing_process_cancel(UltraSequenceProcess *sequence) {
    // UltraSamplingProcess *sampling_process = ULTRA_SAMPLING_PROCESS(sequence);
    gboolean is_done = FALSE;
    valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()), &is_done, NULL, NULL);
}

static void ultra_rinsing_process_init(UltraRinsingProcess *ultra_rinsing_process) {
    UltraRinsingProcessPrivate *priv = ULTRA_RINSING_PROCESS_PRIVATE(ultra_rinsing_process);
    ultra_rinsing_process->priv      = priv;
}

static void ultra_rinsing_process_constructed(GObject *object) {
    UltraRinsingProcess *rinsing_process = ULTRA_RINSING_PROCESS(object);
    G_OBJECT_CLASS(ultra_rinsing_process_parent_class)->constructed(object);
    SequenceWorkersSample *sample = sequence_workers_sample_skeleton_new();
    sequence_object_skeleton_set_workers_sample(SEQUENCE_OBJECT_SKELETON(rinsing_process), sample);
    sequence_workers_sample_set_repeat(sample, 1);
    g_object_unref(sample);
}

static void ultra_rinsing_process_finalize(GObject *object) {
    // UltraRinsingProcess *ultra_sensors = ULTRA_RINSING_PROCESS(object);
    G_OBJECT_CLASS(ultra_rinsing_process_parent_class)->finalize(object);
}

static void ultra_rinsing_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_RINSING_PROCESS(object));
    // UltraRinsingProcess *data = ULTRA_RINSING_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_rinsing_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_RINSING_PROCESS(object));
    // UltraRinsingProcess *data = ULTRA_RINSING_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_rinsing_process_class_init(UltraRinsingProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);

    g_type_class_add_private(klass, sizeof(UltraRinsingProcessPrivate));

    object_class->finalize     = ultra_rinsing_process_finalize;
    object_class->set_property = ultra_rinsing_process_set_property;
    object_class->get_property = ultra_rinsing_process_get_property;
    object_class->constructed  = ultra_rinsing_process_constructed;
    sclass->run                = ultra_rinsing_process_run;
    sclass->cancel             = ultra_rinsing_process_cancel;
}
