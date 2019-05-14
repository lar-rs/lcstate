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

#include "airflow-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>
#include "ultra-airflow-object.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
};


struct _AirflowApplicationObjectPrivate
{
	GDBusObjectManagerServer  *airflow_manager;
};





G_DEFINE_TYPE_WITH_PRIVATE (AirflowApplicationObject, airflow_application_object, TERA_TYPE_SERVICE_OBJECT);



static void
airflow_application_object_initialize( AirflowApplicationObject *airflow_application )
{
	UltraAirflowObject *airflow  = NULL;
	airflow = ULTRA_AIRFLOW_OBJECT(g_object_new(ULTRA_TYPE_AIRFLOW_OBJECT,"g-object-path",ULTRA_AIRFLOW_SENSORS,NULL));
	g_dbus_object_manager_server_export (airflow_application->priv->airflow_manager , G_DBUS_OBJECT_SKELETON (airflow) );
	g_object_unref(airflow);
}



static void
airflow_application_object_init (AirflowApplicationObject *airflow_application_object)
{
	AirflowApplicationObjectPrivate *priv      =  airflow_application_object_get_instance_private (airflow_application_object);
	priv->airflow_manager                      =  NULL;
	airflow_application_object->priv           =  priv;
}



static void
airflow_application_object_finalize (GObject *object)
{
	AirflowApplicationObject *airflow_application = AIRFLOW_APPLICATION_OBJECT(object);
	if(airflow_application->priv->airflow_manager) 	g_object_unref(airflow_application->priv->airflow_manager);
	G_OBJECT_CLASS (airflow_application_object_parent_class)->finalize (object);
}


static void
airflow_application_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (AIRFLOW_IS_APPLICATION_OBJECT(object));
	//AirflowApplicationObject *data = AIRFLOW_APPLICATION_OBJECT(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
airflow_application_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (AIRFLOW_IS_APPLICATION_OBJECT (object));
	//AirflowApplicationObject *data = AIRFLOW_APPLICATION_OBJECT(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
airflow_application_object_activated (TeraServiceObject *service)
{


	AirflowApplicationObject *airflow_application = AIRFLOW_APPLICATION_OBJECT(service);
	airflow_application->priv->airflow_manager = g_dbus_object_manager_server_new (ULTRA_AIRFLOW_MANAGER);
	g_dbus_object_manager_server_set_connection (airflow_application->priv->airflow_manager, tera_service_dbus_connection());

	airflow_application_object_initialize(airflow_application);

	service_simple_set_done(tera_service_get_simple(),TRUE);
	service_simple_emit_initialized(tera_service_get_simple(),TRUE);

}


static void
airflow_application_object_class_init (AirflowApplicationObjectClass *klass)
{
	GObjectClass* object_class        = G_OBJECT_CLASS (klass);
	TeraServiceObjectClass *app_class = TERA_SERVICE_OBJECT_CLASS(klass);
	object_class->finalize            = airflow_application_object_finalize;
	object_class->set_property        = airflow_application_object_set_property;
	object_class->get_property        = airflow_application_object_get_property;
	app_class->activated              = airflow_application_object_activated;



}

