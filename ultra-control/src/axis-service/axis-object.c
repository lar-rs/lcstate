/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup XYSystem
 * @{
 * @file  axis-object.c
 * @brief This is AXIS model object description.
 *
 *  Copyright (C) LAR  2016
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "axis-object.h"
#include "move-object.h"
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>

#include "axis-application-object.h"
#include "d3go-object.h"
#include "d3sensor-object.h"
#include "move-axis.h"
#include "move-object.h"

enum {
    PROP_0,
    PROP_NODE_OBJECT,
    PROP_ERROR_OBJECT,
    PROP_AXIS_MODEL,
    PROP_NODE_PART,
    PROP_INVERT,
    PROP_GO_POSITION,
    PROP_GO_SENSOR,
};

#include "../../config.h"
#include <glib/gi18n-lib.h>

/* signals */

enum { AXIS_DONE, LAST_SIGNAL };

// static guint axis_object_signals[LAST_SIGNAL];

struct _AxisObjectPrivate {
    gchar *            name;
    MktAxis *          axis_data;
    gdouble            timeout;
    GTimer *           timer;
    NodesObject *      nodes_object;
    NodesDoppelmotor3 *doppel_motor;
    guint              repeat;
    guint              max_repeat;
    gboolean           invert;
    guint              part;
    MktErrorsNumbers   errNumber;
    gboolean           isStallGuard;
    // GSetting   *settings;
};

#define AXIS_OBJECT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), AXIS_TYPE_OBJECT, AxisObjectPrivate))

void axis_change_status(AxisObject *axis_object, const gchar *format, ...) {
    g_return_if_fail(axis_object != NULL);
    g_return_if_fail(ACHSEN_IS_OBJECT(axis_object));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    if (achse) achsen_achse_set_status(achse, new_status);
    g_free(new_status);
}

void axis_intern_set_busy(AxisObject *axis_object, gboolean value) {
    AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    if (achse) {
        achsen_achse_set_is_busy(achse, value);
        g_object_unref(achse);
    }
}

gboolean axis_intern_get_busy(AxisObject *axis_object) {
    AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    gboolean     busy  = FALSE;
    if (achse) {
        busy = achsen_achse_get_is_busy(achse);
        g_object_unref(achse);
    }
    return busy;
}

guint axis_intern_get_hold(AxisObject *axis) {
    AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
    return achsen_achse_get_hold(achse);
}

gboolean axis_is_timeout(AxisObject *axis_object) {
    g_return_val_if_fail(axis_object->priv->timer != NULL, TRUE);
    return g_timer_elapsed(axis_object->priv->timer, NULL) > axis_object->priv->timeout;
}

NodesObject *axis_node_object(AxisObject *axis_object) {
    g_return_val_if_fail(axis_object != NULL, NULL);
    return axis_object->priv->nodes_object;
}

const gchar *axis_object_get_name(AxisObject *axis_object) {
    g_return_val_if_fail(axis_object != NULL, "X?");
    g_return_val_if_fail(AXIS_IS_OBJECT(axis_object), "X?");
    const gchar *name  = "X?";
    AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    if (achse) {
        name = achsen_achse_get_name(achse);
        g_object_unref(achse);
    }

    return name;
}

G_DEFINE_TYPE(AxisObject, axis_object, ACHSEN_TYPE_OBJECT_SKELETON)

gboolean axis_object_init_parameter(AxisObject *axis, guint max, guint current, GError **error) {
    NodesDoppelmotor3 *doppelmotor3 = axis->priv->doppel_motor;
    gboolean           is_done;
    gboolean           result = TRUE;
    if (axis->priv->part == 1) {
        if (!nodes_doppelmotor3_call_set_stepper1_mode_sync(doppelmotor3, 0, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper1_max_pos_sync(doppelmotor3, max, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper1_current_sync(doppelmotor3, current, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper1_hold_current_sync(doppelmotor3, 200, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper1_parameter_sync(doppelmotor3, 1, &is_done, MOVE_cancellable(), error))
            result = FALSE;

        if (axis->priv->invert) {
            nodes_doppelmotor3_call_set_stepper1_endschalter_invert_sync(doppelmotor3, TRUE, &is_done, MOVE_cancellable(), error);
        }
    } else {
        if (!nodes_doppelmotor3_call_set_stepper2_mode_sync(doppelmotor3, 0, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper2_max_pos_sync(doppelmotor3, max, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper2_current_sync(doppelmotor3, current, &is_done, MOVE_cancellable(), error))
            result = FALSE;
        else if (!nodes_doppelmotor3_call_set_stepper2_parameter_sync(doppelmotor3, 1, &is_done, MOVE_cancellable(), error))
            result = FALSE;
    }
    return result;
}

static void axis_object_change_max_callback(AchsenAchse *achse, GParamSpec *pspec, AxisObject *axis_object) {
    axis_object_init_parameter(axis_object, achsen_achse_get_max(achse), achsen_achse_get_current(achse), NULL);
}

static void axis_object_change_current_callback(AchsenAchse *achse, GParamSpec *pspec, AxisObject *axis_object) {
    axis_object_init_parameter(axis_object, achsen_achse_get_max(achse), achsen_achse_get_current(achse), NULL);
}

static gboolean axis_object_init_parameter_callback(AchsenAchse *achse, GDBusMethodInvocation *invocation, gpointer user_data) {
    AxisObject *axis_object = AXIS_OBJECT(user_data);
    MOVE_cancel();
    GError *error = NULL;
    if (!axis_object_init_parameter(axis_object, achsen_achse_get_max(achse), achsen_achse_get_current(achse), &error)) {
        g_dbus_method_invocation_return_error(invocation, ERROR_QUARK, axis_object->priv->errNumber, _("%s initialization fail - %s"), achsen_achse_get_name(achse), error != NULL ? error->message : "unknown error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

void axis_position_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    MktTaskObject *        move       = MKT_TASK_OBJECT(source_object);
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    AxisObject *           axis       = AXIS_OBJECT(MOVE_axis(MOVE_OBJECT(move)));
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_CANCELLED, _("%s operation cancelled"), axis_object_get_name(axis));
    } else {
        if (!mkt_task_object_finish(move, res, &error)) {
            // axis_change_status(AXIS_OBJECT(axis), "Axis go pos error - %s", error ? error->message : "unknown");
            g_dbus_method_invocation_return_error(invocation, ERROR_QUARK, E1710, _("%s move fail - %s"), axis_object_get_name(axis), error != NULL ? error->message : "unknown error");
            if (error) g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

gboolean axis_object_go_position_callback(AchsenAchse *interface, GDBusMethodInvocation *invocation, guint position, gpointer user_data) {
    AxisObject *axis_object = AXIS_OBJECT(user_data);
    MOVE_cancel();
    MOVE *move = d3go_new(axis_object, axis_object->priv->part, position, 1, 0);
    mkt_task_object_run(MKT_TASK_OBJECT(move), MOVE_cancellable(), axis_position_done, g_object_ref(invocation));
    g_object_unref(move);
    return TRUE;
}

static gboolean axis_object_go_hold_callback(AchsenAchse *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    AxisObject *axis_object = AXIS_OBJECT(user_data);
    MOVE_cancel();
    // axis_object_init_movements(axis_object, invocation);
    achsen_achse_set_go_to_pos(interface, achsen_achse_get_hold(interface));
    MOVE *move = d3go_new(axis_object, 2, achsen_achse_get_hold(interface), 1, 0);
    mkt_task_object_run(MKT_TASK_OBJECT(move), MOVE_cancellable(), axis_position_done, g_object_ref(invocation));
    g_object_unref(move);
    return TRUE;
}

static void axis_go_sensor_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    MktTaskObject *        move       = MKT_TASK_OBJECT(source_object);
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    AxisObject *           axis       = AXIS_OBJECT(MOVE_axis(MOVE_OBJECT(move)));
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_CANCELLED, _("%s operation cancelled"), axis_object_get_name(axis));
    } else {
        if (!mkt_task_object_finish(move, res, &error)) {
            //  axis_change_status(AXIS_OBJECT(axis), "Axis go sensor error - %s", error ? error->message : "unknown");
            g_dbus_method_invocation_return_error(
                invocation, ERROR_QUARK, E1720, _("%s search final position fail - %s"), axis_object_get_name(axis), error != NULL ? error->message : "unknown error");
            if (error) g_error_free(error);
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean axis_object_go_sensor_callback(AchsenAchse *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    AxisObject *axis_object = AXIS_OBJECT(user_data);
    MOVE_cancel();
    MOVE *move = d3sensor_new(axis_object, axis_object->priv->part);
    mkt_task_object_run(MKT_TASK_OBJECT(move), MOVE_cancellable(), axis_go_sensor_done, g_object_ref(invocation));
    g_object_unref(move);
    return TRUE;
}

static void axis_object_constructed(GObject *object) {
    AxisObject * axis_object = AXIS_OBJECT(object);
    AchsenAchse *achse       = achsen_achse_skeleton_new();
    achsen_object_skeleton_set_achse(ACHSEN_OBJECT_SKELETON(axis_object), achse);
    g_signal_connect(achse, "handle-go-to-position", G_CALLBACK(axis_object_go_position_callback), axis_object);
    g_signal_connect(achse, "handle-go-sensor", G_CALLBACK(axis_object_go_sensor_callback), axis_object);
    g_signal_connect(achse, "handle-go-hold", G_CALLBACK(axis_object_go_hold_callback), axis_object);
    g_signal_connect(achse, "handle-init-parameter", G_CALLBACK(axis_object_init_parameter_callback), axis_object);
    g_signal_connect(achse, "notify::max", G_CALLBACK(axis_object_change_max_callback), axis_object);
    g_signal_connect(achse, "notify::current", G_CALLBACK(axis_object_change_current_callback), axis_object);

    g_object_unref(achse);

    if (axis_object->priv->doppel_motor == NULL) {
        // g_error("Axis %s Doppelmotor interface not found", g_dbus_object_get_object_path(G_DBUS_OBJECT(axis_object)));
    }
    //

    if (G_OBJECT_CLASS(axis_object_parent_class)->constructed) G_OBJECT_CLASS(axis_object_parent_class)->constructed(object);
}

static void axis_object_init(AxisObject *axis_object) {
    AxisObjectPrivate *priv = AXIS_OBJECT_PRIVATE(axis_object);
    priv->timer             = g_timer_new();
    priv->timeout           = 30.0;
    priv->axis_data         = NULL;
    priv->nodes_object      = NULL;
    priv->doppel_motor      = NULL;
    priv->part              = 2;
    priv->isStallGuard      = FALSE;
    axis_object->priv       = priv;

    // Settings property connection ...
    /* TODO: Add initialization code here */
}

static void axis_object_finalize(GObject *object) {
    AxisObject *axis_object = AXIS_OBJECT(object);
    if (axis_object->priv->axis_data) g_object_unref(axis_object->priv->axis_data);
    if (axis_object->priv->timer) g_timer_destroy(axis_object->priv->timer);
    if (axis_object->priv->nodes_object) {
        g_object_unref(axis_object->priv->nodes_object);
        if (axis_object->priv->doppel_motor) {
            g_object_unref(axis_object->priv->doppel_motor);
        }
    }
    G_OBJECT_CLASS(axis_object_parent_class)->finalize(object);
}

static void axis_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(AXIS_IS_OBJECT(object));
    AxisObject *axis = AXIS_OBJECT(object);
    switch (prop_id) {
    case PROP_NODE_OBJECT:
        if (axis->priv->nodes_object) g_object_unref(axis->priv->nodes_object);
        axis->priv->nodes_object = g_value_dup_object(value);
        axis->priv->doppel_motor = nodes_object_get_doppelmotor3(axis->priv->nodes_object);
        break;
    case PROP_AXIS_MODEL:
        if (axis->priv->axis_data) g_object_unref(axis->priv->axis_data);
        axis->priv->axis_data = g_value_dup_object(value);
        break;
    case PROP_NODE_PART:
        axis->priv->part = g_value_get_uint(value);
        break;
    case PROP_INVERT:
        axis->priv->invert = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void axis_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(AXIS_IS_OBJECT(object));
    AxisObject *axis = AXIS_OBJECT(object);
    switch (prop_id) {
    case PROP_NODE_OBJECT:
        g_value_set_object(value, axis->priv->nodes_object);
        break;
    case PROP_AXIS_MODEL:
        g_value_set_object(value, axis->priv->axis_data);
        break;
    case PROP_NODE_PART:
        g_value_set_uint(value, axis->priv->part);
        break;
    case PROP_INVERT:
        g_value_set_boolean(value, axis->priv->invert);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void axis_object_class_init(AxisObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(AxisObjectPrivate));
    object_class->finalize     = axis_object_finalize;
    object_class->set_property = axis_object_set_property;
    object_class->get_property = axis_object_get_property;
    object_class->constructed  = axis_object_constructed;

    g_object_class_install_property(
        object_class, PROP_NODE_OBJECT, g_param_spec_object("node-object", "Axis node object", "Axis node object", NODES_TYPE_OBJECT, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(object_class, PROP_NODE_PART, g_param_spec_uint("part", "part", "part", 1, 2, 2, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(object_class, PROP_INVERT, g_param_spec_boolean("invert", "invert", "invert", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

guint axis_object_get_part(AxisObject *axis_object) {
    g_return_val_if_fail(axis_object, 0);
    g_return_val_if_fail(AXIS_IS_OBJECT(axis_object), 0);
    return axis_object->priv->part;
}

/**
 * axis_object_get_error_number:
 *
 * @returns error number
*/
MktErrorsNumbers axis_object_get_error_number(AxisObject *axis_object) {
    g_return_val_if_fail(axis_object, E1710);
    g_return_val_if_fail(AXIS_IS_OBJECT(axis_object), E1710);
    return axis_object->priv->errNumber;
}
/**
 * axis_object_set_error_number:
 * @number:set axis error number
 *
 * Set axis error number .
*/
void axis_object_set_error_number(AxisObject *axis_object, MktErrorsNumbers number) {
    g_return_if_fail(axis_object != NULL);
    g_return_if_fail(AXIS_IS_OBJECT(axis_object));
    axis_object->priv->errNumber = number;
}

gboolean  axis_is_activate_stall_guard(AxisObject *axis_object){
  g_return_val_if_fail(axis_object, FALSE);
  g_return_val_if_fail(AXIS_IS_OBJECT(axis_object), FALSE);
  return axis_object->priv->isStallGuard;
}

void      axis_activate_stall_guard(AxisObject *axis_object){
  g_return_if_fail(axis_object != NULL);
  g_return_if_fail(AXIS_IS_OBJECT(axis_object));
  axis_object->priv->isStallGuard = TRUE;
}


void axis_init_parameter(AxisObject *axis_object) {
    g_return_if_fail(axis_object != NULL);
    g_return_if_fail(AXIS_IS_OBJECT(axis_object));
    MOVE_cancel();
    GError *     error = NULL;
    AchsenAchse *axis  = achsen_object_get_achse(ACHSEN_OBJECT(axis_object));
    if (!axis_object_init_parameter(axis_object, achsen_achse_get_max(axis), achsen_achse_get_current(axis), &error)) {
        g_warning("init parameter failed - %s", error ? error->message : "unknown");
        if (error) g_error_free(error);
    }
    g_object_unref(axis);
}
/** @} */
