/*
 * @ingroup RowPcdaemon
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

#include "row-pcdaemon.h"



//static RowPcdaemon *__gui_process_desktop = NULL;

struct _RowPcdaemonPrivate
{
	LarpcDevice *pcdevice;
	gboolean scanned;
	guint    tag;
	GTimer  *timer;
};

enum {
	ROW_PCDAEMON_PROP_NULL,
};


enum
{
	ROW_PCDAEMON_LAST_SIGNAL
};


//static guint row_pcdaemon_signals[ROW_PCDAEMON_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (RowPcdaemon, row_pcdaemon, ROW_TYPE_SERVICE);

/*
static void
on_pcdaemon_name_appeared (GDBusConnection *connection,
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
on_pcdaemon_name_vanished (GDBusConnection *connection,
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
pcdaemon_wait_scanned ( gpointer user_data )
{
	RowPcdaemon* row_pcdaemon = ROW_PCDAEMON(user_data);
	g_object_set(row_pcdaemon,"service-done",TRUE,NULL);
	row_service_done(ROW_SERVICE(row_pcdaemon));
	g_signal_emit_by_name(row_pcdaemon,"service-ready",0,TRUE);
	//g_bus_watch_name(G_BUS_TYPE_SYSTEM,CAN_SERVICE_NAME,G_BUS_NAME_WATCHER_FLAGS_AUTO_START,on_pcdaemon_name_appeared,on_pcdaemon_name_vanished,row_pcdaemon,NULL);
	return FALSE;
}

static void
row_pcdaemon_run ( RowService *service )
{
	RowPcdaemon* row_pcdaemon = ROW_PCDAEMON(service);
	if(row_pcdaemon->priv->tag>0)
		g_source_remove(row_pcdaemon->priv->tag);
	g_object_set(service,"service-run",TRUE,NULL);
	row_service_run_spinner(service);
	mkt_pc_manager_client_get_object_manager_remote();
	row_pcdaemon->priv->pcdevice           =  mkt_pc_manager_client_get_device();
	 if(row_pcdaemon->priv->pcdevice != NULL)
	{
		g_timer_start(row_pcdaemon->priv->timer);
		row_pcdaemon->priv->tag = g_timeout_add_seconds(1,pcdaemon_wait_scanned,row_pcdaemon);
	}
	else
	{
		g_object_set(row_pcdaemon,"service-done",FALSE,NULL);
		row_service_error(ROW_SERVICE(row_pcdaemon));
		g_signal_emit_by_name(row_pcdaemon,"service-ready",0,FALSE);
	}
}

static void
row_pcdaemon_init(RowPcdaemon *row_pcdaemon)
{
	g_return_if_fail (row_pcdaemon != NULL);
	g_return_if_fail (ROW_IS_PCDAEMON(row_pcdaemon));
	row_pcdaemon->priv          = row_pcdaemon_get_instance_private (row_pcdaemon);
	row_pcdaemon->priv->tag     = 0;
	row_pcdaemon->priv->timer   = g_timer_new();
}

static void
row_pcdaemon_constructed ( GObject *object )
{
	if(G_OBJECT_CLASS (row_pcdaemon_parent_class)->constructed)
		G_OBJECT_CLASS (row_pcdaemon_parent_class)->constructed(object);
}

static void
row_pcdaemon_finalize (GObject *object)
{
	RowPcdaemon* row_pcdaemon = ROW_PCDAEMON(object);
	g_timer_destroy(row_pcdaemon->priv->timer);
	G_OBJECT_CLASS (row_pcdaemon_parent_class)->finalize(object);
}

static void
row_pcdaemon_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_PCDAEMON(object));
	//RowPcdaemon* row_pcdaemon = ROW_PCDAEMON(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
row_pcdaemon_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_PCDAEMON(object));
	//RowPcdaemon* row_pcdaemon = ROW_PCDAEMON(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
row_pcdaemon_class_init(RowPcdaemonClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	RowServiceClass      *service_class    =  ROW_SERVICE_CLASS(klass);
	object_class -> finalize               =  row_pcdaemon_finalize;
	object_class -> set_property           =  row_pcdaemon_set_property;
	object_class -> get_property           =  row_pcdaemon_get_property;
	object_class -> constructed            =  row_pcdaemon_constructed;
	service_class->run                     =  row_pcdaemon_run;

		/*

	gtk_widget_class_bind_template_callback (widget_class, example_signal_callback);*/
}


/** @} */
