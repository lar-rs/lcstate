/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionrinsing.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionrinsing.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include "ultraconfig.h"


#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionRinsingPrivate
{
	gchar *vessel;
	guint volume;
	guint repeat;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_VESSEL,
	PROP_VOLUME,
	PROP_REPEAT,
};

#define ULTRA_ACTION_RINSING_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_RINSING, UltraActionRinsingPrivate))

G_DEFINE_TYPE(UltraActionRinsing, ultra_action_rinsing, ULTRA_TYPE_ACTION)

static void
ultra_action_rinsing_init(UltraActionRinsing *rinsing)
{
	rinsing->priv = ULTRA_ACTION_RINSING_PRIVATE(rinsing);
	rinsing->priv->volume = 100;
	rinsing->priv->repeat = 1;
	rinsing->priv->vessel = NULL;
}

static void
ultra_action_rinsing_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionRinsing *rinsing = ULTRA_ACTION_RINSING(object);
	if (rinsing->priv->vessel)
		g_free(rinsing->priv->vessel);
	G_OBJECT_CLASS(ultra_action_rinsing_parent_class)->finalize(object);
}

static void
ultra_action_rinsing_set_property(GObject *object,
								   guint prop_id,
								   const GValue *value,
								   GParamSpec *pspec)
{
	UltraActionRinsing *rinsing = ULTRA_ACTION_RINSING(object);
	switch (prop_id)
	{
	case PROP_VESSEL:
		if (rinsing->priv->vessel)
			g_free(rinsing->priv->vessel);
		rinsing->priv->vessel = g_value_dup_string(value);
		break;
	case PROP_VOLUME:
		rinsing->priv->volume = g_value_get_uint(value);
		break;
	case PROP_REPEAT:
		rinsing->priv->repeat = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_rinsing_get_property(GObject *object,
								   guint prop_id,
								   GValue *value,
								   GParamSpec *pspec)
{
	UltraActionRinsing *rinsing = ULTRA_ACTION_RINSING(object);
	switch (prop_id)
	{
	case PROP_VESSEL:
		g_value_set_string(value, rinsing->priv->vessel);
		break;
	case PROP_VOLUME:
		g_value_set_uint(value, rinsing->priv->volume);
		break;
	case PROP_REPEAT:
		g_value_set_uint(value, rinsing->priv->repeat);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionRinsingRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_rinsing_class_init(UltraActionRinsingClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionRinsingClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_rinsing_finalize;
	object_class->set_property = ultra_action_rinsing_set_property;
	object_class->get_property = ultra_action_rinsing_get_property;
	aclass->run = ultraltraActionRinsingRun;
	g_object_class_install_property(object_class, PROP_VOLUME,
									g_param_spec_uint("volume", "volume", "volume", 0, 2000, 100, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_REPEAT,
									g_param_spec_uint("repeat", "repeat", "repeat", 1, 20, 1, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_VESSEL,
									g_param_spec_string("vessel", "vessel", "vessel", NULL, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionRinsingNew(const gchar *vessel, guint volume, guint repeat)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_RINSING, "vessel", vessel, "volume", volume,"repeat",repeat, NULL));
	return action;
}

static void rinsingFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
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

gboolean ultraltraActionRinsingRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_RINSING(action), FALSE);
	UltraActionRinsing *rinsing = ULTRA_ACTION_RINSING(action);
	GTask *task = g_task_new(rinsing, cancellable, callback, userData);
	if (ConfigureIsTestMode())
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	SequenceObject *worker = ULTRA_RINSING_SEQUENCE_WORKER();
	if (worker == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_WORKER_NOT_FOUND, "Rinsing start error - rinsing worker not found"));
		g_object_unref(task);
		return FALSE;
	}
	SequenceWorkersSample *workerSample = sequence_object_get_workers_sample(worker);
	SequenceWorkersProcess *workerProcess = sequence_object_get_workers_process(worker);
	// sequence_workers_sample_set_sample_main(workerSample, rinsing->priv->vessel);
  	sequence_workers_sample_set_repeat(sequence_object_get_workers_sample(ULTRA_RINSING_SEQUENCE_WORKER()),rinsing->priv->repeat);
  	sequence_workers_process_call_run(workerProcess, cancellable, rinsingFinishCallback, task);
	g_object_unref(workerSample);
	g_object_unref(workerProcess);
	return TRUE;
}