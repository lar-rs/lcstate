/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactiontimer.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionTimerPrivate
{
	gint64 interval;
	guint64 wait;
	GDateTime *need;

	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_INTERVAL,
};

#define ULTRA_ACTION_TIMER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_TIMER, UltraActionTimerPrivate))

G_DEFINE_TYPE(UltraActionTimer, ultra_action_timer, ULTRA_TYPE_ACTION)

static void
ultra_action_timer_init(UltraActionTimer *timer)
{
	timer->priv = ULTRA_ACTION_TIMER_PRIVATE(timer);
	timer->priv->wait =	5;
	timer->priv->need = NULL;
}

static void
ultra_action_timer_finalize(GObject *object)
{
	UltraActionTimer *timer = ULTRA_ACTION_TIMER(object);
	// g_debug("Ultra timer finalize %li\n",timer->priv->interval);
	if (timer->priv->need)
		g_date_time_unref(timer->priv->need);
	G_OBJECT_CLASS(ultra_action_timer_parent_class)->finalize(object);
}

static void
ultra_action_timer_set_property(GObject *object,
								guint prop_id,
								const GValue *value,
								GParamSpec *pspec)
{
	UltraActionTimer *action = ULTRA_ACTION_TIMER(object);
	switch (prop_id)
	{
	case PROP_INTERVAL:
		action->priv->interval = g_value_get_int64(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_timer_get_property(GObject *object,
								guint prop_id,
								GValue *value,
								GParamSpec *pspec)
{
	UltraActionTimer *action = ULTRA_ACTION_TIMER(object);
	switch (prop_id)
	{
	case PROP_INTERVAL:
		g_value_set_int64(value, action->priv->interval);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

void timerThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable)
{
	UltraActionTimer *timer = ULTRA_ACTION_TIMER(source_object);
	while (TRUE)
	{
		if (g_task_return_error_if_cancelled(task)){
			return;
		}
		GDateTime *now = rt_now_utc();
		gint64 diff = g_date_time_difference(timer->priv->need, now);
		// gchar *needStr = g_date_time_format(timer->priv->need, "%FT%T");
		// gchar *nowStr = g_date_time_format(now, "%FT%T");
		// g_debug("timer %s > %s diff:%li", nowStr,needStr,diff);
		g_date_time_unref(now);
		// g_free(nowStr);
		// g_free(needStr);
		
		if (diff <= 0)
		{
			// g_debug("timer %s:%"G_GINT64_MODIFIER"d done", m_UltraActionGetName(ULTRA_ACTION(timer)), timer->priv->interval);
			g_task_return_boolean(task, TRUE);
			return;
		}
		g_usleep(timer->priv->wait);
	}
}

static gboolean ultraltraActionTimerRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_TIMER(action), FALSE);
	UltraActionTimer *timer = ULTRA_ACTION_TIMER(action);
	GDateTime *now = rt_now_utc();
	timer->priv->need = g_date_time_add(now, timer->priv->interval);
	g_date_time_unref(now);
	GTask *task = g_task_new(timer, cancellable, callback, userData);
	// g_task_set_check_cancellable (task,FALSE);
	// g_task_set_return_on_cancel(task,TRUE);
	// g_debug("RETURN ON CANCABLE %d",g_task_get_return_on_cancel(task));
	g_task_run_in_thread(task, timerThread);
	g_object_unref(task);
	return TRUE;
}

static void
ultra_action_timer_class_init(UltraActionTimerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionTimerClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_timer_finalize;
	object_class->set_property = ultra_action_timer_set_property;
	object_class->get_property = ultra_action_timer_get_property;
	aclass->run = ultraltraActionTimerRun;
	g_object_class_install_property(object_class, PROP_INTERVAL,
									g_param_spec_int64("interval", "interval", "interval", 0, G_MAXINT64, (gint64)(G_TIME_SPAN_MILLISECOND * 300), G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionTimerNew(gint64 interval)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_TIMER, "interval", interval, NULL));
	return action;
}
UltraAction *UltraActionTimerNewWithName(gint64 interval, const gchar *name)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_TIMER, "interval", interval, "name", name, NULL));
	return action;
}



gint64 m_UltraActionTimerGetInterval(UltraActionTimer *timer){
	g_return_val_if_fail(timer!=NULL,0);
	g_return_val_if_fail(ULTRA_ACTION_TIMER(timer),0);
	return timer->priv->interval;
}

static gint64 RT_TIME = 0;
static gboolean RT_SIMULATION = FALSE;
static guint64 RT_INTERVAL = G_TIME_SPAN_SECOND / G_TIME_SPAN_MILLISECOND;

GDateTime *rt_now_local()
{
	if (!RT_SIMULATION || !RT_SIMULATION == 0 || RT_TIME ==0 )
		return g_date_time_new_now_local();
	GDateTime *now = NULL;
	now = g_date_time_new_from_unix_local(RT_TIME);
	return now;
}

GDateTime *rt_now_utc()
{
	if (!RT_SIMULATION || RT_TIME ==0 )
		return g_date_time_new_now_utc();
	GDateTime *now = NULL;
	now = g_date_time_new_from_unix_utc(RT_TIME);
	return now;
}

gboolean rt_simulation()
{
	return RT_SIMULATION;
}

guint rt_interval()
{
	return RT_INTERVAL;
}

static gboolean simulationRTCallback(gpointer data)
{
	RT_TIME += 1;
	GDateTime *sdt = g_date_time_new_from_unix_utc(RT_TIME);
	gchar *dtStr = g_date_time_format(sdt, "%FT%T");
	// g_debug("simulation time %s", dtStr);
	g_date_time_unref(sdt);
	g_free(dtStr);
	return TRUE;
}

void rt_run_simulation(GDateTime *dt, guint interval)
{
	if(!actions_get_testing_mod()){
		return;
	}
	RT_SIMULATION = TRUE;
	RT_INTERVAL = interval;
	g_timeout_add(RT_INTERVAL, simulationRTCallback, NULL);
	RT_TIME = dt != NULL ? g_date_time_to_unix(dt) : g_get_real_time() / G_TIME_SPAN_SECOND;
}

void rt_clean()
{
	RT_SIMULATION = FALSE;
	RT_INTERVAL = G_TIME_SPAN_SECOND / G_TIME_SPAN_MILLISECOND;
}
