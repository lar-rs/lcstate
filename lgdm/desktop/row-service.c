/*
 * @ingroup RowService
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

#include "row-service.h"



//static RowService *__gui_process_desktop = NULL;

struct _RowServicePrivate
{
	TeraClientObject    *client;
	GtkImage            *service_error;
	GtkImage            *service_done;
	GtkSpinner          *service_runned;
	GtkLabel            *service_name;
};

enum {
	ROW_SERVICE_PROP_NULL,
	ROW_SERVICE_SERVICE_OBJECT,
};


enum
{
	ROW_SERVICE_READY,
	ROW_SERVICE_LAST_SIGNAL
};




G_DEFINE_TYPE_WITH_PRIVATE (RowService, row_service, GTK_TYPE_LIST_BOX_ROW);



static void
row_service_run ( RowService *service )
{
	g_return_if_fail(service != NULL);
	g_return_if_fail(ROW_IS_SERVICE(service));
	tera_client_run(service->priv->client);
}

static void
row_service_init(RowService *row_service)
{
	g_return_if_fail (row_service != NULL);
	g_return_if_fail (ROW_IS_SERVICE(row_service));
	row_service->priv          = row_service_get_instance_private (row_service);
	row_service->priv->client  = NULL;

	gtk_widget_init_template (GTK_WIDGET (row_service));
}

static void
row_service_constructed ( GObject *object )
{
	RowService* row_service = ROW_SERVICE(object);
	g_object_bind_property(row_service->priv->client,"server-id",row_service->priv->service_name,"label",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(row_service->priv->client,"is-run",row_service->priv->service_runned,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(row_service->priv->client,"is-run",row_service->priv->service_runned,"active",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(row_service->priv->client,"is-done",row_service->priv->service_done,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(row_service->priv->client,"critical-error",row_service->priv->service_error,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

	if(G_OBJECT_CLASS (row_service_parent_class)->constructed)
		G_OBJECT_CLASS (row_service_parent_class)->constructed(object);
}

static void
row_service_finalize (GObject *object)
{
	RowService* row_service = ROW_SERVICE(object);
	if(row_service->priv->client)    g_object_unref(row_service->priv->client);
	G_OBJECT_CLASS (row_service_parent_class)->finalize(object);
}

static void
row_service_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_SERVICE(object));
	RowService* row_service = ROW_SERVICE(object);
	switch (prop_id)
	{
	case ROW_SERVICE_SERVICE_OBJECT:
		if(row_service->priv->client)g_object_unref(row_service->priv->client);
		row_service->priv->client = g_value_dup_object(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
row_service_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_SERVICE(object));
	RowService* row_service = ROW_SERVICE(object);
	switch (prop_id)
	{
	case ROW_SERVICE_SERVICE_OBJECT:
		g_value_set_object(value,row_service->priv->client);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
row_service_class_init(RowServiceClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  row_service_finalize;
	object_class -> set_property           =  row_service_set_property;
	object_class -> get_property           =  row_service_get_property;
	object_class -> constructed            =  row_service_constructed;
	klass->run                             =  row_service_run;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/row/service-row.ui");

	gtk_widget_class_bind_template_child_private (widget_class, RowService, service_error);
	gtk_widget_class_bind_template_child_private (widget_class, RowService, service_done);
	gtk_widget_class_bind_template_child_private (widget_class, RowService, service_runned);
	gtk_widget_class_bind_template_child_private (widget_class, RowService, service_name);

	g_object_class_install_property (object_class,ROW_SERVICE_SERVICE_OBJECT,
				g_param_spec_object ("service-client",
						"Service client object",
						"Service client object",
						TERA_TYPE_CLIENT_OBJECT,
						G_PARAM_WRITABLE | G_PARAM_READABLE |  G_PARAM_CONSTRUCT_ONLY ));

	/*

	gtk_widget_class_bind_template_callback (widget_class, example_signal_callback);*/
}


void
row_service_wait_service            ( RowService *row )
{

	g_return_if_fail(row!=NULL);
	g_return_if_fail(ROW_IS_SERVICE(row));
	if(ROW_SERVICE_GET_CLASS(row)->run)
		ROW_SERVICE_GET_CLASS(row)->run(row);
}

/** @} */
