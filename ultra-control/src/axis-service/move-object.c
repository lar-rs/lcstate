/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup XYSystem
 * @{
 * @file  MOVE-object.c
 * @brief This is MOVE model object description.
 *
 *  Copyright (C) LAR  2016
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "move-axis.h"
#include "move-object.h"
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>

enum {
    PROP_0,
    PROP_MESSAGE,
    PROP_NAME,
    PROP_NODE_OBJECT,
    PROP_ERROR_OBJECT,
    PROP_AXIS_OBJECT,
    PROP_CANCELLABLE_OBJECT,
    PROP_INVOCATION_OBJECT,
    PROP_MOVE_POS,
    PROP_PART,
};

#include "../../config.h"
#include <glib/gi18n-lib.h>

/* signals */

enum { MOVE_DONE, LAST_SIGNAL };

// static guint MOVE_signals[LAST_SIGNAL];

struct _MOVEPrivate {
    gchar *            message;
    guint              part;
    guint              result;
    NodesObject *      nodes_object;
    gdouble            timeout;
    GTimer *           timer;
    AxisObject *       axis_object;
    NodesDoppelmotor3 *doppelmotor3;
    // GSetting   *settings;
};

#define MOVE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), MOVE_TYPE, MOVEPrivate))

GQuark move_error_quark(void) {
    static GQuark error;
    if (!error) error = g_quark_from_static_string("axis-move-error");
    return error;
}


void MOVE_message(MOVE *MOVE_object, const gchar *format, ...) {
    g_return_if_fail(MOVE_object != NULL);
    g_return_if_fail(MOVE_OBJECT(MOVE_object));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    g_object_set(MOVE_object, "message", new_status, NULL);
    g_free(new_status);
}

void MOVE_object_done(MOVE *MOVE_object, const gchar *format, ...) {
    g_return_if_fail(MOVE_object != NULL);
    g_return_if_fail(MOVE_OBJECT(MOVE_object));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    g_object_set(MOVE_object, "message", new_status, NULL);
    g_free(new_status);
}

void MOVE_object_error(MOVE *MOVE_object, const gchar *format, ...) {
    g_return_if_fail(MOVE_object != NULL);
    g_return_if_fail(MOVE_OBJECT(MOVE_object));

    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    g_object_set(MOVE_object, "message", new_status, NULL);
    g_free(new_status);
}

static GCancellable *AXIS_CANCEL = NULL;
void                 MOVE_cancel() {
    if (AXIS_CANCEL) {
        g_cancellable_cancel(AXIS_CANCEL);
        g_object_unref(AXIS_CANCEL);
    }
    AXIS_CANCEL = g_cancellable_new();
}

GCancellable *MOVE_cancellable() { return AXIS_CANCEL; }

NodesObject *MOVE_node(MOVE *move) { return move->priv->nodes_object; }

NodesDoppelmotor3 *MOVE_doppelmotor3(MOVE *move) {
    if (move->priv->doppelmotor3 == NULL) move->priv->doppelmotor3 = nodes_object_get_doppelmotor3(move->priv->nodes_object);
    return move->priv->doppelmotor3;
}
const gchar *MOVE_node_name(MOVE *move) { return g_dbus_object_get_object_path(G_DBUS_OBJECT(move->priv->nodes_object)); }
const gchar *MOVE_axis_name(MOVE *move){
  g_return_val_if_fail(move != NULL, "X?");
  g_return_val_if_fail(MOVE_IS(move), "X?");
  const gchar *name = "X?";
  if(move->priv->axis_object){
    AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(move->priv->axis_object));
    if(achse){
      name = achsen_achse_get_name(achse);
      g_object_unref(achse);
    }
  }
  return name;
}

void MOVE_set_position(MOVE *move, guint position) {
    g_return_if_fail(move != NULL);
    g_return_if_fail(MOVE_IS(move));
    g_object_set(move, "current-position", position, NULL);
}

gboolean MOVE_is_timeout(MOVE *move) {
    g_return_val_if_fail(move != NULL, TRUE);
    g_return_val_if_fail(MOVE_IS(move), TRUE);
    return g_timer_elapsed(move->priv->timer, NULL) > move->priv->timeout;
}

gdouble MOVE_timeout(MOVE *move) {
    g_return_val_if_fail(move != NULL, 25.0);
    g_return_val_if_fail(MOVE_IS(move), 25.0);
    return move->priv->timeout;
}

void MOVE_timer_reset(MOVE *move) {
    g_return_if_fail(move != NULL);
    g_return_if_fail(MOVE_IS(move));
    g_timer_reset(move->priv->timer);
}

void MOVE_timer_start(MOVE *move) {
    g_return_if_fail(move != NULL);
    g_return_if_fail(MOVE_IS(move));
    g_timer_start(move->priv->timer);
}
AxisObject *MOVE_axis(MOVE *move) { return move->priv->axis_object; }

gboolean MOVE_parameter_sync(MOVE *move, guint parameter, gboolean *out, GCancellable *cancellable, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_set_stepper1_parameter_sync(MOVE_doppelmotor3(move), parameter, out, cancellable, error);
    case 2:
        return nodes_doppelmotor3_call_set_stepper2_parameter_sync(MOVE_doppelmotor3(move), parameter, out, cancellable, error);
    }
    return FALSE;
}

gboolean MOVE_stepper_go_pos(MOVE *move, guint move_to, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        nodes_doppelmotor3_call_set_stepper1_go_pos(MOVE_doppelmotor3(move), move_to, cancellable, callback, user_data);
        return TRUE;
    case 2:
        nodes_doppelmotor3_call_set_stepper2_go_pos(MOVE_doppelmotor3(move), move_to, cancellable, callback, user_data);
        return TRUE;
    }
    return FALSE;
}

gboolean MOVE_go_pos_finish(MOVE *move, gboolean *result, GAsyncResult *res, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_set_stepper1_go_pos_finish(MOVE_doppelmotor3(move), result, res, error);
    case 2:
        return nodes_doppelmotor3_call_set_stepper2_go_pos_finish(MOVE_doppelmotor3(move), result, res, error);
    }
    return FALSE;
}

gboolean MOVE_current_position(MOVE *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        nodes_doppelmotor3_call_get_stepper1_go_pos(MOVE_doppelmotor3(move), cancellable, callback, user_data);
        return TRUE;
    case 2:
        nodes_doppelmotor3_call_get_stepper2_go_pos(MOVE_doppelmotor3(move), cancellable, callback, user_data);
        return TRUE;
    }

    return FALSE;
}
gboolean MOVE_current_position_finish(MOVE *move, guint *result, GAsyncResult *res, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_get_stepper1_go_pos_finish(MOVE_doppelmotor3(move), result, res, error);
    case 2:
        return nodes_doppelmotor3_call_get_stepper2_go_pos_finish(MOVE_doppelmotor3(move), result, res, error);
    }
    return FALSE;
}

gboolean MOVE_position_old_sync(MOVE *move, guint *result, GCancellable *cancellable, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_get_stepper1_pos_old_sync(MOVE_doppelmotor3(move), result, cancellable, error);
    case 2:
        return nodes_doppelmotor3_call_get_stepper2_pos_old_sync(MOVE_doppelmotor3(move), result, cancellable, error);
    }
    return FALSE;
}
gboolean MOVE_command_status_sync(MOVE *move, guint command, gboolean *out_done, GCancellable *cancellable, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_set_stepper1_command_status_sync(MOVE_doppelmotor3(move), command, out_done, cancellable, error);
    case 2:
        return nodes_doppelmotor3_call_set_stepper2_command_status_sync(MOVE_doppelmotor3(move), command, out_done, cancellable, error);
    }
    return FALSE;
}

gboolean MOVE_command_status(MOVE *move, guint command, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        nodes_doppelmotor3_call_set_stepper1_command_status(MOVE_doppelmotor3(move), command, cancellable, callback, user_data);
        return TRUE;
    case 2:
        nodes_doppelmotor3_call_set_stepper2_command_status(MOVE_doppelmotor3(move), command, cancellable, callback, user_data);
        return TRUE;
    }
    return FALSE;
}
gboolean MOVE_command_status_finish(MOVE *move, gboolean *result, GAsyncResult *res, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_set_stepper1_command_status_finish(MOVE_doppelmotor3(move), result, res, error);
    case 2:
        return nodes_doppelmotor3_call_set_stepper2_command_status_finish(MOVE_doppelmotor3(move), result, res, error);
    }
    return FALSE;
}

gboolean MOVE_final_position(MOVE *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        nodes_doppelmotor3_call_get_stepper1_final_position(MOVE_doppelmotor3(move), cancellable, callback, user_data);
        return TRUE;
    case 2:
        nodes_doppelmotor3_call_get_stepper2_final_position(MOVE_doppelmotor3(move), cancellable, callback, user_data);
        return TRUE;
    }
    return FALSE;
}

gboolean MOVE_final_position_finish(MOVE *move, gboolean *result, GAsyncResult *res, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_get_stepper1_final_position_finish(MOVE_doppelmotor3(move), result, res, error);
    case 2:
        return nodes_doppelmotor3_call_get_stepper2_final_position_finish(MOVE_doppelmotor3(move), result, res, error);
    }
    return FALSE;
}

gboolean MOVE_get_stall_guard(MOVE *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        nodes_doppelmotor3_call_get_stepper1_stall_guard_flag(MOVE_doppelmotor3(move), cancellable, callback, user_data);
        return TRUE;
    case 2:
        nodes_doppelmotor3_call_get_stepper2_stall_guard_flag(MOVE_doppelmotor3(move), cancellable, callback, user_data);
        return TRUE;
    }
    return FALSE;
}
gboolean MOVE_get_stall_guard_finish(MOVE *move, guint *result, GAsyncResult *res, GError **error) {
    g_return_val_if_fail(move != NULL, FALSE);
    g_return_val_if_fail(MOVE_IS(move), FALSE);
    switch (move->priv->part) {
    case 1:
        return nodes_doppelmotor3_call_get_stepper1_stall_guard_flag_finish(MOVE_doppelmotor3(move), result, res, error);
    case 2:
        return nodes_doppelmotor3_call_get_stepper2_stall_guard_flag_finish(MOVE_doppelmotor3(move), result, res, error);
    }
    return FALSE;
}

guint MOVE_result(MOVE *move) {
    g_return_val_if_fail(move != NULL, 0);
    g_return_val_if_fail(MOVE_IS(move), 0);
    return move->priv->result;
}

static void MOVE_init_axis_interface(MoveAxisInterface *iface) {}

G_DEFINE_TYPE_WITH_CODE(MOVE, MOVE, MKT_TYPE_TASK_OBJECT, G_IMPLEMENT_INTERFACE(MOVE_TYPE_AXIS, MOVE_init_axis_interface))

// -------------------------------------------- Help functions --------------------------------------------------------

static void MOVE_init(MOVE *MOVE_object) {
    MOVEPrivate *priv  = MOVE_PRIVATE(MOVE_object);
    priv->timeout      = 25.0;
    priv->timer        = g_timer_new();
    priv->nodes_object = NULL;
    priv->doppelmotor3 = NULL;
    MOVE_object->priv  = priv;

    // Settings property connection ...
    /* TODO: Add initialization code here */
}
static void MOVE_constructed(GObject *object) {
    MOVE *MOVE_object = MOVE_OBJECT(object);
    if (MOVE_object->priv->axis_object) {
        AchsenAchse *axis = achsen_object_get_achse(ACHSEN_OBJECT(MOVE_object->priv->axis_object));
        achsen_achse_set_is_busy(axis, TRUE);
        g_object_bind_property(axis, "status", object, "message", G_BINDING_DEFAULT);
        g_object_unref(axis);
    }
    G_OBJECT_CLASS(MOVE_parent_class)->constructed(object);
}

static void MOVE_finalize(GObject *object) {
    MOVE *MOVE_object = MOVE_OBJECT(object);
    if (MOVE_object->priv->axis_object) {
        AchsenAchse *axis = achsen_object_get_achse(ACHSEN_OBJECT(MOVE_object->priv->axis_object));
        achsen_achse_set_is_busy(axis, FALSE);
        g_object_unref(axis);
        g_object_unref(MOVE_object->priv->axis_object);
    }
    if (MOVE_object->priv->nodes_object) {
        g_object_unref(MOVE_object->priv->nodes_object);
        if (MOVE_object->priv->doppelmotor3) g_object_unref(MOVE_object->priv->doppelmotor3);
    }
    if (MOVE_object->priv->timer) g_timer_destroy(MOVE_object->priv->timer);
    if (MOVE_object->priv->message) g_free(MOVE_object->priv->message);
    G_OBJECT_CLASS(MOVE_parent_class)->finalize(object);
}

static void MOVE_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(MOVE_IS(object));
    MOVE *MOVE_object = MOVE_OBJECT(object);
    switch (prop_id) {
    case PROP_PART:
        MOVE_object->priv->part = g_value_get_uint(value);
        break;
    case PROP_MESSAGE:
        if (MOVE_object->priv->message) g_free(MOVE_object->priv->message);
        MOVE_object->priv->message = g_value_dup_string(value);
        break;
    case PROP_NODE_OBJECT:
        if (MOVE_object->priv->nodes_object) g_object_unref(MOVE_object->priv->nodes_object);
        MOVE_object->priv->nodes_object = g_value_dup_object(value);
        break;
    case PROP_AXIS_OBJECT:
        if (MOVE_object->priv->axis_object) g_object_unref(MOVE_object->priv->axis_object);
        MOVE_object->priv->axis_object = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void MOVE_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(MOVE_IS(object));
    // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.MOVEInterface",value,pspec)) return;
    MOVE *MOVE_object = MOVE_OBJECT(object);
    switch (prop_id) {
    case PROP_PART:
        g_value_set_uint(value, MOVE_object->priv->part);
        break;
    case PROP_MESSAGE:
        g_value_set_string(value, MOVE_object->priv->message);
        break;
    case PROP_NODE_OBJECT:
        g_value_set_object(value, MOVE_object->priv->nodes_object);
        break;
    case PROP_AXIS_OBJECT:
        g_value_set_object(value, MOVE_object->priv->axis_object);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void MOVE_class_init(MOVEClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(MOVEPrivate));
    object_class->finalize     = MOVE_finalize;
    object_class->set_property = MOVE_set_property;
    object_class->get_property = MOVE_get_property;
    object_class->constructed  = MOVE_constructed;

    g_object_class_override_property(object_class, PROP_NODE_OBJECT, "node-object");
    g_object_class_override_property(object_class, PROP_AXIS_OBJECT, "axis-object");
    g_object_class_override_property(object_class, PROP_PART, "part");
    g_object_class_override_property(object_class, PROP_MESSAGE, "message");

    /*	klass->check_MOVEX        = NULL;
    klass->raw_value           = NULL;*/
}

/** @} */
