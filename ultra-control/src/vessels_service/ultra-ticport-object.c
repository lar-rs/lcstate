/*
 * @ingroup UltraTicportObject
 * @{
 * @file  ultra-ticport-object.c
 * @brief This is ticport control object description.
 *
 *
 * @coryright Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "ultra-ticport-object.h"
#include "open-tic.h"
#include "close-tic.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum {
    PROP_0,
    PROP_TICPORT_NODE,
};

#define DIGITAL1_PATH "/com/lar/nodes/Digital1"

struct _UltraTicportObjectPrivate {
    guint           check_state;
    gboolean        can_connected;
    GTimer *        timer;
    gdouble         timeout;
    NodesDigital16 *digital1;
    NodesDigital16 *digital2;

    GCancellable *activity;
};

G_DEFINE_TYPE_WITH_PRIVATE(UltraTicportObject, ultra_ticport_object, VESSELS_TYPE_OBJECT_SKELETON);

NodesDigital16 *TICPORT_DIGITAL1(UltraTicportObject *ticport) {
    g_return_val_if_fail(ticport != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_TICPORT_OBJECT(ticport), NULL);
    return ticport->priv->digital1;
}
NodesDigital16 *TICPORT_DIGITAL2(UltraTicportObject *ticport) {
    g_return_val_if_fail(ticport != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_TICPORT_OBJECT(ticport), NULL);
    return ticport->priv->digital2;
}

static void ultra_ticport_object_reload_data(UltraTicportObject *ultra_vessel_object) {
    const gchar *object_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object));

    MktVessel *vessel_data = MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, "select * from %s where param_object_path = '%s'", g_type_name(MKT_TYPE_VESSEL_MODEL), object_path));

    VesselsSimple * simple  = vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel_object));
    VesselsTicport *ticport = vessels_object_get_ticport(VESSELS_OBJECT(ultra_vessel_object));
    if (simple && ticport) {
        if (vessel_data) {
            g_object_bind_property(vessel_data, "vessel-x-pos", simple, "pos-xachse", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
            g_object_bind_property(vessel_data, "vessel-y-pos", simple, "injection-pos", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
            g_object_bind_property(vessel_data, "vessel-y1-pos", ticport, "needle-pos", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
            g_object_bind_property(vessel_data, "vessel-has-motor", ticport, "is-motor", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        }
        else {
            vessels_simple_set_pos_xachse(simple,832);
            vessels_simple_set_injection_pos(simple,1200);
            vessels_ticport_set_needle_pos(ticport,450);
        }

        // g_object_bind_property(vessel_data, "vessel-pump", simple, "pump-path", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        // g_debug("Vessel %s pos: %d mod-pos %d", object_path, vessels_simple_get_pos_xachse(simple), mkt_vessel_xpos(vessel_data));
        g_object_unref(simple);
        g_object_unref(ticport);
    } 
    if(vessel_data)g_object_unref(vessel_data);
}

static NodesDigital16 *_uticport_get_digital1(UltraTicportObject *object) {
    g_return_val_if_fail(mkt_can_manager_client_nodes() != NULL, NULL);
    NodesObject *node_object = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
    if (node_object) {
        NodesDigital16 *digital16 = nodes_object_get_digital16(node_object);
        return digital16;
    }
    return NULL;
}

static NodesDigital16 *_uticport_get_digital2(UltraTicportObject *object) {
    g_return_val_if_fail(mkt_can_manager_client_nodes() != NULL, NULL);
    NodesObject *node_object = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
    if (node_object) {
        NodesDigital16 *digital16 = nodes_object_get_digital16(node_object);
        return digital16;
    }
    return NULL;
}

// ------------------------------------------------------- Help access functions
// -----------  --------------------------------------
void TICPORT_SET_BUSY(UltraTicportObject *object, gboolean value) {
    g_return_if_fail(vessels_object_get_ticport(VESSELS_OBJECT(object)) != NULL);
    vessels_ticport_set_is_busy(vessels_object_get_ticport(VESSELS_OBJECT(object)), value);
}

void TICPORT_SET_OPEN(UltraTicportObject *object, gboolean value) {
    g_return_if_fail(vessels_object_get_ticport(VESSELS_OBJECT(object)) != NULL);
    vessels_ticport_set_is_open(vessels_object_get_ticport(VESSELS_OBJECT(object)), value);
}

static gboolean IS_TIC_MOTOR(UltraTicportObject *object) {
    g_return_val_if_fail(vessels_object_get_ticport(VESSELS_OBJECT(object)) != NULL, FALSE);
    return vessels_ticport_get_is_motor(vessels_object_get_ticport(VESSELS_OBJECT(object)));
}

static gboolean ultra_ticport_check_motor(UltraTicportObject *object) {
    if (IS_TIC_MOTOR(object)) {
        if (object->priv->digital1 == NULL || object->priv->digital2 == NULL) {
            mkt_errors_report(E1822, "TIC-Port with motor: control hardware not found (check nodes Digital1(ID:18) and Digital2(ID:19))");
        }
    }
    return FALSE;
}

static gboolean ultra_ticport_object_close_callback(VesselsTicport *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraTicportObject *uticport = ULTRA_TICPORT_OBJECT(user_data);
    if (uticport->priv->activity) {
        g_cancellable_cancel(uticport->priv->activity);
        g_cancellable_reset(uticport->priv->activity);
    }
    uticport->priv->activity = g_cancellable_new();
    if (IS_TIC_MOTOR(uticport))
        close_ticport_operation(uticport, uticport->priv->activity, invocation);
    else
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));

    return TRUE;
}

static gboolean ultra_ticport_object_open_callback(VesselsTicport *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraTicportObject *uticport = ULTRA_TICPORT_OBJECT(user_data);
    if (uticport->priv->activity) {
        g_cancellable_cancel(uticport->priv->activity);
        g_cancellable_reset(uticport->priv->activity);
    }
    uticport->priv->activity = g_cancellable_new();
    if (IS_TIC_MOTOR(uticport))
        open_tic_operation(uticport, uticport->priv->activity, invocation);
    else
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

static void ultra_furnace_durnace_is_motor_callback(VesselsTicport *tic, GParamSpec *psprc, UltraTicportObject *ticport) { ultra_ticport_check_motor(ticport); }

static void ultra_ticport_object_init(UltraTicportObject *ultra_ticport_object) {
    UltraTicportObjectPrivate *priv      = ultra_ticport_object_get_instance_private(ultra_ticport_object);
    ultra_ticport_object->priv           = priv;
    ultra_ticport_object->priv->digital1 = NULL;
    ultra_ticport_object->priv->digital2 = NULL;
    ultra_ticport_object->priv->activity = g_cancellable_new();
    ultra_ticport_object->priv->timer    = g_timer_new();
    ultra_ticport_object->priv->timeout  = 10.0;
    mkt_errors_clean(E1822);

    /* TODO: Add initialization code here */
}

static void ultra_ticport_object_finalize(GObject *object) {
    UltraTicportObject *uticport = ULTRA_TICPORT_OBJECT(object);
    if (uticport->priv->activity) {
        g_cancellable_cancel(uticport->priv->activity);
        g_object_unref(uticport->priv->activity);
    }
    G_OBJECT_CLASS(ultra_ticport_object_parent_class)->finalize(object);
}

static void ultra_ticport_object_constructed(GObject *object) {
    UltraTicportObject *uticport = ULTRA_TICPORT_OBJECT(object);
    VesselsSimple *     simple   = vessels_simple_skeleton_new();
    vessels_object_skeleton_set_simple(VESSELS_OBJECT_SKELETON(uticport), simple);
    g_object_unref(simple);
    VesselsTicport *ticport = vessels_ticport_skeleton_new();
    vessels_object_skeleton_set_ticport(VESSELS_OBJECT_SKELETON(uticport), ticport);
    g_signal_connect(ticport, "handle-close", G_CALLBACK(ultra_ticport_object_close_callback), uticport);
    g_signal_connect(ticport, "handle-open", G_CALLBACK(ultra_ticport_object_open_callback), uticport);
    g_object_unref(ticport);
    uticport->priv->digital1 = _uticport_get_digital1(uticport);
    uticport->priv->digital2 = _uticport_get_digital2(uticport);
    ultra_ticport_check_motor(uticport);
    ultra_ticport_object_reload_data(uticport);
    g_signal_connect(ticport, "notify::is-motor", G_CALLBACK(ultra_furnace_durnace_is_motor_callback), uticport);
}

static void ultra_ticport_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_TICPORT_OBJECT(object));
    // UltraTicportObject *ticport = ULTRA_TICPORT_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_ticport_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_TICPORT_OBJECT(object));
    // UltraTicportObject *ticport = ULTRA_TICPORT_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_ticport_object_class_init(UltraTicportObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = ultra_ticport_object_finalize;
    object_class->set_property = ultra_ticport_object_set_property;
    object_class->get_property = ultra_ticport_object_get_property;
    object_class->constructed  = ultra_ticport_object_constructed;
}

/** @} */
