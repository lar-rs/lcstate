/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) LAR 2016 <sascha@sascha-VirtualBox>
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
#include "ultra-axishold-process.h"

enum {
    PROP_0,
};

struct _UltraAxisholdProcessPrivate {
    gboolean reserved;
};

#define ULTRA_AXISHOLD_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_AXISHOLD_PROCESS, UltraAxisholdProcessPrivate))

G_DEFINE_TYPE(UltraAxisholdProcess, ultra_axishold_process, ULTRA_TYPE_SEQUENCE_PROCESS)

static void axishold_ptp2_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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
    g_task_return_boolean(task,TRUE);
}


static void axishold_ptp_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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
      tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), axishold_ptp2_done_async_callback, task);
      return;
    }
    g_task_return_boolean(task,TRUE);
}


static void ultra_axishold_process_run(UltraSequenceProcess *sequence_process, GTask *task) {
  gboolean init_parameter =FALSE;
  GError *error = NULL ;

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
  g_string_append_printf(commands, "MoveX(1,1,1);SensorX();HoldX(1,1);");
  g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
  tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), axishold_ptp_done_async_callback, task);
  g_string_free(commands, TRUE);
}

static void ultra_axishold_process_cancel(UltraSequenceProcess *sequence_process) {
  gboolean out;
  valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()), &out, NULL, NULL);
}

static void ultra_axishold_process_init(UltraAxisholdProcess *ultra_axishold_process) {
    UltraAxisholdProcessPrivate *priv = ULTRA_AXISHOLD_PROCESS_PRIVATE(ultra_axishold_process);
    ultra_axishold_process->priv      = priv;
    /// com/lar/valves/TICVentile
}

static void ultra_axishold_process_finalize(GObject *object) {
    // UltraAxisholdProcess *ultra_sensors = ULTRA_AXISHOLD_PROCESS(object);

    G_OBJECT_CLASS(ultra_axishold_process_parent_class)->finalize(object);
}

static void ultra_axishold_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_AXISHOLD_PROCESS(object));
    // UltraAxisholdProcess *data = ULTRA_AXISHOLD_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_axishold_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_AXISHOLD_PROCESS(object));
    // UltraAxisholdProcess *data = ULTRA_AXISHOLD_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_axishold_process_class_init(UltraAxisholdProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraAxisholdProcessPrivate));
    object_class->finalize     = ultra_axishold_process_finalize;
    object_class->set_property = ultra_axishold_process_set_property;
    object_class->get_property = ultra_axishold_process_get_property;
    sclass->run                = ultra_axishold_process_run;
    sclass->cancel             = ultra_axishold_process_cancel;
}
