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

#include "pumps-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum {
    PROP_0,
};

struct _PumpsApplicationObjectPrivate {
    GDBusObjectManagerServer *pumps_manager;
    MktPumpObject *           GP;
    MktPumpObject *           Pump1;
    MktPumpObject *           Pump2;
    MktPumpObject *           Pump3;
    MktPumpObject *           Pump4;
    MktPumpObject *           Pump5;
    MktPumpObject *           Pump6;
};

G_DEFINE_TYPE_WITH_PRIVATE(PumpsApplicationObject, pumps_application_object, TERA_TYPE_SERVICE_OBJECT);

static gboolean ultra_pump_initialize_all(gpointer data) {
    PumpsApplicationObject *pumps_application = PUMPS_APPLICATION_OBJECT(data);
    mkt_pump_object_start(pumps_application->priv->GP);
    if (pumps_application->priv->Pump1) mkt_pump_object_stop(pumps_application->priv->Pump1);
    if (pumps_application->priv->Pump2) mkt_pump_object_stop(pumps_application->priv->Pump2);
    if (pumps_application->priv->Pump3) mkt_pump_object_stop(pumps_application->priv->Pump3);
    if (pumps_application->priv->Pump4) mkt_pump_object_stop(pumps_application->priv->Pump4);
    if (pumps_application->priv->Pump5) mkt_pump_object_stop(pumps_application->priv->Pump5);
    if (pumps_application->priv->Pump6) mkt_pump_object_stop(pumps_application->priv->Pump6);
    service_simple_set_done(tera_service_get_simple(), TRUE);
    service_simple_emit_initialized(tera_service_get_simple(), TRUE);
    return FALSE;
}

static void ultra_hardware_initialize_pumps(PumpsApplicationObject *pumps_application) {
    PumpsPump *ps = NULL;
    // Condensate pump
    NodesObject *digital_node1 = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
    NodesObject *digital_node2 = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
    if (digital_node1 == NULL) {
        mkt_log_error_message("Ultra-Pumpen Node:/com/lar/nodes/Digital1 not found");
        // g_error("Ultra-Pumpen Node:/com/lar/nodes/Digital1 not found");
    }

    pumps_application->priv->GP = MKT_PUMP_OBJECT(g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_CONDENSATE_PATH, "pump-node", digital_node1, "pump-address", 13, NULL));
    ps                          = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->GP));
    pumps_pump_set_name(ps, _("Condensate"));
    pumps_pump_set_number(ps, 0);
    g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->GP));
    g_object_unref(pumps_application->priv->GP);

    // Sample pump stream1
    pumps_application->priv->Pump1 = MKT_PUMP_OBJECT(g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_1_PATH, "pump-node", digital_node1, "pump-address", 14, NULL));
    ps                             = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->Pump1));
    pumps_pump_set_name(ps, _("Pump1"));
    pumps_pump_set_number(ps, 1);
    g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->Pump1));
    g_object_unref(pumps_application->priv->Pump1);

    // Sample pump stream2
    pumps_application->priv->Pump2 = MKT_PUMP_OBJECT(g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_2_PATH, "pump-node", digital_node1, "pump-address", 7, NULL));
    ps                             = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->Pump2));
    pumps_pump_set_name(ps, _("Pump2"));
    pumps_pump_set_number(ps, 2);
    g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->Pump2));
    g_object_unref(pumps_application->priv->Pump2);

    if (digital_node2 != NULL) {
        // Sample pump stream3
        pumps_application->priv->Pump3 = MKT_PUMP_OBJECT(g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_3_PATH, "pump-node", digital_node2, "pump-address", 2, NULL));
        ps                             = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->Pump3));
        pumps_pump_set_name(ps, _("Pump3"));
        pumps_pump_set_number(ps, 3);
        g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->Pump3));
        g_object_unref(pumps_application->priv->Pump3);

        // Sample pump stream4
        pumps_application->priv->Pump4 = MKT_PUMP_OBJECT(g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_4_PATH, "pump-node", digital_node2, "pump-address", 3, NULL));
        ps                             = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->Pump4));
        pumps_pump_set_name(ps, _("Pump4"));
        pumps_pump_set_number(ps, 4);
        g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->Pump4));
        g_object_unref(pumps_application->priv->Pump4);

        // Sample pump stream5
        pumps_application->priv->Pump5 = MKT_PUMP_OBJECT(g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_5_PATH, "pump-node", digital_node2, "pump-address", 4, NULL));
        ps                             = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->Pump5));
        pumps_pump_set_name(ps, _("Pump5"));
        pumps_pump_set_number(ps, 5);
        g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->Pump5));
        g_object_unref(pumps_application->priv->Pump5);

        // Sample pump stream5
        pumps_application->priv->Pump6 = MKT_PUMP_OBJECT(
            g_object_new(MKT_TYPE_PUMP_OBJECT, "g-object-path", TERA_PUMPS_6_PATH, "pump-node", digital_node2, "pump-address", 5, "fluid-node", digital_node1, "fluid-address", 5, NULL));
        ps = pumps_object_get_pump(PUMPS_OBJECT(pumps_application->priv->Pump6));
        pumps_pump_set_name(ps, _("Pump6"));
        pumps_pump_set_number(ps, 6);
        g_dbus_object_manager_server_export(pumps_application->priv->pumps_manager, G_DBUS_OBJECT_SKELETON(pumps_application->priv->Pump6));
        g_object_unref(pumps_application->priv->Pump6);
        g_object_unref(digital_node1);
    }
    g_timeout_add(20, ultra_pump_initialize_all, pumps_application);
    g_object_unref(digital_node1);
}

static void pumps_application_object_init(PumpsApplicationObject *pumps_application_object) {
    PumpsApplicationObjectPrivate *priv = pumps_application_object_get_instance_private(pumps_application_object);
    priv->pumps_manager                 = NULL;
    pumps_application_object->priv      = priv;
}

static void pumps_application_object_finalize(GObject *object) {
    PumpsApplicationObject *pumps_application = PUMPS_APPLICATION_OBJECT(object);
    if (pumps_application->priv->pumps_manager) g_object_unref(pumps_application->priv->pumps_manager);
    G_OBJECT_CLASS(pumps_application_object_parent_class)->finalize(object);
}

static void pumps_application_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(PUMPS_IS_APPLICATION_OBJECT(object));
    // PumpsApplicationObject *data = PUMPS_APPLICATION_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void pumps_application_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(PUMPS_IS_APPLICATION_OBJECT(object));
    // PumpsApplicationObject *data = PUMPS_APPLICATION_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void pumps_application_object_activated(TeraServiceObject *service) {
    PumpsApplicationObject *pumps_application = PUMPS_APPLICATION_OBJECT(service);

    pumps_application->priv->pumps_manager = g_dbus_object_manager_server_new(TERA_PUMPS_MANAGER);
    g_dbus_object_manager_server_set_connection(pumps_application->priv->pumps_manager, tera_service_dbus_connection());
    ultra_hardware_initialize_pumps(pumps_application);
}

static void pumps_application_object_class_init(PumpsApplicationObjectClass *klass) {
    GObjectClass *          object_class = G_OBJECT_CLASS(klass);
    TeraServiceObjectClass *app_class    = TERA_SERVICE_OBJECT_CLASS(klass);
    object_class->finalize               = pumps_application_object_finalize;
    object_class->set_property           = pumps_application_object_set_property;
    object_class->get_property           = pumps_application_object_get_property;
    app_class->activated                 = pumps_application_object_activated;
}
