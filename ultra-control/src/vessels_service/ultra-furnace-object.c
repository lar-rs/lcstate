/*
 * @ingroup UltraFurnaceObject
 * @{
 * @file  ultra-furnace-object.c
 * @brief This is furnace control object description.
 *
 *
 * @coryright Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "close-furnace.h"
#include "open-furnace.h"
#include "ultra-furnace-object.h"

#include "../../config.h"

#include <glib/gi18n-lib.h>

enum {
    PROP_0,
    PROP_FURNACE_NODE,
};

struct _UltraFurnaceObjectPrivate {
    guint processed_tag;
    guint runned_counter;
    guint internal_state;

    guint check_state;

    MktOperationGroup *operation_group;
    gboolean           can_connected;

    NodesDigital16 *digital1;
    GCancellable *  check_furnace;
    GCancellable *  activity;

    guint tag;

    gboolean warme_phase;
    gboolean cooler_init;

    GDBusMethodInvocation *invocation;
};

G_DEFINE_TYPE_WITH_PRIVATE(UltraFurnaceObject, ultra_furnace_object, VESSELS_TYPE_OBJECT_SKELETON);

static void ultra_furnace_object_reload_data(UltraFurnaceObject *ultra_vessel_object) {
    const gchar *object_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object));
    MktVessel *vessel_data = MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, "select * from %s where param_object_path = '%s'", g_type_name(MKT_TYPE_VESSEL_MODEL), object_path));
    VesselsSimple * simple  = vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel_object));
    VesselsFurnace *furnace = vessels_object_get_furnace(VESSELS_OBJECT(ultra_vessel_object));
    if (simple && furnace) {
        if (vessel_data) {
            g_object_bind_property(vessel_data, "vessel-x-pos", simple, "pos-xachse", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
            g_object_bind_property(vessel_data, "vessel-y-pos", simple, "injection-pos", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
            g_object_bind_property(vessel_data, "vessel-y1-pos", furnace, "needle-pos", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        }
		else{
			vessels_simple_set_pos_xachse(simple,120);
            vessels_simple_set_injection_pos(simple,1700);
            vessels_furnace_set_needle_pos(furnace,720);

		}
        g_object_unref(simple);
        g_object_unref(furnace);
    }
    if(vessel_data)g_object_unref(vessel_data);
}

static NodesDigital16 *_ufurnace_get_digital(UltraFurnaceObject *object) {
    g_return_val_if_fail(mkt_can_manager_client_nodes() != NULL, NULL);
    NodesObject *node_object = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
    if (node_object) {
        NodesDigital16 *digital16 = nodes_object_get_digital16(node_object);
        return digital16;
    }
    return NULL;
}

// ------------------------------------------------------- Help access functions
// -----------  --------------------------------------
void FURNACE_SET_BUSY(UltraFurnaceObject *object, gboolean value) {
    VesselsFurnace *f = vessels_object_get_furnace(VESSELS_OBJECT(object));
    g_return_if_fail(f != NULL);
    vessels_furnace_set_is_busy(f, value);
    g_object_unref(f);
}

void FURNACE_SET_OPEN(UltraFurnaceObject *object, gboolean value) {
    VesselsFurnace *f = vessels_object_get_furnace(VESSELS_OBJECT(object));
    g_return_if_fail(f != NULL);
    vessels_furnace_set_is_open(vessels_object_get_furnace(VESSELS_OBJECT(object)), value);
    g_object_unref(f);
}

NodesDigital16 *FURNACE_DIGITAL1(UltraFurnaceObject *furnace) {
    g_return_val_if_fail(furnace != NULL, NULL);
    g_return_val_if_fail(ULTRA_IS_FURNACE_OBJECT(furnace), NULL);
    return furnace->priv->digital1;
}

/*static gboolean
   IS_OPEN ( UltraFurnaceObject *object )
   {
        g_return_val_if_fail(vessels_object_get_furnace(VESSELS_OBJECT(object))!=NULL,TRUE);
        return
   vessels_furnace_get_is_open(vessels_object_get_furnace(VESSELS_OBJECT(object)));
   }*/

static void SET_ON(UltraFurnaceObject *object, gboolean value) {
    g_return_if_fail(vessels_object_get_furnace(VESSELS_OBJECT(object)) != NULL);
    vessels_furnace_set_is_on(vessels_object_get_furnace(VESSELS_OBJECT(object)), value);
}

static gboolean IS_ON(UltraFurnaceObject *object) {
    g_return_val_if_fail(vessels_object_get_furnace(VESSELS_OBJECT(object)) != NULL, TRUE);
    return vessels_furnace_get_is_on(vessels_object_get_furnace(VESSELS_OBJECT(object)));
}

static gboolean ultra_furnace_onoff_real(UltraFurnaceObject *ufurnace, gboolean value) {
    if (ufurnace->priv->digital1 == NULL) {
        return FALSE;
    }
    GError * error  = NULL;
    gboolean result = FALSE;
    if (!nodes_digital16_call_set_digital_out_sync(ufurnace->priv->digital1, FURNACE_ON_bit, value, &result, NULL, &error)) {
        if (error) g_error_free(error);
        return FALSE;
    }
    if (result == FALSE) {
        return FALSE;
    }

    SET_ON(ufurnace, value);
    return TRUE;
}

static void uface_set_IS_DEAD_err(UltraFurnaceObject *object) {
    g_return_if_fail(vessels_object_get_furnace(VESSELS_OBJECT(object)) != NULL);
    if (!vessels_furnace_get_is_dead(vessels_object_get_furnace(VESSELS_OBJECT(object)))) mkt_errors_come(E1810);
    vessels_furnace_set_is_dead(vessels_object_get_furnace(VESSELS_OBJECT(object)), TRUE);
}

static void COOLER_IS_OUT_OF_RANGE_err(UltraFurnaceObject *object) {
    if (!vessels_furnace_get_cooler_out_of_range(vessels_object_get_furnace(VESSELS_OBJECT(object)))) {
        mkt_errors_come(E1815);
    }
    vessels_furnace_set_cooler_out_of_range(vessels_object_get_furnace(VESSELS_OBJECT(object)), TRUE);
}

static void COOLER_IS_OUT_OF_RANGE_ok(UltraFurnaceObject *object) {
    if (object->priv->cooler_init) {
        object->priv->cooler_init = FALSE;
        mkt_errors_clean(E1815);
    }
}

static void IS_OUT_OF_RANGE_err(UltraFurnaceObject *object) {
    g_return_if_fail(vessels_object_get_furnace(VESSELS_OBJECT(object)) != NULL);
    if (!vessels_furnace_get_out_of_range(vessels_object_get_furnace(VESSELS_OBJECT(object)))) {
        mkt_errors_come(E1820);
    }
    vessels_furnace_set_out_of_range(vessels_object_get_furnace(VESSELS_OBJECT(object)), TRUE);
}

static void IS_OUT_OF_RANGE_ok(UltraFurnaceObject *object) {
    g_return_if_fail(vessels_object_get_furnace(VESSELS_OBJECT(object)) != NULL);
    if (object->priv->warme_phase) {
        object->priv->warme_phase = FALSE;
        mkt_errors_clean(E1820);
    }
    vessels_furnace_set_out_of_range(vessels_object_get_furnace(VESSELS_OBJECT(object)), FALSE);
}

static void ultra_furnace_durnace_is_dead_callback(GObject *object, GParamSpec *psprc, UltraFurnaceObject *furnace) {
    gboolean is_dead         = vessels_furnace_get_is_dead(VESSELS_FURNACE(object));
    gboolean auto_monitoring = vessels_furnace_get_is_monitoring(VESSELS_FURNACE(object));
    if (is_dead && auto_monitoring) {
        if (IS_ON(furnace)) {
            ultra_furnace_onoff_real(furnace, FALSE);
        }
    }
 }

//--------------------------------------------------- Check IS_DEAD -----------------------------------------------------------

static gboolean ufurnace_check_IS_DEAD_wait_iteration(gpointer user_data);
static void     ufurnace_check_IS_DEAD_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *            error           = NULL;
    UltraFurnaceObject *ufurnace_object = ULTRA_FURNACE_OBJECT(user_data);
    if (ufurnace_object->priv->digital1 != NULL) {
        gboolean result = FALSE;
        if (!nodes_digital16_call_get_digital_in_finish(ufurnace_object->priv->digital1, &result, res, &error)) {
            if (error) g_error_free(error);
            uface_set_IS_DEAD_err(ufurnace_object);
        } else if (!result)
            uface_set_IS_DEAD_err(ufurnace_object);
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, CHECK_FUNC_WAIT_SECONDS, ufurnace_check_IS_DEAD_wait_iteration, g_object_ref(user_data), g_object_unref);
    }
}
static gboolean ufurnace_check_IS_DEAD_wait_iteration(gpointer user_data) {
    UltraFurnaceObject *ufurnace_object = ULTRA_FURNACE_OBJECT(user_data);
    if (g_cancellable_is_cancelled(ufurnace_object->priv->check_furnace)) return FALSE;
    nodes_digital16_call_get_digital_in(ufurnace_object->priv->digital1, FURNACE_IS_DEAD_bit, ufurnace_object->priv->check_furnace, ufurnace_check_IS_DEAD_async_callback, ufurnace_object);
    return FALSE;
}

static void ultra_furnace_start_IS_DEAD_monitoring(UltraFurnaceObject *ufurnace) {
    if (ufurnace->priv->digital1) {
        nodes_digital16_call_get_digital_in(ufurnace->priv->digital1, FURNACE_IS_DEAD_bit, ufurnace->priv->check_furnace, ufurnace_check_IS_DEAD_async_callback, ufurnace);
    }
}

//--------------------------------------------------- Check
// COOLER_IS_OUT_OF_RANGE
//--------------------------------------------------------------------------------------------

static gboolean ufurnace_check_COOLER_IS_OUT_OF_RANGE_wait_iteration(gpointer user_data);

static void ufurnace_check_COOLER_IS_OUT_OF_RANGE_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *            error           = NULL;
    UltraFurnaceObject *ufurnace_object = ULTRA_FURNACE_OBJECT(user_data);
    if (ufurnace_object->priv->digital1 != NULL) {
        gboolean result = FALSE;
        if (!nodes_digital16_call_get_digital_in_finish(ufurnace_object->priv->digital1, &result, res, &error)) {
            if (error) g_error_free(error);
            COOLER_IS_OUT_OF_RANGE_err(ufurnace_object);
        } else if (result == TRUE)
            COOLER_IS_OUT_OF_RANGE_ok(ufurnace_object);
        else
            COOLER_IS_OUT_OF_RANGE_err(ufurnace_object);
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, CHECK_FUNC_WAIT_SECONDS, ufurnace_check_COOLER_IS_OUT_OF_RANGE_wait_iteration, g_object_ref(user_data), g_object_unref);
    }
}

static gboolean ufurnace_check_COOLER_IS_OUT_OF_RANGE_wait_iteration(gpointer user_data) {
    UltraFurnaceObject *ufurnace_object = ULTRA_FURNACE_OBJECT(user_data);
    if (g_cancellable_is_cancelled(ufurnace_object->priv->check_furnace)) return FALSE;
    nodes_digital16_call_get_digital_in(ufurnace_object->priv->digital1, COOLER_IS_OUT_OF_RANGE_bit, ufurnace_object->priv->check_furnace, ufurnace_check_COOLER_IS_OUT_OF_RANGE_async_callback,
                                        ufurnace_object);
    return FALSE;
}

static void ultra_furnace_start_COOLER_IS_OUT_OF_RANGE_monitoring(UltraFurnaceObject *ufurnace) {
    if (ufurnace->priv->digital1)
        nodes_digital16_call_get_digital_in(ufurnace->priv->digital1, COOLER_IS_OUT_OF_RANGE_bit, ufurnace->priv->check_furnace, ufurnace_check_COOLER_IS_OUT_OF_RANGE_async_callback, ufurnace);
}

//--------------------------------------------------- Check Furnace
// OUT_OF_RANGE --------------------------------------------------
static gboolean ufurnace_check_OUT_OF_RANGE_wait_iteration(gpointer user_data);
static void     ufurnace_check_OUT_OF_RANGE_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *            error           = NULL;
    UltraFurnaceObject *ufurnace_object = ULTRA_FURNACE_OBJECT(user_data);
    if (ufurnace_object->priv->digital1 != NULL) {
        gboolean result = FALSE;
        if (!nodes_digital16_call_get_digital_in_finish(ufurnace_object->priv->digital1, &result, res, &error)) {
            if (error) g_error_free(error);
            IS_OUT_OF_RANGE_err(ufurnace_object);
        } else if (result == TRUE)
            IS_OUT_OF_RANGE_ok(ufurnace_object);
        else
            IS_OUT_OF_RANGE_err(ufurnace_object);
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, CHECK_FUNC_WAIT_SECONDS, ufurnace_check_OUT_OF_RANGE_wait_iteration, g_object_ref(user_data), g_object_unref);
    }
}
gboolean ufurnace_check_OUT_OF_RANGE_wait_iteration(gpointer user_data) {
    UltraFurnaceObject *ufurnace_object = ULTRA_FURNACE_OBJECT(user_data);
    if (g_cancellable_is_cancelled(ufurnace_object->priv->check_furnace)) return FALSE;
    nodes_digital16_call_get_digital_in(ufurnace_object->priv->digital1, FURNACE_IS_OUT_OF_RANGE_bit, ufurnace_object->priv->check_furnace, ufurnace_check_OUT_OF_RANGE_async_callback,
                                        ufurnace_object);
    return FALSE;
}

static void ultra_furnace_start_OUT_OF_RANGE_monitoring(UltraFurnaceObject *ufurnace) {
    if (ufurnace->priv->digital1)
        nodes_digital16_call_get_digital_in(ufurnace->priv->digital1, FURNACE_IS_OUT_OF_RANGE_bit, ufurnace->priv->check_furnace, ufurnace_check_OUT_OF_RANGE_async_callback, ufurnace);
}

static void ultra_furnace_start_all_check_operations(UltraFurnaceObject *ufurnace) {
    ultra_furnace_start_IS_DEAD_monitoring(ufurnace);
    ultra_furnace_start_OUT_OF_RANGE_monitoring(ufurnace);
    ultra_furnace_start_COOLER_IS_OUT_OF_RANGE_monitoring(ufurnace);
}

// ------------------------------------------------------------------------------handle
// signals callback
// -------------------------------------------------------------------

static gboolean ultra_furnace_object_get_on_callback(VesselsFurnace *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraFurnaceObject *ufurnace = ULTRA_FURNACE_OBJECT(user_data);
    if (ufurnace->priv->digital1 == NULL) {
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", FALSE));
        return TRUE;
    }

    GError * error = NULL;
    gboolean is_on = FALSE;
    if (!nodes_digital16_call_get_digital_out_sync(ufurnace->priv->digital1, FURNACE_ON_bit, &is_on, NULL, &error)) {
        if (error) g_error_free(error);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", FALSE));
        SET_ON(ufurnace, FALSE);
        return TRUE;
    }
    SET_ON(ufurnace, is_on);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", is_on));
    return TRUE;
}

static gboolean ultra_furnace_object_on_off_callback(VesselsFurnace *interface, GDBusMethodInvocation *invocation, gboolean on, gpointer user_data) {
    UltraFurnaceObject *ufurnace = ULTRA_FURNACE_OBJECT(user_data);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", ultra_furnace_onoff_real(ufurnace, on)));
    return TRUE;
}

// ------------------------------------------------------- Furnace Close Open
// activity ---------------------------------------------------------------

static gboolean ultra_furnace_object_close_callback(VesselsFurnace *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraFurnaceObject *ufurnace = ULTRA_FURNACE_OBJECT(user_data);
    if (ufurnace->priv->activity) {
        g_cancellable_cancel(ufurnace->priv->activity);
        g_object_unref(ufurnace->priv->activity);
    }
    ufurnace->priv->activity = g_cancellable_new();
    close_furnace_operation(ufurnace, ufurnace->priv->activity, invocation);
    return TRUE;
}

static gboolean ultra_furnace_object_open_callback(VesselsFurnace *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraFurnaceObject *ufurnace = ULTRA_FURNACE_OBJECT(user_data);
    if (ufurnace->priv->activity) {
        g_cancellable_cancel(ufurnace->priv->activity);
        g_object_unref(ufurnace->priv->activity);
    }
    ufurnace->priv->activity = g_cancellable_new();
    open_furnace_operation(ufurnace, ufurnace->priv->activity, invocation);
    return TRUE;
}

static gboolean ultra_furnace_object_monitoring_callback(VesselsFurnace *interface, GDBusMethodInvocation *invocation, gboolean value, gpointer user_data) {
    vessels_furnace_set_is_monitoring(interface, value);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
    return TRUE;
}

static void ultra_furnace_object_init(UltraFurnaceObject *ultra_furnace_object) {
    UltraFurnaceObjectPrivate *priv           = ultra_furnace_object_get_instance_private(ultra_furnace_object);
    ultra_furnace_object->priv                = priv;
    ultra_furnace_object->priv->digital1      = NULL;
    ultra_furnace_object->priv->check_furnace = NULL;
    ultra_furnace_object->priv->tag           = 0;
    ultra_furnace_object->priv->activity      = NULL;
    mkt_errors_init(FALSE);
    mkt_errors_clean(E1810);
    mkt_errors_clean(E1815);
    mkt_errors_clean(E1821);
    mkt_errors_clean(E1822);
    mkt_errors_clean(E1825);

    ultra_furnace_object->priv->warme_phase = TRUE;
    ultra_furnace_object->priv->cooler_init = TRUE;
    /* TODO: Add initialization code here */
}

static void ultra_furnace_object_finalize(GObject *object) {
    UltraFurnaceObject *ufurnace = ULTRA_FURNACE_OBJECT(object);
    if (ufurnace->priv->operation_group) g_object_unref(ufurnace->priv->operation_group);
    if (ufurnace->priv->activity) {
        g_cancellable_cancel(ufurnace->priv->activity);
        g_object_unref(ufurnace->priv->activity);
    }
    if (ufurnace->priv->check_furnace) {
        g_cancellable_cancel(ufurnace->priv->check_furnace);
        g_object_unref(ufurnace->priv->check_furnace);
    }
    G_OBJECT_CLASS(ultra_furnace_object_parent_class)->finalize(object);
}

static void ultra_furnace_object_constructed(GObject *object) {
    UltraFurnaceObject *ufurnace = ULTRA_FURNACE_OBJECT(object);
    VesselsSimple *     simple   = vessels_simple_skeleton_new();
    vessels_object_skeleton_set_simple(VESSELS_OBJECT_SKELETON(ufurnace), simple);
    g_object_unref(simple);
    VesselsFurnace *furnace = vessels_furnace_skeleton_new();
    vessels_object_skeleton_set_furnace(VESSELS_OBJECT_SKELETON(ufurnace), furnace);
    vessels_furnace_set_is_monitoring(furnace, TRUE);  // TODO: Auto monitor is on.
    g_signal_connect(furnace, "handle-close", G_CALLBACK(ultra_furnace_object_close_callback), ufurnace);
    g_signal_connect(furnace, "handle-open", G_CALLBACK(ultra_furnace_object_open_callback), ufurnace);
    g_signal_connect(furnace, "handle-get-on", G_CALLBACK(ultra_furnace_object_get_on_callback), ufurnace);
    g_signal_connect(furnace, "handle-on-off", G_CALLBACK(ultra_furnace_object_on_off_callback), ufurnace);
    g_signal_connect(furnace, "handle-monitoring", G_CALLBACK(ultra_furnace_object_monitoring_callback), ufurnace);
    g_signal_connect(furnace, "notify::is-dead", G_CALLBACK(ultra_furnace_durnace_is_dead_callback), ufurnace);

    g_object_unref(furnace);
    ultra_furnace_object_reload_data(ufurnace);
    ufurnace->priv->digital1 = _ufurnace_get_digital(ufurnace);
    if (ufurnace->priv->digital1 != NULL) {
        ufurnace->priv->check_furnace = g_cancellable_new();
        ultra_furnace_start_all_check_operations(ufurnace);

    } else {
        mkt_errors_report(E1810,
                          "Emergency shutdown furnace - Digital node "
                          "/com/lar/nodes/Digital1 not found");
    }
}

static void ultra_furnace_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_FURNACE_OBJECT(object));
    // UltraFurnaceObject *furnace = ULTRA_FURNACE_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_furnace_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_FURNACE_OBJECT(object));
    // UltraFurnaceObject *furnace = ULTRA_FURNACE_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_furnace_object_class_init(UltraFurnaceObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = ultra_furnace_object_finalize;
    object_class->set_property = ultra_furnace_object_set_property;
    object_class->get_property = ultra_furnace_object_get_property;
    object_class->constructed  = ultra_furnace_object_constructed;
}

/** @} */
