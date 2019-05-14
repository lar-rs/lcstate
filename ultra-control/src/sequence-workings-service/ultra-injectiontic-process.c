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
#include "ultra-injectiontic-process.h"
enum {
    PROP_0,
};

struct _UltraInjectionticProcessPrivate {
    GDBusMethodInvocation *invocation;
};

#define ULTRA_INJECTIONTIC_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_INJECTIONTIC_PROCESS, UltraInjectionticProcessPrivate))

G_DEFINE_TYPE(UltraInjectionticProcess, ultra_injectiontic_process, ULTRA_TYPE_SEQUENCE_PROCESS)


static void ptp_ticinj_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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
    g_task_return_boolean(task,TRUE);
}

static void TIC_CLOSED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
      return;
  }

  UltraInjectionticProcess *injectiontic_process = ULTRA_INJECTIONTIC_PROCESS(g_task_get_source_object(task));
    gboolean                  result               = FALSE;
    GError *                  error                = NULL;
    if (!vessels_ticport_call_close_finish(VESSELS_TICPORT(source_object), &result, res, &error)) {
        sequence_workers_process_change_status_error(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectiontic_process)), _("Tic closed fail - %s"), error ? error->message : "unknown");
        g_task_return_error(task, error);
    } else {
      GString *commands = g_string_new("");
      g_string_append_printf(commands, "MoveY(1,1,1);SensorY();HoldY(1,1);MoveX(1,1,1);SensorX();HoldX(1,1);");
      g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
      tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_ticinj_done_async_callback, task);
      g_string_free(commands,TRUE);
    }
}

static void ptp_ticinjec_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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
    UltraInjectionticProcess *injectiontic_process = ULTRA_INJECTIONTIC_PROCESS(g_task_get_source_object(task));
    sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectiontic_process)), _("TIC-port close"));
    vessels_ticport_call_close(
        vessels_object_get_ticport(ULTRA_TICPORT()), g_task_get_cancellable(task), TIC_CLOSED_async_callback,task);
}

static void TIC_OPENED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
      return;
  }
    UltraInjectionticProcess *injectiontic_process = ULTRA_INJECTIONTIC_PROCESS(g_task_get_source_object(task));
    gboolean                  result               = FALSE;
    GError *                  error                = NULL;
    if (!vessels_ticport_call_open_finish(VESSELS_TICPORT(source_object), &result, res, &error)) {
        sequence_workers_process_change_status_error(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectiontic_process)), _("TIC open fail - %s"), error ? error->message : "unknown");
        g_task_return_error(task, error);
    } else {
        GString *commands = g_string_new("");
        AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
        AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()));
        gdouble          air             = ((((gdouble)achsen_injection_get_air(injection)) * 2.5) / 2.0);
        guint position = achsen_achse_get_hold(achse) + (guint)air;
        guint injection_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(ULTRA_TICPORT()));
        guint needle_pos = vessels_ticport_get_needle_pos(vessels_object_get_ticport(ULTRA_TICPORT()));

        guint injpar =  achsen_injection_get_injection_stepper_parameter(injection);
        g_string_append_printf(commands, "MoveY(%d,1,0);",injection_pos);
        g_string_append_printf(commands, "MoveInj(%d,%d,0);WAIT(2.0);",position,injpar);
        g_string_append_printf(commands, "MoveY(%d,1,0);",needle_pos);
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 20000);
        tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_ticinjec_done_async_callback, task);
        g_string_free(commands,TRUE);
    }

}

static void ptp_on_needle_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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
    UltraInjectionticProcess *injectiontic_process = ULTRA_INJECTIONTIC_PROCESS(g_task_get_source_object(task));
    sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectiontic_process)), _("Open tic port"));
    vessels_ticport_call_open(
        vessels_object_get_ticport(ULTRA_TICPORT()), g_task_get_cancellable(task), TIC_OPENED_async_callback,task);
}


static void start_tic_injection(GTask *task){

  GString *commands = g_string_new("");
  if(achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))!= achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))){
    g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
  }
  guint tic_pos = vessels_simple_get_pos_xachse(vessels_object_get_simple(ULTRA_TICPORT()));
  guint needle_pos = vessels_ticport_get_needle_pos(vessels_object_get_ticport(ULTRA_TICPORT()));
  g_string_append_printf(commands, "MoveX(%d,1,1);MoveY(%d,1,0);",tic_pos,needle_pos);
  g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 20000);
  tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_on_needle_done_async_callback, task);
  g_string_free(commands,TRUE);
}

static void ultra_injectiontic_process_run(UltraSequenceProcess *sequence, GTask *task) {
    start_tic_injection(task);
}


static void ultra_injectiontic_process_init(UltraInjectionticProcess *ultra_injectiontic_process) {
    UltraInjectionticProcessPrivate *priv = ULTRA_INJECTIONTIC_PROCESS_PRIVATE(ultra_injectiontic_process);
    ultra_injectiontic_process->priv      = priv;
}

static void ultra_injectiontic_process_finalize(GObject *object) {
    // UltraInjectionticProcess *ultra_sensors = ULTRA_INJECTIONTIC_PROCESS(object);
    G_OBJECT_CLASS(ultra_injectiontic_process_parent_class)->finalize(object);
}

static void ultra_injectiontic_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_INJECTIONTIC_PROCESS(object));
    // UltraInjectionticProcess *data = ULTRA_INJECTIONTIC_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_injectiontic_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_INJECTIONTIC_PROCESS(object));
    // UltraInjectionticProcess *data = ULTRA_INJECTIONTIC_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_injectiontic_process_class_init(UltraInjectionticProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);

    g_type_class_add_private(klass, sizeof(UltraInjectionticProcessPrivate));

    object_class->finalize     = ultra_injectiontic_process_finalize;
    object_class->set_property = ultra_injectiontic_process_set_property;
    object_class->get_property = ultra_injectiontic_process_get_property;
    sclass->run                = ultra_injectiontic_process_run;
}
