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

#include <gio/gio.h>

#include "node-control-app-object.h"
#include "node-device-object.h"

#define NODE_TYPE_CONTROL_APP_OBJECT (node_control_app_object_get_type())
#define NODE_CONTROL_APP_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), NODE_TYPE_CONTROL_APP_OBJECT, NodeControlAppObject))
#define NODE_CONTROL_APP_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), NODE_TYPE_CONTROL_APP_OBJECT, NodeControlAppObjectClass))
#define NODE_IS_CONTROL_APP_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), NODE_TYPE_CONTROL_APP_OBJECT))
#define NODE_IS_CONTROL_APP_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), NODE_TYPE_CONTROL_APP_OBJECT))
#define NODE_CONTROL_APP_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), NODE_TYPE_CONTROL_APP_OBJECT, NodeControlAppObjectClass))

typedef struct _NodeControlAppObjectClass   NodeControlAppObjectClass;
typedef struct _NodeControlAppObject        NodeControlAppObject;
typedef struct _NodeControlAppObjectPrivate NodeControlAppObjectPrivate;

struct _NodeControlAppObjectClass {
    GObjectClass parent_class;
};

struct _NodeControlAppObject {

    GObject                      parent_instance;
    NodeControlAppObjectPrivate *priv;

};

#define GETTEXT_PACKAGE "candaemon"

#include <glib/gi18n-lib.h>

enum {

    PROP_0,

};

static NodeControlAppObject *_node_app_control = NULL;

struct _NodeControlAppObjectPrivate {
	GDBusConnection*          connection;
	GDBusObjectManagerServer* device_manager;
	GDBusObjectManagerServer* nodes_manager;
};

G_DEFINE_TYPE_WITH_PRIVATE(NodeControlAppObject, node_control_app_object, G_TYPE_OBJECT);

static gboolean node_control_app_initialize_all (gpointer user_data)
{
    static gboolean device_can0 = FALSE;
    if(device_can0) {
        return FALSE;
    }
    device_can0 = TRUE;
    NodeControlAppObject* app_object =  NODE_CONTROL_APP_OBJECT(user_data);
	CandeviceObjectSkeleton* device  =  CANDEVICE_OBJECT_SKELETON (g_object_new (NODE_TYPE_DEVICE_OBJECT, "g-object-path", CAN_DEVICE_CAN0, NULL));
	g_dbus_object_manager_server_export (app_object->priv->device_manager,         G_DBUS_OBJECT_SKELETON(device));
	candevice_simple_set_path           (candevice_object_get_simple(CANDEVICE_OBJECT(device)), "/dev/can0");
	node_device_clean_report            (NODE_DEVICE_OBJECT (device));
	node_device_object_open             (NODE_DEVICE_OBJECT (device));
	service_simple_set_is_system        (tera_service_get_simple(), TRUE);
	node_device_init_ultra_nodes        (NODE_DEVICE_OBJECT (device));

    return FALSE;
}

static void node_control_app_object_init(NodeControlAppObject *node_control_app_object) {
    NodeControlAppObjectPrivate *priv = node_control_app_object_get_instance_private(node_control_app_object);
    node_control_app_object->priv     = priv;
}

static void node_control_app_object_constructed (GObject* object)
{
    // NodeControlAppObject *node_control_app = NODE_CONTROL_APP_OBJECT(object);
}

static void node_control_app_object_finalize(GObject *object) {
    // NodeControlAppObject *node_control_app = NODE_CONTROL_APP_OBJECT(object);

    G_OBJECT_CLASS(node_control_app_object_parent_class)->finalize(object);
}

static void node_control_app_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_CONTROL_APP_OBJECT(object));
    // NodeControlAppObject *data = NODE_CONTROL_APP_OBJECT(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_control_app_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_CONTROL_APP_OBJECT(object));
    // NodeControlAppObject *data = NODE_CONTROL_APP_OBJECT(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static gboolean node_control_app_object_dbus_register (NodeControlAppObject* application)
{
    NodeControlAppObject* control_app = NODE_CONTROL_APP_OBJECT (application);
    control_app->priv->device_manager = g_dbus_object_manager_server_new (CAN_DEVICE_MANAGER);
    g_dbus_object_manager_server_set_connection (control_app->priv->device_manager, application->priv->connection);
    control_app->priv->nodes_manager = g_dbus_object_manager_server_new (CAN_NODES_MANAGER);
    g_dbus_object_manager_server_set_connection (control_app->priv->nodes_manager, application->priv->connection);
    g_timeout_add(300,node_control_app_initialize_all,control_app);
    return TRUE;
}

static void node_control_app_object_class_init(NodeControlAppObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize     = node_control_app_object_finalize;
    object_class->set_property = node_control_app_object_set_property;
    object_class->get_property = node_control_app_object_get_property;
    object_class->constructed  = node_control_app_object_constructed;
}

void node_control_app_new (GDBusConnection* connection)
{
    if (connection == NULL) {
        g_error("Connectio to dbus failed ");
        return;
    }
    if (_node_app_control != NULL) {
        return;
    }
    _node_app_control                   = g_object_new (NODE_TYPE_CONTROL_APP_OBJECT, NULL);
    _node_app_control->priv->connection = connection;

    node_control_app_object_dbus_register (_node_app_control);
}

GDBusObjectManager *node_control_app_device_manager(void) {
    g_return_val_if_fail(_node_app_control != NULL, NULL);
    g_return_val_if_fail(NODE_IS_CONTROL_APP_OBJECT(_node_app_control), NULL);
    return G_DBUS_OBJECT_MANAGER(NODE_CONTROL_APP_OBJECT(_node_app_control)->priv->device_manager);
}

GDBusObjectManager *node_control_app_nodes_manager(void) {
    g_return_val_if_fail(_node_app_control != NULL, NULL);
    g_return_val_if_fail(NODE_IS_CONTROL_APP_OBJECT(_node_app_control), NULL);
    return G_DBUS_OBJECT_MANAGER(NODE_CONTROL_APP_OBJECT(_node_app_control)->priv->nodes_manager);
}

GDBusConnection *node_control_app_get_connection(void) {
    g_return_val_if_fail(_node_app_control != NULL, NULL);
    return _node_app_control->priv->connection;
}
