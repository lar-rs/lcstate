/*
 * @ingroup RowCandaemon
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

#include "row-candaemon.h"



//static RowCandaemon *__gui_process_desktop = NULL;

struct _RowCandaemonPrivate
{
	CandeviceObject *candevice;
	gboolean scanned;
	guint    tag;
	GTimer  *timer;
};

enum {
	ROW_CANDAEMON_PROP_NULL,
};


enum
{
	ROW_CANDAEMON_LAST_SIGNAL
};


//static guint row_candaemon_signals[ROW_CANDAEMON_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (RowCandaemon, row_candaemon, ROW_TYPE_SERVICE);

/*
static void
on_candaemon_name_appeared (GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	g_print ("Name %s on %s is owned by %s\n",
			name,
			"the system bus",
			name_owner);
}

static void
on_candaemon_name_vanished (GDBusConnection *connection,
		const gchar     *name,
		gpointer         user_data)
{
	g_print ("Name %s does not exist on %s\n",
			name,
			"the system bus");
	RowService* row_service = ROW_SERVICE(user_data);
	g_object_set(row_service,"service-done",FALSE,NULL);
	row_service_error(row_service);
}
*/
static gboolean
candaemon_wait_scanned ( gpointer user_data )
{
	RowCandaemon* row_candaemon = ROW_CANDAEMON(user_data);
	if(candevice_simple_get_scanned(candevice_object_get_simple(row_candaemon->priv->candevice)))
	{
		g_object_set(row_candaemon,"service-done",TRUE,NULL);
		row_service_done(ROW_SERVICE(row_candaemon));
		g_signal_emit_by_name(row_candaemon,"service-ready",0,TRUE);
		//g_bus_watch_name(G_BUS_TYPE_SYSTEM,CAN_SERVICE_NAME,G_BUS_NAME_WATCHER_FLAGS_AUTO_START,on_candaemon_name_appeared,on_candaemon_name_vanished,row_candaemon,NULL);
		return FALSE;
	}
	if(g_timer_elapsed(row_candaemon->priv->timer,NULL)>180.0)
	{
		g_object_set(row_candaemon,"service-done",FALSE,NULL);
		row_service_error(ROW_SERVICE(row_candaemon));
		g_signal_emit_by_name(row_candaemon,"service-ready",0,FALSE);
		return FALSE;
	}
	return TRUE;
}

static void
row_candaemon_run ( RowService *service )
{
	RowCandaemon* row_candaemon = ROW_CANDAEMON(service);
	if(row_candaemon->priv->tag>0)
		g_source_remove(row_candaemon->priv->tag);
	g_object_set(service,"service-run",TRUE,NULL);
	row_service_run_spinner(service);
	if(mkt_dbus_system_remote())
	{
		if(mkt_dbus_system_remote_ip()!=NULL && mkt_dbus_system_remote_port() >0)
			mkt_can_manager_client_new_remote_address(mkt_dbus_system_remote_ip(),mkt_dbus_system_remote_port());
		else
			mkt_can_manager_client_new_remote(TRUE);

	}
	else
	{
		mkt_can_manager_client_new();
	}
	row_candaemon->priv->candevice = CANDEVICE_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_devices(),CAN_DEVICE_CAN0));
	if(row_candaemon->priv->candevice)
	{
		g_timer_start(row_candaemon->priv->timer);
		row_candaemon->priv->tag = g_timeout_add_seconds(1,candaemon_wait_scanned,row_candaemon);

	}
	else
	{
		g_object_set(row_candaemon,"service-done",FALSE,NULL);
		row_service_error(ROW_SERVICE(row_candaemon));
		g_signal_emit_by_name(row_candaemon,"service-ready",0,FALSE);
	}
}

static void
row_candaemon_init(RowCandaemon *row_candaemon)
{
	g_return_if_fail (row_candaemon != NULL);
	g_return_if_fail (ROW_IS_CANDAEMON(row_candaemon));
	row_candaemon->priv          = row_candaemon_get_instance_private (row_candaemon);
	row_candaemon->priv->tag     = 0;
	row_candaemon->priv->timer   = g_timer_new();
}

static void
row_candaemon_constructed ( GObject *object )
{
	if(G_OBJECT_CLASS (row_candaemon_parent_class)->constructed)
		G_OBJECT_CLASS (row_candaemon_parent_class)->constructed(object);
}

static void
row_candaemon_finalize (GObject *object)
{
	RowCandaemon* row_candaemon = ROW_CANDAEMON(object);
	g_timer_destroy(row_candaemon->priv->timer);
	G_OBJECT_CLASS (row_candaemon_parent_class)->finalize(object);
}

static void
row_candaemon_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_CANDAEMON(object));
	//RowCandaemon* row_candaemon = ROW_CANDAEMON(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
row_candaemon_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_CANDAEMON(object));
	//RowCandaemon* row_candaemon = ROW_CANDAEMON(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
row_candaemon_class_init(RowCandaemonClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	RowServiceClass      *service_class    =  ROW_SERVICE_CLASS(klass);
	object_class -> finalize               =  row_candaemon_finalize;
	object_class -> set_property           =  row_candaemon_set_property;
	object_class -> get_property           =  row_candaemon_get_property;
	object_class -> constructed            =  row_candaemon_constructed;
	service_class->run                     =  row_candaemon_run;

		/*

	gtk_widget_class_bind_template_callback (widget_class, example_signal_callback);*/
}


/** @} */
