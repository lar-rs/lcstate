/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * can-manager-app.c
 *
 * Author: A. Smolkov / G. Stützer
 *
 */

#include <gio/gio.h>
#include <glib/gprintf.h>
#include <ultra-pc-generated-code.h>
#include <ultra-can-generated-code.h>
#include <ultra-analogs-generated-code.h>
#include <ultra-sensors-generated-code.h>
#include <ultra-level-generated-code.h>
#include <ultra-errors-generated-code.h>
#include <ultra-measurement-generated-code.h>
#include <ultra-panel-pc.h>
#include <ultra-can-api.h>
#include <ultra-can-dbus.h>
#include <ultra-ams-dbus.h>

#include "ultra-state.h"

// #include <ultra-can-api.h>
// #include <ultra-can-dbus.h>
// #include <ultra-ams-dbus.h>
// #include <ultra-panel-pc.h>

#define ULTRA_TYPE_STATE           (ultra_state_get_type())
#define ULTRA_STATE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj),   ULTRA_TYPE_STATE, UltraState))
#define ULTRA_STATE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST    ((klass), ULTRA_TYPE_STATE, UltraStateClass))
#define ULTRA_STATE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS  ((obj),   ULTRA_TYPE_STATE, UltraStateClass))

typedef struct _UltraStateClass   UltraStateClass;
typedef struct _UltraState        UltraState;
typedef struct _UltraStatePrivate UltraStatePrivate;

struct _UltraStateClass {
	GApplicationClass parent_class;
};

struct _UltraState {
	GApplication          parent_instance;
	UltraStatePrivate*   priv;

};

#include "ultra-config.h"
#include <glib/gi18n-lib.h>

struct _UltraStatePrivate {
    gboolean cfg;

};

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

G_DEFINE_TYPE_WITH_PRIVATE (UltraState, ultra_state, G_TYPE_APPLICATION);

// static void ultra_state_initialize_hardware (UltraState* ultra_state)
// {
	// UltraStatePrivate* priv = ultra_state -> priv;
 	// priv -> canDevice  = CAN_DEVICE_OBJECT      (g_object_new (CAN_TYPE_DEVICE_OBJECT, "g-object-path", CAN_DEVICE_CAN0, NULL));
	// priv -> digNode0   = newNodeDigitalObject   (CAN_NODES_MANAGER);
	// priv -> digNode1   = newNodeDigitalObject   (CAN_NODES_MANAGER);
	// priv -> digNode2   = newNodeDigitalObject   (CAN_NODES_MANAGER);
	// priv -> anaNode    = newNodeAnalogObject    (CAN_NODES_MANAGER);
	// priv -> motorNode0 = newNodeMotor3Object    (CAN_NODES_MANAGER);
	// priv -> motorNode1 = newNodeMotor3Object    (CAN_NODES_MANAGER);
	// priv -> anaExtNode = newNodeAnalogExtObject (CAN_NODES_MANAGER);

	// g_dbus_object_manager_server_export (ultra_state->priv->device_manager, G_DBUS_OBJECT_SKELETON (priv->canDevice));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->digNode0));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->digNode1));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->digNode2));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->anaNode));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->motorNode0));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->motorNode1));
	// g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager,  G_DBUS_OBJECT_SKELETON (priv->anaExtNode));

	/* Nach dem Export kann 'canDevice' wiedergefunden werden:
	--
	-- CanDeviceObject* deviceObj = CAN_DEVICE_OBJECT (g_dbus_object_manager_get_object (G_DBUS_OBJECT_MANAGER(ultra_state->priv->device_manager), CAN_DEVICE_CAN0));
	*/


	/*
	node_device_clean_report (NODE_DEVICE_OBJECT (skeleton));
	node_device_object_open  (NODE_DEVICE_OBJECT (skeleton));

	if (candevice_simple_get_is_open(candevice_object_get_simple(CANDEVICE_OBJECT(skeleton))))
		node_device_object_scan(NODE_DEVICE_OBJECT(skeleton));

	service_simple_set_is_system(tera_service_get_simple(), TRUE);
	*/
// }

static void ultra_state_init (UltraState* ultra_state)
{
    UltraStatePrivate* priv = ultra_state_get_instance_private(ultra_state);

    ultra_state -> priv = priv;

    // Test sensor signal files ... Point 1 ...

    /* TODO: Add initialization code here */
}

static void ultra_state_activate (GApplication* application)
{
//	UltraState*        ultra_state = ULTRA_STATE (application);
//	UltraStatePrivate* priv            = ultra_state -> priv;

//	priv -> digNode0  = newNodeDigitalObject (CAN_NODES_MANAGER);

//	g_dbus_object_manager_server_export (ultra_state->priv->nodes_manager, G_DBUS_OBJECT_SKELETON(priv->digNode0));

	// g_action_group_activate_action (G_ACTION_GROUP(application), "online-state-machine", NULL);
}

static void ultra_state_constructed (GObject* object)
{
	GApplication* application;

	if (G_OBJECT_CLASS (ultra_state_parent_class) -> constructed)
		G_OBJECT_CLASS (ultra_state_parent_class) -> constructed (object);

	application = G_APPLICATION (object);

	g_signal_connect (application, "activate", G_CALLBACK (ultra_state_activate), NULL);
	g_signal_connect (application, "startup",  G_CALLBACK (ultra_state_activate), NULL);
}

static void ultra_state_dispose (GObject* object)
{
	G_OBJECT_CLASS (ultra_state_parent_class) -> dispose (object);
}

static void ultra_state_finalize (GObject* object)
{
    // UltraState *ultra_state = ULTRA_STATE(object);

    G_OBJECT_CLASS(ultra_state_parent_class)->finalize(object);
}

static void ultra_state_set_property (GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
	g_return_if_fail (ULTRA_STATE (object));
	// UltraState *data = ULTRA_STATE(object);
	switch (prop_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void ultra_state_get_property (GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
	g_return_if_fail(ULTRA_STATE(object));
	// UltraState *data = ULTRA_STATE(object);
	switch (prop_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

// static gboolean ultra_state_dbus_register(UltraState *application)
static gboolean ultra_state_dbus_register (GApplication* application, GDBusConnection* connection, const gchar* object_path, GError** error)
{
	// UltraState* ultra_state = ULTRA_STATE (application);

	if (! G_APPLICATION_CLASS (ultra_state_parent_class) -> dbus_register (application, connection, object_path, error)){
		return FALSE;
    }

	// ultra_state -> priv -> device_manager = g_dbus_object_manager_server_new (CAN_DEVICE_MANAGER);
	// can_device_set_can_device_manager           (G_DBUS_OBJECT_MANAGER (ultra_state->priv->device_manager));
	// g_dbus_object_manager_server_set_connection (ultra_state->priv->device_manager, connection);
	// ultra_state -> priv -> nodes_manager = g_dbus_object_manager_server_new (CAN_NODES_MANAGER);
	// can_device_set_nodes_device_manager         (G_DBUS_OBJECT_MANAGER (ultra_state->priv->nodes_manager));
    // g_dbus_object_manager_server_set_connection (ultra_state->priv->nodes_manager, connection);
    g_debug("ultimate app initialize hardware");
    ultra_ams_panel_pc_state(connection);
    ultra_can_dbus_state(connection);


	return TRUE;
}

static void ultra_state_dbus_unregister (GApplication* application, GDBusConnection* connection, const gchar* object_path)
{
    // UltraState* hwd_app = ULTRA_STATE (application);

    // g_object_unref (hwd_app->priv->device_manager);
    // g_object_unref (hwd_app->priv->nodes_manager);

    G_APPLICATION_CLASS (ultra_state_parent_class) -> dbus_unregister (application, connection, object_path);
}


static void ultra_state_class_init (UltraStateClass* klass)
{
	GApplicationClass* g_application_class = G_APPLICATION_CLASS (klass);
	GObjectClass*      object_class        = G_OBJECT_CLASS      (klass);

	object_class -> finalize               = ultra_state_finalize;
	object_class -> dispose                = ultra_state_dispose;
	object_class -> set_property           = ultra_state_set_property;
	object_class -> get_property           = ultra_state_get_property;
	object_class -> constructed            = ultra_state_constructed;
	g_application_class -> dbus_register   = ultra_state_dbus_register;
	g_application_class -> dbus_unregister = ultra_state_dbus_unregister;
}

GApplication* ultra_state_new (const gchar* application_id, GApplicationFlags flags)
{
	g_return_val_if_fail (g_application_id_is_valid (application_id), NULL);

	GApplication* result = G_APPLICATION (g_object_new (ULTRA_TYPE_STATE, "application-id", application_id, "flags", G_APPLICATION_IS_SERVICE, NULL));
	// g_application_new()

	return G_APPLICATION (result);
}
