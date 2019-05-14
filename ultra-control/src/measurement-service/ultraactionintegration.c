/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultrabussensor.h"
#include "ultraactionintegration.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include <mktlib.h>
#include "ultraconfig.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionIntegrationPrivate
{
	UltraBusChannel *channel;
	LarIntgrec *intgRec;
	LarIntegration *intgPar;
	UltraBusSensor *sensor;
	gulong handlerSig;
	gboolean tic;
	GDateTime *startIntegration;
	gboolean isMinStartTime;
	gboolean isMinStopTime;
	gboolean isMaxStopTime;
	gboolean isIntegration;
	// StreamsObject       *stream;
	GDateTime *currentDT;
	GDateTime *startMinDT;
	GDateTime *stopMinDT;
	GDateTime *stopMaxDT;
};

enum
{
	PROP0,
	PROP_CHANNEL,
	PROP_TIC,
};

#define ULTRA_ACTION_INTEGRATION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_INTEGRATION, UltraActionIntegrationPrivate))

G_DEFINE_TYPE(UltraActionIntegration, ultra_action_integration, ULTRA_TYPE_ACTION)

static void
ultra_action_integration_init(UltraActionIntegration *integration)
{
	integration->priv = ULTRA_ACTION_INTEGRATION_PRIVATE(integration);
	integration->priv->channel = NULL;
	integration->priv->intgRec = NULL;
	integration->priv->intgPar = NULL;
	integration->priv->handlerSig = 0;

	integration->priv->isMinStartTime = FALSE;
	integration->priv->isMinStopTime = FALSE;
	integration->priv->isMaxStopTime = FALSE;
	integration->priv->isIntegration = FALSE;

	integration->priv->tic = FALSE;
	integration->priv->startIntegration = NULL;
	integration->priv->currentDT = NULL;
	integration->priv->startMinDT = NULL;
	integration->priv->stopMinDT = NULL;
	integration->priv->stopMaxDT = NULL;
	integration->priv->sensor = NULL;
}

static void
ultra_action_integration_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionIntegration *integration = ULTRA_ACTION_INTEGRATION(object);
	if (integration->priv->sensor)
	{
		g_object_unref(integration->priv->sensor);
		integration->priv->sensor = NULL;
	}
	if (integration->priv->currentDT)
	{
		g_date_time_unref(integration->priv->currentDT);
	}
	if (integration->priv->startIntegration)
	{
		g_date_time_unref(integration->priv->startIntegration);
	}
	if (integration->priv->startMinDT)
	{
		g_date_time_unref(integration->priv->startMinDT);
	}
	if (integration->priv->stopMinDT)
	{
		g_date_time_unref(integration->priv->stopMinDT);
	}
	if (integration->priv->stopMaxDT)
	{
		g_date_time_unref(integration->priv->stopMaxDT);
	}

	if (integration->priv->intgRec)
		g_object_unref(integration->priv->intgRec);
	if (integration->priv->intgPar)
		g_object_unref(integration->priv->intgPar);
	if (integration->priv->channel)
	{
		g_object_unref(integration->priv->channel);
	}
	G_OBJECT_CLASS(ultra_action_integration_parent_class)->finalize(object);
}

static void
ultra_action_integration_set_property(GObject *object,
									  guint prop_id,
									  const GValue *value,
									  GParamSpec *pspec)
{
	UltraActionIntegration *integration = ULTRA_ACTION_INTEGRATION(object);
	switch (prop_id)
	{

	case PROP_TIC:
		integration->priv->tic = g_value_get_boolean(value);
		break;
	case PROP_CHANNEL:
		if (integration->priv->channel)
			g_object_unref(integration->priv->channel);
		integration->priv->channel = g_value_dup_object(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_integration_get_property(GObject *object,
									  guint prop_id,
									  GValue *value,
									  GParamSpec *pspec)
{
	UltraActionIntegration *integration = ULTRA_ACTION_INTEGRATION(object);
	switch (prop_id)
	{
	case PROP_TIC:
		g_value_set_boolean(value, integration->priv->tic);
		break;
	case PROP_CHANNEL:
		g_value_set_object(value, integration->priv->channel);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionIntegrationRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_integration_class_init(UltraActionIntegrationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionIntegrationClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_integration_finalize;
	object_class->set_property = ultra_action_integration_set_property;
	object_class->get_property = ultra_action_integration_get_property;
	aclass->run = ultraltraActionIntegrationRun;
	g_object_class_install_property(object_class, PROP_CHANNEL,
									g_param_spec_object("channel", "channel", "channel", ULTRA_TYPE_BUS_CHANNEL, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_TIC,
									g_param_spec_boolean("tic", "tic", "tic", FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionIntegrationNew(UltraBusChannel *channel, gboolean tic)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_INTEGRATION, "channel", channel, "tic", tic, NULL));
	return action;
}

UltraBusChannel *m_UltraActionIntegrationGetChannel(UltraActionIntegration *integration)
{
	g_return_val_if_fail(integration != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_INTEGRATION(integration), NULL);
	return integration->priv->channel;
}

void m_UltraActionIntegrationRunJustification(UltraActionIntegration *integration)
{
	g_return_if_fail(integration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_INTEGRATION(integration));
	// g_debug("Run justification");
	m_LarIntgrecStartLastZero(integration->priv->intgRec);
	m_UltraBusChannelUpdateAnalyze(integration->priv->channel);
}
void m_UltraActionIntegrationStopJustification(UltraActionIntegration *integration)
{
	g_return_if_fail(integration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_INTEGRATION(integration));
	m_LarIntgrecStopLastZero(integration->priv->intgRec);
	m_LarIntgrecCalculateZero(integration->priv->intgRec);
	m_UltraBusChannelUpdateAnalyze(integration->priv->channel);
	// g_debug("ZERO:%f", m_LarIntgrecGetZero(integration->priv->intgRec));
}
void m_UltraActionIntegrationRunIntegration(UltraActionIntegration *integration)
{
	// g_debug("Integratin: intgration run");
	g_return_if_fail(integration != NULL);
	g_return_if_fail(ULTRA_IS_ACTION_INTEGRATION(integration));
	integration->priv->startIntegration = rt_now_utc();
	integration->priv->startMinDT = g_date_time_add_seconds(integration->priv->startIntegration, m_LarIntegrationStartMin(integration->priv->intgPar));
	integration->priv->stopMinDT = g_date_time_add_seconds(integration->priv->startIntegration, m_LarIntegrationStopMin(integration->priv->intgPar));
	integration->priv->stopMaxDT = g_date_time_add_seconds(integration->priv->startIntegration, m_LarIntegrationStopMax(integration->priv->intgPar));
	m_LarIntgrecRunIntegration(integration->priv->intgRec);
	m_UltraBusChannelUpdateAnalyze(integration->priv->channel);
}

LarIntgrec *m_UltraActionIntegrationGetIntgrec(UltraActionIntegration *integration)
{
	g_return_val_if_fail(integration != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_INTEGRATION(integration), NULL);
	return integration->priv->intgRec;
}

static void printIngTime(UltraActionIntegration *integration)
{

	if (!integration->priv->isMinStartTime)
	{
		gchar *dtstr = g_date_time_format(integration->priv->startMinDT, "%M:%S");
		g_free(dtstr);
	}
	if (!integration->priv->isMinStopTime)
	{
		gchar *dtstr = g_date_time_format(integration->priv->stopMinDT, "%M:%S");
		g_free(dtstr);
	}
	if (!integration->priv->isMaxStopTime)
	{
		gchar *dtstr = g_date_time_format(integration->priv->stopMaxDT, "%M:%S");
		g_free(dtstr);
	}
}

void integrationDone(UltraActionIntegration *integration)
{
	// ultra_action_integration_disconnect(integration);
}

static gboolean integrationPushValue(UltraActionIntegration *integration, gdouble value)
{
	m_LarIntgrecAddValue(integration->priv->intgRec, value);
	if (integration->priv->currentDT != NULL)
	{
		GDateTime *new = rt_now_utc();
		if (g_date_time_difference(new, integration->priv->currentDT) < G_TIME_SPAN_MILLISECOND)
		{
			GDateTime *t = integration->priv->currentDT;
			integration->priv->currentDT = g_date_time_add_seconds(t, 0.3);
			g_date_time_unref(t);
		}
		else
		{
			g_date_time_unref(integration->priv->currentDT);
			integration->priv->currentDT = g_date_time_ref(new);
			g_date_time_unref(new);
		}
	}
	else
	{
		integration->priv->currentDT = rt_now_utc();
	}
	gchar *dtstr = g_date_time_format(integration->priv->currentDT, "%M:%S");
	g_free(dtstr);
	if (integration->priv->startIntegration != NULL)
	{
		if (!integration->priv->isMinStartTime)
		{
			if (g_date_time_difference(integration->priv->startMinDT, integration->priv->currentDT) <= 0)
			{
				integration->priv->isMinStartTime = TRUE;
				gchar *dtstr = g_date_time_format(integration->priv->startMinDT, "%M:%S");
				g_free(dtstr);
			}
		}
		else if (!integration->priv->isMinStopTime)
		{
			if (g_date_time_difference(integration->priv->stopMinDT, integration->priv->currentDT) <= 0)
			{
				integration->priv->isMinStopTime = TRUE;
				gchar *dtstr = g_date_time_format(integration->priv->stopMinDT, "%M:%S");
				g_free(dtstr);
			}
		}
		else if (!integration->priv->isMaxStopTime)
		{
			if (g_date_time_difference(integration->priv->stopMaxDT, integration->priv->currentDT) < 0)
			{
				integration->priv->isMaxStopTime = TRUE;
				gchar *dtstr = g_date_time_format(integration->priv->stopMaxDT, "%M:%S");
				g_free(dtstr);
			}
		}
		printIngTime(integration);
		if (integration->priv->isIntegration)
		{
			if (integration->priv->isMinStopTime)
			{
				gboolean isStopThreshold = m_LarIntegrationStopThreshold(integration->priv->intgPar) > (value - m_LarIntgrecGetZero(integration->priv->intgRec));
				if (isStopThreshold)
				{
					m_LarIntgrecStopIntegration(integration->priv->intgRec);
					m_LarIntgrecCalculateIntegration(integration->priv->intgRec);
					m_UltraBusChannelUpdateAnalyze(integration->priv->channel);
					integrationDone(integration);
					return TRUE;
				}
			}
		}
		else if (integration->priv->isMinStartTime)
		{

			integration->priv->isIntegration = m_LarIntegrationStartThreshold(integration->priv->intgPar) < (value - m_LarIntgrecGetZero(integration->priv->intgRec));
			if (integration->priv->isIntegration)
				m_LarIntgrecStartIntegration(integration->priv->intgRec);
		}
		if (integration->priv->isMaxStopTime)
		{
			if (integration->priv->isIntegration)
				m_LarIntgrecStopIntegration(integration->priv->intgRec);
			m_UltraBusChannelUpdateAnalyze(integration->priv->channel);
			integrationDone(integration);
			return TRUE;
		}
	}
	m_UltraBusChannelUpdateAnalyze(integration->priv->channel);
	return FALSE;
}

static void sensorReadCallback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GTask *task = G_TASK(user_data);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	GError *error = NULL;
	gdouble value = 0.0;
	gboolean result = sensor_read_finish(SENSOR_READ(source_object), &value, res, &error);
	if (!result)
	{
		g_task_return_error(task, error);
		g_object_unref(task);
		return;
	}
	if (error)
	{
		//Fehler meldung bearbeiten falls error != NULL
	}
	UltraActionIntegration *integration = ULTRA_ACTION_INTEGRATION(g_task_get_source_object(task));
	if (integrationPushValue(integration, value))
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return;
	}
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	SensorRead *read = ultra_bus_sensor_read(integration->priv->sensor);
	sensor_read_async(read, g_task_get_cancellable(task), sensorReadCallback, task);
	g_object_unref(read);
}
static void sensorInitializeCallback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	// g_debug("sensor initialize callback");
	GTask *task = G_TASK(user_data);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	GError *error = NULL;
	gboolean value = 0.0;
	gboolean result = sensor_initalize_finish(SENSOR_INITIAL(source_object), &value, res, &error);
	if (!result)
	{
		g_task_return_error(task, error);
		g_object_unref(task);
		return;
	}
	if (error)
	{
		//Fehler meldung bearbeiten falls error != NULL
	}
	UltraActionIntegration *integration = ULTRA_ACTION_INTEGRATION(g_task_get_source_object(task));
	SensorRead *read = ultra_bus_sensor_read(integration->priv->sensor);
	if (read == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_SENSOR, "Integration start error - sensor %s reading interface not found",
											  m_UltraBusChannelGetSensor(integration->priv->channel)));
		g_object_unref(task);
		return;
	}
	m_UltraActionIntegrationRunJustification(ULTRA_ACTION_INTEGRATION(integration));
	sensor_read_async(read,g_task_get_cancellable(task), sensorReadCallback, task);
	g_object_unref(read);
}

UltraBusSensor *getSensor(UltraActionIntegration *integration)
{
	UltraBusSensor *sensor = UltraDBusSensorsLookup(m_UltraBusChannelGetSensor(integration->priv->channel));
	return sensor;
}

gboolean ultraltraActionIntegrationRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_INTEGRATION(action), FALSE);

	UltraActionIntegration *integration = ULTRA_ACTION_INTEGRATION(action);
	GTask *task = g_task_new(integration, cancellable, callback, userData);
	// g_debug("run intgration 1");
	if (integration->priv->channel == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_SENSOR, "Integration start error - channel should not be null"));
		g_object_unref(task);
		return FALSE;
	}
	// g_debug("init 1");
	integration->priv->sensor = getSensor(integration);
	if (integration->priv->sensor == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_SENSOR, "Integration start error - sensor not found"));
		g_object_unref(task);
		return FALSE;
	}
	// g_debug("run get intg par");
	integration->priv->intgPar = ultra_bus_sensor_get_integration(integration->priv->sensor, integration->priv->tic);
	if (integration->priv->intgPar == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_SENSOR, "Integration start error - integration parameter object should not be null"));
		g_object_unref(task);
		return FALSE;
	}
	// g_debug("create integration rec");
	integration->priv->intgRec = LarIntgrecNew(integration->priv->intgPar);
	m_UltraBusChannelNextAnalyze(integration->priv->channel, integration->priv->intgRec);
	SensorInitial *initial = ultra_bus_sensor_initial(integration->priv->sensor);
	if (initial == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_SENSOR, "Integration start error - sensor %s initialization interface not found",
											  m_UltraBusChannelGetSensor(integration->priv->channel)));
		g_object_unref(task);
		return FALSE;
	}
	sensor_initialize_async(initial, cancellable,  sensorInitializeCallback, task);
	g_object_unref(initial);
	return TRUE;
}