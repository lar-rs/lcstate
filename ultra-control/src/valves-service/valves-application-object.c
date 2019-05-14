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

#include "valves-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
};


struct _ValvesApplicationObjectPrivate
{
	GDBusObjectManagerServer  *valves_manager;
};





G_DEFINE_TYPE_WITH_PRIVATE (ValvesApplicationObject, valves_application_object, TERA_TYPE_SERVICE_OBJECT);



static void
ultra_hardware_initialize_valves( ValvesApplicationObject *valves_application )
{
	MktValveObject *valve_object = NULL;
	valve_object = MKT_VALVE_OBJECT(g_object_new(MKT_TYPE_VALVE_OBJECT,
			"g-object-path",ULTRA_VALVES_INJECTION_PATH,
			"valve-node-number",1,
			"valve-node-address",3,
			"valve-normally-open",FALSE,NULL));
	g_dbus_object_manager_server_export (valves_application->priv->valves_manager, G_DBUS_OBJECT_SKELETON (valve_object));
	g_object_unref(valve_object);

	// valve_object = MKT_VALVE_OBJECT(g_object_new(MKT_TYPE_VALVE_OBJECT,
	// 		"g-object-path",ULTRA_VALVES_TOCDIRECT_PATH,
	// 		"valve-node-number",1,
	// 		"valve-node-address",12,
	// 		"valve-normally-open",TRUE,NULL));
	// g_dbus_object_manager_server_export (valves_application->priv->valves_manager, G_DBUS_OBJECT_SKELETON (valve_object));
	// g_object_unref(valve_object);
}


static void
valves_application_object_init (ValvesApplicationObject *valves_application_object)
{
	ValvesApplicationObjectPrivate *priv      =  valves_application_object_get_instance_private (valves_application_object);
	priv->valves_manager                      =  NULL;
	valves_application_object->priv           =  priv;
}


static void
valves_application_object_finalize (GObject *object)
{
	ValvesApplicationObject *valves_application = VALVES_APPLICATION_OBJECT(object);
	if(valves_application->priv->valves_manager) 	g_object_unref(valves_application->priv->valves_manager);
	G_OBJECT_CLASS (valves_application_object_parent_class)->finalize (object);
}


static void
valves_application_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (VALVES_IS_APPLICATION_OBJECT(object));
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
valves_application_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (VALVES_IS_APPLICATION_OBJECT (object));
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
valves_application_object_activated (TeraServiceObject *service)
{
	ValvesApplicationObject *valves_application = VALVES_APPLICATION_OBJECT(service);

	valves_application->priv->valves_manager = g_dbus_object_manager_server_new (ULTRA_VALVES_MANAGER);
	g_dbus_object_manager_server_set_connection (valves_application->priv->valves_manager, tera_service_dbus_connection());
	ultra_hardware_initialize_valves(valves_application);
	service_simple_set_done(tera_service_get_simple(),TRUE);
	service_simple_emit_initialized(tera_service_get_simple(),TRUE);
}


static void
valves_application_object_class_init (ValvesApplicationObjectClass *klass)
{
	GObjectClass* object_class        = G_OBJECT_CLASS (klass);
	TeraServiceObjectClass *app_class = TERA_SERVICE_OBJECT_CLASS(klass);
	object_class->finalize            = valves_application_object_finalize;
	object_class->set_property        = valves_application_object_set_property;
	object_class->get_property        = valves_application_object_get_property;
	app_class->activated              = valves_application_object_activated;


}

