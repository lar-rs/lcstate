/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "axis-application-object.h"

#include <gio/gio.h>
#include <market-time.h>
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "axis-object.h"
#include "axisZ-object.h"
#include "control-ptp.h"
#include "move-object.h"
// #include "axisy-object.h"

// #include "injection-object.h"

enum {
    PROP_0,
};

static GDBusObjectManagerServer *_LAST_MANAGER = NULL;

struct _AxisApplicationObjectPrivate {
    NodesObject *             doppel_motor1;
    NodesObject *             doppel_motor2;
    GDBusObjectManagerServer *axis_manager;

    AxisObject *  X;
    AxisObject *  Y;
    AxisObject *  Z;
    TeraXysystem *xy_system;
};

G_DEFINE_TYPE_WITH_PRIVATE(AxisApplicationObject, axis_application_object, TERA_TYPE_SERVICE_OBJECT);
static void xy_system_ptp_done_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_CANCELLED, "PTP control cancelled");
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
            g_dbus_method_invocation_return_error(invocation, ERROR_QUARK, error ? error->code : E1700, "%s", error != NULL ? error->message : "PTP operation unknown error");
        } else {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(invocation);
}

static gboolean xy_system_ptp_run_callback(TeraXysystem *xysystem, GDBusMethodInvocation *invocation, const gchar *commands, AxisApplicationObject *axis_application) {
    MOVE_cancel();
    ControlPtp *ptp = CONTROL_PTP(g_object_new(CONTROL_TYPE_PTP, "commands", commands, "axis", axis_application->priv->axis_manager, NULL));
    cotrol_ptp_run(ptp, MOVE_cancellable(), xy_system_ptp_done_callback, g_object_ref(invocation));
    g_object_unref(ptp);
    return TRUE;
}

static void axis_application_initialize_axis(AxisApplicationObject *axis_application) {
    /*AxisObject  *XAxis      = NULL;
    UltraAxisYObject  *YAxis      = NULL;
    UltraAxisZObject  *ZAxis      = NULL;*/

    axis_application->priv->doppel_motor1 = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Doppelmotor1"));
    if (axis_application->priv->doppel_motor1 == NULL) {
        mkt_log_error_message_sync("XY-System: connection to Doppelmotor1 is failed");
    }
    // else {
    //     NodesSimple *simple = nodes_object_get_simple(axis_application->priv->doppel_motor1);
    //     if (nodes_simple_get_node_id(simple) != 18) {
    //         mkt_log_error_message_sync("XY-System: Doppelmotor1 wrong ID %x");
    //         mkt_log_error_message_sync("XY-System Doppelmotor1 hat falsche ID %x (Empfohlen %x)", nodes_simple_get_node_id(simple), 18);
    //         g_error("XY-System Doppelmotor1 hat falsche ID %x (Empfohlen %x)", nodes_simple_get_node_id(simple), 18);
    //     }
    // }
    axis_application->priv->doppel_motor2 = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Doppelmotor2"));
    if (axis_application->priv->doppel_motor2 == NULL) {
      mkt_log_error_message_sync("XY-System: connection to Doppelmotor2 is failed");
    }
    // else {
    //     NodesSimple *simple = nodes_object_get_simple(axis_application->priv->doppel_motor2);
    //     if (nodes_simple_get_node_id(simple) != 20) {
    //         mkt_log_error_message_sync("XY-System Doppelmotor1 hat falsche ID %x (Empfohlen %x)", nodes_simple_get_node_id(simple), 20);
    //         g_error("XY-System Doppelmotor1 hat falsche ID %x (Empfohlen %x)", nodes_simple_get_node_id(simple), 20);
    //     }
    // }
    NodesDoppelmotor3 *doppelmotor1 = nodes_object_get_doppelmotor3(axis_application->priv->doppel_motor1);
    NodesDoppelmotor3 *doppelmotor2 = nodes_object_get_doppelmotor3(axis_application->priv->doppel_motor2);
    // Create X Axis .
    axis_application->priv->X = AXIS_OBJECT(g_object_new(AXIS_TYPE_OBJECT, "g-object-path", ULTRA_AXIS_X_PATH, "node-object", axis_application->priv->doppel_motor1, NULL));
    AchsenAchse *achse        = achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->X));
    MktAxis *    axis_data    = MKT_AXIS(mkt_model_select_one(MKT_TYPE_AXIS_MODEL, "select * from %s where param_object_path = '%s';", g_type_name(MKT_TYPE_AXIS_MODEL), ULTRA_AXIS_X_PATH));
    if (axis_data != NULL) {
        g_object_bind_property(axis_data, "axis-max", achse, "max", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(axis_data, "axis-hold", achse, "hold", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(axis_data, "axis-current", achse, "current", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_unref(axis_data);
    }

    g_dbus_object_manager_server_export(axis_application->priv->axis_manager, G_DBUS_OBJECT_SKELETON(axis_application->priv->X));
    g_object_bind_property(doppelmotor1, "stepper2-position", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->X)), "position", G_BINDING_DEFAULT);
    g_object_bind_property(doppelmotor1, "stepper2-final-position", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->X)), "final-position", G_BINDING_DEFAULT);
    achsen_achse_set_name(achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->X)), "X-Axis");
    axis_object_set_error_number(AXIS_OBJECT(axis_application->priv->X), E1710);
    axis_activate_stall_guard(AXIS_OBJECT(axis_application->priv->X));
    // Create Y Axis .
    axis_data = MKT_AXIS(mkt_model_select_one(MKT_TYPE_AXIS_MODEL, "select * from %s where param_object_path = '%s';", g_type_name(MKT_TYPE_AXIS_MODEL), ULTRA_AXIS_Y_PATH));
    if (axis_data == NULL) {
        axis_data = MKT_AXIS(mkt_model_new(MKT_TYPE_AXIS_MODEL, "param-object-id", tera_service_id(), "param-object-path", ULTRA_AXIS_Y_PATH, "axis-hold", 80, "axis-max", 2000, "axis-current", 1200,
                                           "axis-reverse", FALSE, NULL));
    }
    axis_application->priv->Y = AXIS_OBJECT(g_object_new(AXIS_TYPE_OBJECT, "g-object-path", ULTRA_AXIS_Y_PATH, "node-object", axis_application->priv->doppel_motor2, NULL));
    achse                     = achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Y));
    if (achse) {
        g_object_bind_property(axis_data, "axis-max", achse, "max", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(axis_data, "axis-hold", achse, "hold", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(axis_data, "axis-current", achse, "current", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_unref(achse);
    }

    g_object_unref(axis_data);

    g_dbus_object_manager_server_export(axis_application->priv->axis_manager, G_DBUS_OBJECT_SKELETON(axis_application->priv->Y));
    g_object_bind_property(doppelmotor2, "stepper2-position", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Y)), "position", G_BINDING_DEFAULT);
    g_object_bind_property(doppelmotor2, "stepper2-final-position", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Y)), "final-position", G_BINDING_DEFAULT);
    achsen_achse_set_name(achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Y)), "Y-Axis");
    axis_object_set_error_number(AXIS_OBJECT(axis_application->priv->Y), E1720);

    // Create Z Axis .
    axis_data = MKT_AXIS(mkt_model_select_one(MKT_TYPE_AXIS_MODEL, "select * from %s where param_object_path = '%s';", g_type_name(MKT_TYPE_AXIS_MODEL), ULTRA_AXIS_Z_PATH));
    if (axis_data == NULL) {
        g_warning("AXIS %s PARAMETER NOT FOUND", ULTRA_AXIS_Z_PATH);
        axis_data = MKT_AXIS(mkt_model_new(MKT_TYPE_AXIS_MODEL, "param-object-id", tera_service_id(), "param-object-path", ULTRA_AXIS_Z_PATH, "axis-hold", 80, "axis-max", 1800, "axis-current", 1200,
                                           "axis-reverse", FALSE, NULL));
    }
    guint max = 0;
    g_object_get(axis_data, "axis-max", &max, NULL);
    if (max > 1800) g_object_set(axis_data, "axis-max", 1800, NULL);
    axis_application->priv->Z =
        AXIS_OBJECT(g_object_new(ULTRA_TYPE_AXISZ_OBJECT, "g-object-path", ULTRA_AXIS_Z_PATH, "node-object", axis_application->priv->doppel_motor2, "part", 1, "invert", TRUE, NULL));
    achse = achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Z));
    if (achse) {
        g_object_bind_property(axis_data, "axis-max", achse, "max", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(axis_data, "axis-hold", achse, "hold", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(axis_data, "axis-current", achse, "current", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_unref(achse);
    }
    g_object_unref(axis_data);
    g_dbus_object_manager_server_export(axis_application->priv->axis_manager, G_DBUS_OBJECT_SKELETON(axis_application->priv->Z));
    g_object_bind_property(doppelmotor2, "stepper1-position", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Z)), "position", G_BINDING_DEFAULT);
    g_object_bind_property(doppelmotor2, "stepper1-final-position", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Z)), "final-position", G_BINDING_DEFAULT);
    achsen_achse_set_name(achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Z)), "Inject");
    axis_object_set_error_number(AXIS_OBJECT(axis_application->priv->Z), E1730);

    MktInjection *injection_data =
        MKT_INJECTION(mkt_model_select_one(MKT_TYPE_INJECTION_MODEL, "select * from %s where param_object_path = '%s'", g_type_name(MKT_TYPE_INJECTION_MODEL), ULTRA_AXIS_Z_PATH));
    if (injection_data == NULL) {
        // g_warning("INJECTION %s PARAMETER NOT FOUND", ULTRA_AXIS_Z_PATH);
        injection_data = MKT_INJECTION(mkt_model_new(MKT_TYPE_INJECTION_MODEL, "param-object-path", ULTRA_AXIS_Z_PATH, NULL));
    }

    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis_application->priv->Z));
    if (injection) {
        g_object_bind_property(injection_data, "injection-air", injection, "air", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "injection-rest", injection, "rest", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "injection-furnace-air", injection, "furnace-air", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "injection-dilution", injection, "dilution", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "injection-rinsing", injection, "rinsing", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "injection-stepper-parameter", injection, "injection-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "sample-stepper-parameter", injection, "sample-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "rinsing-up-stepper-parameter", injection, "rinsing-up-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_bind_property(injection_data, "rinsing-down-stepper-parameter", injection, "rinsing-down-stepper-parameter", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
        g_object_unref(injection);
    }
    g_object_bind_property(achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Y)), "is-busy", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->X)), "is-busy",
                           G_BINDING_BIDIRECTIONAL);
    g_object_bind_property(achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Y)), "is-busy", achsen_object_get_achse(ACHSEN_OBJECT(axis_application->priv->Z)), "is-busy",
                           G_BINDING_BIDIRECTIONAL);

    axis_application->priv->xy_system = tera_xysystem_skeleton_new();
    GError *error                     = NULL;
    if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(axis_application->priv->xy_system), tera_service_dbus_connection(), ULTRA_XY_SYSTEM_PATH, &error)) {
        // g_error("Service interface create fail");
    }
    g_signal_connect(axis_application->priv->xy_system, "handle-ptprun", G_CALLBACK(xy_system_ptp_run_callback), axis_application);
    g_object_unref(doppelmotor1);
    g_object_unref(doppelmotor2);
}

static void axis_application_object_init(AxisApplicationObject *axis_application_object) {
    AxisApplicationObjectPrivate *priv = axis_application_object_get_instance_private(axis_application_object);
    axis_application_object->priv      = priv;
    ULTRA_AXIS_ERRORS;
}

static void axis_application_object_finalize(GObject *object) {
    AxisApplicationObject *axis_application = AXIS_APPLICATION_OBJECT(object);
    if (axis_application->priv->axis_manager) g_object_unref(axis_application->priv->axis_manager);
    G_OBJECT_CLASS(axis_application_object_parent_class)->finalize(object);
}

static void axis_application_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(AXIS_IS_APPLICATION_OBJECT(object));
    // AxisApplicationObject *data = AXIS_APPLICATION_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void axis_application_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(AXIS_IS_APPLICATION_OBJECT(object));
    // AxisApplicationObject *data = AXIS_APPLICATION_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void axis_application_object_activated(TeraServiceObject *service) {
    AxisApplicationObject *axis_application = AXIS_APPLICATION_OBJECT(service);
    axis_application->priv->axis_manager    = g_dbus_object_manager_server_new(ULTRA_AXIS_MANAGE_PATH);
    g_dbus_object_manager_server_set_connection(axis_application->priv->axis_manager, tera_service_dbus_connection());
    _LAST_MANAGER = axis_application->priv->axis_manager;
    axis_application_initialize_axis(axis_application);
    axis_init_parameter(axis_application->priv->X);
    axis_init_parameter(axis_application->priv->Y);
    axis_init_parameter(axis_application->priv->Z);
    service_simple_set_done(tera_service_get_simple(), TRUE);
    service_simple_emit_initialized(tera_service_get_simple(), TRUE);
}

static void axis_application_object_class_init(AxisApplicationObjectClass *klass) {
    GObjectClass *          object_class = G_OBJECT_CLASS(klass);
    TeraServiceObjectClass *app_class    = TERA_SERVICE_OBJECT_CLASS(klass);
    object_class->finalize               = axis_application_object_finalize;
    object_class->set_property           = axis_application_object_set_property;
    object_class->get_property           = axis_application_object_get_property;
    app_class->activated                 = axis_application_object_activated;
}

GDBusObjectManager *axis_application_get_object_manajer() { return G_DBUS_OBJECT_MANAGER(_LAST_MANAGER); }

GQuark domain_error_quark(void) {
    static GQuark domain;
    if (!domain) domain = g_quark_from_static_string("com.lar.tera.axis");
    return domain;
}
