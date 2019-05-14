/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactioninterval.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactiontimer.h"
#include "ultraactioninterval.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionIntervalPrivate
{
	UltraAction *action;
	gint64 interval;
	guint64 wait;
	GDateTime *need;

	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_ACTION,
	PROP_INTERVAL,
};

#define ULTRA_ACTION_INTERVAL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_INTERVAL, UltraActionIntervalPrivate))

G_DEFINE_TYPE(UltraActionInterval, ultra_action_interval, ULTRA_TYPE_ACTION)

static void
ultra_action_interval_init(UltraActionInterval *interval)
{
	interval->priv = ULTRA_ACTION_INTERVAL_PRIVATE(interval);
	interval->priv->wait = 500;
	interval->priv->need = NULL;
	interval->priv->action = NULL;
}

static void
ultra_action_interval_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionInterval *interval = ULTRA_ACTION_INTERVAL(object);
	if (interval->priv->need)
		g_date_time_unref(interval->priv->need);
	if (interval->priv->action)
		g_object_unref(interval->priv->action);
	G_OBJECT_CLASS(ultra_action_interval_parent_class)->finalize(object);
}

static void
ultra_action_interval_set_property(GObject *object,
								   guint prop_id,
								   const GValue *value,
								   GParamSpec *pspec)
{
	UltraActionInterval *action = ULTRA_ACTION_INTERVAL(object);
	switch (prop_id)
	{
	case PROP_ACTION:
		if (action->priv->action)
			g_object_unref(action->priv->action);
		action->priv->action = g_value_dup_object(value);
		break;
	case PROP_INTERVAL:
		action->priv->interval = g_value_get_int64(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_interval_get_property(GObject *object,
								   guint prop_id,
								   GValue *value,
								   GParamSpec *pspec)
{
	UltraActionInterval *action = ULTRA_ACTION_INTERVAL(object);
	switch (prop_id)
	{
	case PROP_ACTION:
		g_value_set_object(value, action->priv->action);
		break;

	case PROP_INTERVAL:
		g_value_set_int64(value, action->priv->interval);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void intervalThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable)
{
	UltraActionInterval *interval = ULTRA_ACTION_INTERVAL(source_object);
	while (TRUE)
	{
		GDateTime *now = rt_now_utc();
		gint64 diff = g_date_time_difference(interval->priv->need, now);
		// gchar *needStr = g_date_time_format(interval->priv->need, "%FT%T");
		// gchar *nowStr = g_date_time_format(now, "%FT%T");
		// g_debug("interval %s > %s diff:%li", nowStr,needStr,diff);
		// g_free(nowStr);
		// g_free(needStr);
		g_date_time_unref(now);
		if (g_task_return_error_if_cancelled(task))
			return;
		if (diff <= 0)
		{
			g_task_return_boolean(task, TRUE);
			return;
		}
		g_usleep(interval->priv->wait);
	}
}

static gboolean ultraltraActionIntervalRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_INTERVAL(action), FALSE);
	UltraActionInterval *interval = ULTRA_ACTION_INTERVAL(action);
	GTask *task = g_task_new(interval, cancellable, callback, userData);
	g_task_run_in_thread(task, intervalThread);
	g_object_unref(task);
	return TRUE;
}

static void
ultra_action_interval_class_init(UltraActionIntervalClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionIntervalClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_interval_finalize;
	object_class->set_property = ultra_action_interval_set_property;
	object_class->get_property = ultra_action_interval_get_property;
	aclass->run = ultraltraActionIntervalRun;
	g_object_class_install_property(object_class, PROP_INTERVAL,
									g_param_spec_int64("interval", "interval", "interval", 0, G_MAXINT64, (gint64)(G_TIME_SPAN_MILLISECOND * 300), G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_ACTION,
									g_param_spec_object("action", "action", "action", ULTRA_TYPE_ACTION, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}


/**
 * UltraActionIntervalNew:
 * @to_run: a #UltraAction
 * @interval: a #gint64
 * 
 * Insert new measurement.
 *
 * Return: UltraAction .
 */

UltraAction *UltraActionIntervalNew(UltraAction *to_run,gint64 tspec)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_INTERVAL, "action",to_run, "interval",tspec, NULL));
	UltraActionInterval *interval = ULTRA_ACTION_INTERVAL(action);
	if (interval <= 0)
		return FALSE;
	GDateTime *now = rt_now_utc();
	interval->priv->need = g_date_time_add(now, tspec);
	g_date_time_unref(now);
	return action;
}

UltraAction *m_UltraActionIntervalGetNextRun(UltraActionInterval *interval)
{
	g_return_val_if_fail(ULTRA_IS_ACTION_INTERVAL(interval), NULL);
	return interval->priv->action;
}

void m_UltraActionIntervalSetImmediately(UltraActionInterval *interval){
	if(interval->priv->need){
		g_date_time_unref(interval->priv->need);
	}
	GDateTime *now = rt_now_utc();
	interval->priv->need = g_date_time_add_seconds(now,1.0);
	g_date_time_unref(now);
}