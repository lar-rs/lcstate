/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionfillsample.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include "ultraconfig.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionFillSamplePrivate
{
	gchar *pump;
	gint64 interval;
	UltraAction *timer;
	PumpsObject *pumpObject;

	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_PUMP,
	PROP_INTERVAL,
};

#define ULTRA_ACTION_FILL_SAMPLE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_FILL_SAMPLE, UltraActionFillSamplePrivate))

G_DEFINE_TYPE(UltraActionFillSample, ultra_action_fill_sample, ULTRA_TYPE_ACTION)

static void
ultra_action_fill_sample_init(UltraActionFillSample *fillSample)
{
	fillSample->priv = ULTRA_ACTION_FILL_SAMPLE_PRIVATE(fillSample);
	fillSample->priv->interval = 0;
	fillSample->priv->pump = NULL;
	fillSample->priv->timer = NULL;
}

static void
ultra_action_fill_sample_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionFillSample *fillSample = ULTRA_ACTION_FILL_SAMPLE(object);
	if (fillSample->priv->pumpObject)
	{
		PumpsPump *pump = pumps_object_get_pump(fillSample->priv->pumpObject);
		if (pumps_pump_get_is_on(pump))
			pumps_pump_call_stop(pump, NULL, NULL, NULL);
		g_object_unref(pump);
		g_object_unref(fillSample->priv->pumpObject);
	}
	if (fillSample->priv->pump)
		g_free(fillSample->priv->pump);
	if (fillSample->priv->timer)
		g_object_unref(fillSample->priv->timer);
	G_OBJECT_CLASS(ultra_action_fill_sample_parent_class)->finalize(object);
}

static void
ultra_action_fill_sample_set_property(GObject *object,
									  guint prop_id,
									  const GValue *value,
									  GParamSpec *pspec)
{
	UltraActionFillSample *fillSample = ULTRA_ACTION_FILL_SAMPLE(object);
	switch (prop_id)
	{
	case PROP_PUMP:
		if (fillSample->priv->pump)
			g_free(fillSample->priv->pump);
		fillSample->priv->pump = g_value_dup_string(value);
		break;
	case PROP_INTERVAL:
		fillSample->priv->interval = g_value_get_int64(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_fill_sample_get_property(GObject *object,
									  guint prop_id,
									  GValue *value,
									  GParamSpec *pspec)
{
	UltraActionFillSample *fillSample = ULTRA_ACTION_FILL_SAMPLE(object);
	switch (prop_id)
	{
	case PROP_PUMP:
		g_value_set_string(value, fillSample->priv->pump);
		break;
	case PROP_INTERVAL:
		g_value_set_int64(value, fillSample->priv->interval);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionFillSampleRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_fill_sample_class_init(UltraActionFillSampleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionFillSampleClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_fill_sample_finalize;
	object_class->set_property = ultra_action_fill_sample_set_property;
	object_class->get_property = ultra_action_fill_sample_get_property;
	aclass->run = ultraltraActionFillSampleRun;
	g_object_class_install_property(object_class, PROP_INTERVAL,
									g_param_spec_int64("interval", "interval", "interval", 0, G_MAXINT64, 0, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_PUMP,
									g_param_spec_string("pump", "pump", "pump", TERA_PUMPS_1_PATH, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionFillSampleNew(const gchar *pump, gint64 interval)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_FILL_SAMPLE,"name","fill sample", "pump", pump, "interval", interval, NULL));
	return action;
}

static void fill_sampleTimerFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(res)))
	{
		g_object_unref(task);
		return;
	}
	UltraActionFillSample *fillSample = ULTRA_ACTION_FILL_SAMPLE(g_task_get_source_object(task));
	if (fillSample->priv->pumpObject)
	{
		PumpsPump *pump = pumps_object_get_pump(fillSample->priv->pumpObject);
		pumps_pump_call_stop(pump, NULL, NULL, NULL);
		g_object_unref(pump);
	}
	UltraAction *action = ULTRA_ACTION(sourceObject);
	g_task_return_boolean(task, m_UltraActionFinish(action, res));
	g_object_unref(task);
}

gboolean ultraltraActionFillSampleRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_FILL_SAMPLE(action), FALSE);
	UltraActionFillSample *fillSample = ULTRA_ACTION_FILL_SAMPLE(action);
	GTask *task = g_task_new(fillSample, cancellable, callback, userData);
	// g_debug("ultraltraActionFillSampleRun");
	if (fillSample->priv->interval < 10 * G_TIME_SPAN_MILLISECOND)
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	fillSample->priv->pumpObject = tera_pumps_manager_client_get_pump(fillSample->priv->pump);
	if (fillSample->priv->pumpObject == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_SAMPLE_PUMP, "Fill sample start error - pump is null"));
		g_object_unref(task);
		return FALSE;
	}
	if (ConfigureIsTestMode())
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	if (fillSample->priv->timer)
		g_object_unref(fillSample->priv->timer);

	PumpsPump *pump = pumps_object_get_pump(fillSample->priv->pumpObject);
	pumps_pump_call_start(pump, NULL, NULL, NULL);
	g_object_unref(pump);
	fillSample->priv->timer = UltraActionTimerNew(fillSample->priv->interval);
	m_UltraActionRun(fillSample->priv->timer, cancellable, fill_sampleTimerFinishCallback, task);
	return TRUE;
}