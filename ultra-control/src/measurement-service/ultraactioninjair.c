/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactioninjair.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactioninjair.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include "ultraconfig.h"


#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionInjairPrivate
{
	UltraBusStream *stream;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_STREAM,
};

#define ULTRA_ACTION_INJAIR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_INJAIR, UltraActionInjairPrivate))

G_DEFINE_TYPE(UltraActionInjair, ultra_action_injair, ULTRA_TYPE_ACTION)

static void
ultra_action_injair_init(UltraActionInjair *injair)
{
	injair->priv = ULTRA_ACTION_INJAIR_PRIVATE(injair);
	injair->priv->stream = NULL;
}

static void
ultra_action_injair_finalize(GObject *object)
{
	UltraActionInjair *injair = ULTRA_ACTION_INJAIR(object);
	if (injair->priv->stream)
		g_object_unref(injair->priv->stream);
	G_OBJECT_CLASS(ultra_action_injair_parent_class)->finalize(object);
}

static gboolean ultraltraActionInjairRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_injair_class_init(UltraActionInjairClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionInjairClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_injair_finalize;
	aclass->run = ultraltraActionInjairRun;
}

UltraAction *UltraActionInjairNew(UltraBusStream *stream)
{
	UltraActionInjair *action = ULTRA_ACTION_INJAIR(g_object_new(ULTRA_TYPE_ACTION_INJAIR, NULL));
	action->priv->stream = g_object_ref(stream);
	return ULTRA_ACTION(action);
}

static void injairFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	GError * error     = NULL;
    gboolean out_value = FALSE;
    if (!airflow_sensor_call_injection_analyse_out_finish(AIRFLOW_SENSOR(sourceObject), &out_value, res, &error)) {
        if (error){
			g_dbus_error_strip_remote_error(error);
			g_task_return_error(task, error);
			g_object_unref(task);
			return;
		} 
    } else {
		UltraActionInjair *injair = ULTRA_ACTION_INJAIR(g_task_get_source_object(task));

		guint number = (guint)m_UltraBusStreamGetNumber(injair->priv->stream)-1;
		if(number >5)number= 5;
        if (!out_value)
            mkt_errors_come(E1841 + number);
        else
            mkt_errors_clean(E1841 + number);
    }
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
}

static void injairTestFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
}

gboolean ultraltraActionInjairRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_INJAIR(action), FALSE);
	UltraActionInjair *injair = ULTRA_ACTION_INJAIR(action);
	GTask *task = g_task_new(injair, cancellable, callback, userData);
	if (ConfigureIsTestMode())
	{
		UltraAction *test_timer = UltraActionTimerNew(14*G_TIME_SPAN_SECOND);
		m_UltraActionSetName(test_timer,"I:test-air-analyze");
		m_UltraActionRun(test_timer,cancellable,injairTestFinishCallback,task);
		g_object_unref(test_timer);
		return TRUE;
	}
	AirflowSensor *sensor = airflow_object_get_sensor(ULTRA_AIRFLOW());
	airflow_sensor_set_inj_analyse_timeout(sensor, 20.0);
    airflow_sensor_call_injection_analyse_out(sensor, cancellable, injairFinishCallback, task);
	g_object_unref(sensor);
	return TRUE;
}