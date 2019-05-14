/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionsampling.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionsampling.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include "ultraconfig.h"


#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionSamplingPrivate
{
	gchar *vessel;
	gint volume;
	gboolean isCodo;
	GDBusObjectManager *manager;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_VESSEL,
	PROP_VOLUME,
	PROP_ISCODO,
};

#define ULTRA_ACTION_SAMPLING_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_SAMPLING, UltraActionSamplingPrivate))

G_DEFINE_TYPE(UltraActionSampling, ultra_action_sampling, ULTRA_TYPE_ACTION)

static void
ultra_action_sampling_init(UltraActionSampling *sampling)
{
	sampling->priv = ULTRA_ACTION_SAMPLING_PRIVATE(sampling);
	sampling->priv->volume = 100;
	sampling->priv->isCodo = FALSE;
	sampling->priv->vessel = NULL;
}

static void
ultra_action_sampling_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionSampling *sampling = ULTRA_ACTION_SAMPLING(object);
	if (sampling->priv->vessel)
		g_free(sampling->priv->vessel);
	if(sampling->priv->manager){
		g_object_unref(sampling->priv->manager);
	}
	G_OBJECT_CLASS(ultra_action_sampling_parent_class)->finalize(object);
}

static void
ultra_action_sampling_set_property(GObject *object,
								   guint prop_id,
								   const GValue *value,
								   GParamSpec *pspec)
{
	UltraActionSampling *sampling = ULTRA_ACTION_SAMPLING(object);
	switch (prop_id)
	{
	case PROP_VESSEL:
		if (sampling->priv->vessel)
			g_free(sampling->priv->vessel);
		sampling->priv->vessel = g_value_dup_string(value);
		break;
	case PROP_VOLUME:
		sampling->priv->volume = g_value_get_uint(value);
		break;
	case PROP_ISCODO:
		sampling->priv->isCodo = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_sampling_get_property(GObject *object,
								   guint prop_id,
								   GValue *value,
								   GParamSpec *pspec)
{
	UltraActionSampling *sampling = ULTRA_ACTION_SAMPLING(object);
	switch (prop_id)
	{
	case PROP_VESSEL:
		g_value_set_string(value, sampling->priv->vessel);
		break;
	case PROP_VOLUME:
		g_value_set_int64(value, sampling->priv->volume);
		break;
	case PROP_ISCODO:
		g_value_set_boolean(value, sampling->priv->isCodo);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionSamplingRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_sampling_class_init(UltraActionSamplingClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionSamplingClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_sampling_finalize;
	object_class->set_property = ultra_action_sampling_set_property;
	object_class->get_property = ultra_action_sampling_get_property;
	aclass->run = ultraltraActionSamplingRun;
	g_object_class_install_property(object_class, PROP_VOLUME,
									g_param_spec_uint("volume", "volume", "volume", 0, 2000, 100, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_VOLUME,
									g_param_spec_uint("iscodo", "volume", "volume", 0, 2000, 100, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_VESSEL,
									g_param_spec_string("vessel", "vessel", "vessel", NULL, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionSamplingNew(const gchar *vessel, guint volume, gboolean isCodo)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_SAMPLING, "vessel", vessel, "volume", volume,"iscodo",isCodo, NULL));
	return action;
}

static void samplingFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	gboolean result = FALSE;
	GError *error = NULL;

	if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(sourceObject), &result, res, &error))
	{
		if (error)
			g_dbus_error_strip_remote_error(error);

		g_task_return_error(task, error);
		g_object_unref(task);
		return;
	}
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
}

gboolean ultraltraActionSamplingRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_SAMPLING(action), FALSE);
	UltraActionSampling *sampling = ULTRA_ACTION_SAMPLING(action);
	GTask *task = g_task_new(sampling, cancellable, callback, userData);
	// g_debug("Sampling sequense run");
	if (ConfigureIsTestMode())
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	GError *error = NULL;
	sampling->priv->manager = sequence_object_manager_client_new_for_bus_sync(G_BUS_TYPE_SESSION,G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
	ULTRA_SEQUENCE_WORKERS_NAME,ULTRA_SEQUENCE_WORKERS_MANAGE_PATH,cancellable,&error);
	if(error){
		g_dbus_error_strip_remote_error(error);
		g_task_return_error(task, error);
		g_object_unref(task);
		return FALSE;
	}
	const gchar *path = sampling->priv->isCodo?ULTRA_SEQUENCE_WORKERS_SAMPLINGCOD_PATH:ULTRA_SEQUENCE_WORKERS_SAMPLING_PATH;
	SequenceObject *worker =SEQUENCE_OBJECT(g_dbus_object_manager_get_object(sampling->priv->manager,path));
	if (worker == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_WORKER_NOT_FOUND, "Sampling start error - sampling worker not found"));
		g_object_unref(task);
		return FALSE;
	}
	SequenceWorkersSample *workerSample = sequence_object_get_workers_sample(worker);
	SequenceWorkersProcess *workerProcess = sequence_object_get_workers_process(worker);
	sequence_workers_sample_set_sample_main(workerSample, sampling->priv->vessel);
	sequence_workers_sample_set_volume(workerSample, sampling->priv->volume);
	sequence_workers_process_call_run(workerProcess, cancellable, samplingFinishCallback, task);
	g_object_unref(workerSample);
	g_object_unref(workerProcess);
	g_object_unref(worker);
	return TRUE;
}