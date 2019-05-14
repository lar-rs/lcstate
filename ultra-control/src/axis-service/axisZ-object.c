/* -*- Mode:`` C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */ /*
 * @ingroup UltraAxisZObject
 * @{
 * @file  ultra-axisZ-object.c
 * @brief This is AXIS X model object description.
 *
 *  Copyright (C) LAR  2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "../../config.h"
#include "axis-application-object.h"
#include "axis-object.h"
#include "axisZ-object.h"
#include "control-ptp.h"
#include "move-object.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <mktlib.h>

enum {
    PROP_0,
};

struct _UltraAxisZObjectPrivate {
    GDBusMethodInvocation *invocation;
    MktAxis *              axis_data;
    MktInjection *         injection_data;

    gdouble            timeout;
    GTimer *           timer;
    NodesObject *      nodes_object;
    NodesDoppelmotor3 *doppel_motor2;
    guint              signal;

    guint wait_timer;

    guint     last_volume;
    guint     tail_pos;
    gint      injection_counter;
    guint     count;
    guint     injection_pos;
    guint     injection_air;
    // GSetting   *settings;
};

#define ULTRA_AXISZ_OBJECT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_AXISZ_OBJECT, UltraAxisZObjectPrivate))

static void ultra_axisZ_reload_injection_model(UltraAxisZObject *axis_object) {
    if (axis_object->priv->injection_data) g_object_unref(axis_object->priv->axis_data);
    axis_object->priv->injection_data =
        MKT_INJECTION(mkt_model_select_one(MKT_TYPE_INJECTION_MODEL, "select * from $tablename where param_object_path = '%s'", g_dbus_object_get_object_path(G_DBUS_OBJECT(axis_object))));
    if (axis_object->priv->injection_data == NULL) {
        axis_object->priv->injection_data = MKT_INJECTION(mkt_model_new(MKT_TYPE_INJECTION_MODEL, "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(axis_object)), NULL));
    }

    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    g_object_bind_property(axis_object->priv->injection_data, "injection-air", injection, "air", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "injection-rest", injection, "rest", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "injection-furnace-air", injection, "furnace-air", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "injection-dilution", injection, "dilution", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "injection-rinsing", injection, "rinsing", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    g_object_bind_property(axis_object->priv->injection_data, "injection-stepper-parameter", injection, "injection-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "sample-stepper-parameter", injection, "sample-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "rinsing-up-stepper-parameter", injection, "rinsing-up-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(axis_object->priv->injection_data, "rinsing-down-stepper-parameter", injection, "rinsing-down-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
}

G_DEFINE_TYPE(UltraAxisZObject, ultra_axisZ_object, AXIS_TYPE_OBJECT)

// ----------------------------------- Axis Z Injection Tc process ----------------------------------------------

void injection_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
            g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection tc fail - %s"), error != NULL ? error->message : "unknown error");
            if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_injection_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    axis_change_status(AXIS_OBJECT(axis_object), "%s", _("Injecting"));
    AchsenAchse *    achse           = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    AchsenInjection *injection       = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    gdouble          air             = ((((gdouble)achsen_injection_get_air(injection)) * 2.5) / 2.0);
    axis_object->priv->injection_pos = achsen_achse_get_hold(achse) + (guint)air;
    GString *command                 = g_string_new("");
    gdouble  volume                  = ((gdouble)achsen_achse_get_position(achsen_object_get_achse(ACHSEN_OBJECT(axis_object))));
    gint     injection_counter       = (gint)((volume / 2.5) / 100.0);
    guint injpar =  achsen_injection_get_injection_stepper_parameter(interface);

    if (injection_counter == 0)
        injection_counter = 1;
    else if (injection_counter > 4)
        injection_counter = 4;
    if (injection_counter == 1 || volume - 290 < axis_object->priv->injection_pos) {
        g_string_append_printf(command, "WAIT(5.0);MoveInj(%d,%d,0);", axis_object->priv->injection_pos,injpar);
    } else {
        guint tail_pos = (guint)((volume) / (gdouble)injection_counter);
        gint  next_pos = volume - tail_pos;
        g_string_append_printf(command, "WAIT(5.0);MoveInj(%d,%d,0);", next_pos,injpar);
        for (; next_pos > axis_object->priv->injection_pos;) {
            next_pos = next_pos - tail_pos;
            if (next_pos < axis_object->priv->injection_pos) {
                next_pos = axis_object->priv->injection_pos;
            }
            g_string_append_printf(command, "WAIT(2.0);MoveInj(%d,%d,0);", next_pos,injpar);
        }
    }
    ControlPtp *ptp = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", command->str, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), injection_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_string_free(command, TRUE);
    return TRUE;
}

void injection_tic_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection tic fail - %s"), error != NULL ? error->message : "unknown error");
            if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_injection_tic_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    axis_change_status(AXIS_OBJECT(axis_object), "%s", _("Injecting for TIC"));
    AchsenAchse *    achse           = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    AchsenInjection *injection       = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    gdouble          air             = ((((gdouble)achsen_injection_get_air(injection)) * 2.5) / 2.0);
    axis_object->priv->injection_pos = achsen_achse_get_hold(achse) + (guint)air;
    guint injpar =  achsen_injection_get_injection_stepper_parameter(interface);
    gchar *commands                  = g_strdup_printf("MoveInj(%d,%d,1);", axis_object->priv->injection_pos,injpar);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), axis_object->priv->injection_pos);

    ControlPtp *ptp = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), injection_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), axis_object->priv->injection_pos);
    return TRUE;
}

void injection_codo_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection codo fail - %s"), error != NULL ? error->message : "unknown error");
          if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_injection_cod_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    axis_change_status(AXIS_OBJECT(axis_object), "%s", _("Injecting for COD"));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    guint            position  = (guint)(achsen_achse_get_hold(achse) + ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection)) * 2.5));
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    guint luftpolster = (guint)(achsen_achse_get_hold(achsen_object_get_achse(ACHSEN_OBJECT(axis_object))) +
                                ((achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rest(injection)) * 2.5));
    guint injpar =  achsen_injection_get_injection_stepper_parameter(interface);
    gchar *commands = g_strdup_printf("MoveInj(%d,%d,0);WAIT(2.0);MoveInj(%d,2,0);", position,injpar, luftpolster);

    ControlPtp *ptp = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), injection_codo_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}

static void rest_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection rest fail - %s"), error != NULL ? error->message : "unknown error");
          if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_go_rest_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    axis_change_status(AXIS_OBJECT(axis_object), "%s", _("Injecting go rest"));
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection)) * 2.5;
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *commands = g_strdup_printf("MoveInj(%d,%d,0);", position, achsen_injection_get_injection_stepper_parameter(interface));

    ControlPtp *ptp = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), rest_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}
static void rinsing_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection rinsing fail - %s"), error != NULL ? error->message : "unknown error");
          if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_go_rinsing_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    axis_change_status(AXIS_OBJECT(axis_object), "%s", _("Running to rinsing"));
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + achsen_injection_get_rest(injection) + achsen_injection_get_rinsing(injection)) * 2.5;
    if (position > achsen_achse_get_max(achse)) position = achsen_achse_get_max(achse) - 15;
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    nodes_doppelmotor3_call_set_stepper1_endschalter_invert(nodes_object_get_doppelmotor3(axis_node_object(AXIS_OBJECT(axis_object))), TRUE, NULL, NULL, NULL);
    gchar *     commands = g_strdup_printf("MoveInj(%d,2,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), rinsing_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}

static void air_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection air fail - %s"), error != NULL ? error->message : "unknown error");
          if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_go_air_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection)) * 2.5;
    axis_change_status(AXIS_OBJECT(axis_object), _("Running to air position %d"), position);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *     commands = g_strdup_printf("MoveInj(%d,2,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), air_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}

static void furnace_air_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection furnace air fail - %s"), error != NULL ? error->message : "unknown error");
          if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_furnace_air_injection_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {

    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_furnace_air(injection) * 2.5);
    axis_change_status(AXIS_OBJECT(axis_object), _("Running to air position %d"), position);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *     commands = g_strdup_printf("MoveInj(%d,1,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), furnace_air_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}
static void sample_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
            g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.axis.Sample.Error", error != NULL ? error->message : "unknown error");
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}
static gboolean ultra_axisZ_object_go_sample_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, guint volume, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + volume) * 2.5;
    guint            max       = achsen_achse_get_max(achse);
    if (position > max) {
        mkt_errors_report(E1740,"injection sample volume:%d > max:%d - check injection volume parameter", position, max);
        position = max;
    }
    axis_change_status(AXIS_OBJECT(axis_object), _("Running to sample position %d"), position);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *     commands = g_strdup_printf("MoveInj(%d,2,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), sample_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}

static void dilution_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
      g_dbus_method_invocation_return_error(invocation,G_IO_ERROR, G_IO_ERROR_CANCELLED, _("Injection operation cancelled"));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
          g_dbus_method_invocation_return_error(invocation,ERROR_QUARK, E1710, _("Injection dilution fail - %s"), error != NULL ? error->message : "unknown error");
          if(error)g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean ultra_axisZ_object_push_dilution_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection)) * 2.5;
    guint            max       = achsen_achse_get_max(achse);
    if (position > max) {
      mkt_errors_report(E1740,"push dilution volume:%d > max:%d - check injection volume parameter", position, max);
        position = max - 5;
    }
    axis_change_status(AXIS_OBJECT(axis_object), _("Running to dilution extra push position %d"), position);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *     commands = g_strdup_printf("MoveInj(%d,2,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), dilution_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}
static gboolean ultra_axisZ_object_take_dilution_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_air(injection) + achsen_injection_get_take_volume(injection)) * 2.5;
    guint            max       = achsen_achse_get_max(achse);
    if (position > max) {
        mkt_errors_report(E1740,"take dilution volume:%d > max:%d - check injection volume parameter", position, max);
        position = max - 5;
    }
    axis_change_status(AXIS_OBJECT(axis_object), _("Running to take extra dilution position %d"), position);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *     commands = g_strdup_printf("MoveInj(%d,2,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), dilution_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}

static gboolean ultra_axisZ_object_go_sample_cod_callback(AchsenInjection *interface, GDBusMethodInvocation *invocation, guint volume, gpointer user_data) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(user_data);
    MOVE_cancel();
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_object));
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    guint            position  = achsen_achse_get_hold(achse) + (achsen_injection_get_furnace_air(injection) + achsen_injection_get_rest(injection) + volume) * 2.5;
    guint            max       = achsen_achse_get_max(achse);
    if (position > max) {
        mkt_errors_report(E1740,_("injection cod sample volume:%d > max:%d - check injection volume parameter"), position, max);
        position = max;
    }
    // TODO: error_gone mkt_error_gone(2330);
    axis_change_status(AXIS_OBJECT(axis_object), _("Running to cod sample position %d"), position);
    achsen_achse_set_go_to_pos(achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), position);
    gchar *     commands = g_strdup_printf("MoveInj(%d,2,0);", position);
    ControlPtp *ptp      = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application_get_object_manajer(), NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), sample_done, g_object_ref(invocation));
    g_object_unref(ptp);
    g_free(commands);
    return TRUE;
}

//

static void ultra_axisZ_object_constructed(GObject *object) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(object);
    if (G_OBJECT_CLASS(ultra_axisZ_object_parent_class)->constructed) G_OBJECT_CLASS(ultra_axisZ_object_parent_class)->constructed(object);

    AchsenInjection *injection = achsen_injection_skeleton_new();
    achsen_object_skeleton_set_injection(ACHSEN_OBJECT_SKELETON(axis_object), injection);
    g_signal_connect(injection, "handle-injection", G_CALLBACK(ultra_axisZ_object_injection_callback), axis_object);
    g_signal_connect(injection, "handle-injection-tic", G_CALLBACK(ultra_axisZ_object_injection_tic_callback), axis_object);
    g_signal_connect(injection, "handle-injection-cod", G_CALLBACK(ultra_axisZ_object_injection_cod_callback), axis_object);
    g_signal_connect(injection, "handle-go-rest", G_CALLBACK(ultra_axisZ_object_go_rest_callback), axis_object);
    g_signal_connect(injection, "handle-go-rinsing", G_CALLBACK(ultra_axisZ_object_go_rinsing_callback), axis_object);
    g_signal_connect(injection, "handle-go-air", G_CALLBACK(ultra_axisZ_object_go_air_callback), axis_object);
    g_signal_connect(injection, "handle-go-furnace-air", G_CALLBACK(ultra_axisZ_furnace_air_injection_callback), axis_object);
    g_signal_connect(injection, "handle-go-sample", G_CALLBACK(ultra_axisZ_object_go_sample_callback), axis_object);
    g_signal_connect(injection, "handle-take-dilution", G_CALLBACK(ultra_axisZ_object_take_dilution_callback), axis_object);
    g_signal_connect(injection, "handle-push-dilution", G_CALLBACK(ultra_axisZ_object_push_dilution_callback), axis_object);

    g_signal_connect(injection, "handle-go-sample-cod", G_CALLBACK(ultra_axisZ_object_go_sample_cod_callback), axis_object);
    ultra_axisZ_reload_injection_model(axis_object);

    nodes_doppelmotor3_call_set_stepper1_endschalter_invert(nodes_object_get_doppelmotor3(axis_node_object(AXIS_OBJECT(axis_object))), TRUE, NULL, NULL, NULL);

    // Default volume.
    achsen_injection_set_take_volume(injection, 580);

    g_object_unref(injection);

    MktParamint32 *int32 = NULL;
    int32                = mkt_paramint32_get("com.lar.tera.axis", g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "take-volume");
    if (int32 == NULL) {
        int32 = MKT_PARAMINT32(mkt_model_new(MKT_TYPE_PARAMINT32_MODEL, "param-object-id", "com.lar.tera.axis", "param-object-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "param-name",
            "take-volume", "param-activated", TRUE, "value", 580, NULL));
    }
    g_object_bind_property(int32, "value", injection, "take-volume", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
}

static void ultra_axisZ_object_init(UltraAxisZObject *ultra_axisZ_object) {
    UltraAxisZObjectPrivate *priv   = ULTRA_AXISZ_OBJECT_PRIVATE(ultra_axisZ_object);
    ultra_axisZ_object->priv        = priv;
    // Settings property connection ...
    /* TODO: Add initialization code here */
}

static void ultra_axisZ_object_finalize(GObject *object) {
    UltraAxisZObject *axis_object = ULTRA_AXISZ_OBJECT(object);
    if (axis_object->priv->injection_data) g_object_unref(axis_object->priv->injection_data);
    G_OBJECT_CLASS(ultra_axisZ_object_parent_class)->finalize(object);
}

static void ultra_axisZ_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_AXISZ_OBJECT(object));
    // UltraAxisZObject *axisZ = ULTRA_AXISZ_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_axisZ_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_AXISZ_OBJECT(object));
    // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.UltraAxisZInterface",value,pspec)) return;
    // UltraAxisZObject *axisZ = ULTRA_AXISZ_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_axisZ_object_class_init(UltraAxisZObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraAxisZObjectPrivate));
    object_class->finalize     = ultra_axisZ_object_finalize;
    object_class->set_property = ultra_axisZ_object_set_property;
    object_class->get_property = ultra_axisZ_object_get_property;
    object_class->constructed  = ultra_axisZ_object_constructed;

    /*	klass->check_axisZ        = NULL;
    klass->raw_value           = NULL;*/
}

/** @} */
