/*
 * @ingroup RowSensors
 * @{
 * @file  row-channel-info.c	generated object file
 * @brief generated object file
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include <largdm.h>
#include <mktlib.h>
#include <mktbus.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

#include "row-sensors.h"



//static RowSensors *__gui_process_desktop = NULL;

struct _RowSensorsPrivate
{
	CandeviceObject *candevice;
	gboolean scanned;
	guint    wath_plug;
	guint    wath_signal;

	GTimer  *timer;
	guint    plug_tag;
};

enum {
	ROW_SENSORS_PROP_NULL,
};


enum
{
	ROW_SENSORS_LAST_SIGNAL
};


//static guint row_sensors_signals[ROW_SENSORS_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (RowSensors, row_sensors, ROW_TYPE_SERVICE);


static void
on_signal_name_appeared (GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	RowSensors* row_sensors = ROW_SENSORS(user_data);
	g_object_set(row_sensors,"service-done",TRUE,NULL);
	row_service_done(ROW_SERVICE(row_sensors));
	g_signal_emit_by_name(row_sensors, "service-ready",0,TRUE );
}

static void
on_signal_name_vanished (GDBusConnection *connection,
		const gchar     *name,
		gpointer         user_data)
{
	RowService* row_service = ROW_SERVICE(user_data);
	g_object_set(row_service,"service-done",FALSE,NULL);
	row_service_error(row_service);
}


static gboolean
wait_sensors_plugs_timeout ( gpointer user_data)
{
	RowSensors* row_sensors = ROW_SENSORS(user_data);
	PlugObject *plug = sensors_plug_manager_client_get_plug_object();
	PlugIfc *ifc = plug_object_get_ifc(plug);
	gboolean ready = FALSE;
	ready = plug_ifc_get_sensor_at_a1__ain5__ready(ifc)
	&&plug_ifc_get_sensor_at_dm1__x1__ready(ifc)
	&&plug_ifc_get_sensor_at_dm1__x2__ready(ifc)
	&&plug_ifc_get_sensor_at_dm2__x1__ready(ifc);

	if(ready)
	{
		sensors_signal_manager_client_new();
		row_sensors->priv->wath_signal = g_bus_watch_name(G_BUS_TYPE_SESSION,"com.lar.sensors.signal.ObjectManager",G_BUS_NAME_WATCHER_FLAGS_AUTO_START,on_signal_name_appeared,on_signal_name_vanished,row_sensors,NULL);
		return FALSE;
	}

	if(g_timer_elapsed(row_sensors->priv->timer,NULL)>120.0)
	{
		g_object_set(row_sensors,"service-done",FALSE,NULL);
		row_service_error(ROW_SERVICE(row_sensors));
		g_signal_emit_by_name(row_sensors, "service-ready",0,FALSE );
		return FALSE;
	}
	return TRUE;
}

static void
on_plug_name_appeared (GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	RowSensors* row_sensors = ROW_SENSORS(user_data);
	row_sensors->priv->plug_tag = g_timeout_add_seconds(1,wait_sensors_plugs_timeout,row_sensors);
}

static void
on_plug_name_vanished (GDBusConnection *connection,
		const gchar     *name,
		gpointer         user_data)
{
	RowService* row_service = ROW_SERVICE(user_data);
	g_object_set(row_service,"service-done",FALSE,NULL);
	row_service_error(row_service);
}

static void
row_sensors_run ( RowService *service )
{
	RowSensors* row_sensors = ROW_SENSORS(service);
	if(row_sensors->priv->plug_tag>0)
		g_source_remove(row_sensors->priv->plug_tag);
	g_object_set(service,"service-run",TRUE,NULL);
	row_service_run_spinner(service);
	g_timer_start(row_sensors->priv->timer);
	if(row_sensors->priv->wath_plug != 0)
		g_bus_unwatch_name(row_sensors->priv->wath_plug);
	g_object_set(row_sensors,"service-run",TRUE,NULL);
	sensors_plug_manager_client_new();
	row_sensors->priv->wath_plug = g_bus_watch_name(G_BUS_TYPE_SESSION,"com.lar.sensors.plug.ObjectManager",G_BUS_NAME_WATCHER_FLAGS_AUTO_START,on_plug_name_appeared,on_plug_name_vanished,row_sensors,NULL);
}

static void
row_sensors_init(RowSensors *row_sensors)
{
	g_return_if_fail (row_sensors != NULL);
	g_return_if_fail (ROW_IS_SENSORS(row_sensors));
	row_sensors->priv              = row_sensors_get_instance_private (row_sensors);
	row_sensors->priv->plug_tag    = 0;
	row_sensors->priv->timer       = g_timer_new();
}

static void
row_sensors_constructed ( GObject *object )
{
	if(G_OBJECT_CLASS (row_sensors_parent_class)->constructed)
		G_OBJECT_CLASS (row_sensors_parent_class)->constructed(object);
}

static void
row_sensors_finalize (GObject *object)
{
	RowSensors* row_sensors = ROW_SENSORS(object);
	g_timer_destroy(row_sensors->priv->timer);
	G_OBJECT_CLASS (row_sensors_parent_class)->finalize(object);
}

static void
row_sensors_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_SENSORS(object));
	//RowSensors* row_sensors = ROW_SENSORS(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
row_sensors_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_SENSORS(object));
	//RowSensors* row_sensors = ROW_SENSORS(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
row_sensors_class_init(RowSensorsClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	RowServiceClass      *service_class    =  ROW_SERVICE_CLASS(klass);
	object_class -> finalize               =  row_sensors_finalize;
	object_class -> set_property           =  row_sensors_set_property;
	object_class -> get_property           =  row_sensors_get_property;
	object_class -> constructed            =  row_sensors_constructed;
	service_class->run                     =  row_sensors_run;

		/*

	gtk_widget_class_bind_template_callback (widget_class, example_signal_callback);*/
}


/** @} */
