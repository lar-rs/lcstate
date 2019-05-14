/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionmeasurement.h"
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

struct _UltraActionMeasurementPrivate
{
	UltraBusStream *stream;
	gchar *pump;
	gchar *vessel;
	gdouble fillTime;
	gboolean noFill;
	gboolean noPreRinsing;
	GDateTime *startMeasurement;

	guint replicates;

	GList *channels;
	GList *tcChannels;
	GList *ticChannels;
	Statistic *statistic;
	guint measurement;
	UltraAction *measGroup;
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

#define ULTRA_ACTION_MEASUREMENT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_MEASUREMENT, UltraActionMeasurementPrivate))

// if(measurement->priv->state & (LAR_STATE_MEAS|LAR_STATE_ONLINE)) {
// 		measurement->priv->channels = m_UltraBusStreamChannels(measurement->priv->stream);
// 		measurement->priv->statistic = m_UltraBusStreamStatistic(measurement->priv->stream);
// 	}else if(measurement->priv->state & (LAR_STATE_MEAS|LAR_STATE_OFFLINE)){
// 		measurement->priv->channels = m_UltraBusStreamChannels(measurement->priv->stream);
// 		measurement->priv->statistic = m_UltraBusStreamStatisticSingle(measurement->priv->stream);
// 	}else if(measurement->priv->state & (LAR_STATE_CHECK|LAR_STATE_ONLINE)){
// 		measurement->priv->channels = m_UltraBusStreamChannelsCheck(measurement->priv->stream);
// 		measurement->priv->statistic = m_UltraBusStreamStatistic(measurement->priv->stream);
// 	}

//check..
static gboolean
action_measurement_check_iface_next(ActionMeasurement *iface, GError **error)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);
	if (measurement->priv->stream == NULL)
	{
		*error = g_error_new(UltraActionErrorQuark(), UERROR_EMPTY_STREAM, "Measurement start error - stream is null");
		return FALSE;
	}
	if (!m_UltraBusStreamCheckLicense(measurement->priv->stream))
	{
		*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_STREAM_LIC, "Measurement start error - stream %" G_GUINT64_FORMAT " license not found", m_UltraBusStreamGetNumber(measurement->priv->stream));
		return FALSE;
	}
	StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(measurement->priv->stream));
	m_UltraBusStreamNextMeasurement(measurement->priv->stream);
	measurement->priv->channels = m_UltraBusStreamChannelsCheck(measurement->priv->stream);
	measurement->priv->statistic = m_UltraBusStreamStatistic(measurement->priv->stream);
	if (measurement->priv->channels == NULL)
	{
		if (error != NULL)
		{
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								 "Measurement start error - stream %s doesn't have activated channels",
								 m_UltraBusStreamGetTitle(measurement->priv->stream));
		}
		streams_simple_set_status(simple, "No activated channels");
		g_object_unref(simple);
		return FALSE;
	}
	GList *lch = measurement->priv->channels;
	for (; lch != NULL; lch = lch->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(lch->data)))
		{
			measurement->priv->ticChannels = g_list_append(measurement->priv->ticChannels, g_object_ref(lch->data));
		}
		else if (!m_UltraBusChannelIsOnlyCalculated(lch->data))
		{
			measurement->priv->tcChannels = g_list_append(measurement->priv->tcChannels, g_object_ref(lch->data));
		}
		m_UltraBusChannelNextMeasurement(ULTRA_BUS_CHANNEL(lch->data), measurement->priv->statistic);
	}
	return TRUE;
}

static UltraQueries *
action_measurement_check_iface_query(ActionMeasurement *iface)
{
	UltraQueries *query = UltraQueriesNew(NULL);
	// UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);
	ultra_queries_check(query);
	// ultra_queries_stream(query, m_UltraBusStreamGetNumber(measurement->priv->stream));
	return query;
}

static void
action_measurement_check_interface_init(ActionMeasurementInterface *iface)
{
	iface->next = action_measurement_check_iface_next;
	iface->query = action_measurement_check_iface_query;
}

static UltraAction *
action_measurementent_check_iface_next_run(ActionOnline *iface)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);
	UltraAction *next_run = UltraActionMeasurementNewCopy(measurement);
	gint64 sec = m_UltraBusStreamGetCheckInterval(ULTRA_ACTION_MEASUREMENT(next_run)->priv->stream);
	UltraAction *interval = UltraActionIntervalNew(next_run, sec);
	m_UltraActionSetName(ULTRA_ACTION(interval), "interval-check");
	g_object_unref(next_run);
	return interval;
}

static void
action_check_measurement_interface_init(ActionOnlineInterface *iface)
{
	iface->next_run = action_measurementent_check_iface_next_run;
}

G_DEFINE_TYPE_WITH_CODE(UltraActionMeasCheck, ultra_action_meas_check, ULTRA_TYPE_ACTION_MEASUREMENT,
						G_IMPLEMENT_INTERFACE(ACTION_TYPE_MEASUREMENT,
											  action_measurement_check_interface_init)
							G_IMPLEMENT_INTERFACE(ACTION_TYPE_ONLINE,
												  action_check_measurement_interface_init))
static void
ultra_action_meas_check_init(UltraActionMeasCheck *self)
{
	/* Instance variable initialisation code. */

}

static guint ultraActionMeasurementCheckGetId(UltraAction *action)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(action);
	guint id = g_str_hash(G_OBJECT_TYPE_NAME(measurement)) + m_UltraBusStreamGetNumber(measurement->priv->stream) + 200;
	return id;
}

static void
ultra_action_meas_check_class_init(UltraActionMeasCheckClass *class)
{
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(class);
	aclass->get_id = ultraActionMeasurementCheckGetId;
}

//Online measurement interface ..
static gboolean
action_measurement_online_iface_next(ActionMeasurement *iface, GError **error)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);
	if (measurement->priv->stream == NULL)
	{
		if(error!=NULL) *error = g_error_new(UltraActionErrorQuark(), UERROR_EMPTY_STREAM, "Measurement start error - stream is null");
		return FALSE;
	}
	if (!m_UltraBusStreamCheckLicense(measurement->priv->stream))
	{
		if (error != NULL)
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_STREAM_LIC, "Measurement start error - stream %" G_GUINT64_FORMAT " license not found", m_UltraBusStreamGetNumber(measurement->priv->stream));
		return FALSE;
	}
	StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(measurement->priv->stream));
	m_UltraBusStreamNextMeasurement(measurement->priv->stream);
	measurement->priv->channels = m_UltraBusStreamChannels(measurement->priv->stream);
	measurement->priv->statistic = m_UltraBusStreamStatistic(measurement->priv->stream);
	if (measurement->priv->channels == NULL)
	{
		if (error != NULL)
		{
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								 "Measurement start error - stream %s doesn't have activated channels",
								 m_UltraBusStreamGetTitle(measurement->priv->stream));
		}
		streams_simple_set_status(simple, "No activated channels");
		g_object_unref(simple);
		return FALSE;
	}
	GList *lch = measurement->priv->channels;
	for (; lch != NULL; lch = lch->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(lch->data)))
		{
			measurement->priv->ticChannels = g_list_append(measurement->priv->ticChannels, g_object_ref(lch->data));
		}
		else if (!m_UltraBusChannelIsOnlyCalculated(lch->data))
		{
			measurement->priv->tcChannels = g_list_append(measurement->priv->tcChannels, g_object_ref(lch->data));
		}
		m_UltraBusChannelNextMeasurement(ULTRA_BUS_CHANNEL(lch->data), measurement->priv->statistic);
	}
	return TRUE;
}

static UltraQueries *
action_measurement_online_iface_query(ActionMeasurement *iface)
{
	UltraQueries *query = UltraQueriesNew(NULL);
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);

	ultra_queries_online(query);
	return query;
}

static void
action_measurement_online_interface_init(ActionMeasurementInterface *iface)
{
	iface->next = action_measurement_online_iface_next;
	iface->query = action_measurement_online_iface_query;
}

static UltraAction *
action_measurementent_online_iface_next_run(ActionOnline *iface)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);
	UltraAction *next_run = UltraActionMeasurementNewCopy(measurement);
	gint64 sec = m_UltraBusStreamGetMeasurementInterval(ULTRA_ACTION_MEASUREMENT(next_run)->priv->stream);
	UltraAction *interval = UltraActionIntervalNew(next_run, sec);
	g_object_unref(next_run);
	return interval;
}

static void
action_online_measurement_interface_init(ActionOnlineInterface *iface)
{
	iface->next_run = action_measurementent_online_iface_next_run;
}

G_DEFINE_TYPE_WITH_CODE(UltraActionMeasOnline, ultra_action_meas_online, ULTRA_TYPE_ACTION_MEASUREMENT,
						G_IMPLEMENT_INTERFACE(ACTION_TYPE_MEASUREMENT,
											  action_measurement_online_interface_init)
							G_IMPLEMENT_INTERFACE(ACTION_TYPE_ONLINE,
												  action_online_measurement_interface_init))
static void
ultra_action_meas_online_init(UltraActionMeasOnline *self)
{
	/* Instance variable initialisation code. */
}

static guint ultraActionMeasurementOnlineGetId(UltraAction *action)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(action);
	guint id = g_str_hash(G_OBJECT_TYPE_NAME(measurement)) + m_UltraBusStreamGetNumber(measurement->priv->stream) + 100;
	return id;
}

static void
ultra_action_meas_online_class_init(UltraActionMeasOnlineClass *class)
{
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(class);
	aclass->get_id = ultraActionMeasurementOnlineGetId;
}

static UltraBusStream *
action_measurement_iface_stream(ActionMeasurement *iface)
{
	UltraActionMeasurement *meas = ULTRA_ACTION_MEASUREMENT(iface);
	return meas->priv->stream;
}

static gboolean
action_measurement_iface_next(ActionMeasurement *iface, GError **error)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(iface);
	if (measurement->priv->stream == NULL)
	{
		if (error != NULL)
			*error = g_error_new(UltraActionErrorQuark(), UERROR_EMPTY_STREAM, "Measurement start error - stream is null");
		return FALSE;
	}
	if (!m_UltraBusStreamCheckLicense(measurement->priv->stream))
	{
		if (error != NULL)
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_STREAM_LIC, "Measurement start error - stream %" G_GUINT64_FORMAT " license not found", m_UltraBusStreamGetNumber(measurement->priv->stream));
		return FALSE;
	}
	StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(measurement->priv->stream));
	m_UltraBusStreamNextMeasurement(measurement->priv->stream);
	measurement->priv->channels = m_UltraBusStreamChannels(measurement->priv->stream);
	measurement->priv->statistic = m_UltraBusStreamStatisticSingle(measurement->priv->stream);
	if (measurement->priv->channels == NULL)
	{
		if (error != NULL)
		{
			*error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								 "start error - stream %s doesn't have activated channels",
								 m_UltraBusStreamGetTitle(measurement->priv->stream));
		}
		streams_simple_set_status(simple, "No activated channels");
		g_object_unref(simple);
		return FALSE;
	}
	g_object_unref(simple);
	GList *lch = measurement->priv->channels;
	for (; lch != NULL; lch = lch->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(lch->data)))
		{
			measurement->priv->ticChannels = g_list_append(measurement->priv->ticChannels, g_object_ref(lch->data));
		}
		else if (!m_UltraBusChannelIsOnlyCalculated(lch->data))
		{
			measurement->priv->tcChannels = g_list_append(measurement->priv->tcChannels, g_object_ref(lch->data));
		}
		m_UltraBusChannelNextMeasurement(ULTRA_BUS_CHANNEL(lch->data), measurement->priv->statistic);
	}
	return TRUE;
}

static UltraQueries *
action_measurement_iface_query(ActionMeasurement *iface)
{
	UltraQueries *query = UltraQueriesNew(NULL);
	ultra_queries_offline(query);
	// g_debug("CheckMeasurement QueryOffline");
	return query;
}

static void
action_measurement_interface_init(ActionMeasurementInterface *iface)
{
	iface->stream = action_measurement_iface_stream;
	iface->next = action_measurement_iface_next;
	iface->query = action_measurement_iface_query;
}

G_DEFINE_TYPE_WITH_CODE(UltraActionMeasurement, ultra_action_measurement, ULTRA_TYPE_ACTION,
						G_IMPLEMENT_INTERFACE(ACTION_TYPE_MEASUREMENT,
											  action_measurement_interface_init))

static void
ultra_action_measurement_init(UltraActionMeasurement *measurement)
{
	measurement->priv = ULTRA_ACTION_MEASUREMENT_PRIVATE(measurement);
	measurement->priv->pump = NULL;
	measurement->priv->stream = NULL;
	measurement->priv->channels = NULL;
	measurement->priv->statistic = NULL;
	measurement->priv->measurement = 0;
	measurement->priv->ticChannels = NULL;
	measurement->priv->tcChannels = NULL;
	measurement->priv->noFill = FALSE;
	measurement->priv->measGroup = NULL;
	measurement->priv->interval = NULL;
	measurement->priv->replicates = 0;
	measurement->priv->startMeasurement = NULL;
}

static void
ultra_action_measurement_finalize(GObject *object)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(object);
	if (measurement->priv->pump)
		g_free(measurement->priv->pump);
	if (measurement->priv->vessel)
		g_free(measurement->priv->vessel);
	if (measurement->priv->stream)
		g_object_unref(measurement->priv->stream);
	if (measurement->priv->tcChannels)
		g_list_free_full(measurement->priv->tcChannels, g_object_unref);
	if (measurement->priv->ticChannels)
		g_list_free_full(measurement->priv->ticChannels, g_object_unref);
	if (measurement->priv->channels)
		g_list_free_full(measurement->priv->channels, g_object_unref);
	if (measurement->priv->measGroup)
		g_object_unref(measurement->priv->measGroup);
	if (measurement->priv->interval)
		g_object_unref(measurement->priv->interval);
	if (measurement->priv->statistic)
		g_object_unref(measurement->priv->statistic);
	if (measurement->priv->startMeasurement)
		g_date_time_unref(measurement->priv->startMeasurement);
	G_OBJECT_CLASS(ultra_action_measurement_parent_class)->finalize(object);
}

static void
ultra_action_measurement_set_property(GObject *object,
									  guint prop_id,
									  const GValue *value,
									  GParamSpec *pspec)
{
	UltraActionMeasurement *action = ULTRA_ACTION_MEASUREMENT(object);
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
ultra_action_measurement_get_property(GObject *object,
									  guint prop_id,
									  GValue *value,
									  GParamSpec *pspec)
{
	UltraActionMeasurement *action = ULTRA_ACTION_MEASUREMENT(object);
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
static gboolean ultraActionMeasurementRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static guint ultraActionMeasurementGetId(UltraAction *action)
{
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(action);
	guint id = g_str_hash(G_OBJECT_TYPE_NAME(measurement)) + m_UltraBusStreamGetNumber(measurement->priv->stream);
	return id;
}

static void
ultra_action_measurement_class_init(UltraActionMeasurementClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionMeasurementClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	//object_class->dispose              = ultimate_atom_dispose;
	object_class->finalize = ultra_action_measurement_finalize;
	object_class->set_property = ultra_action_measurement_set_property;
	object_class->get_property = ultra_action_measurement_get_property;
	aclass->run = ultraActionMeasurementRun;
	aclass->get_id = ultraActionMeasurementGetId;
	g_object_class_install_property(object_class, PROP_STREAM,
									g_param_spec_object("stream", "Measurement stream", "Measurement stream", ULTRA_TYPE_BUS_STREAM, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_PUMP,
									g_param_spec_string("pump", "Measurement pump", "Measurement pump", "", G_PARAM_WRITABLE | G_PARAM_READABLE));
	g_object_class_install_property(object_class, PROP_VESSEL,
									g_param_spec_string("vessel", "Measurement vessel", "Measurement vessel", "", G_PARAM_WRITABLE | G_PARAM_READABLE));
}

UltraAction *ultraMeasurementNextReplicate(UltraActionMeasurement *measurement, gboolean noStabilization)
{
	UltraAction *meas = UltraActionGroupSyncNew();
	m_UltraActionSetName(meas, "M:group-replicate");
	if (measurement->priv->tcChannels)
	{
		if (!noStabilization)
		{
			gint64 delay = (gint64)m_UltraBusStreamGetDelay(measurement->priv->stream);
			UltraAction *stabilization = UltraActionTimerNew(delay * G_TIME_SPAN_SECOND);
			m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(meas), stabilization);
		}
		else
		{
			noStabilization = TRUE;
		}
		// Dilution [IsDilution]
		UltraAction *sampling = UltraActionSamplingNew(measurement->priv->vessel, m_UltraBusStreamGetInjectionVolumeTC(measurement->priv->stream),
													   m_UltraBusStreamIsCODoInjection(measurement->priv->stream));
		m_UltraActionSetName(sampling, "M:sampling");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(meas), sampling);
		UltraAction *analyze = UltraActionAnalyzeNew(measurement->priv->stream, measurement->priv->tcChannels);
		m_UltraActionSetName(analyze, "M:analyze");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(meas), analyze);
	}
	if (measurement->priv->ticChannels)
	{
		if (!noStabilization)
		{
			gint64 delay = (gint64)m_UltraBusStreamGetDelayTIC(measurement->priv->stream);
			UltraAction *stabilization = UltraActionTimerNew(delay * G_TIME_SPAN_SECOND);
			m_UltraActionSetName(stabilization, "M:stabilization");
			m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(meas), stabilization);
		}
		// Dilution [IsDilution]
		UltraAction *sampling = UltraActionSamplingNew(measurement->priv->vessel, m_UltraBusStreamGetInjectionVolumeTIC(measurement->priv->stream), FALSE);
		m_UltraActionSetName(sampling, "M:sampling");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(meas), sampling);
		UltraAction *analyze = UltraActionAnalyzeNew(measurement->priv->stream, measurement->priv->ticChannels);
		m_UltraActionSetName(analyze, "M:analyze");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(meas), analyze);
	}
	return meas;
}

static void measurementFinishReplicateCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
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
			g_task_return_error(task, g_error_new(UltraErrorsQuark(), error ? error->code : 6, "measurement fail - %s", error ? error->message : "unknown"));
			g_object_unref(task);
			return;
		}
	}
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(g_task_get_source_object(task));
	gboolean statisticDone = measurement->priv->replicates >= StatisticGetReplicates(measurement->priv->statistic);
	gboolean enoughReplicates = (measurement->priv->replicates >= StatisticGetReplicates(measurement->priv->statistic) + StatisticGetOutliers(measurement->priv->statistic));
	GList *l = measurement->priv->channels;
	for (; l != NULL; l = l->next)
	{
		if (!m_UlraBusChannelMeasurementPublished(ULTRA_BUS_CHANNEL(l->data)))
		{
			statisticDone = FALSE;
		}
	}
	if (!statisticDone && !enoughReplicates)
	{
		measurement->priv->replicates++;
		if (measurement->priv->measGroup != NULL)
			g_object_unref(measurement->priv->measGroup);
		measurement->priv->measGroup = UltraActionGroupSyncNew();
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(measurement->priv->measGroup), ultraMeasurementNextReplicate(measurement, FALSE));
		m_UltraActionRun(measurement->priv->measGroup, g_task_get_cancellable(task), measurementFinishReplicateCallback, task);
		return;
	}
	UltraQueries *query = action_measurement_query(ACTION_MEASUREMENT(measurement));
	GList *lr = measurement->priv->channels;
	for (; lr != NULL; lr = lr->next)
	{
		m_UltraBusChannelResultPublished(ULTRA_BUS_CHANNEL(lr->data), query);
	}
	g_object_unref(query);
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
	return;
}

gboolean ultraActionMeasurementRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_MEASUREMENT(action), FALSE);
	UltraActionMeasurement *measurement = ULTRA_ACTION_MEASUREMENT(action);
	GTask *task = g_task_new(measurement, cancellable, callback, userData);
	// g_debug("Next meeasurement. test .. ");
	GDateTime *now = rt_now_utc();
	gchar *nowStr = g_date_time_format(now, "%FT%T");
	g_free(nowStr);
	g_date_time_unref(now);
	//TEST : run measurement
	// g_task_return_boolean(task, TRUE);
	// g_object_unref(task);
	// return TRUE;
	GError *error = NULL;
	if (!action_measurement_next(ACTION_MEASUREMENT(measurement), &error))
	{
		if (error == NULL)
		{
			error = g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS,
								"Measurement start error -%s next run fail",
								m_UltraBusStreamGetTitle(measurement->priv->stream));
		}
		g_task_return_error(task, error);
		return FALSE;
	}
	// g_task_run_in_thread()
	measurement->priv->measGroup = UltraActionGroupSyncNew();
	m_UltraActionSetName(measurement->priv->measGroup, "M:measurement-group");
	if (!measurement->priv->noFill)
	{
		gint64 fillTime = (gint64)m_UltraBusStreamGetSampleFillInterval(measurement->priv->stream);
		UltraAction *fill = UltraActionFillSampleNew(measurement->priv->pump, fillTime * G_TIME_SPAN_SECOND);
		m_UltraActionSetName(fill, "M:fill-sample");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(measurement->priv->measGroup), fill);
	}
	UltraAction *strippingAndRinsing = UltraActionGroupNew();
	m_UltraActionSetName(strippingAndRinsing, "M:rinsing-group");
	if (!measurement->priv->noFill && m_UltraBusStreamIsNeedStripping(measurement->priv->stream))
	{
		gint64 strippingTime = (gint64)m_UltraBusStreamGetStrippingTime(measurement->priv->stream);
		UltraAction *stripping = UltraActionStrippingNew(strippingTime * G_TIME_SPAN_SECOND);
		m_UltraActionSetName(stripping, "M:stripping");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(strippingAndRinsing), stripping);
	}
	if (m_UltraBusStreamIsPreRinsing(measurement->priv->stream))
	{
		guint count = m_UltraBusStreamGetPreRinsingCount(measurement->priv->stream);
		UltraAction *rinsing = UltraActionRinsingNew(m_UltraBusStreamGetDrainVessel(measurement->priv->stream), 400, count);
		m_UltraActionSetName(rinsing, "M:rinsing");
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(strippingAndRinsing), rinsing);
	}
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(measurement->priv->measGroup), strippingAndRinsing);
	// // Dilution [IsDilution]
	measurement->priv->replicates = 1;
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(measurement->priv->measGroup), ultraMeasurementNextReplicate(measurement, FALSE));
	m_UltraActionRun(measurement->priv->measGroup, cancellable, measurementFinishReplicateCallback, task);
	return TRUE;
}

UltraAction *UltraActionMeasurementNew(UltraBusStream *stream)
{
	const gchar *pump = m_UltraBusStreamGetPump(stream);
	const gchar *vessel = m_UltraBusStreamGetOnlineVessel(stream);
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_MEASUREMENT, "stream", stream, "pump", pump, "vessel", vessel, NULL));
	return action;
}
UltraAction *UltraActionMeasurementOnlineNew(UltraBusStream *stream)
{
	const gchar *pump = m_UltraBusStreamGetPump(stream);
	const gchar *vessel = m_UltraBusStreamGetOnlineVessel(stream);
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_MEAS_ONLINE, "stream", stream, "pump", pump, "vessel", vessel, NULL));
	return action;
}
UltraAction *UltraActionMeasurementCheckNew(UltraBusStream *stream)
{
	const gchar *pump = m_UltraBusStreamGetPump(stream);
	const gchar *vessel = m_UltraBusStreamGetOnlineVessel(stream);
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_MEAS_CHECK, "stream", stream, "pump", pump, "vessel", vessel, NULL));
	return action;
}

UltraAction *UltraActionMeasurementNewCopy(UltraActionMeasurement *measurement)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(G_OBJECT_TYPE(measurement),
													"name", m_UltraActionGetName(ULTRA_ACTION(measurement)),
													"stream", measurement->priv->stream, "pump", measurement->priv->pump, "vessel", measurement->priv->vessel, NULL));
	return action;
}

void m_UltraActionMeasurementSetPump(UltraActionMeasurement *measurement, const gchar *pump)
{
	g_return_if_fail(measurement != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement));
	g_object_set(measurement, "pump", pump, NULL);
}

void m_UltraActionMeasurementNoFill(UltraActionMeasurement *measurement)
{
	g_return_if_fail(measurement != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement));
	measurement->priv->noFill = TRUE;
}

void m_UltraActionMeasurementNoPreRinsing(UltraActionMeasurement *measurement)
{
	g_return_if_fail(measurement != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement));
	measurement->priv->noPreRinsing = TRUE;
}

void m_UltraActionMeasurementSetVessel(UltraActionMeasurement *measurement, const gchar *vessel)
{
	g_return_if_fail(measurement != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement));
	g_object_set(measurement, "vessel", vessel, NULL);
}
UltraBusStream *m_UltraActionMeasurementGetStream(UltraActionMeasurement *measurement)
{
	g_return_val_if_fail(measurement != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement), NULL);
	return measurement->priv->stream;
}
void m_UltraActionMeasurementAddInterval(UltraActionMeasurement *measurement, UltraActionInterval *interval)
{
	g_return_if_fail(measurement != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement));
	if (measurement->priv->interval)
		g_object_unref(measurement->priv->interval);
	if (interval)
		measurement->priv->interval = g_object_ref(interval);
}

UltraAction *m_UltraActionMeasurementGetInterval(UltraActionMeasurement *measurement)
{
	g_return_val_if_fail(measurement != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_MEASUREMENT(measurement), NULL);
	return measurement->priv->interval;
}