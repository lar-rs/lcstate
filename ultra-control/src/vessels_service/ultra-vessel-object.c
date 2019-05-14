/*
 * @ingroup UltraVesselObject
 * @{
 * @file  ultra-vessel-object.c	Vessel object
 * @brief This is Vessel control object description.
 *
 *
 *  Copyright (C) A.Smolkov 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 */

#include "ultra-vessel-object.h"

#include <mktlib.h>
#include "mkt-can-manager-client.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum {
    PROP_0,
    PROP_SAMPLE_NUMBER,
    PROP_VESSEL_DEFAULT_POS,
};

struct _UltraVesselObjectPrivate {
    guint    default_pos;
    guint    number;
    GTimer * timer;
    gboolean new_data;
};

G_DEFINE_TYPE_WITH_PRIVATE(UltraVesselObject, ultra_vessel_object, VESSELS_TYPE_OBJECT_SKELETON);

static void ultra_vessel_object_read_data(UltraVesselObject *ultra_vessel_object) {
    const gchar *object_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object));
    MktVessel *vessel_data = MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, "select * from %s where param_object_path = '%s'", g_type_name(MKT_TYPE_VESSEL_MODEL), object_path));
    VesselsSimple *simple = vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel_object));
    if (simple) {
        if (vessel_data) {
            g_object_bind_property(vessel_data, "vessel-x-pos", simple, "pos-xachse", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
            g_object_bind_property(vessel_data, "vessel-y-pos", simple, "injection-pos", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        }
        else {
            vessels_simple_set_injection_pos(simple,1250);
            vessels_simple_set_pos_xachse(simple,ultra_vessel_object->priv->default_pos);
        }
        g_object_unref(simple);
    } 
    if(vessel_data)g_object_unref(vessel_data);
}

/*
static gboolean
ultra_vessel_object_run_pump_real ( UltraVesselObject *vessel_object , gdouble time )
{

        return TRUE;
}
static gboolean
ultra_vessel_object_run_pump_callback (VesselsPump *interface, GDBusMethodInvocation *invocation, gdouble time , gpointer user_data)
{
        gboolean is_run = FALSE;
        UltraVesselObject *vessel_object = ULTRA_VESSEL_OBJECT(user_data);
        is_run = ultra_vessel_object_run_pump_real(vessel_object,time);
        g_dbus_method_invocation_return_value (invocation,g_variant_new ("(b)", is_run));
        return TRUE;
}
*/

static void ultra_vessel_object_init(UltraVesselObject *ultra_vessel_object) {
    UltraVesselObjectPrivate *priv = ultra_vessel_object_get_instance_private(ultra_vessel_object);
    priv->timer                    = NULL;
    ultra_vessel_object->priv      = priv;
}

static void ultra_vessel_object_finalize(GObject *object) {
    // UltraVesselObject *vessel_object = ULTRA_VESSEL_OBJECT(object);
    G_OBJECT_CLASS(ultra_vessel_object_parent_class)->finalize(object);
}

static void ultra_vessel_object_constructed(GObject *object) {
    UltraVesselObject *vessel_object = ULTRA_VESSEL_OBJECT(object);
    VesselsSimple *    simple        = vessels_simple_skeleton_new();
    vessels_object_skeleton_set_simple(VESSELS_OBJECT_SKELETON(vessel_object), simple);
    g_object_unref(simple);
    VesselsPump *pump = vessels_pump_skeleton_new();
    vessels_object_skeleton_set_pump(VESSELS_OBJECT_SKELETON(vessel_object), pump);
    // g_signal_connect (pump,"handle-run-pump",G_CALLBACK( ultra_vessel_object_run_pump_callback),vessel_object);
    g_object_unref(pump);
    vessels_simple_set_number(simple, vessel_object->priv->number);
    ultra_vessel_object_read_data(vessel_object);
    G_OBJECT_CLASS(ultra_vessel_object_parent_class)->constructed(object);
}


static void ultra_vessel_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_VESSEL_OBJECT(object));
    UltraVesselObject *vessel = ULTRA_VESSEL_OBJECT(object);
    switch (prop_id) {
        case PROP_SAMPLE_NUMBER:
            vessel->priv->number = g_value_get_uint(value);
            break;
        case PROP_VESSEL_DEFAULT_POS:
            vessel->priv->default_pos = g_value_get_uint(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_vessel_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_VESSEL_OBJECT(object));
    UltraVesselObject *vessel = ULTRA_VESSEL_OBJECT(object);
    switch (prop_id) {
        case PROP_SAMPLE_NUMBER:
            g_value_set_uint(value, vessel->priv->number);
            break;
        case PROP_VESSEL_DEFAULT_POS:
            g_value_set_uint(value, vessel->priv->default_pos);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void ultra_vessel_object_class_init(UltraVesselObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // UltraHdwControlObjectClass* parent_class = ULTRA_HDW_CONTROL_OBJECT_CLASS (klass);
    object_class->finalize     = ultra_vessel_object_finalize;
    object_class->set_property = ultra_vessel_object_set_property;
    object_class->get_property = ultra_vessel_object_get_property;
    object_class->constructed  = ultra_vessel_object_constructed;

    g_object_class_install_property(object_class, PROP_SAMPLE_NUMBER,
                                    g_param_spec_uint("vessel-number", "Vessel number", "Vessel number", 0, 125, 0, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(object_class, PROP_VESSEL_DEFAULT_POS,
                                    g_param_spec_uint("vessel-default-pos", "Vessel number", "Vessel number", 0, 2800, 1200, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));

}