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
#include "ultra-injectioncod-process.h"
enum {
    PROP_0,
};

struct _UltraInjectioncodProcessPrivate {
    gboolean step_injection;
};

#define ULTRA_INJECTIONCOD_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_INJECTIONCOD_PROCESS, UltraInjectioncodProcessPrivate))

G_DEFINE_TYPE(UltraInjectioncodProcess, ultra_injectioncod_process, ULTRA_TYPE_SEQUENCE_PROCESS)


static void ptp_codinj_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
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

static void FURNACE_CLOSED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
      return;
  }
    UltraInjectioncodProcess *injectioncod_process = ULTRA_INJECTIONCOD_PROCESS(g_task_get_source_object(task));
    gboolean                  result               = FALSE;
    GError *                  error                = NULL;
    if (!vessels_furnace_call_close_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
        sequence_workers_process_change_status_error(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectioncod_process)), _("Furnace closed fail - %s"), error ? error->message : "unknown");
        g_task_return_error(task, error);
    } else {
      GString *commands = g_string_new("");
      g_string_append_printf(commands, "MoveY(1,1,1);SensorY();HoldY(1,1);MoveX(1,1,1);SensorX();HoldX(1,1);");
      g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
      tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_codinj_done_async_callback, task);
      g_string_free(commands,TRUE);
    }
}

static void ptp_injec_done_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)){
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        return;
    }
    UltraInjectioncodProcess *injectioncod_process = ULTRA_INJECTIONCOD_PROCESS(g_task_get_source_object(task));
    sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectioncod_process)), _("Close furnace"));
    vessels_furnace_call_close(
        vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), FURNACE_CLOSED_async_callback,task);
}

static void FURNACE_OPENED_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
      return;
  }

    UltraInjectioncodProcess *injectioncod_process = ULTRA_INJECTIONCOD_PROCESS(g_task_get_source_object(task));
    gboolean                  result               = FALSE;
    GError *                  error                = NULL;
    if (!vessels_furnace_call_open_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error)) {
        sequence_workers_process_change_status_error(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectioncod_process)), _("Furnace open fail - %s"), error ? error->message : "unknown");
        g_task_return_error(task, error);
    } else {
        if(injectioncod_process->priv->step_injection){
          guint    injectio_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(ULTRA_FURNACE()));
          GString *commands     = g_string_new("");
          g_string_append_printf(commands, "MoveY(%d,1,0);", injectio_pos);
          AchsenObject *   axis_object       = Z_AXIS();
          AchsenAchse *    achse             = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
          AchsenInjection *injection         = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
          guint luftpolster = (guint)(achsen_achse_get_hold(achse) +
          ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rest(injection)) * 2.5));

          gint             injection_pos     = (guint)(achsen_achse_get_hold(achse) + ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection)) * 2.5));
          gdouble          volume            = ((gdouble)achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object))))-injection_pos;
          gint             injection_counter = (gint)((volume/2.5)/100.0);
          guint injpar = achsen_injection_get_injection_stepper_parameter(injection);
          if (injection_counter < 1)
              injection_counter = 1;
          else if (injection_counter > 4)
              injection_counter = 4;
          if (injection_counter == 1 ) {
              g_string_append_printf(commands, "MoveInj(%d,%d,0);", injection_pos, injpar);
          } else {
              guint injection_volume =(guint)(volume/(gdouble)injection_counter);
              gint next_volume = achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)));
              next_volume = next_volume - injection_volume;
              if (next_volume - 75 < injection_pos) {
                next_volume = injection_pos;
              }
              g_string_append_printf(commands, "MoveInj(%d,%d,0);", next_volume, injpar);
              for (; next_volume > injection_pos;) {
                  next_volume = next_volume - injection_volume;
                  if (next_volume -75 < injection_pos) {
                      next_volume = injection_pos;
                  }
                  g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,%d,0);", next_volume, injpar);
              }
          }
          guint needle_pos = vessels_furnace_get_needle_pos(vessels_object_get_furnace(ULTRA_FURNACE()));
          g_string_append_printf(commands, "WAIT(2.0);MoveInj(%d,2,0);Wait(2.0);MoveY(%d,1,0);",luftpolster, needle_pos);
          g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
          tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_injec_done_async_callback, task);
          g_string_free(commands, TRUE);
        }else{
          GString *commands = g_string_new("");
          AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()));
          AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()));
          guint            position  = (guint)(achsen_achse_get_hold(achse) + ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection)) * 2.5));
          guint luftpolster = (guint)(achsen_achse_get_hold(achse) +
          ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rest(injection)) * 2.5));
          guint injpar =  achsen_injection_get_injection_stepper_parameter(injection);

          guint injectio_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(ULTRA_FURNACE()));
          guint needle_pos = vessels_furnace_get_needle_pos(vessels_object_get_furnace(ULTRA_FURNACE()));
          g_string_append_printf(commands, "MoveY(%d,1,0);",injectio_pos);
          g_string_append_printf(commands, "MoveInj(%d,%d,0);WAIT(2.0);MoveInj(%d,2,0);",position,injpar, luftpolster);
          g_string_append_printf(commands, "MoveY(%d,1,0);",needle_pos);
          g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 20000);
          tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_injec_done_async_callback, task);
          g_string_free(commands,TRUE);
        }
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
    UltraInjectioncodProcess *injectioncod_process = ULTRA_INJECTIONCOD_PROCESS(g_task_get_source_object(task));
    sequence_workers_process_change_status(sequence_object_get_workers_process(SEQUENCE_OBJECT(injectioncod_process)), _("Open furnace"));
    vessels_furnace_call_open(
        vessels_object_get_furnace(ULTRA_FURNACE()), g_task_get_cancellable(task), FURNACE_OPENED_async_callback,task);
}


static void start_cod_injection(GTask *task){

  GString *commands = g_string_new("");
  if(achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))!= achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS())))){
    g_string_append_printf(commands, HOLD_Y_AXIS_TASK);
  }
  guint furnace_pos = vessels_simple_get_pos_xachse(vessels_object_get_simple(ULTRA_FURNACE()));
  guint needle_pos = vessels_furnace_get_needle_pos(vessels_object_get_furnace(ULTRA_FURNACE()));
  g_string_append_printf(commands, "MoveX(100,1,1);HoldX(1,1);SensorX();MoveX(%d,1,1);MoveY(%d,1,0);",furnace_pos,needle_pos);
  g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 20000);
  tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(task), ptp_on_needle_done_async_callback, task);
  g_string_free(commands,TRUE);
}

static void ultra_injectioncod_process_run(UltraSequenceProcess *sequence, GTask *task) {

    start_cod_injection(task);
}

static void ultra_injectioncod_process_cancel(UltraSequenceProcess *sequence) {
    // UltraInjectioncodProcess *injection_process = ULTRA_INJECTIONCOD_PROCESS(sequence);
}

static void ultra_injectioncod_process_init(UltraInjectioncodProcess *ultra_injectioncod_process) {
    UltraInjectioncodProcessPrivate *priv = ULTRA_INJECTIONCOD_PROCESS_PRIVATE(ultra_injectioncod_process);
    ultra_injectioncod_process->priv      = priv;
    ultra_injectioncod_process->priv->step_injection = TRUE;
}

static void ultra_injectioncod_process_finalize(GObject *object) {
    // UltraInjectioncodProcess *ultra_injectioncod = ULTRA_INJECTIONCOD_PROCESS(object);
    G_OBJECT_CLASS(ultra_injectioncod_process_parent_class)->finalize(object);
}

static void ultra_injectioncod_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_INJECTIONCOD_PROCESS(object));
    // UltraInjectioncodProcess *data = ULTRA_INJECTIONCOD_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_injectioncod_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_INJECTIONCOD_PROCESS(object));
    // UltraInjectioncodProcess *data = ULTRA_INJECTIONCOD_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_injectioncod_process_class_init(UltraInjectioncodProcessClass *klass) {
    GObjectClass *             object_class = G_OBJECT_CLASS(klass);
    UltraSequenceProcessClass *sclass       = ULTRA_SEQUENCE_PROCESS_CLASS(klass);

    g_type_class_add_private(klass, sizeof(UltraInjectioncodProcessPrivate));

    object_class->finalize     = ultra_injectioncod_process_finalize;
    object_class->set_property = ultra_injectioncod_process_set_property;
    object_class->get_property = ultra_injectioncod_process_get_property;
    sclass->run                = ultra_injectioncod_process_run;
    sclass->cancel             = ultra_injectioncod_process_cancel;
}
