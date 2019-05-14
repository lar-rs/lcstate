/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionstripping.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"

#include "ultraconfig.h"
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionStrippingPrivate
{
	gint64 interval;
	UltraAction *timer;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_INTERVAL,
};

#define ULTRA_ACTION_STRIPPING_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_STRIPPING, UltraActionStrippingPrivate))

G_DEFINE_TYPE(UltraActionStripping, ultra_action_stripping, ULTRA_TYPE_ACTION)

static void
ultra_action_stripping_init(UltraActionStripping *fillSample)
{
	fillSample->priv = ULTRA_ACTION_STRIPPING_PRIVATE(fillSample);
	fillSample->priv->interval = 0;
	fillSample->priv->timer = NULL;
}

static void
ultra_action_stripping_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionStripping *fillSample = ULTRA_ACTION_STRIPPING(object);
	if (fillSample->priv->timer)
		g_object_unref(fillSample->priv->timer);
	G_OBJECT_CLASS(ultra_action_stripping_parent_class)->finalize(object);
}

static void
ultra_action_stripping_set_property(GObject *object,
									  guint prop_id,
									  const GValue *value,
									  GParamSpec *pspec)
{
	UltraActionStripping *fillSample = ULTRA_ACTION_STRIPPING(object);
	switch (prop_id)
	{
	case PROP_INTERVAL:
		fillSample->priv->interval = g_value_get_int64(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_stripping_get_property(GObject *object,
									  guint prop_id,
									  GValue *value,
									  GParamSpec *pspec)
{
	UltraActionStripping *fillSample = ULTRA_ACTION_STRIPPING(object);
	switch (prop_id)
	{
	case PROP_INTERVAL:
		g_value_set_int64(value, fillSample->priv->interval);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionStrippingRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_stripping_class_init(UltraActionStrippingClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionStrippingClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_stripping_finalize;
	object_class->set_property = ultra_action_stripping_set_property;
	object_class->get_property = ultra_action_stripping_get_property;
	aclass->run = ultraltraActionStrippingRun;
	g_object_class_install_property(object_class, PROP_INTERVAL,
									g_param_spec_int64("interval", "interval", "interval", 0, G_MAXINT64, 0, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionStrippingNew(gint64 interval)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_STRIPPING, "interval", interval, NULL));
	return action;
}

static void strippingTimerFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(res)))
	{
		g_object_unref(task);
		return;
	}
	UltraAction *action = ULTRA_ACTION(sourceObject);
	g_task_return_boolean(task, m_UltraActionFinish(action, res));
	g_object_unref(task);
}

gboolean ultraltraActionStrippingRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_STRIPPING(action), FALSE);
	UltraActionStripping *fillSample = ULTRA_ACTION_STRIPPING(action);
	GTask *task = g_task_new(fillSample, cancellable, callback, userData);
	if (fillSample->priv->interval < 10 * G_TIME_SPAN_MILLISECOND)
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	if (fillSample->priv->timer)
		g_object_unref(fillSample->priv->timer);

	fillSample->priv->timer = UltraActionTimerNew(fillSample->priv->interval);
	m_UltraActionRun(fillSample->priv->timer, cancellable, strippingTimerFinishCallback, task);
	return TRUE;
}