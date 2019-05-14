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
#include "ultra-allhold-process.h"

enum {
    PROP_0,
};

struct _UltraAllholdProcessPrivate {
    GCancellable *cancellable;
};

#define ULTRA_ALLHOLD_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ALLHOLD_PROCESS, UltraAllholdProcessPrivate))

G_DEFINE_TYPE(UltraAllholdProcess, ultra_allhold_process, ULTRA_TYPE_SEQUENCE_PROCESS)

static void ultra_hold_process_init_valves(UltraAllholdProcess *hold_process,GTask *task) {

    valves_simple_call_reset(valves_object_get_simple(ULTRA_VALVE_INJECTION()), NULL, NULL, NULL);
    // valves_simple_call_reset(valves_object_get_simple(ULTRA_VALVE_TOCDIRECT()),NULL, NULL, NULL);
}

static void ultra_hold_process_init_pumps(UltraAllholdProcess *hold_process,GTask *task) {
    pumps_pump_call_start(pumps_object_get_pump(TERA_PUMP_CONDENSATE()), g_task_get_cancellable(task), NULL, NULL);
    pumps_pump_call_stop(pumps_object_get_pump(TERA_PUMP_1()), g_task_get_cancellable(task), NULL, NULL);
    pumps_pump_call_stop(pumps_object_get_pump(TERA_PUMP_2()), g_task_get_cancellable(task), NULL, NULL);
    if (TERA_PUMP_3()) pumps_pump_call_stop(pumps_object_get_pump(TERA_PUMP_3()), g_task_get_cancellable(task), NULL, NULL);
    if (TERA_PUMP_4()) pumps_pump_call_stop(pumps_object_get_pump(TERA_PUMP_4()), g_task_get_cancellable(task), NULL, NULL);
    if (TERA_PUMP_5()) pumps_pump_call_stop(pumps_object_get_pump(TERA_PUMP_5()), g_task_get_cancellable(task), NULL, NULL);
    if (TERA_PUMP_6()) pumps_pump_call_stop(pumps_object_get_pump(TERA_PUMP_6()), g_task_get_cancellable(task), NULL, NULL);
}

static void TIC_CLOSED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) return;
    UltraAllholdProcess *allhold_process = ULTRA_ALLHOLD_PROCESS(g_task_get_source_object(task));
    gboolean             result          = FALSE;
    GError *             error           = NULL;
    if (!vessels_ticport_call_close_finish(vessels_object_get_ticport(ULTRA_TICPORT()), &result, res, &error)) {
      g_task_return_error(task, error);
    } else {
        sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(allhold_process)), _("Process done"));
        g_task_return_boolean(task,TRUE);

    }
}

static void FURNACE_CLOSED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) return;
    UltraAllholdProcess *allhold_process = ULTRA_ALLHOLD_PROCESS(g_task_get_source_object(task));
    gboolean             result          = FALSE;
    GError *             error           = NULL;
    if (!vessels_furnace_call_close_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
      g_task_return_error(task, error);

    } else {
        sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(allhold_process)), _("Close TIC-Port"));
        vessels_ticport_call_close(vessels_object_get_ticport(ULTRA_TICPORT()), g_task_get_cancellable(task), TIC_CLOSED_async_callback, task);
    }
}
static void allhold_ptp2_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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

static void allhold_ptp_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        GString *commands = g_string_new("");
        g_string_append_printf(commands, "SensorY();SensorX();HoldY(1,1);HoldX(1,1);");
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 20000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), allhold_ptp2_done_async_callback, task);
        g_string_free(commands, TRUE);
        return;
    }
    gboolean is_done;
    vessels_furnace_call_monitoring_sync(
      vessels_object_get_furnace(ULTRA_FURNACE()), FALSE, &is_done, NULL,
      NULL);
    vessels_furnace_call_close(vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), FURNACE_CLOSED_async_callback, task);
}


static void ultra_allhold_process_run(UltraSequenceProcess *sequence_process, GTask *task) {
    UltraAllholdProcess *allhold_process = ULTRA_ALLHOLD_PROCESS(sequence_process);
    vessels_furnace_call_monitoring(vessels_object_get_furnace(ULTRA_FURNACE()), TRUE, g_task_get_cancellable(task), NULL, NULL);
    vessels_furnace_call_on_off(vessels_object_get_furnace(ULTRA_FURNACE()), TRUE, g_task_get_cancellable(task), NULL, NULL);
    ultra_hold_process_init_valves(allhold_process,task);
    ultra_hold_process_init_pumps(allhold_process,task);
    gboolean init_parameter =FALSE;
    GError *error = NULL ;
    stirrers_simple_call_reset_sync(stirrers_object_get_simple(ULTRA_STIRRER_1()), &init_parameter, g_task_get_cancellable(task), &error);
    if(error) {
        g_task_return_error(task, error);
        return;
    }
    achsen_achse_call_init_parameter_sync(achsen_object_get_achse(X_AXIS()), &init_parameter, g_task_get_cancellable(task), &error);
    if(error) {
        g_task_return_error(task, error);
        return;
    }
    achsen_achse_call_init_parameter_sync(achsen_object_get_achse(Y_AXIS()), &init_parameter, g_task_get_cancellable(task), &error);
    if(error) {
        g_task_return_error(task, error);
        return;
    }
    achsen_achse_call_init_parameter_sync(achsen_object_get_achse(Z_AXIS()), &init_parameter, g_task_get_cancellable(task), &error);
    if(error) {
        g_task_return_error(task, error);
        return;
    }
    GString *commands = g_string_new("");
    if (achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()))) != achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()))))
      g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
    g_string_append_printf(commands, "HoldX(1,1);SensorX();HoldX(1,1);");
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
    tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), allhold_ptp_done_async_callback, task);
    g_string_free(commands, TRUE);
}
static void ultra_allhold_process_cancel(UltraSequenceProcess *sequence_process){
  // gboolean out = FALSE;
  // valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()), &out, NULL, NULL);
}
static void ultra_allhold_process_init(UltraAllholdProcess *ultra_allhold_process) {
    UltraAllholdProcessPrivate *priv = ULTRA_ALLHOLD_PROCESS_PRIVATE(ultra_allhold_process);
    ultra_allhold_process->priv      = priv;
    /// com/lar/valves/TICVentile
}

static void ultra_allhold_process_finalize(GObject *object) {
    // UltraAllholdProcess *ultra_sensors = ULTRA_ALLHOLD_PROCESS(object);

    G_OBJECT_CLASS(ultra_allhold_process_parent_class)->finalize(object);
}

static void ultra_allhold_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_ALLHOLD_PROCESS(object));
    // UltraAllholdProcess *data = ULTRA_ALLHOLD_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_allhold_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_ALLHOLD_PROCESS(object));
    // UltraAllholdProcess *data = ULTRA_ALLHOLD_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_allhold_process_class_init(UltraAllholdProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraAllholdProcessPrivate));
    object_class->finalize     = ultra_allhold_process_finalize;
    object_class->set_property = ultra_allhold_process_set_property;
    object_class->get_property = ultra_allhold_process_get_property;
    sclass->run                = ultra_allhold_process_run;
    sclass->cancel             = ultra_allhold_process_cancel;
}
