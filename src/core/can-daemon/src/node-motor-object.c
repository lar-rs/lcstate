/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mktlibrary
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
 * mktlibrary is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mktlibrary is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "node-device-object.h"
#include "node-motor-object.h"
#include <mkt-value.h>

struct _NodeMotorObjectPrivate {
    gint temp;
};

G_DEFINE_TYPE_WITH_PRIVATE(NodeMotorObject, node_motor_object, NODE_TYPE_OBJECT);

enum {
    PROP_MOTOR_0,

};

static gboolean node_motor_object_write_integer(NodeMotorObject *motor, const gchar *index_id, gint val) {
    g_return_val_if_fail(index_id != NULL, FALSE);
    // g_debug("Motor index %s test set int value to  %d",index_id,val);
    GValue *value = mkt_value_new(G_TYPE_INT);
    g_value_set_int(value, val);
    gboolean ret = node_object_write_value(NODE_OBJECT(motor), index_id, value);
    mkt_value_free(value);
    return ret;
}

static gboolean node_motor_object_write_unsigned(NodeMotorObject *motor, const gchar *index_id, guint val) {
    g_return_val_if_fail(index_id != NULL, FALSE);
    // g_debug("Motor index %s test set int value to  %d",index_id,val);
    GValue *value = mkt_value_new(G_TYPE_UINT);
    g_value_set_uint(value, val);
    gboolean ret = node_object_write_value(NODE_OBJECT(motor), index_id, value);
    mkt_value_free(value);
    return ret;
}

static gboolean node_motor_object_write_string(NodeMotorObject *motor, const gchar *index_id, const gchar *val) {
    g_return_val_if_fail(index_id != NULL, FALSE);
    g_return_val_if_fail(val != NULL, FALSE);
    GValue *value = mkt_value_new(G_TYPE_STRING);
    g_value_set_string(value, val);
    gboolean ret = node_object_write_value(NODE_OBJECT(motor), index_id, value);
    mkt_value_free(value);
    return ret;
}

static gboolean node_motor_object_get_stepper1_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6100sub1", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GtStepper1Mode", "Read 6100sub1 error");
        return TRUE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", node_motor_object_write_integer(motor, "6100sub1", (gint)value)));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_final_position_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6100sub2", G_TYPE_INT);
    gboolean         res   = 0;
    if (value) {
        res = (gboolean)g_value_get_int(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1FinalPosition", "Read 6100sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_endschalter_invert_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6100sub3", G_TYPE_INT);
    gboolean         res   = 0;
    if (value) {

        res = (gboolean)g_value_get_int(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1EndschalterInvert", "Read 6100sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_endschalter_invert_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6100sub3", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_diagnose_byte_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6100sub4", G_TYPE_UINT);
    guint            res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1DiagnoseByte", "Read 6100sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_parameter_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6101sub5", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GtStepper1Parameter", "Read 6101sub5 error");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_parameter_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6101sub5", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_command_status_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6101sub1", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1CommandStatus", "Read 6101sub1 error");
        return TRUE; // TODO: check what should return if fehler
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_command_status_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6101sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_go_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6101sub2", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1GoPos", "Read 6101sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_go_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6101sub2", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_pos_old_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub3", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1PosOld", "Read 6201sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_max_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6101sub4", G_TYPE_UINT);
    guint            res   = 0;
    if (value) {

        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1MaxPos", "Read 6101sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_max_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6101sub4", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_stall_guard_flag_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6101sub6", G_TYPE_UINT);
    guint            res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.StallGuardFlag", "Read 6101sub6 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_stall_guard_flag_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6101sub6", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_on_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6102sub1", G_TYPE_INT);
    gboolean         res   = 0;
    if (value) {
        res = (gboolean)g_value_get_int(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1EndschalterInvert", "Read 6102sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_on_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6102sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_delay_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6102sub2", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1Delay", "Read 6102sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_delay_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6102sub2", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6103sub1", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1Current", "Read 6103sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6103sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_hold_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6103sub3", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1HoldCurrent", "Read 6103sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_hold_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         ret   = node_motor_object_write_unsigned(motor, "6103sub3", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", ret));

    return TRUE;
}

static gboolean node_motor_object_get_stepper1_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6103sub2", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1CurrentDigit", "Read 6103sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6103sub2", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper1_hold_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6103sub4", G_TYPE_UINT);
    guint   res   = 0;
    if (value) {
        res = g_value_get_uint(value);
        mkt_value_free(value);
    }
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper1HoldCurrentDigit", "Read 6103sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper1_hold_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", node_motor_object_write_unsigned(motor, "6103sub4", (gint)value)));
    return TRUE;
}

// -------------------------------------------  Stepper 2 --------------------------------------------------------------------------------

static gboolean node_motor_object_get_stepper2_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6200sub1", G_TYPE_INT);
    guint            res   = value ? (guint)g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GtStepper2Mode", "Read 6100sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", node_motor_object_write_integer(motor, "6200sub1", (gint)value)));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_final_position_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6200sub2", G_TYPE_INT);
    gboolean         res   = value ? (gboolean)g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2FinalPosition", "Read 6100sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_endschalter_invert_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6200sub3", G_TYPE_INT);
    gboolean         res   = value ? (gboolean)g_value_get_int(value) : FALSE;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2EndschalterInvert", "Read 6100sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_endschalter_invert_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", node_motor_object_write_integer(motor, "6200sub3", (gint)value)));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_diagnose_byte_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6200sub4", G_TYPE_INT);
    guint            res   = value ? (guint)g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2DiagnoseByte", "Read 6100sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_parameter_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub5", G_TYPE_INT);
    guint            res   = value ? (guint)g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GtStepper2Parameter", "Read 6101sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_parameter_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6201sub5", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_command_status_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub1", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        return TRUE;
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2CommandStatus", "Read 6101sub1 error");
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_command_status_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6201sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_go_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub2", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2GoPos", "Read 6101sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_go_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6201sub2", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_pos_old_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub3", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2PosOld", "Read 6201sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_max_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub4", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2MaxPos", "Read 6101sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_max_pos_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6201sub4", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_stall_guard_flag_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6201sub6", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.StallGuardFlag", "Read 6101sub6 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_stall_guard_flag_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6201sub6", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_on_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6202sub1", G_TYPE_INT);
    gboolean         res   = value ? (gboolean)g_value_get_int(value) : FALSE;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2EndschalterInvert", "Read 6102sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_on_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6202sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_delay_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6202sub2", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2Delay", "Read 6102sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_delay_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6202sub2", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6203sub1", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2Current", "Read 6103sub1 error");
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6203sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_hold_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6203sub3", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2HoldCurrent", "Read 6103sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_hold_current_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6203sub3", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6203sub2", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2CurrentDigit", "Read 6103sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6203sub2", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_stepper2_hold_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6203sub4", G_TYPE_UINT);
    guint   res   = value ? (guint)g_value_get_uint(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetStepper2HoldCurrentDigit", "Read 6103sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", res));
    return TRUE;
}

static gboolean node_motor_object_set_stepper2_hold_current_digital_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_unsigned(motor, "6203sub4", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

// -------------------------------------------  Pump 1 --------------------------------------------------------------------------------

static gboolean node_motor_object_get_pump1_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6110sub1", G_TYPE_INT);
    gboolean         res   = value ? (gboolean)g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump1", "Read 6110sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump1_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6110sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump1_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6110sub2", G_TYPE_INT);
    gint             res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump1Mode", "Read 6110sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump1_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6110sub2", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump1_left_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6110sub3", G_TYPE_INT);
    gint             res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump1Left", "Read 6110sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump1_speed_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6110sub3", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump1_speed_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6110sub4", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump1Speed", "Read 6110sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump1_left_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6110sub4", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump1_interval_pulse_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6110sub4", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump1IntervalPulse", "Read 6110sub5 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump1_interval_pulse_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6110sub5", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump1_interval_time_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6110sub6", G_TYPE_INT);
    gint             res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump1IntervalTime", "Read 6110sub6 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump1_interval_time_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6110sub6", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

// -------------------------------------------  Pump 2 --------------------------------------------------------------------------------

static gboolean node_motor_object_get_pump2_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6210sub1", G_TYPE_INT);
    gboolean         res   = value ? (gboolean)g_value_get_int(value) : FALSE;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump2", "Read 6110sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump2_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6210sub1", (gint)value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump2_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6210sub2", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump2Mode", "Read 6110sub2 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump2_mode_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6210sub2", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump2_left_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6210sub3", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump2Left", "Read 6110sub3 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump2_speed_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6210sub3", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump2_speed_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6210sub4", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump2Speed", "Read 6110sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump2_left_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6210sub4", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump2_interval_pulse_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6210sub4", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump2IntervalPulse", "Read 6110sub5 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump2_interval_pulse_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6210sub5", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_object_get_pump2_interval_time_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    GValue *         value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6210sub6", G_TYPE_INT);
    gint             res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetPump2IntervalTime", "Read 6110sub6 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", res));
    return TRUE;
}

static gboolean node_motor_object_set_pump2_interval_time_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6210sub6", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_get_uart01_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *analog = NODE_MOTOR_OBJECT(user_data);
    GValue *         value  = node_object_read_value_type_transform(NODE_OBJECT(analog), "6000sub1", G_TYPE_STRING);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        if (value) mkt_value_free(value);
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Doppelmotor3.Error.GetUart1", "Read data string fail");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", g_value_get_string(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_motor_get_uart02_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *analog = NODE_MOTOR_OBJECT(user_data);
    GValue *         value  = node_object_read_value_type_transform(NODE_OBJECT(analog), "6010sub1", G_TYPE_STRING);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        if (value) mkt_value_free(value);
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Doppelmotor3.Error.GetUart1", "Read data string fail");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", g_value_get_string(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_motor_set_uart01_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, const gchar *value, gpointer user_data) {
    NodeMotorObject *analog = NODE_MOTOR_OBJECT(user_data);
    gboolean         res    = node_motor_object_write_string(analog, "6000sub1", value);

    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_set_uart02_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, const gchar *value, gpointer user_data) {
    NodeMotorObject *analog = NODE_MOTOR_OBJECT(user_data);
    gboolean         res    = node_motor_object_write_string(analog, "6010sub1", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_get_bautrate1_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6000sub4", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetBautrate1", "Read 6000sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", (guint)res));
    return TRUE;
}
static gboolean node_motor_set_bautrate1_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6000sub4", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static gboolean node_motor_get_bautrate2_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);

    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(motor), "6010sub4", G_TYPE_INT);
    gint    res   = value ? g_value_get_int(value) : 0;
    if (value) mkt_value_free(value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.doppelmotor3.error.GetBautrate2", "Read 6010sub4 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", (guint)res));
    return TRUE;
}
static gboolean node_motor_set_bautrate2_callback(NodesDoppelmotor3 *interface, GDBusMethodInvocation *invocation, gint value, gpointer user_data) {
    NodeMotorObject *motor = NODE_MOTOR_OBJECT(user_data);
    gboolean         res   = node_motor_object_write_integer(motor, "6010sub4", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        g_debug("connection closed");
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}

static void node_motor_object_init(NodeMotorObject *motor) {
    NodeMotorObjectPrivate *priv = node_motor_object_get_instance_private(motor);
    motor->priv                  = priv;
}

static void node_motor_object_constructed(GObject *object) {
    G_OBJECT_CLASS(node_motor_object_parent_class)->constructed(object);
    NodeObject *       data   = NODE_OBJECT(object);
    NodesDoppelmotor3 *motor3 = nodes_doppelmotor3_skeleton_new();
    nodes_object_skeleton_set_doppelmotor3(NODES_OBJECT_SKELETON(object), motor3);

    // ------------------------- Stepper1 -----------------------------------------------------
    g_signal_connect(motor3, "handle-get-stepper1-mode", G_CALLBACK(node_motor_object_get_stepper1_mode_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-mode", G_CALLBACK(node_motor_object_set_stepper1_mode_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-final-position", G_CALLBACK(node_motor_object_get_stepper1_final_position_callback), data);
    g_signal_connect(motor3, "handle-get-stepper1-endschalter-invert", G_CALLBACK(node_motor_object_get_stepper1_endschalter_invert_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-endschalter-invert", G_CALLBACK(node_motor_object_set_stepper1_endschalter_invert_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-diagnose-byte", G_CALLBACK(node_motor_object_get_stepper1_diagnose_byte_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-parameter", G_CALLBACK(node_motor_object_get_stepper1_parameter_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-parameter", G_CALLBACK(node_motor_object_set_stepper1_parameter_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-command-status", G_CALLBACK(node_motor_object_get_stepper1_command_status_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-command-status", G_CALLBACK(node_motor_object_set_stepper1_command_status_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-go-pos", G_CALLBACK(node_motor_object_get_stepper1_go_pos_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-go-pos", G_CALLBACK(node_motor_object_set_stepper1_go_pos_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-pos-old", G_CALLBACK(node_motor_object_get_stepper1_pos_old_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-max-pos", G_CALLBACK(node_motor_object_get_stepper1_max_pos_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-max-pos", G_CALLBACK(node_motor_object_set_stepper1_max_pos_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-stall-guard-flag", G_CALLBACK(node_motor_object_get_stepper1_stall_guard_flag_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-stall-guard-flag", G_CALLBACK(node_motor_object_set_stepper1_stall_guard_flag_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-on", G_CALLBACK(node_motor_object_get_stepper1_on_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-on", G_CALLBACK(node_motor_object_set_stepper1_on_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-delay", G_CALLBACK(node_motor_object_get_stepper1_delay_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-delay", G_CALLBACK(node_motor_object_set_stepper1_delay_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-current", G_CALLBACK(node_motor_object_get_stepper1_current_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-current", G_CALLBACK(node_motor_object_set_stepper1_current_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-hold-current", G_CALLBACK(node_motor_object_get_stepper1_hold_current_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-hold-current", G_CALLBACK(node_motor_object_set_stepper1_hold_current_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-current-digit", G_CALLBACK(node_motor_object_get_stepper1_current_digital_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-current-digit", G_CALLBACK(node_motor_object_set_stepper1_current_digital_callback), data);

    g_signal_connect(motor3, "handle-get-stepper1-hold-current-digit", G_CALLBACK(node_motor_object_get_stepper1_hold_current_digital_callback), data);
    g_signal_connect(motor3, "handle-set-stepper1-hold-current-digit", G_CALLBACK(node_motor_object_set_stepper1_hold_current_digital_callback), data);

    // ------------------------- Stepper2 -----------------------------------------------------
    g_signal_connect(motor3, "handle-get-stepper2-mode", G_CALLBACK(node_motor_object_get_stepper2_mode_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-mode", G_CALLBACK(node_motor_object_set_stepper2_mode_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-final-position", G_CALLBACK(node_motor_object_get_stepper2_final_position_callback), data);
    g_signal_connect(motor3, "handle-get-stepper2-endschalter-invert", G_CALLBACK(node_motor_object_get_stepper2_endschalter_invert_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-endschalter-invert", G_CALLBACK(node_motor_object_set_stepper2_endschalter_invert_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-diagnose-byte", G_CALLBACK(node_motor_object_get_stepper2_diagnose_byte_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-parameter", G_CALLBACK(node_motor_object_get_stepper2_parameter_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-parameter", G_CALLBACK(node_motor_object_set_stepper2_parameter_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-command-status", G_CALLBACK(node_motor_object_get_stepper2_command_status_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-command-status", G_CALLBACK(node_motor_object_set_stepper2_command_status_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-go-pos", G_CALLBACK(node_motor_object_get_stepper2_go_pos_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-go-pos", G_CALLBACK(node_motor_object_set_stepper2_go_pos_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-pos-old", G_CALLBACK(node_motor_object_get_stepper2_pos_old_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-max-pos", G_CALLBACK(node_motor_object_get_stepper2_max_pos_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-max-pos", G_CALLBACK(node_motor_object_set_stepper2_max_pos_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-stall-guard-flag", G_CALLBACK(node_motor_object_get_stepper2_stall_guard_flag_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-stall-guard-flag", G_CALLBACK(node_motor_object_set_stepper2_stall_guard_flag_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-on", G_CALLBACK(node_motor_object_get_stepper2_on_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-on", G_CALLBACK(node_motor_object_set_stepper2_on_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-delay", G_CALLBACK(node_motor_object_get_stepper2_delay_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-delay", G_CALLBACK(node_motor_object_set_stepper2_delay_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-current", G_CALLBACK(node_motor_object_get_stepper2_current_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-current", G_CALLBACK(node_motor_object_set_stepper2_current_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-hold-current", G_CALLBACK(node_motor_object_get_stepper2_hold_current_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-hold-current", G_CALLBACK(node_motor_object_set_stepper2_hold_current_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-current-digit", G_CALLBACK(node_motor_object_get_stepper2_current_digital_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-current-digit", G_CALLBACK(node_motor_object_set_stepper2_current_digital_callback), data);

    g_signal_connect(motor3, "handle-get-stepper2-hold-current-digit", G_CALLBACK(node_motor_object_get_stepper2_hold_current_digital_callback), data);
    g_signal_connect(motor3, "handle-set-stepper2-hold-current-digit", G_CALLBACK(node_motor_object_set_stepper2_hold_current_digital_callback), data);
    // Pump 1

    g_signal_connect(motor3, "handle-get-pump1", G_CALLBACK(node_motor_object_get_pump1_callback), data);
    g_signal_connect(motor3, "handle-set-pump1", G_CALLBACK(node_motor_object_set_pump1_callback), data);

    g_signal_connect(motor3, "handle-get-pump1-mode", G_CALLBACK(node_motor_object_get_pump1_mode_callback), data);
    g_signal_connect(motor3, "handle-set-pump1-mode", G_CALLBACK(node_motor_object_set_pump1_mode_callback), data);

    g_signal_connect(motor3, "handle-get-pump1-left", G_CALLBACK(node_motor_object_get_pump1_left_callback), data);
    g_signal_connect(motor3, "handle-set-pump1-left", G_CALLBACK(node_motor_object_set_pump1_left_callback), data);

    g_signal_connect(motor3, "handle-get-pump1-speed", G_CALLBACK(node_motor_object_get_pump1_speed_callback), data);
    g_signal_connect(motor3, "handle-set-pump1-speed", G_CALLBACK(node_motor_object_set_pump1_speed_callback), data);

    g_signal_connect(motor3, "handle-get-pump1-interval-pulse", G_CALLBACK(node_motor_object_get_pump1_interval_pulse_callback), data);
    g_signal_connect(motor3, "handle-set-pump1-interval-pulse", G_CALLBACK(node_motor_object_set_pump1_interval_pulse_callback), data);

    g_signal_connect(motor3, "handle-get-pump1-interval-time", G_CALLBACK(node_motor_object_get_pump1_interval_time_callback), data);
    g_signal_connect(motor3, "handle-set-pump1-interval-time", G_CALLBACK(node_motor_object_set_pump1_interval_time_callback), data);

    // Pump 2

    g_signal_connect(motor3, "handle-get-pump2", G_CALLBACK(node_motor_object_get_pump2_callback), data);
    g_signal_connect(motor3, "handle-set-pump2", G_CALLBACK(node_motor_object_set_pump2_callback), data);

    g_signal_connect(motor3, "handle-get-pump2-mode", G_CALLBACK(node_motor_object_get_pump2_mode_callback), data);
    g_signal_connect(motor3, "handle-set-pump2-mode", G_CALLBACK(node_motor_object_set_pump2_mode_callback), data);

    g_signal_connect(motor3, "handle-get-pump2-left", G_CALLBACK(node_motor_object_get_pump2_left_callback), data);
    g_signal_connect(motor3, "handle-set-pump2-left", G_CALLBACK(node_motor_object_set_pump2_left_callback), data);

    g_signal_connect(motor3, "handle-get-pump2-speed", G_CALLBACK(node_motor_object_get_pump2_speed_callback), data);
    g_signal_connect(motor3, "handle-set-pump2-speed", G_CALLBACK(node_motor_object_set_pump2_speed_callback), data);

    g_signal_connect(motor3, "handle-get-pump2-interval-pulse", G_CALLBACK(node_motor_object_get_pump2_interval_pulse_callback), data);
    g_signal_connect(motor3, "handle-set-pump2-interval-pulse", G_CALLBACK(node_motor_object_set_pump2_interval_pulse_callback), data);

    g_signal_connect(motor3, "handle-get-pump2-interval-time", G_CALLBACK(node_motor_object_get_pump2_interval_time_callback), data);
    g_signal_connect(motor3, "handle-set-pump2-interval-time", G_CALLBACK(node_motor_object_set_pump2_interval_time_callback), data);

    g_signal_connect(motor3, "handle-get-uart1", G_CALLBACK(node_motor_get_uart01_value_callback), data);
    g_signal_connect(motor3, "handle-get-uart2", G_CALLBACK(node_motor_get_uart02_value_callback), data);
    g_signal_connect(motor3, "handle-set-uart1", G_CALLBACK(node_motor_set_uart01_value_callback), data);
    g_signal_connect(motor3, "handle-set-uart2", G_CALLBACK(node_motor_set_uart02_value_callback), data);

    g_signal_connect(motor3, "handle-get-bautrate1", G_CALLBACK(node_motor_get_bautrate1_callback), data);
    g_signal_connect(motor3, "handle-get-bautrate2", G_CALLBACK(node_motor_get_bautrate2_callback), data);
    g_signal_connect(motor3, "handle-set-bautrate1", G_CALLBACK(node_motor_set_bautrate1_callback), data);
    g_signal_connect(motor3, "handle-set-bautrate2", G_CALLBACK(node_motor_set_bautrate2_callback), data);

    g_object_unref(motor3);
}

static void node_motor_object_finalize(GObject *object) {
    /* TODO: Add deinitalization code here */
    // NodeMotorObject *motor = NODE_MOTOR_OBJECT(object);
    G_OBJECT_CLASS(node_motor_object_parent_class)->finalize(object);
}

static void node_motor_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_MOTOR_OBJECT(object));
    // NodeMotorObject *motor = NODE_MOTOR_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_motor_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_MOTOR_OBJECT(object));
    // NodeMotorObject *motor = NODE_MOTOR_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_motor_object_class_init(NodeMotorObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // NodeObjectClass* parent_class = NODE_OBJECT_CLASS (klass);

    object_class->finalize     = node_motor_object_finalize;
    object_class->set_property = node_motor_object_set_property;
    object_class->get_property = node_motor_object_get_property;
    object_class->constructed  = node_motor_object_constructed;
}
