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

#include "node-digital-object.h"
//#include "node-atom.h"
//#include "node-flags.h"
#include "node-device-object.h"

struct _NodeDigitalObjectPrivate {
    guint    digitVal;   // Digital node 16 bit IN value
    guint    digitINVal; // Digital node 16 bit OUT value
    gboolean OUT00;
    gboolean OUT01;
    gboolean OUT02;
    gboolean OUT03;
    gboolean OUT04;
    gboolean OUT05;
    gboolean OUT06;
    gboolean OUT07;
    gboolean OUT08;
    gboolean OUT09;
    gboolean OUT10;
    gboolean OUT11;
    gboolean OUT12;
    gboolean OUT13;
    gboolean OUT14;
    gboolean OUT15;

    gboolean IN00;
    gboolean IN01;
    gboolean IN02;
    gboolean IN03;
    gboolean IN04;
    gboolean IN05;
    gboolean IN06;
    gboolean IN07;
    gboolean IN08;
    gboolean IN09;
    gboolean IN10;
    gboolean IN11;
    gboolean IN12;
    gboolean IN13;
    gboolean IN14;
    gboolean IN15;
    /*	StNodeAtom *linkIn;
            StNodeAtom *linkOut;
            StNodeAtom *linkOutIn;*/
};

G_DEFINE_TYPE_WITH_PRIVATE(NodeDigitalObject, node_digital_object, NODE_TYPE_OBJECT);

enum {
    PROP_0,
    PROP_DIGOUT00,
    PROP_DIGOUT01,
    PROP_DIGOUT02,
    PROP_DIGOUT03,
    PROP_DIGOUT04,
    PROP_DIGOUT05,
    PROP_DIGOUT06,
    PROP_DIGOUT07,
    PROP_DIGOUT08,
    PROP_DIGOUT09,
    PROP_DIGOUT10,
    PROP_DIGOUT11,
    PROP_DIGOUT12,
    PROP_DIGOUT13,
    PROP_DIGOUT14,
    PROP_DIGOUT15,
    PROP_DIGIN00,
    PROP_DIGIN01,
    PROP_DIGIN02,
    PROP_DIGIN03,
    PROP_DIGIN04,
    PROP_DIGIN05,
    PROP_DIGIN06,
    PROP_DIGIN07,
    PROP_DIGIN08,
    PROP_DIGIN09,
    PROP_DIGIN10,
    PROP_DIGIN11,
    PROP_DIGIN12,
    PROP_DIGIN13,
    PROP_DIGIN14,
    PROP_DIGIN15
};

static void node_rigital_reload_out_values(NodeDigitalObject *dout) {
    NodesDigital16 *digital16 = nodes_object_get_digital16(NODES_OBJECT(dout));
    dout->priv->OUT00         = ((1 << 0) & (dout->priv->digitVal) ? 1 : 0);
    nodes_digital16_set_out00(digital16, dout->priv->OUT00);
    dout->priv->OUT01 = 1 & (dout->priv->digitVal) >> 1;
    nodes_digital16_set_out01(digital16, dout->priv->OUT01);
    dout->priv->OUT02 = 1 & (dout->priv->digitVal) >> 2;
    nodes_digital16_set_out02(digital16, dout->priv->OUT02);
    dout->priv->OUT03 = 1 & (dout->priv->digitVal) >> 3;
    nodes_digital16_set_out03(digital16, dout->priv->OUT03);
    dout->priv->OUT04 = 1 & (dout->priv->digitVal) >> 4;
    nodes_digital16_set_out04(digital16, dout->priv->OUT04);
    dout->priv->OUT05 = 1 & (dout->priv->digitVal) >> 5;
    nodes_digital16_set_out05(digital16, dout->priv->OUT05);
    dout->priv->OUT06 = 1 & (dout->priv->digitVal) >> 6;
    nodes_digital16_set_out06(digital16, dout->priv->OUT06);
    dout->priv->OUT07 = 1 & (dout->priv->digitVal) >> 7;
    nodes_digital16_set_out07(digital16, dout->priv->OUT07);
    dout->priv->OUT08 = 1 & (dout->priv->digitVal) >> 8;
    nodes_digital16_set_out08(digital16, dout->priv->OUT08);
    dout->priv->OUT09 = 1 & (dout->priv->digitVal) >> 9;
    nodes_digital16_set_out09(digital16, dout->priv->OUT09);
    dout->priv->OUT10 = 1 & (dout->priv->digitVal) >> 10;
    nodes_digital16_set_out10(digital16, dout->priv->OUT10);
    dout->priv->OUT11 = 1 & (dout->priv->digitVal) >> 11;
    nodes_digital16_set_out11(digital16, dout->priv->OUT11);
    dout->priv->OUT12 = 1 & (dout->priv->digitVal) >> 12;
    nodes_digital16_set_out12(digital16, dout->priv->OUT12);
    dout->priv->OUT13 = 1 & (dout->priv->digitVal) >> 13;
    nodes_digital16_set_out13(digital16, dout->priv->OUT13);
    dout->priv->OUT14 = 1 & (dout->priv->digitVal) >> 14;
    nodes_digital16_set_out14(digital16, dout->priv->OUT14);
    dout->priv->OUT15 = 1 & (dout->priv->digitVal) >> 15;
    nodes_digital16_set_out15(digital16, dout->priv->OUT15);
}

static gboolean node_digital_object_set_value(NodeDigitalObject *dout, gboolean val, guint bit) {
    if (G_MAXUINT32 == dout->priv->digitVal) {
        GValue *value = node_object_read_value(NODE_OBJECT(dout), "6300sub1");
        if (value == NULL) {
            return FALSE;
        }
        if (value->g_type == G_TYPE_UINT)
            dout->priv->digitVal = (guint)g_value_get_uint(value);
        else if (value->g_type == G_TYPE_INT)
            dout->priv->digitVal = (guint)g_value_get_int(value);
        mkt_value_free(value);
    }
    val &= 1;
    dout->priv->digitVal = (dout->priv->digitVal & ~(1 << bit)) | (val << bit);
    GValue value         = {0};
    g_value_init(&value, G_TYPE_UINT);
    g_value_set_uint(&value, dout->priv->digitVal);
    gboolean ret = FALSE;
    if (node_object_write_value(NODE_OBJECT(dout), "6300sub1", &value)) ret = TRUE;
    g_value_unset(&value);
    node_rigital_reload_out_values(dout);
    return ret;
}

static gboolean node_digital_object_get_out_value(NodeDigitalObject *dout) {
    GValue *value = node_object_read_value(NODE_OBJECT(dout), "6300sub1");
    if (value == NULL) {
        return FALSE;
    }
    NodesDigital16 *digital16 = nodes_object_get_digital16(NODES_OBJECT(dout));

    if (value->g_type == G_TYPE_UINT)
        dout->priv->digitVal = (guint)g_value_get_uint(value);
    else if (value->g_type == G_TYPE_INT)
        dout->priv->digitVal = (guint)g_value_get_int(value);
    mkt_value_free(value);
    nodes_digital16_set_out_val(digital16, dout->priv->digitVal);
    node_rigital_reload_out_values(dout);

    return TRUE;
}

static gboolean node_digital_object_get_out_number(NodeDigitalObject *dout, guint number) {
    switch (number) {
    case 0:
        return dout->priv->OUT00;
    case 1:
        return dout->priv->OUT01;
    case 2:
        return dout->priv->OUT02;
    case 3:
        return dout->priv->OUT03;
    case 4:
        return dout->priv->OUT04;
    case 5:
        return dout->priv->OUT05;
    case 6:
        return dout->priv->OUT06;
    case 7:
        return dout->priv->OUT07;
    case 8:
        return dout->priv->OUT08;
    case 9:
        return dout->priv->OUT09;
    case 10:
        return dout->priv->OUT10;
    case 11:
        return dout->priv->OUT11;
    case 12:
        return dout->priv->OUT12;
    case 13:
        return dout->priv->OUT13;
    case 14:
        return dout->priv->OUT14;
    case 15:
        return dout->priv->OUT15;
    }
    return FALSE;
}

static gboolean node_digital_object_get_in_value(NodeDigitalObject *dout) {
    GValue *value = node_object_read_value(NODE_OBJECT(dout), "6100sub1");
    g_return_val_if_fail(value != NULL, FALSE);

    NodesDigital16 *digital16 = nodes_object_get_digital16(NODES_OBJECT(dout));

    if (value->g_type == G_TYPE_UINT)
        dout->priv->digitINVal = (guint)g_value_get_uint(value);
    else if (value->g_type == G_TYPE_INT)
        dout->priv->digitINVal = (guint)g_value_get_int(value);
    mkt_value_free(value);
    nodes_digital16_set_in_val(digital16, dout->priv->digitINVal);
    dout->priv->IN00 = 1 & (dout->priv->digitINVal) >> 0;
    nodes_digital16_set_in00(digital16, dout->priv->IN00);
    dout->priv->IN01 = 1 & (dout->priv->digitINVal) >> 1;
    nodes_digital16_set_in01(digital16, dout->priv->IN01);
    dout->priv->IN02 = 1 & (dout->priv->digitINVal) >> 2;
    nodes_digital16_set_in02(digital16, dout->priv->IN02);
    dout->priv->IN03 = 1 & (dout->priv->digitINVal) >> 3;
    nodes_digital16_set_in03(digital16, dout->priv->IN03);
    dout->priv->IN04 = 1 & (dout->priv->digitINVal) >> 4;
    nodes_digital16_set_in04(digital16, dout->priv->IN04);
    dout->priv->IN05 = 1 & (dout->priv->digitINVal) >> 5;
    nodes_digital16_set_in05(digital16, dout->priv->IN05);
    dout->priv->IN06 = 1 & (dout->priv->digitINVal) >> 6;
    nodes_digital16_set_in06(digital16, dout->priv->IN06);
    dout->priv->IN07 = 1 & (dout->priv->digitINVal) >> 7;
    nodes_digital16_set_in07(digital16, dout->priv->IN07);
    dout->priv->IN08 = 1 & (dout->priv->digitINVal) >> 8;
    nodes_digital16_set_in08(digital16, dout->priv->IN08);
    dout->priv->IN09 = 1 & (dout->priv->digitINVal) >> 9;
    nodes_digital16_set_in09(digital16, dout->priv->IN09);
    dout->priv->IN10 = 1 & (dout->priv->digitINVal) >> 10;
    nodes_digital16_set_in10(digital16, dout->priv->IN10);
    dout->priv->IN11 = 1 & (dout->priv->digitINVal) >> 11;
    nodes_digital16_set_in11(digital16, dout->priv->IN11);
    dout->priv->IN12 = 1 & (dout->priv->digitINVal) >> 12;
    nodes_digital16_set_in12(digital16, dout->priv->IN12);
    dout->priv->IN13 = 1 & (dout->priv->digitINVal) >> 13;
    nodes_digital16_set_in13(digital16, dout->priv->IN13);
    dout->priv->IN14 = 1 & (dout->priv->digitINVal) >> 14;
    nodes_digital16_set_in14(digital16, dout->priv->IN14);
    dout->priv->IN15 = 1 & (dout->priv->digitINVal) >> 15;
    nodes_digital16_set_in15(digital16, dout->priv->IN15);
    return TRUE;
}

static gboolean node_digital_object_get_in_number(NodeDigitalObject *dout, guint number) {
    switch (number) {
    case 0:
        return dout->priv->IN00;
    case 1:
        return dout->priv->IN01;
    case 2:
        return dout->priv->IN02;
    case 3:
        return dout->priv->IN03;
    case 4:
        return dout->priv->IN04;
    case 5:
        return dout->priv->IN05;
    case 6:
        return dout->priv->IN06;
    case 7:
        return dout->priv->IN07;
    case 8:
        return dout->priv->IN08;
    case 9:
        return dout->priv->IN09;
    case 10:
        return dout->priv->IN10;
    case 11:
        return dout->priv->IN11;
    case 12:
        return dout->priv->IN12;
    case 13:
        return dout->priv->IN13;
    case 14:
        return dout->priv->IN14;
    case 15:
        return dout->priv->IN15;
    }
    return FALSE;
}

static gboolean node_digital_object_get_in_value_callback(NodesDigital16 *interface, GDBusMethodInvocation *invocation, guint number, gpointer user_data) {
    NodeDigitalObject *digital = NODE_DIGITAL_OBJECT(user_data);
    gboolean           result  = node_digital_object_get_in_value(digital);

    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    if (!result) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.simple.GtDigitalIn", "Read digital IN error");
        return TRUE;
    }
    if (number > 15) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.simple.GtDigitalIn", "Digital1 In > 15");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", node_digital_object_get_in_number(digital, number)));
    return TRUE;
}

static gboolean node_digital_object_get_out_value_callback(NodesDigital16 *interface, GDBusMethodInvocation *invocation, guint number, gpointer user_data) {
    NodeDigitalObject *digital = NODE_DIGITAL_OBJECT(user_data);
    gboolean           result  = node_digital_object_get_out_value(digital);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    if (!result) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.simple.GtDigitalOut", "Read digital IN error");
        return TRUE;
    }
    if (number > 15) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.simple.GtDigitalOut", "Digital1 In > 15");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", node_digital_object_get_out_number(digital, number)));
    return TRUE;
}

static gboolean node_digital_object_set_out_value_callback(NodesDigital16 *interface, GDBusMethodInvocation *invocation, guint number, gboolean value, gpointer user_data) {
    NodeDigitalObject *digital = NODE_DIGITAL_OBJECT(user_data);
    gboolean           result  = node_digital_object_set_value(digital, value, number);
    if (NULL == g_dbus_method_invocation_get_connection(invocation) || g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) {
        return FALSE;
    }
    if (!result) {
        g_dbus_method_invocation_return_dbus_error(invocation, "com.lar.nodes.simple.GtDigitalIn", "Read digital IN error");
        return TRUE;
    }
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

static void node_digital_object_init(NodeDigitalObject *digital) {
    NodeDigitalObjectPrivate *priv = node_digital_object_get_instance_private(digital);

    priv->OUT00      = FALSE;
    priv->OUT01      = FALSE;
    priv->OUT02      = FALSE;
    priv->OUT03      = FALSE;
    priv->OUT04      = FALSE;
    priv->OUT05      = FALSE;
    priv->OUT06      = FALSE;
    priv->OUT07      = FALSE;
    priv->OUT08      = FALSE;
    priv->OUT09      = FALSE;
    priv->OUT10      = FALSE;
    priv->OUT11      = FALSE;
    priv->OUT12      = FALSE;
    priv->OUT13      = FALSE;
    priv->OUT14      = FALSE;
    priv->OUT15      = FALSE;
    priv->digitVal   = G_MAXUINT32;
    priv->digitINVal = 0;
    digital->priv    = priv;

    /* TODO: Add initialization code here */
}

static void node_digital_object_constructed(GObject *object) {
    G_OBJECT_CLASS(node_digital_object_parent_class)->constructed(object);
    NodeObject *    data      = NODE_OBJECT(object);
    NodesDigital16 *digital16 = nodes_digital16_skeleton_new();
    nodes_object_skeleton_set_digital16(NODES_OBJECT_SKELETON(object), digital16);
    g_signal_connect(digital16, "handle-get-digital-in", G_CALLBACK(node_digital_object_get_in_value_callback), data);
    g_signal_connect(digital16, "handle-get-digital-out", G_CALLBACK(node_digital_object_get_out_value_callback), data);
    g_signal_connect(digital16, "handle-set-digital-out", G_CALLBACK(node_digital_object_set_out_value_callback), data);
    // g_object_unref(digital16);
}

static void node_digital_object_finalize(GObject *object) { G_OBJECT_CLASS(node_digital_object_parent_class)->finalize(object); }

static void node_digital_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_DIGITAL_OBJECT(object));
    // NodeDigitalObject *dout = NODE_DIGITAL_OBJECT(object);
    switch (prop_id) {
    /*case PROP_DIGOUT00:
            dout->priv->OUT00 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT00,0);
            break;
    case PROP_DIGOUT01:
            dout->priv->OUT01 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT01,1);
            break;
    case PROP_DIGOUT02:
            dout->priv->OUT02 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT02,2);
            break;
    case PROP_DIGOUT03:
            dout->priv->OUT03 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT03,3);
            break;
    case PROP_DIGOUT04:
            dout->priv->OUT04 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT04,4);
            break;
    case PROP_DIGOUT05:
            dout->priv->OUT05 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT05,5);
            break;
    case PROP_DIGOUT06:
            dout->priv->OUT06 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT06,6);
            break;
    case PROP_DIGOUT07:
            dout->priv->OUT07 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT07,7);
            break;
    case PROP_DIGOUT08:
            dout->priv->OUT08 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT08,8);
            break;
    case PROP_DIGOUT09:
            dout->priv->OUT09 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT09,9);
            break;
    case PROP_DIGOUT10:
            dout->priv->OUT10 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT10,10);
            break;
    case PROP_DIGOUT11:
            dout->priv->OUT11 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT11,11);
            break;
    case PROP_DIGOUT12:
            dout->priv->OUT12 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT12,12);
            break;
    case PROP_DIGOUT13:
            dout->priv->OUT13 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT13,13);
            break;
    case PROP_DIGOUT14:
            dout->priv->OUT14 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT14,14);
            break;
    case PROP_DIGOUT15:
            dout->priv->OUT15 = g_value_get_boolean (value);
            node_digital_object_set_value(dout,dout->priv->OUT15,15);
            break;*/
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_digital_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_DIGITAL_OBJECT(object));
    // NodeDigitalObject *dout = NODE_DIGITAL_OBJECT(object);
    switch (prop_id) {
    /*case PROP_DIGOUT00:

            g_value_set_boolean(value,dout->priv->OUT00);
            break;
    case PROP_DIGOUT01:
            g_value_set_boolean(value,dout->priv->OUT01);
            break;
    case PROP_DIGOUT02:
            g_value_set_boolean(value,dout->priv->OUT02);
            break;
    case PROP_DIGOUT03:
            g_value_set_boolean(value,dout->priv->OUT03);
            break;
    case PROP_DIGOUT04:
            g_value_set_boolean(value,dout->priv->OUT04);
            break;
    case PROP_DIGOUT05:
            g_value_set_boolean(value,dout->priv->OUT05);
            break;
    case PROP_DIGOUT06:
            g_value_set_boolean(value,dout->priv->OUT06);
            break;
    case PROP_DIGOUT07:
            g_value_set_boolean(value,dout->priv->OUT07);
            break;
    case PROP_DIGOUT08:
            g_value_set_boolean(value,dout->priv->OUT08);
            break;
    case PROP_DIGOUT09:
            g_value_set_boolean(value,dout->priv->OUT09);
            break;
    case PROP_DIGOUT10:
            g_value_set_boolean(value,dout->priv->OUT10);
            break;
    case PROP_DIGOUT11:
            g_value_set_boolean(value,dout->priv->OUT11);
            break;
    case PROP_DIGOUT12:
            g_value_set_boolean(value,dout->priv->OUT12);
            break;
    case PROP_DIGOUT13:
            g_value_set_boolean(value,dout->priv->OUT13);
            break;
    case PROP_DIGOUT14:
            g_value_set_boolean(value,dout->priv->OUT14);
            break;
    case PROP_DIGOUT15:
            g_value_set_boolean(value,dout->priv->OUT15);
            break;*/
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_digital_object_class_init(NodeDigitalObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // NodeObjectClass* parent_class = NODE_OBJECT_CLASS (klass);

    object_class->finalize     = node_digital_object_finalize;
    object_class->set_property = node_digital_object_set_property;
    object_class->get_property = node_digital_object_get_property;
    object_class->constructed  = node_digital_object_constructed;
}
