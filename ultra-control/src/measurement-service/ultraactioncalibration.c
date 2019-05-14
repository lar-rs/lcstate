/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactioncalibration.h"
#include "ultraactionanalyze.h"
#include "ultraactionfillsample.h"
#include "ultraactionstripping.h"
#include "ultraactionsampling.h"
#include "ultraactionrinsing.h"
#include "ultraactionhold.h"
#include "ultraactiontimer.h"
#include "ultraactiongroup.h"
#include "ultraactioninterval.h"
#include "ultraerrors.h"
#include "ultraconfig.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionCalibrationPrivate
{
	UltraBusStream *stream;
	gchar *pump;
	gchar *vessel;
	gdouble fillTime;
	gboolean noFill;
	gboolean noPreRinsing;
	GDateTime *startCalibration;

	guint replicates;

	GList *channels;
	GList *tcChannels;
	GList *ticChannels;
	Statistic *statistic;
	guint calibration;
	UltraAction *calGroup;
	UltraAction *interval;
	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_STATE,
	PROP_STREAM,
	PROP_PUMP,
	PROP_VESSEL,
};

#define ULTRA_ACTION_CALIBRATION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_CALIBRATION, UltraActionCalibrationPrivate))

// if(calibration->priv->state & (LAR_STATE_CAL|LAR_STATE_ONLINE)) {
// 		calibration->priv->channels = m_UltraBusStreamChannels(calibration->priv->stream);
// 		calibration->priv->statistic = m_UltraBusStreamStatistic(calibration->priv->stream);
// 	}else if(calibration->priv->state & (LAR_STATE_CAL|LAR_STATE_OFFLINE)){
// 		calibration->priv->channels = m_UltraBusStreamChannels(calibration->priv->stream);
// 		calibration->priv->statistic = m_UltraBusStreamStatisticSingle(calibration->priv->stream);
// 	}else if(calibration->priv->state & (LAR_STATE_CHECK|LAR_STATE_ONLINE)){
// 		calibration->priv->channels = m_UltraBusStreamChannelsCheck(calibration->priv->stream);
// 		calibration->priv->statistic = m_UltraBusStreamStatistic(calibration->priv->stream);
// 	}

//check..
static gboolean
action_calibration_check_iface_next(ActionCalibration *iface, GError **error)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);
	if (calibration->priv->stream == NULL)
	{
		*error = g_error_new(UltraActionErrorQuark(), UERROR_EMPTY_STREAM, "Calibration start error - stream is null");
		return FALSE;
	}
	if (!m_UltraBusStreamCheckLicense(calibration->priv->stream))
	{
		*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_STREAM_LIC, "Calibration start error - stream %" G_GUINT64_FORMAT " license not found", m_UltraBusStreamGetNumber(calibration->priv->stream));
		return FALSE;
	}
	StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(calibration->priv->stream));
	m_UltraBusStreamNextCalibration(calibration->priv->stream);
	calibration->priv->channels = m_UltraBusStreamChannelsCheck(calibration->priv->stream);
	calibration->priv->statistic = m_UltraBusStreamStatistic(calibration->priv->stream);
	if (calibration->priv->channels == NULL)
	{
		if (error != NULL)
		{
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								 "Calibration start error - stream %s doesn't have activated channels",
								 m_UltraBusStreamGetTitle(calibration->priv->stream));
		}
		streams_simple_set_status(simple, "No activated channels");
		g_object_unref(simple);
		return FALSE;
	}
	GList *lch = calibration->priv->channels;
	for (; lch != NULL; lch = lch->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(lch->data)))
		{
			calibration->priv->ticChannels = g_list_append(calibration->priv->ticChannels, g_object_ref(lch->data));
		}
		else if (!m_UltraBusChannelIsOnlyCalculated(lch->data))
		{
			calibration->priv->tcChannels = g_list_append(calibration->priv->tcChannels, g_object_ref(lch->data));
		}
		m_UltraBusChannelNextCalibration(ULTRA_BUS_CHANNEL(lch->data), calibration->priv->statistic);
	}
	return TRUE;
}

static UltraQueries *
action_calibration_check_iface_query(ActionCalibration *iface)
{
	UltraQueries *query = UltraQueriesNew(NULL);
	// UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);
	ultra_queries_check(query);
	// ultra_queries_stream(query, m_UltraBusStreamGetNumber(calibration->priv->stream));
	return query;
}

static void
action_calibration_check_interface_init(ActionCalibrationInterface *iface)
{
	iface->next = action_calibration_check_iface_next;
	iface->query = action_calibration_check_iface_query;
}

static UltraAction *
action_calibrationent_check_iface_next_run(ActionOnline *iface)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);
	UltraAction *next_run = UltraActionCalibrationNewCopy(calibration);
	gint64 sec = m_UltraBusStreamGetCheckInterval(ULTRA_ACTION_CALIBRATION(next_run)->priv->stream);
	UltraAction *interval = UltraActionIntervalNew(next_run, sec);
	m_UltraActionSetName(ULTRA_ACTION(interval), "interval-check");
	g_object_unref(next_run);
	return interval;
}

static void
action_check_calibration_interface_init(ActionOnlineInterface *iface)
{
	iface->next_run = action_calibrationent_check_iface_next_run;
}

G_DEFINE_TYPE_WITH_CODE(UltraActionCalCheck, ultra_action_cal_check, ULTRA_TYPE_ACTION_CALIBRATION,
						G_IMPLEMENT_INTERFACE(ACTION_TYPE_CALIBRATION,
											  action_calibration_check_interface_init)
							G_IMPLEMENT_INTERFACE(ACTION_TYPE_ONLINE,
												  action_check_calibration_interface_init))
static void
ultra_action_cal_check_init(UltraActionCalCheck *self)
{
	/* Instance variable initialisation code. */

}

static guint ultraActionCalibrationCheckGetId(UltraAction *action)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(action);
	guint id = g_str_hash(G_OBJECT_TYPE_NAME(calibration)) + m_UltraBusStreamGetNumber(calibration->priv->stream) + 200;
	return id;
}

static void
ultra_action_cal_check_class_init(UltraActionCalCheckClass *class)
{
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(class);
	aclass->get_id = ultraActionCalibrationCheckGetId;
}

//Online calibration interface ..
static gboolean
action_calibration_online_iface_next(ActionCalibration *iface, GError **error)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);
	if (calibration->priv->stream == NULL)
	{
		if(error!=NULL) *error = g_error_new(UltraActionErrorQuark(), UERROR_EMPTY_STREAM, "Calibration start error - stream is null");
		return FALSE;
	}
	if (!m_UltraBusStreamCheckLicense(calibration->priv->stream))
	{
		if (error != NULL)
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_STREAM_LIC, "Calibration start error - stream %" G_GUINT64_FORMAT " license not found", m_UltraBusStreamGetNumber(calibration->priv->stream));
		return FALSE;
	}
	StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(calibration->priv->stream));
	m_UltraBusStreamNextCalibration(calibration->priv->stream);
	calibration->priv->channels = m_UltraBusStreamChannels(calibration->priv->stream);
	calibration->priv->statistic = m_UltraBusStreamStatistic(calibration->priv->stream);
	if (calibration->priv->channels == NULL)
	{
		if (error != NULL)
		{
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								 "Calibration start error - stream %s doesn't have activated channels",
								 m_UltraBusStreamGetTitle(calibration->priv->stream));
		}
		streams_simple_set_status(simple, "No activated channels");
		g_object_unref(simple);
		return FALSE;
	}
	GList *lch = calibration->priv->channels;
	for (; lch != NULL; lch = lch->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(lch->data)))
		{
			calibration->priv->ticChannels = g_list_append(calibration->priv->ticChannels, g_object_ref(lch->data));
		}
		else if (!m_UltraBusChannelIsOnlyCalculated(lch->data))
		{
			calibration->priv->tcChannels = g_list_append(calibration->priv->tcChannels, g_object_ref(lch->data));
		}
		m_UltraBusChannelNextCalibration(ULTRA_BUS_CHANNEL(lch->data), calibration->priv->statistic);
	}
	return TRUE;
}

static UltraQueries *
action_calibration_online_iface_query(ActionCalibration *iface)
{
	UltraQueries *query = UltraQueriesNew(NULL);
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);

	ultra_queries_online(query);
	return query;
}

static void
action_calibration_online_interface_init(ActionCalibrationInterface *iface)
{
	iface->next = action_calibration_online_iface_next;
	iface->query = action_calibration_online_iface_query;
}

static UltraAction *
action_calibrationent_online_iface_next_run(ActionOnline *iface)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);
	UltraAction *next_run = UltraActionCalibrationNewCopy(calibration);
	gint64 sec = m_UltraBusStreamGetCalibrationInterval(ULTRA_ACTION_CALIBRATION(next_run)->priv->stream);
	UltraAction *interval = UltraActionIntervalNew(next_run, sec);
	g_object_unref(next_run);
	return interval;
}

static void
action_online_calibration_interface_init(ActionOnlineInterface *iface)
{
	iface->next_run = action_calibrationent_online_iface_next_run;
}

G_DEFINE_TYPE_WITH_CODE(UltraActionCalOnline, ultra_action_cal_online, ULTRA_TYPE_ACTION_CALIBRATION,
						G_IMPLEMENT_INTERFACE(ACTION_TYPE_CALIBRATION,
											  action_calibration_online_interface_init)
							G_IMPLEMENT_INTERFACE(ACTION_TYPE_ONLINE,
												  action_online_calibration_interface_init))
static void
ultra_action_cal_online_init(UltraActionCalOnline *self)
{
	/* Instance variable initialisation code. */
}

static guint ultraActionCalibrationOnlineGetId(UltraAction *action)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(action);
	guint id = g_str_hash(G_OBJECT_TYPE_NAME(calibration)) + m_UltraBusStreamGetNumber(calibration->priv->stream) + 100;
	return id;
}

static void
ultra_action_cal_online_class_init(UltraActionCalOnlineClass *class)
{
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(class);
	aclass->get_id = ultraActionCalibrationOnlineGetId;
}

static UltraBusStream *
action_calibration_iface_stream(ActionCalibration *iface)
{
	UltraActionCalibration *cal = ULTRA_ACTION_CALIBRATION(iface);
	return cal->priv->stream;
}

static gboolean
action_calibration_iface_next(ActionCalibration *iface, GError **error)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(iface);
	if (calibration->priv->stream == NULL)
	{
		if (error != NULL)
			*error = g_error_new(UltraActionErrorQuark(), UERROR_EMPTY_STREAM, "Calibration start error - stream is null");
		return FALSE;
	}
	if (!m_UltraBusStreamCheckLicense(calibration->priv->stream))
	{
		if (error != NULL)
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_STREAM_LIC, "Calibration start error - stream %" G_GUINT64_FORMAT " license not found", m_UltraBusStreamGetNumber(calibration->priv->stream));
		return FALSE;
	}
	StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(calibration->priv->stream));
	m_UltraBusStreamNextCalibration(calibration->priv->stream);
	calibration->priv->channels = m_UltraBusStreamChannels(calibration->priv->stream);
	calibration->priv->statistic = m_UltraBusStreamStatisticSingle(calibration->priv->stream);
	if (calibration->priv->channels == NULL)
	{
		if (error != NULL)
		{
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								 "start error - stream %s doesn't have activated channels",
								 m_UltraBusStreamGetTitle(calibration->priv->stream));
		}
		streams_simple_set_status(simple, "No activated channels");
		g_object_unref(simple);
		return FALSE;
	}
	g_object_unref(simple);
	GList *lch = calibration->priv->channels;
	for (; lch != NULL; lch = lch->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(lch->data)))
		{
			calibration->priv->ticChannels = g_list_append(calibration->priv->ticChannels, g_object_ref(lch->data));
		}
		else if (!m_UltraBusChannelIsOnlyCalculated(lch->data))
		{
			calibration->priv->tcChannels = g_list_append(calibration->priv->tcChannels, g_object_ref(lch->data));
		}
		m_UltraBusChannelNextCalibration(ULTRA_BUS_CHANNEL(lch->data), calibration->priv->statistic);
	}
	return TRUE;
}

static UltraQueries *
action_calibration_iface_query(ActionCalibration *iface)
{
	UltraQueries *query = UltraQueriesNew(NULL);
	ultra_queries_offline(query);
	// g_debug("CheckCalibration QueryOffline");
	return query;
}

static void
action_calibration_interface_init(ActionCalibrationInterface *iface)
{
	iface->stream = action_calibration_iface_stream;
	iface->next = action_calibration_iface_next;
	iface->query = action_calibration_iface_query;
}

G_DEFINE_TYPE_WITH_CODE(UltraActionCalibration, ultra_action_calibration, ULTRA_TYPE_ACTION,
						G_IMPLEMENT_INTERFACE(ACTION_TYPE_CALIBRATION,
											  action_calibration_interface_init))

static void
ultra_action_calibration_init(UltraActionCalibration *calibration)
{
	calibration->priv = ULTRA_ACTION_CALIBRATION_PRIVATE(calibration);
	calibration->priv->pump = NULL;
	calibration->priv->stream = NULL;
	calibration->priv->channels = NULL;
	calibration->priv->statistic = NULL;
	calibration->priv->calibration = 0;
	calibration->priv->ticChannels = NULL;
	calibration->priv->tcChannels = NULL;
	calibration->priv->noFill = FALSE;
	calibration->priv->calGroup = NULL;
	calibration->priv->interval = NULL;
	calibration->priv->replicates = 0;
	calibration->priv->startCalibration = NULL;
}

static void
ultra_action_calibration_finalize(GObject *object)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(object);
	if (calibration->priv->pump)
		g_free(calibration->priv->pump);
	if (calibration->priv->vessel)
		g_free(calibration->priv->vessel);
	if (calibration->priv->stream)
		g_object_unref(calibration->priv->stream);
	if (calibration->priv->tcChannels)
		g_list_free_full(calibration->priv->tcChannels, g_object_unref);
	if (calibration->priv->ticChannels)
		g_list_free_full(calibration->priv->ticChannels, g_object_unref);
	if (calibration->priv->channels)
		g_list_free_full(calibration->priv->channels, g_object_unref);
	if (calibration->priv->calGroup)
		g_object_unref(calibration->priv->calGroup);
	if (calibration->priv->interval)
		g_object_unref(calibration->priv->interval);
	if (calibration->priv->statistic)
		g_object_unref(calibration->priv->statistic);
	if (calibration->priv->startCalibration)
		g_date_time_unref(calibration->priv->startCalibration);
	G_OBJECT_CLASS(ultra_action_calibration_parent_class)->finalize(object);
}

static void
ultra_action_calibration_set_property(GObject *object,
									  guint prop_id,
									  const GValue *value,
									  GParamSpec *pspec)
{
	UltraActionCalibration *action = ULTRA_ACTION_CALIBRATION(object);
	switch (prop_id)
	{
	case PROP_STREAM:
		if (action->priv->stream)
			g_object_unref(action->priv->stream);
		action->priv->stream = g_value_dup_object(value);
		break;
	case PROP_PUMP:
		if (action->priv->pump)
			g_free(action->priv->pump);
		action->priv->pump = g_value_dup_string(value);
		break;
	case PROP_VESSEL:
		if (action->priv->vessel)
			g_free(action->priv->vessel);
		action->priv->vessel = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_calibration_get_property(GObject *object,
									  guint prop_id,
									  GValue *value,
									  GParamSpec *pspec)
{
	UltraActionCalibration *action = ULTRA_ACTION_CALIBRATION(object);
	switch (prop_id)
	{
	case PROP_STREAM:
		g_value_set_object(value, action->priv->stream);
		break;
	case PROP_PUMP:
		g_value_set_string(value, action->priv->pump);
		break;
	case PROP_VESSEL:
		g_value_set_string(value, action->priv->vessel);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}
static gboolean ultraActionCalibrationRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static guint ultraActionCalibrationGetId(UltraAction *action)
{
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(action);
	guint id = g_str_hash(G_OBJECT_TYPE_NAME(calibration)) + m_UltraBusStreamGetNumber(calibration->priv->stream);
	return id;
}

static void
ultra_action_calibration_class_init(UltraActionCalibrationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionCalibrationClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	//object_class->dispose              = ultimate_atom_dispose;
	object_class->finalize = ultra_action_calibration_finalize;
	object_class->set_property = ultra_action_calibration_set_property;
	object_class->get_property = ultra_action_calibration_get_property;
	aclass->run = ultraActionCalibrationRun;
	aclass->get_id = ultraActionCalibrationGetId;
	g_object_class_install_property(object_class, PROP_STREAM,
									g_param_spec_object("stream", "Calibration stream", "Calibration stream", ULTRA_TYPE_BUS_STREAM, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_PUMP,
									g_param_spec_string("pump", "Calibration pump", "Calibration pump", "", G_PARAM_WRITABLE | G_PARAM_READABLE));
	g_object_class_install_property(object_class, PROP_VESSEL,
									g_param_spec_string("vessel", "Calibration vessel", "Calibration vessel", "", G_PARAM_WRITABLE | G_PARAM_READABLE));
}

UltraAction *ultraCalibrationNextReplicate(UltraActionCalibration *calibration, gboolean noStabilization)
{
	UltraAction *cal = UltraActionGroupSyncNew();
	m_UltraActionSetName(cal, "M:group-replicate");
	if (calibration->priv->tcChannels)
	{
		if (!noStabilization)
		{
			gint64 delay = (gint64)m_UltraBusStreamGetDelay(calibration->priv->stream);
			UltraAction *stabilization = UltraActionTimerNew(delay * G_TIME_SPAN_SECOND);
			m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(cal), stabilization);
		}
		else
		{
			noStabilization = TRUE;
		}
		// Dilution [IsDilution]
		UltraAction *sampling = UltraActionSamplingNew(calibration->priv->vessel, m_UltraBusStreamGetInjectionVolumeTC(calibration->priv->stream),
													   m_UltraBusStreamIsCODoInjection(calibration->priv->stream));
		m_UltraActionSetName(sampling, "M:sampling");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(cal), sampling);
		UltraAction *analyze = UltraActionAnalyzeNew(calibration->priv->stream, calibration->priv->tcChannels);
		m_UltraActionSetName(analyze, "M:analyze");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(cal), analyze);
	}
	if (calibration->priv->ticChannels)
	{
		if (!noStabilization)
		{
			gint64 delay = (gint64)m_UltraBusStreamGetDelayTIC(calibration->priv->stream);
			UltraAction *stabilization = UltraActionTimerNew(delay * G_TIME_SPAN_SECOND);
			m_UltraActionSetName(stabilization, "M:stabilization");
			m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(cal), stabilization);
		}
		// Dilution [IsDilution]
		UltraAction *sampling = UltraActionSamplingNew(calibration->priv->vessel, m_UltraBusStreamGetInjectionVolumeTIC(calibration->priv->stream), FALSE);
		m_UltraActionSetName(sampling, "M:sampling");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(cal), sampling);
		UltraAction *analyze = UltraActionAnalyzeNew(calibration->priv->stream, calibration->priv->ticChannels);
		m_UltraActionSetName(analyze, "M:analyze");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(cal), analyze);
	}
	return cal;
}

static void calibrationFinishReplicateCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(task))
	{
		g_object_unref(task);
		return;
	}
	if(ULTRA_IS_ACTION(sourceObject))
	{
		UltraAction *action = ULTRA_ACTION(sourceObject);
		if (!m_UltraActionFinish(action,res))
		{
			GError *error = m_UltraActionError(action);
			g_task_return_error(task, g_error_new(UltraErrorsQuark(), error ? error->code : 6, "calibration fail - %s", error ? error->message : "unknown"));
			g_object_unref(task);
			return;
		}
	}
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(g_task_get_source_object(task));
	gboolean statisticDone = calibration->priv->replicates >= StatisticGetReplicates(calibration->priv->statistic);
	gboolean enoughReplicates = (calibration->priv->replicates >= StatisticGetReplicates(calibration->priv->statistic) + StatisticGetOutliers(calibration->priv->statistic));
	GList *l = calibration->priv->channels;
	for (; l != NULL; l = l->next)
	{
		if (!m_UlraBusChannelCalibrationPublished(ULTRA_BUS_CHANNEL(l->data)))
		{
			statisticDone = FALSE;
		}
	}
	if (!statisticDone && !enoughReplicates)
	{
		calibration->priv->replicates++;
		if (calibration->priv->calGroup != NULL)
			g_object_unref(calibration->priv->calGroup);
		calibration->priv->calGroup = UltraActionGroupSyncNew();
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(calibration->priv->calGroup), ultraCalibrationNextReplicate(calibration, FALSE));
		m_UltraActionRun(calibration->priv->calGroup, g_task_get_cancellable(task), calibrationFinishReplicateCallback, task);
		return;
	}
	UltraQueries *query = action_calibration_query(ACTION_CALIBRATION(calibration));
	GList *lr = calibration->priv->channels;
	for (; lr != NULL; lr = lr->next)
	{
		m_UltraBusChannelResultPublished(ULTRA_BUS_CHANNEL(lr->data), query);
	}
	g_object_unref(query);
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
	return;
}

gboolean ultraActionCalibrationRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_CALIBRATION(action), FALSE);
	UltraActionCalibration *calibration = ULTRA_ACTION_CALIBRATION(action);
	GTask *task = g_task_new(calibration, cancellable, callback, userData);
	// g_debug("Next meeasurement. test .. ");
	GDateTime *now = rt_now_utc();
	gchar *nowStr = g_date_time_format(now, "%FT%T");
	g_free(nowStr);
	g_date_time_unref(now);
	//TEST : run calibration
	// g_task_return_boolean(task, TRUE);
	// g_object_unref(task);
	// return TRUE;
	GError *error = NULL;
	if (!action_calibration_next(ACTION_CALIBRATION(calibration), &error))
	{
		if (error == NULL)
		{
			error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								"Calibration start error -%s next run fail",
								m_UltraBusStreamGetTitle(calibration->priv->stream));
		}
		g_task_return_error(task, error);
		return FALSE;
	}
	// g_task_run_in_thread()
	calibration->priv->calGroup = UltraActionGroupSyncNew();
	m_UltraActionSetName(calibration->priv->calGroup, "M:calibration-group");
	if (!calibration->priv->noFill)
	{
		gint64 fillTime = (gint64)m_UltraBusStreamGetSampleFillInterval(calibration->priv->stream);
		UltraAction *fill = UltraActionFillSampleNew(calibration->priv->pump, fillTime * G_TIME_SPAN_SECOND);
		m_UltraActionSetName(fill, "M:fill-sample");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(calibration->priv->calGroup), fill);
	}
	UltraAction *strippingAndRinsing = UltraActionGroupNew();
	m_UltraActionSetName(strippingAndRinsing, "M:rinsing-group");
	if (!calibration->priv->noFill && m_UltraBusStreamIsNeedStripping(calibration->priv->stream))
	{
		gint64 strippingTime = (gint64)m_UltraBusStreamGetStrippingTime(calibration->priv->stream);
		UltraAction *stripping = UltraActionStrippingNew(strippingTime * G_TIME_SPAN_SECOND);
		m_UltraActionSetName(stripping, "M:stripping");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(strippingAndRinsing), stripping);
	}
	if (m_UltraBusStreamIsPreRinsing(calibration->priv->stream))
	{
		guint count = m_UltraBusStreamGetPreRinsingCount(calibration->priv->stream);
		UltraAction *rinsing = UltraActionRinsingNew(m_UltraBusStreamGetDrainVessel(calibration->priv->stream), 400, count);
		m_UltraActionSetName(rinsing, "M:rinsing");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(strippingAndRinsing), rinsing);
	}
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(calibration->priv->calGroup), strippingAndRinsing);
	// // Dilution [IsDilution]
	calibration->priv->replicates = 1;
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(calibration->priv->calGroup), ultraCalibrationNextReplicate(calibration, FALSE));
	m_UltraActionRun(calibration->priv->calGroup, cancellable, calibrationFinishReplicateCallback, task);
	return TRUE;
}

UltraAction *UltraActionCalibrationNew(UltraBusStream *stream)
{
	const gchar *pump = m_UltraBusStreamGetPump(stream);
	const gchar *vessel = m_UltraBusStreamGetOnlineVessel(stream);
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_CALIBRATION, "stream", stream, "pump", pump, "vessel", vessel, NULL));
	return action;
}
UltraAction *UltraActionCalibrationOnlineNew(UltraBusStream *stream)
{
	const gchar *pump = m_UltraBusStreamGetPump(stream);
	const gchar *vessel = m_UltraBusStreamGetOnlineVessel(stream);
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_CAL_ONLINE, "stream", stream, "pump", pump, "vessel", vessel, NULL));
	return action;
}
UltraAction *UltraActionCalibrationCheckNew(UltraBusStream *stream)
{
	const gchar *pump = m_UltraBusStreamGetPump(stream);
	const gchar *vessel = m_UltraBusStreamGetOnlineVessel(stream);
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_CAL_CHECK, "stream", stream, "pump", pump, "vessel", vessel, NULL));
	return action;
}

UltraAction *UltraActionCalibrationNewCopy(UltraActionCalibration *calibration)
{

	UltraAction *action = ULTRA_ACTION(g_object_new(G_OBJECT_TYPE(calibration),
													"name", m_UltraActionGetName(ULTRA_ACTION(calibration)),
													"stream", calibration->priv->stream, "pump", calibration->priv->pump, "vessel", calibration->priv->vessel, NULL));
	return action;
}

void m_UltraActionCalibrationSetPump(UltraActionCalibration *calibration, const gchar *pump)
{
	g_return_if_fail(calibration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration));
	g_object_set(calibration, "pump", pump, NULL);
}

void m_UltraActionCalibrationNoFill(UltraActionCalibration *calibration)
{
	g_return_if_fail(calibration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration));
	calibration->priv->noFill = TRUE;
}

void m_UltraActionCalibrationNoPreRinsing(UltraActionCalibration *calibration)
{
	g_return_if_fail(calibration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration));
	calibration->priv->noPreRinsing = TRUE;
}

void m_UltraActionCalibrationSetVessel(UltraActionCalibration *calibration, const gchar *vessel)
{
	g_return_if_fail(calibration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration));
	g_object_set(calibration, "vessel", vessel, NULL);
}
UltraBusStream *m_UltraActionCalibrationGetStream(UltraActionCalibration *calibration)
{
	g_return_val_if_fail(calibration != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration), NULL);
	return calibration->priv->stream;
}
void m_UltraActionCalibrationAddInterval(UltraActionCalibration *calibration, UltraActionInterval *interval)
{
	g_return_if_fail(calibration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration));
	if (calibration->priv->interval)
		g_object_unref(calibration->priv->interval);
	if (interval)
		calibration->priv->interval = g_object_ref(interval);
}

UltraAction *m_UltraActionCalibrationGetInterval(UltraActionCalibration *calibration)
{
	g_return_val_if_fail(calibration != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_CALIBRATION(calibration), NULL);
	return calibration->priv->interval;
}