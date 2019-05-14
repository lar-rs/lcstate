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

#include "sequence-workers-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>

#include "ultra-allhold-process.h"
#include "ultra-axishold-process.h"
#include "ultra-injection-process.h"
#include "ultra-injectiontic-process.h"
#include "ultra-injectioncod-process.h"
#include "ultra-rinsing-process.h"
#include "ultra-sampling-process.h"
#include "ultra-samplingcod-process.h"
#include "ultra-dilution-process.h"


#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
	PROP_SEQUENCE_WORKERS_IS_BUSY
};


struct _SequenceApplicationWorkersObjectPrivate
{
	GDBusObjectManagerServer  *sequence_manager;
	gboolean                   busy;
	GCancellable              *cancelable;
};





G_DEFINE_TYPE_WITH_PRIVATE (SequenceApplicationWorkersObject, sequence_application_workers_object, TERA_TYPE_SERVICE_OBJECT);

static void
sequence_application_workers_initialize_process (SequenceApplicationWorkersObject *sequence_workers)
{

	if(X_AXIS()==NULL || achsen_object_get_achse(ACHSEN_OBJECT(X_AXIS()) )== NULL){
			mkt_log_error_message_sync("Sequence service:X axis not found");
			// g_error("Sequence service:X axis not found");
	}
	if(Y_AXIS()==NULL || achsen_object_get_achse(ACHSEN_OBJECT(Y_AXIS()) )== NULL){
			mkt_log_error_message_sync("Sequence service:Y axis not found");
			// g_error("Sequence service:Y axis not found");
	}
	if(Z_AXIS()==NULL || achsen_object_get_achse(ACHSEN_OBJECT(Z_AXIS()))  == NULL|| achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS())) == NULL){
			mkt_log_error_message_sync("Sequence service:Injection axis not found");
			// g_error("Sequence service:Injection axis not found");
	}
	mkt_errors_clean(E1750);

	GDBusObjectSkeleton *axishold_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_AXISHOLD_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_AXISHOLD_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,axishold_process);

	GDBusObjectSkeleton *hold_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_ALLHOLD_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_HOLD_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,hold_process);

	GDBusObjectSkeleton *injection_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_INJECTION_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_INJECTION_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,injection_process);

	GDBusObjectSkeleton *injection_tic_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_INJECTIONTIC_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_INJECTIONTIC_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,injection_tic_process);

	GDBusObjectSkeleton *injection_cod_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_INJECTIONCOD_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_INJECTIONCOD_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,injection_cod_process);

	GDBusObjectSkeleton *rinsing_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_RINSING_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_RINSING_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,rinsing_process);

	GDBusObjectSkeleton *sampling_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_SAMPLING_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_SAMPLING_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,sampling_process);

	GDBusObjectSkeleton *sampling_cod_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_SAMPLINGCOD_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_SAMPLINGCOD_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,sampling_cod_process);

	GDBusObjectSkeleton *dilution_process  = G_DBUS_OBJECT_SKELETON(g_object_new(ULTRA_TYPE_DILUTION_PROCESS,"g-object-path",ULTRA_SEQUENCE_WORKERS_DILUTION_PATH,NULL));
	g_dbus_object_manager_server_export(sequence_workers->priv->sequence_manager,dilution_process);


	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(axishold_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(hold_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(injection_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(injection_tic_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(injection_cod_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(rinsing_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(sampling_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(sampling_cod_process)),"busy",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(sequence_workers,"workers-busy",sequence_object_get_workers_process(SEQUENCE_OBJECT(dilution_process)),"busy",G_BINDING_BIDIRECTIONAL);


}


static void
sequence_application_workers_object_init (SequenceApplicationWorkersObject *sequence_application_workers_object)
{
	SequenceApplicationWorkersObjectPrivate *priv      =  sequence_application_workers_object_get_instance_private (sequence_application_workers_object);
	priv->sequence_manager                             =  NULL;
	priv->cancelable                                   =  g_cancellable_new();
	ULTRA_SEQUENCE_WORKERS_ERRORS;
	sequence_application_workers_object->priv          =  priv;
}



static void
sequence_application_workers_object_finalize (GObject *object)
{
	SequenceApplicationWorkersObject *sequence_application_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(object);
	if(sequence_application_workers->priv->sequence_manager) 	g_object_unref(sequence_application_workers->priv->sequence_manager);
	G_OBJECT_CLASS (sequence_application_workers_object_parent_class)->finalize (object);
}


static void
sequence_application_workers_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (SEQUENCE_IS_WORKERS_APPLICATION_OBJECT(object));
	SequenceApplicationWorkersObject *sequence_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(object);
	switch (prop_id)
	{
	case PROP_SEQUENCE_WORKERS_IS_BUSY:
		sequence_workers->priv->busy = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
sequence_application_workers_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (SEQUENCE_IS_WORKERS_APPLICATION_OBJECT (object));
	SequenceApplicationWorkersObject *sequence_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(object);
	switch (prop_id)
	{
	case PROP_SEQUENCE_WORKERS_IS_BUSY:
		g_value_set_boolean(value,sequence_workers->priv->busy);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
sequence_application_workers_object_activated (TeraServiceObject *service)
{
	SequenceApplicationWorkersObject *sequence_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(service);
	sequence_workers->priv->sequence_manager = g_dbus_object_manager_server_new (ULTRA_SEQUENCE_WORKERS_MANAGE_PATH);
	g_dbus_object_manager_server_set_connection (sequence_workers->priv->sequence_manager , tera_service_dbus_connection());
	sequence_application_workers_initialize_process(sequence_workers);
	service_simple_set_done(tera_service_get_simple(),TRUE);
	service_simple_emit_initialized(tera_service_get_simple(),TRUE);
}


static void
sequence_application_workers_object_class_init (SequenceApplicationWorkersObjectClass *klass)
{
	GObjectClass* object_class        = G_OBJECT_CLASS (klass);
	TeraServiceObjectClass *app_class = TERA_SERVICE_OBJECT_CLASS(klass);
	object_class->finalize            = sequence_application_workers_object_finalize;
	object_class->set_property        = sequence_application_workers_object_set_property;
	object_class->get_property        = sequence_application_workers_object_get_property;
	app_class->activated              = sequence_application_workers_object_activated;


	g_object_class_install_property(object_class,PROP_SEQUENCE_WORKERS_IS_BUSY,
				g_param_spec_boolean("workers-busy",
						"workers busy",
						"workers busy",
						FALSE,
						G_PARAM_READABLE | G_PARAM_WRITABLE ) );

}



GCancellable*
sequence_application_cancelable                               ( void )
{
	SequenceApplicationWorkersObject *sequence_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(tera_service_get_default());
	g_return_val_if_fail(sequence_workers!=NULL,NULL);
	g_return_val_if_fail(SEQUENCE_IS_WORKERS_APPLICATION_OBJECT(sequence_workers),NULL);
	return sequence_workers->priv->cancelable;
}

void
sequence_application_cancelable_init                          ( void )
{
	SequenceApplicationWorkersObject *sequence_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(tera_service_get_default());
	g_return_if_fail(sequence_workers!=NULL);
	g_return_if_fail(SEQUENCE_IS_WORKERS_APPLICATION_OBJECT(sequence_workers));
	g_cancellable_cancel(sequence_workers->priv->cancelable);
	g_cancellable_reset(sequence_workers->priv->cancelable);
}

void
sequence_application_cancelable_cancel                        ( void )
{
	SequenceApplicationWorkersObject *sequence_workers = SEQUENCE_WORKERS_APPLICATION_OBJECT(tera_service_get_default());
	g_return_if_fail(sequence_workers!=NULL);
	g_return_if_fail(SEQUENCE_IS_WORKERS_APPLICATION_OBJECT(sequence_workers));
	g_cancellable_cancel(sequence_workers->priv->cancelable);
}
