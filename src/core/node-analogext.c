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


#include "ultra-nodes-generated-code.h"


static gdouble node_analog_ext_read_double(NodeAnalogExtObject *analog, const gchar *index) {
    GValue *value = node_object_read_value(NODE_OBJECT(analog), index);
    g_return_val_if_fail(value != NULL, 0.0);
    gdouble dval = 0.0;
    if (value->g_type == G_TYPE_UINT)
        dval = (gdouble)g_value_get_uint(value);
    else if (value->g_type == G_TYPE_INT)
        dval = (gdouble)g_value_get_int(value);
    else if (value->g_type == G_TYPE_FLOAT)
        dval = (gdouble)g_value_get_float(value);
    else if (value->g_type == G_TYPE_DOUBLE)
        dval = (gdouble)g_value_get_float(value);
    else
        g_warning("Node analog get IN unknown type");
    mkt_value_free(value);
    return dval;
}

static gdouble node_analog_ext_read_uint(NodeAnalogExtObject *analog, const gchar *index) {
    GValue *value = node_object_read_value(NODE_OBJECT(analog), index);
    g_return_val_if_fail(value != NULL, 0.0);
    guint uval = 0;
    if (value->g_type == G_TYPE_UINT)
        uval = g_value_get_uint(value);
    else if (value->g_type == G_TYPE_INT)
        uval = (guint)g_value_get_int(value);
    else if (value->g_type == G_TYPE_FLOAT)
        uval = (guint)g_value_get_float(value);
    else if (value->g_type == G_TYPE_DOUBLE)
        uval = (guint)g_value_get_float(value);
    else
        g_warning("Node analog get IN unknown type");
    mkt_value_free(value);
    return uval;
}

static gboolean node_analog_ext_write_double(NodeAnalogExtObject *analog, const gchar *index, gdouble val) {
    GValue value = {0};
    g_value_init(&value, G_TYPE_UINT);
    g_value_set_uint(&value, (guint)val);
    gboolean ret = FALSE;
    if (node_object_write_value(NODE_OBJECT(analog), index, &value)) ret = TRUE;
    g_value_unset(&value);
    return ret;
}

static gboolean node_digital_object_get_nout_callback(NodesAnalogext *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogExtObject *analog = NODE_ANALOGEXT_OBJECT(user_data);
    guint                nout   = node_analog_ext_read_uint(NODE_ANALOGEXT_OBJECT(analog), "6411sub0");
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(u)", nout));
    return TRUE;
}

static gboolean node_digital_object_set_nout_value_callback(NodesAnalogext *interface, GDBusMethodInvocation *invocation, guint out, gdouble value, gpointer user_data) {
    NodeAnalogExtObject *analog  = NODE_ANALOGEXT_OBJECT(user_data);
    gchar *              address = g_strdup_printf("6411sub%X", out);
    gboolean             ret     = node_analog_ext_write_double(analog, address, value);
    g_free(address);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", ret));
    return TRUE;
}

static gboolean node_digital_object_get_nout_value_callback(NodesAnalogext *interface, GDBusMethodInvocation *invocation, guint out, gpointer user_data) {
    NodeAnalogExtObject *analog  = NODE_ANALOGEXT_OBJECT(user_data);
    gchar *              address = g_strdup_printf("6411sub%X", out);
    gdouble              value   = node_analog_ext_read_double(analog, address);
    g_free(address);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", value));
    return TRUE;
}


static void ect_constructed(GObject *object) {
    G_OBJECT_CLASS(node_analog_ext_object_parent_class)->constructed(object);
    NodeObject *    data      = NODE_OBJECT(object);
    NodesAnalogext *analogext = nodes_analogext_skeleton_new();
    nodes_object_skeleton_set_analogext(NODES_OBJECT_SKELETON(object), analogext);
    g_signal_connect(analogext, "handle-get-nout", G_CALLBACK(node_digital_object_get_nout_callback), data);
    g_signal_connect(analogext, "handle-set-value", G_CALLBACK(node_digital_object_set_nout_value_callback), data);
    g_signal_connect(analogext, "handle-get-value", G_CALLBACK(node_digital_object_get_nout_value_callback), data);
    g_object_unref(analogext);
}
