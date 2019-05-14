/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactiononline.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactiononline.h"
#include "actioninterfaces.h"
#include "ultraactionanalyze.h"
#include "ultraactionfillsample.h"
#include "ultraactionstripping.h"
#include "ultraactionsampling.h"
#include "ultraactionrinsing.h"
#include "ultraactionhold.h"
#include "ultraactiontimer.h"
#include "ultraactioninterval.h"
#include "ultraactionqueue.h"
#include "ultraactiongroup.h"
#include "ultraerrors.h"
#include "ultraconfig.h"
#include "ultraactionmeasurement.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionOnlinePrivate
{
	GCancellable *internal;
	GCancellable *main;
	GList *streams;
	UltraActionQueue *measurements;
	UltraActionQueue *remote_measurements;
	UltraActionQueue *immediately;

	GDateTime *last_reset;
	GDateTime *pre_reset;
	guint reset_counter;
	gboolean reseted;
	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_SWITCH_RANGE,
	PROP_RANGE1,
	PROP_RANGE2,
	PROP_PUMP,
	PROP_VESSEL,
};

#define ULTRA_ACTION_ONLINE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_ONLINE, UltraActionOnlinePrivate))

G_DEFINE_TYPE(UltraActionOnline, ultra_action_online, ULTRA_TYPE_ACTION)

static void
ultra_action_online_init(UltraActionOnline *online)
{
	online->priv = ULTRA_ACTION_ONLINE_PRIVATE(online);
	online->priv->measurements = ULTRA_ACTION_QUEUE(UltraActionQueueNew());
	online->priv->remote_measurements = ULTRA_ACTION_QUEUE(UltraActionQueueNew());
	online->priv->immediately = ULTRA_ACTION_QUEUE(UltraActionQueueNew());
}

static void
ultra_action_online_finalize(GObject *object)
{
	UltraActionOnline *online = ULTRA_ACTION_ONLINE(object);
	if (online->priv->internal)
	{
		g_cancellable_cancel(online->priv->internal);
	}
	if (online->priv->remote_measurements)
		g_object_unref(online->priv->remote_measurements);
	if (online->priv->immediately)
		g_object_unref(online->priv->immediately);
	if (online->priv->measurements)
		g_object_unref(online->priv->measurements);
	G_OBJECT_CLASS(ultra_action_online_parent_class)->finalize(object);
}

static void
ultra_action_online_set_property(GObject *object,
								 guint prop_id,
								 const GValue *value,
								 GParamSpec *pspec)
{
	// UltraActionOnline *action = ULTRA_ACTION_ONLINE(object);
	// switch (prop_id)
	// {
	// case PROP_STATE:
	// 	action->priv->state = g_value_get_uint(value);
	// 	break;
	// case PROP_STREAM:
	// 	if (action->priv->stream)
	// 		g_object_unref(action->priv->stream);
	// 	action->priv->stream = g_value_dup_object(value);
	// 	break;
	// case PROP_PUMP:
	// 	if (action->priv->pump)
	// 		g_free(action->priv->pump);
	// 	action->priv->pump = g_value_dup_string(value);
	// 	break;
	// case PROP_VESSEL:
	// 	if (action->priv->vessel)
	// 		g_free(action->priv->vessel);
	// 	action->priv->vessel = g_value_dup_string(value);
	// 	break;
	// default:
	// 	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	// 	break;
	// }
}

static void
ultra_action_online_get_property(GObject *object,
								 guint prop_id,
								 GValue *value,
								 GParamSpec *pspec)
{
	// UltraActionOnline *action = ULTRA_ACTION_ONLINE(object);
	// switch (prop_id)
	// {
	// case PROP_STATE:
	// 	g_value_set_uint(value, action->priv->state);
	// 	break;
	// case PROP_STREAM:
	// 	g_value_set_object(value, action->priv->stream);
	// 	break;
	// case PROP_PUMP:
	// 	g_value_set_string(value, action->priv->pump);
	// 	break;
	// case PROP_VESSEL:
	// 	g_value_set_string(value, action->priv->vessel);
	// 	break;
	// default:
	// 	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	// 	break;
	// }
}

static gboolean ultraActionOnlineRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_online_class_init(UltraActionOnlineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionOnlineClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	//object_class->dispose              = ultimate_atom_dispose;
	object_class->finalize = ultra_action_online_finalize;
	object_class->set_property = ultra_action_online_set_property;
	object_class->get_property = ultra_action_online_get_property;
	aclass->run = ultraActionOnlineRun;
	// g_object_class_install_property(object_class, PROP_STATE,
	// 								g_param_spec_uint("state", "Online state", "Online state", LAR_MEAS_ONLINE_STATE, LAR_MEAS_CHECK_STATE, LAR_MEAS_SINGLE_STATE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	// g_object_class_install_property(object_class, PROP_STREAM,
	// 								g_param_spec_object("stream", "Online stream", "Online stream", ULTRA_TYPE_BUS_STREAM, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	// g_object_class_install_property(object_class, PROP_PUMP,
	// 								g_param_spec_string("pump", "Online pump", "Online pump", "", G_PARAM_WRITABLE | G_PARAM_READABLE));
	// g_object_class_install_property(object_class, PROP_VESSEL,
	// 								g_param_spec_string("vessel", "Online vessel", "Online vessel", "", G_PARAM_WRITABLE | G_PARAM_READABLE));
}
static void online_cancelled(GCancellable *cancel, GCancellable *internal)
{
	g_cancellable_cancel(internal);
}
static void onlineJobFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
static void intervalFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);

static void runNextOnlineJob(GTask *task)
{
	UltraActionOnline *online = ULTRA_ACTION_ONLINE(g_task_get_source_object(task));
	if (m_UltraActionQueueRun(online->priv->immediately, online->priv->internal, onlineJobFinishCallback, task) != ACTION_QUEUE_ERROR)
		return;
	m_UltraActionQueueRun(online->priv->measurements, online->priv->internal, onlineJobFinishCallback, task);
}

void onlineJobFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(task))
	{
		g_object_unref(task);
		return;
	}
	if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res))))
	{
		return;
	}
	UltraActionOnline *online = ULTRA_ACTION_ONLINE(g_task_get_source_object(task));
	UltraAction *action = m_UltraActionQueueFinish(ULTRA_ACTION_QUEUE(sourceObject), res);
	g_debug("onlineJobFinish Measurement (%s)", m_UltraActionGetName(action));
	if (action && ACTION_IS_ONLINE(action))
	{
		UltraAction *interval = action_online_next_run(ACTION_ONLINE(action));
		m_UltraActionSetName(ULTRA_ACTION(interval), "interval");
		m_UltraActionRun(interval, online->priv->internal, intervalFinishCallback, task);
	}
	runNextOnlineJob(task);
}

void intervalFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(task))
	{
		g_object_unref(task);
		return;
	}
	if (ULTRA_IS_ACTION(sourceObject))
	{
		UltraAction *action = ULTRA_ACTION(sourceObject);
		if (m_UltraActionFinish(action, res))
		{
			UltraActionOnline *online = ULTRA_ACTION_ONLINE(g_task_get_source_object(task));
			if (ULTRA_IS_ACTION_INTERVAL(action))
			{
				// g_debug("CheckMeasurement Measurement (%s)", m_UltraActionGetName(action));
				UltraAction *next_run = m_UltraActionIntervalGetNextRun(ULTRA_ACTION_INTERVAL(action));
				// if(ULTRA_IS_ACTION_MEAS_CHECK(next_run))g_debug("CheckMeasurement check-measurement");
				if(ACTION_IS_MEASUREMENT(next_run))
				{
					m_UltraActionQueueAddAction(online->priv->measurements, next_run);
				}
			}
		}
	}
	runNextOnlineJob(task);
}

void destroy_local_cancellable(gpointer data)
{
}
static void onlinePocessResetNodeDetectedCallback(SecurityDevice *device, GParamSpec *pspec, GTask *task)
{
	UltraActionOnline *online = ULTRA_ACTION_ONLINE(g_task_get_source_object(task));
	if (online->priv->last_reset)
		g_date_time_unref(online->priv->last_reset);
	online->priv->last_reset = rt_now_local();
	online->priv->reset_counter++;
	mkt_errors_come(E1890);
	UltraAction *hold = UltraActionHoldNew();
	m_UltraActionQueuePushAction(online->priv->immediately, hold);
	g_cancellable_cancel(online->priv->internal);
	g_object_unref(online->priv->internal);
	online->priv->internal = g_cancellable_new();
	if (g_task_get_cancellable(task))
		g_cancellable_connect(g_task_get_cancellable(task), G_CALLBACK(online_cancelled), g_object_ref(online->priv->internal), g_object_unref);
	runNextOnlineJob(task);
}

gboolean ultraActionOnlineRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_ONLINE(action), FALSE);
	UltraActionOnline *online = ULTRA_ACTION_ONLINE(action);
	GTask *task = g_task_new(online, cancellable, callback, userData);
	online->priv->internal = g_cancellable_new();
	if (cancellable)
		g_cancellable_connect(cancellable, G_CALLBACK(online_cancelled), g_object_ref(online->priv->internal), g_object_unref);
	// g_debug("Next meeasurement. test .. ");
	GList *stream_list = g_dbus_object_manager_get_objects(G_DBUS_OBJECT_MANAGER(UltraDBusStreamsManager()));
	GList *sl = NULL;
	for (sl = stream_list; sl != NULL; sl = sl->next)
	{
		if (m_UltraBusStreamCheckLicense(ULTRA_BUS_STREAM(sl->data)))
		{
			// g_debug("test 1");
			// UltraAction *interval = NULL;
			if (m_UltraBusStreamGetRemoteControl(ULTRA_BUS_STREAM(sl->data)))
			{
			}
			else if (m_UltraBusStreamGetMeasurementInterval(ULTRA_BUS_STREAM(sl->data)) > 0)
			{
				// g_debug("test 2");
				// online->priv->streams = g_list_append(online->priv->streams, g_object_ref(sl->data));
				// UltraAction *measurement = UltraActionMeasurementOnlineNew(ULTRA_BUS_STREAM(sl->data));
				// gchar *pname = g_strdup_printf("online-measurement-%s", m_UltraBusStreamGetTitle(ULTRA_BUS_STREAM(sl->data)));
				// m_UltraActionSetName(measurement, pname);
				// g_free(pname);
				// m_UltraActionQueueAddAction(online->priv->measurements, ULTRA_ACTION(measurement));
				// g_debug("test 3");
				// g_object_set(measurement, "name", pname, NULL);
				// m_UltraActionRun(interval, cancellable, intervalFinishCallback, task);
				// g_object_unref(interval);
			}
			g_debug("CheckInterval:%s-%li",m_UltraBusStreamGetTitle(ULTRA_BUS_STREAM(sl->data)),m_UltraBusStreamGetCheckInterval(ULTRA_BUS_STREAM(sl->data)));
			if (m_UltraBusStreamGetCheckInterval(ULTRA_BUS_STREAM(sl->data)) > 0)
			{
				UltraAction *measurement = UltraActionMeasurementCheckNew(ULTRA_BUS_STREAM(sl->data));
				gchar *pname = g_strdup_printf("check-measurement-%s", m_UltraBusStreamGetTitle(ULTRA_BUS_STREAM(sl->data)));
				m_UltraActionSetName(measurement, pname);
				g_free(pname);
				UltraAction *interval = action_online_next_run(ACTION_ONLINE(measurement));
				m_UltraActionSetName(ULTRA_ACTION(interval), "interval-check-measurement");
				m_UltraActionRun(interval, online->priv->internal, intervalFinishCallback, task);
				g_object_unref(measurement);
			}

			// interval = UltraActionIntervalNew(ULTRA_BUS_STREAM(sl->data),state | LAR_STATE_CAL);
			// m_UltraActionRun(interval, cancellable, intervalFinishCallback, task);
			// g_object_unref(interval);
			// interval = UltraActionIntervalNew(ULTRA_BUS_STREAM(sl->data),state | LAR_STATE_CHECK);
			// m_UltraActionRun(interval, cancellable, intervalFinishCallback, task);
			// g_object_unref(interval);
		}
	}
	runNextOnlineJob(task);
	// m_UltraActionQueueRun(online->priv->measurements,cancellable,);
	if (!ConfigureIsTestMode())
	{
		GDBusObjectManager *can_device_manager = mkt_can_manager_client_devices();
		if (can_device_manager)
		{
			CandeviceObject *object = CANDEVICE_OBJECT(g_dbus_object_manager_get_object(can_device_manager, CAN_DEVICE_CAN0));
			if (object)
			{
				g_signal_connect(candevice_object_get_simple(object), "node-reseted", G_CALLBACK(onlinePocessResetNodeDetectedCallback), online);
			}
		}
	}
	return TRUE;
}

UltraAction *ultra_action_online_new()
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_ONLINE, NULL));
	return action;
}
