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

#include "vessels-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>
#include "ultra-furnace-object.h"
#include "ultra-ticport-object.h"
#include "ultra-vessel-object.h"
#include "ultra-dilution-object.h"


#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
};


struct _VesselsApplicationObjectPrivate
{
	GDBusObjectManagerServer  *vessels_manager;
};





G_DEFINE_TYPE_WITH_PRIVATE (VesselsApplicationObject, vessels_application_object, TERA_TYPE_SERVICE_OBJECT);



static void
vessels_application_object_initialize_vessels( VesselsApplicationObject *vessels_application )
{
	UltraVesselObject     *ultra_vessel    = NULL;
	UltraDilutionObject   *ultra_dilution  = NULL;
	UltraFurnaceObject    *furnace         = NULL;
	UltraTicportObject    *ticport         = NULL;


	furnace = ULTRA_FURNACE_OBJECT(g_object_new(ULTRA_TYPE_FURNACE_OBJECT,"g-object-path",ULTRA_VESSELS_FURNACE,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(furnace)),_("Furnace"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(furnace)),1);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager , G_DBUS_OBJECT_SKELETON (furnace));
	g_object_unref(furnace);

	ticport = ULTRA_TICPORT_OBJECT(g_object_new(ULTRA_TYPE_TICPORT_OBJECT,"g-object-path",ULTRA_VESSELS_TIC,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ticport)),_("TICport"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ticport)),2);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager , G_DBUS_OBJECT_SKELETON (ticport));
	g_object_unref(ticport);

	ultra_vessel = ULTRA_VESSEL_OBJECT(g_object_new(ULTRA_TYPE_VESSEL_OBJECT,
			"g-object-path",ULTRA_VESSELS_1,"vessel-number",1,"vessel-default-pos",1130,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),_("V1"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),3);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager, G_DBUS_OBJECT_SKELETON (ultra_vessel));
	g_object_unref(ultra_vessel);

	ultra_vessel = ULTRA_VESSEL_OBJECT(g_object_new(ULTRA_TYPE_VESSEL_OBJECT,
			"g-object-path",ULTRA_VESSELS_2,"vessel-number",2,"vessel-default-pos",1420,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),_("V2"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),4);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager, G_DBUS_OBJECT_SKELETON (ultra_vessel));
	g_object_unref(ultra_vessel);

	ultra_vessel = ULTRA_VESSEL_OBJECT(g_object_new(ULTRA_TYPE_VESSEL_OBJECT,
			"g-object-path",ULTRA_VESSELS_3,"vessel-number",3,"vessel-default-pos",1700,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),_("V3"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),5);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager, G_DBUS_OBJECT_SKELETON (ultra_vessel));
	g_object_unref(ultra_vessel);

	ultra_vessel = ULTRA_VESSEL_OBJECT(g_object_new(ULTRA_TYPE_VESSEL_OBJECT,
			"g-object-path",ULTRA_VESSELS_4,"vessel-number",4,"vessel-default-pos",2000,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),_("V4"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),6);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager, G_DBUS_OBJECT_SKELETON (ultra_vessel));
	g_object_unref(ultra_vessel);

	ultra_vessel = ULTRA_VESSEL_OBJECT(g_object_new(ULTRA_TYPE_VESSEL_OBJECT,
			"g-object-path",ULTRA_VESSELS_5,"vessel-number",5,"vessel-default-pos",2280,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),_("V5"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ultra_vessel)),7);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager, G_DBUS_OBJECT_SKELETON (ultra_vessel));
	g_object_unref(ultra_vessel);

	ultra_dilution = ULTRA_DILUTION_OBJECT(g_object_new(ULTRA_TYPE_DILUTION_OBJECT,
			"g-object-path",ULTRA_VESSELS_6,"vessel-number",6,"vessel-default-pos",2400,NULL));
	vessels_simple_set_name(vessels_object_get_simple(VESSELS_OBJECT(ultra_dilution)),_("V6"));
	vessels_simple_set_number(vessels_object_get_simple(VESSELS_OBJECT(ultra_dilution)),8);
	g_dbus_object_manager_server_export (vessels_application->priv->vessels_manager, G_DBUS_OBJECT_SKELETON (ultra_dilution));
	g_object_unref(ultra_dilution);
}



static void
vessels_application_object_init (VesselsApplicationObject *vessels_application_object)
{
	VesselsApplicationObjectPrivate *priv      =  vessels_application_object_get_instance_private (vessels_application_object);
	priv->vessels_manager                      =  NULL;
	vessels_application_object->priv           =  priv;
}



static void
vessels_application_object_finalize (GObject *object)
{
	VesselsApplicationObject *vessels_application = VESSELS_APPLICATION_OBJECT(object);
	if(vessels_application->priv->vessels_manager) 	g_object_unref(vessels_application->priv->vessels_manager);
	G_OBJECT_CLASS (vessels_application_object_parent_class)->finalize (object);
}


static void
vessels_application_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (VESSELS_IS_APPLICATION_OBJECT(object));
	//VesselsApplicationObject *data = VESSELS_APPLICATION_OBJECT(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
vessels_application_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (VESSELS_IS_APPLICATION_OBJECT (object));
	//VesselsApplicationObject *data = VESSELS_APPLICATION_OBJECT(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
vessels_application_object_activated (TeraServiceObject *service)
{

	VesselsApplicationObject *vessels_application = VESSELS_APPLICATION_OBJECT(service);

	vessels_application->priv->vessels_manager = g_dbus_object_manager_server_new (ULTRA_VESSELS_MANAGER);
	g_dbus_object_manager_server_set_connection (vessels_application->priv->vessels_manager, tera_service_dbus_connection());
	vessels_application_object_initialize_vessels(vessels_application);
	service_simple_set_done(tera_service_get_simple(),TRUE);
	service_simple_emit_initialized(tera_service_get_simple(),TRUE);

}

static void
vessels_application_object_class_init (VesselsApplicationObjectClass *klass)
{
	GObjectClass* object_class        = G_OBJECT_CLASS (klass);
	TeraServiceObjectClass *app_class = TERA_SERVICE_OBJECT_CLASS(klass);
	object_class->finalize            = vessels_application_object_finalize;
	object_class->set_property        = vessels_application_object_set_property;
	object_class->get_property        = vessels_application_object_get_property;
	app_class->activated              = vessels_application_object_activated;
}
