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

#include "node-analog-object.h"
#include "node-device-object.h"
#include <mkt-value.h>

struct _NodeAnalogObjectPrivate {
    gdouble in01;
};

G_DEFINE_TYPE_WITH_PRIVATE(NodeAnalogObject, node_analog_object, NODE_TYPE_OBJECT);

enum {
    PROP_ANALOG_0,

};

static gdouble node_analog_read_double(NodeAnalogObject *analog, const gchar *index) {
    GValue *value = node_object_read_value(NODE_OBJECT(analog), index);
    if (value == NULL) {
        return 0.0;
    }
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
        g_warning("node analog get IN unknown type");
    mkt_value_free(value);
    return dval;
}

static gboolean node_analog_write_double(NodeAnalogObject *analog, const gchar *index, gdouble val) {
    GValue value = {0};
    g_value_init(&value, G_TYPE_UINT);
    g_value_set_uint(&value, (guint)val);
    gboolean ret = FALSE;
    if (node_object_write_value(NODE_OBJECT(analog), index, &value)) ret = TRUE;
    g_value_unset(&value);
    return ret;
}

static gboolean node_analog_get_in01_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub1", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN01", "Read 6101sub1 error");
        return TRUE;
    }
    nodes_analog1_set_in01(interface, g_value_get_double(value));
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)",g_value_get_double(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_get_in02_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub2", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN02", "Read 6101sub2 error");
        return TRUE;
    }
    nodes_analog1_set_in02(interface, g_value_get_double(value));
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)",g_value_get_double(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_get_in03_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub3", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN03", "Read 6101sub3 error");
        return TRUE;
    }
    nodes_analog1_set_in03(interface, g_value_get_double(value));
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)",g_value_get_double(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_get_in04_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub4", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN04", "Read 6101sub4 error");
        return TRUE;
    }
    nodes_analog1_set_in04(interface,g_value_get_double(value));
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)",g_value_get_double(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_get_in05_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6111sub1", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN05", "Read 6111sub1 error");
        return TRUE;
    }
    nodes_analog1_set_in05(interface,g_value_get_double(value));
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)",g_value_get_double(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_read_inputs_values_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub1", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN01", "Read 6101sub1 error");
        return TRUE;
    }
    nodes_analog1_set_in01(interface, g_value_get_double(value));
    mkt_value_free(value);
    value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub2", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN02", "Read 6101sub2 error");
        return TRUE;
    }
    nodes_analog1_set_in02(interface, g_value_get_double(value));
    mkt_value_free(value);
    value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub3", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN02", "Read 6101sub3 error");
        return TRUE;
    }
    nodes_analog1_set_in03(interface, g_value_get_double(value));
    mkt_value_free(value);

    value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6101sub4", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN02", "Read 6101sub4 error");
        return TRUE;
    }
    nodes_analog1_set_in04(interface, g_value_get_double(value));
    mkt_value_free(value);

    value = node_object_read_value_type_transform(NODE_OBJECT(analog), "6111sub1", G_TYPE_DOUBLE);
    if(value==NULL){
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.analog1.error.GetIN05", "Read 6111sub1 error");
        return TRUE;
    }
    nodes_analog1_set_in05(interface, g_value_get_double(value));
    mkt_value_free(value);

    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", TRUE));
    return TRUE;
}

// Set and Get Analog output value

static gboolean node_analog_set_out_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gdouble value, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    gboolean          res    = node_analog_write_double(analog, "6120sub1", value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", res));
    return TRUE;
}
static gboolean node_analog_get_out_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    gdouble           value  = node_analog_read_double(analog, "6120sub1");
    nodes_analog1_set_in05(interface, value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", value));
    return TRUE;
}

// Get Analog1 temperature value

static gboolean node_analog_get_temp01_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    gdouble           temp   = node_analog_read_double(analog, "6020sub1");
    nodes_analog1_set_temperatur01(interface, temp / 10.0);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", temp));
    return TRUE;
}

static gboolean node_analog_get_temp02_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    gdouble           temp   = node_analog_read_double(analog, "6020sub2");
    nodes_analog1_set_temperatur02(interface, temp / 10.0);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", temp));
    return TRUE;
}

static gboolean node_analog_get_temp03_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    gdouble           temp   = node_analog_read_double(analog, "6020sub3");
    nodes_analog1_set_temperatur03(interface, temp / 10.0);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", temp));
    return TRUE;
}

// Set and Get Analog1 uart data , bautrate value

static gboolean node_analog_get_uart01_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *          value  = node_object_read_value_type_transform(NODE_OBJECT(analog), "6000sub1", G_TYPE_STRING);
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.GetUart1", "Read string at address 6000sub1");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", g_value_get_string(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_get_uart02_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    NodeAnalogObject *analog = NODE_ANALOG_OBJECT(user_data);
    GValue *          value  = node_object_read_value_type_transform(NODE_OBJECT(analog), "6010sub1", G_TYPE_STRING);
    if (value == NULL) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.GetUart1", "Read string at address 6000sub1");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", g_value_get_string(value)));
    mkt_value_free(value);
    return TRUE;
}

static gboolean node_analog_set_uart01_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, const gchar *value, gpointer user_data) {
    NodeAnalogObject *analog     = NODE_ANALOG_OBJECT(user_data);
    GValue *          temp_value = mkt_value_new(G_TYPE_STRING);
    g_value_set_string(temp_value, value);
    gboolean result = node_object_write_value(NODE_OBJECT(analog), "6000sub1", temp_value);
    mkt_value_free(temp_value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    if (!result) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.SetUart1", "Write a string to 6000sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

static gboolean node_analog_set_uart02_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, const gchar *value, gpointer user_data) {
    NodeAnalogObject *analog     = NODE_ANALOG_OBJECT(user_data);
    GValue *          temp_value = mkt_value_new(G_TYPE_STRING);
    g_value_set_string(temp_value, value);
    gboolean result = node_object_write_value(NODE_OBJECT(analog), "6010sub1", temp_value);
    mkt_value_free(temp_value);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }

    if (!result) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.SetUart1", "Write a string to 6010sub1 error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

static gboolean node_analog_get_bautrate1_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.GetBautrate1", "dummy object method");
    return TRUE;
}

static gboolean node_analog_get_bautrate2_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.GetBautrate2", "dummy method");
    return TRUE;
}

static gboolean node_analog_set_bautrate1_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.SetBautrate1", "dummy method");
    return TRUE;
}

static gboolean node_analog_set_bautrate2_value_callback(NodesAnalog1 *interface, GDBusMethodInvocation *invocation, guint value, gpointer user_data) {
    g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.Analog1.Error.SetBautrate2", "empty method");
    return TRUE;
}

static void node_analog_object_init(NodeAnalogObject *analog) {
    NodeAnalogObjectPrivate *priv = node_analog_object_get_instance_private(analog);
    analog->priv                  = priv;
}

static void node_analog_object_constructed(GObject *object) {
    G_OBJECT_CLASS(node_analog_object_parent_class)->constructed(object);
    NodeAnalogObject *data    = NODE_ANALOG_OBJECT(object);
    NodesAnalog1 *    analog1 = nodes_analog1_skeleton_new();
    nodes_object_skeleton_set_analog1(NODES_OBJECT_SKELETON(object), analog1);
    g_signal_connect(analog1, "handle-get-in1", G_CALLBACK(node_analog_get_in01_value_callback), data);
    g_signal_connect(analog1, "handle-get-in2", G_CALLBACK(node_analog_get_in02_value_callback), data);
    g_signal_connect(analog1, "handle-get-in3", G_CALLBACK(node_analog_get_in03_value_callback), data);
    g_signal_connect(analog1, "handle-get-in4", G_CALLBACK(node_analog_get_in04_value_callback), data);
    g_signal_connect(analog1, "handle-get-in5", G_CALLBACK(node_analog_get_in05_value_callback), data);
    g_signal_connect(analog1, "handle-read-inputs", G_CALLBACK(node_analog_read_inputs_values_callback), data);
    g_signal_connect(analog1, "handle-set-out", G_CALLBACK(node_analog_set_out_value_callback), data);
    g_signal_connect(analog1, "handle-get-out", G_CALLBACK(node_analog_get_out_value_callback), data);

    g_signal_connect(analog1, "handle-get-temp1", G_CALLBACK(node_analog_get_temp01_value_callback), data);
    g_signal_connect(analog1, "handle-get-temp2", G_CALLBACK(node_analog_get_temp02_value_callback), data);
    g_signal_connect(analog1, "handle-get-temp3", G_CALLBACK(node_analog_get_temp03_value_callback), data);

    g_signal_connect(analog1, "handle-get-uart1", G_CALLBACK(node_analog_get_uart01_value_callback), data);
    g_signal_connect(analog1, "handle-get-uart2", G_CALLBACK(node_analog_get_uart02_value_callback), data);
    g_signal_connect(analog1, "handle-set-uart1", G_CALLBACK(node_analog_set_uart01_value_callback), data);
    g_signal_connect(analog1, "handle-set-uart2", G_CALLBACK(node_analog_set_uart02_value_callback), data);

    g_signal_connect(analog1, "handle-get-bautrate1", G_CALLBACK(node_analog_get_bautrate1_value_callback), data);
    g_signal_connect(analog1, "handle-get-bautrate2", G_CALLBACK(node_analog_get_bautrate2_value_callback), data);
    g_signal_connect(analog1, "handle-set-bautrate1", G_CALLBACK(node_analog_set_bautrate1_value_callback), data);
    g_signal_connect(analog1, "handle-set-bautrate2", G_CALLBACK(node_analog_set_bautrate2_value_callback), data);
    g_object_unref(analog1);
}

static void node_analog_object_finalize(GObject *object) {
    /* TODO: Add deinitalization code here */
    // NodeAnalogObject *analog = NODE_ANALOG_OBJECT(object);
    G_OBJECT_CLASS(node_analog_object_parent_class)->finalize(object);
}

static void node_analog_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_ANALOG_OBJECT(object));

    // NodeAnalogObject *analog = NODE_ANALOG_OBJECT(object);
    switch (prop_id) {
    /*case PROP_ANALOG_IN01:
            analog->priv->in01 = g_value_get_double(value);
            break;
    case PROP_ANALOG_IN02:
            analog->priv->in02 = g_value_get_double(value);
            break;
    case PROP_ANALOG_IN03:
            analog->priv->in03 = g_value_get_double(value);
            break;
    case PROP_ANALOG_IN04:
            analog->priv->in04 = g_value_get_double(value);
            break;
    case PROP_ANALOG_IN05:
            analog->priv->in05 = g_value_get_double(value);
            break;
    case PROP_ANALOG_OUT01:
            analog->priv->out01 = g_value_get_double(value);
            node_analog_write_double(analog,"6120sub1",(analog->priv->out01/analog->priv->out01_slope-analog->priv->out01_intercept));
            break;
    case PROP_ANALOG_IN01_SLOPE:
            analog->priv->in01_slope =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN01_INTERCEPT:
            analog->priv->in01_intercept =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN02_SLOPE:
            analog->priv->in02_slope =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN02_INTERCEPT:
            analog->priv->in02_intercept =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN03_SLOPE:
            analog->priv->in03_slope =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN03_INTERCEPT:
            analog->priv->in03_intercept =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN04_SLOPE:
            analog->priv->in04_slope =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN04_INTERCEPT:
            analog->priv->in04_intercept =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN05_SLOPE:
            analog->priv->in05_slope =  g_value_get_double(value);
            break;
    case PROP_ANALOG_IN05_INTERCEPT:
            analog->priv->in05_intercept =  g_value_get_double(value);
            break;
    case PROP_ANALOG_UART_DATA01:
            if(analog->priv->data01)g_free(analog->priv->data01);
            analog->priv->data01 = g_value_dup_string(value);
            //node_analog_object_send_uart_data(analog,analog->priv->linkUartData01,analog->priv->data01);
            break;
    case PROP_ANALOG_UART_DATA02:
            if(analog->priv->data02)g_free(analog->priv->data02);
            analog->priv->data02 = g_value_dup_string(value);
            //node_analog_object_send_uart_data(analog,analog->priv->linkUartData02,analog->priv->data02);
            break;
    case PROP_ANALOG_BAUTRATE01:
            analog->priv->bautrate01 =  g_value_get_int(value);

            break;
    case PROP_ANALOG_BAUTRATE02:
            analog->priv->bautrate02 =  g_value_get_int(value);
            break;
            */
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_analog_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_ANALOG_OBJECT(object));
    // NodeAnalogObject *analog = NODE_ANALOG_OBJECT(object);
    switch (prop_id) {
    /*case PROP_ANALOG_IN01:
            g_value_set_double(value,analog->priv->in01);
            break;
    case PROP_ANALOG_IN02:
            g_value_set_double(value,analog->priv->in02);
            break;
    case PROP_ANALOG_IN03:
            g_value_set_double(value,analog->priv->in03);
            break;
    case PROP_ANALOG_IN04:
            g_value_set_double(value,analog->priv->in04);
            break;
    case PROP_ANALOG_IN05:
            g_value_set_double(value,analog->priv->in05);
            break;
    case PROP_ANALOG_OUT01:
            g_value_set_double(value,analog->priv->out01);
            break;
    case PROP_ANALOG_IN01_SLOPE:
            g_value_set_double(value,analog->priv->in01_slope);
            break;
    case PROP_ANALOG_IN01_INTERCEPT:
            g_value_set_double(value,analog->priv->in01_intercept);
            break;
    case PROP_ANALOG_IN02_SLOPE:
            g_value_set_double(value,analog->priv->in02_slope);
            break;
    case PROP_ANALOG_IN02_INTERCEPT:
            g_value_set_double(value,analog->priv->in02_intercept);
            break;
    case PROP_ANALOG_IN03_SLOPE:
            g_value_set_double(value,analog->priv->in03_slope);
            break;
    case PROP_ANALOG_IN03_INTERCEPT:
            g_value_set_double(value,analog->priv->in03_intercept);
            break;
    case PROP_ANALOG_IN04_SLOPE:
            g_value_set_double(value,analog->priv->in04_slope);
            break;
    case PROP_ANALOG_IN04_INTERCEPT:
            g_value_set_double(value,analog->priv->in04_intercept);
            break;
    case PROP_ANALOG_IN05_SLOPE:
            g_value_set_double(value,analog->priv->in05_slope);
            break;
    case PROP_ANALOG_IN05_INTERCEPT:
            g_value_set_double(value,analog->priv->in05_intercept);
            break;
    case PROP_ANALOG_UART_DATA01:
            g_value_set_string(value,analog->priv->data01 );
            break;
    case PROP_ANALOG_UART_DATA02:
            g_value_set_string(value,analog->priv->data02 );
            break;
    case PROP_ANALOG_BAUTRATE01:
            g_value_set_int(value,analog->priv->bautrate01);
            break;
    case PROP_ANALOG_BAUTRATE02:
            g_value_set_int(value,analog->priv->bautrate02);
            break;
    */
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_analog_object_class_init(NodeAnalogObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // NodeObjectClass* parent_class = NODE_OBJECT_CLASS (klass);

    object_class->finalize     = node_analog_object_finalize;
    object_class->set_property = node_analog_object_set_property;
    object_class->get_property = node_analog_object_get_property;
    object_class->constructed  = node_analog_object_constructed;
}
