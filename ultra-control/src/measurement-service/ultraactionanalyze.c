/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionanalyze.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionanalyze.h"
#include "ultraactioninjection.h"
#include "ultraactioninjair.h"
#include "ultraactionholdxy.h"
#include "ultraactionhold.h"
#include "ultraactiongroup.h"
#include "ultraactionrinsing.h"
#include "ultraactioninjection.h"
#include "ultraactionintegration.h"
#include "ultraactiontimer.h"
#include "ultraconfig.h"
#include "ultraerrors.h"
#include "lartestsensor.h"
#include "ultrabussensor.h"
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionAnalyzePrivate
{
	UltraBusStream *stream;
	GList *channels;
	UltraAction *timeout;
	UltraAction *intgGroup;
	UltraAction *mainGroup;
	gboolean isTic;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_PUMP,
	PROP_INTERVAL,
};

#define ULTRA_ACTION_ANALYZE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_ANALYZE, UltraActionAnalyzePrivate))

G_DEFINE_TYPE(UltraActionAnalyze, ultra_action_analyze, ULTRA_TYPE_ACTION)

static void
ultra_action_analyze_init(UltraActionAnalyze *analyze)
{
	analyze->priv = ULTRA_ACTION_ANALYZE_PRIVATE(analyze);
	analyze->priv->timeout = NULL;
	analyze->priv->intgGroup = NULL;
	analyze->priv->mainGroup = NULL;
	analyze->priv->stream = NULL;
	analyze->priv->channels = NULL;
}

static void
ultra_action_analyze_finalize(GObject *object)
{
	UltraActionAnalyze *analyze = ULTRA_ACTION_ANALYZE(object);
	if (analyze->priv->timeout)
		g_object_unref(analyze->priv->timeout);
	if(analyze->priv->intgGroup)
		g_object_unref(analyze->priv->intgGroup);
	if (analyze->priv->mainGroup)
		g_object_unref(analyze->priv->mainGroup);
	if (analyze->priv->channels)
		g_list_free_full(analyze->priv->channels,g_object_unref);
	if (analyze->priv->stream)
		g_object_unref(analyze->priv->stream);
	
	G_OBJECT_CLASS(ultra_action_analyze_parent_class)->finalize(object);
}

static gboolean ultraltraActionAnalyzeRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_analyze_class_init(UltraActionAnalyzeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionAnalyzeClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_analyze_finalize;
	aclass->run = ultraltraActionAnalyzeRun;
}

UltraAction *UltraActionAnalyzeNew(UltraBusStream *stream, GList *channels)
{
	UltraActionAnalyze *action = ULTRA_ACTION_ANALYZE(g_object_new(ULTRA_TYPE_ACTION_ANALYZE, NULL));
	action->priv->stream = g_object_ref(stream);
	action->priv->channels =  g_list_copy_deep (channels, (GCopyFunc) g_object_ref, NULL);
	return ULTRA_ACTION(action);
}

// static void analyzeTimeoutCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
// {
// 	g_debug("analyze timeout callback");
// 	GTask *task = G_TASK(userData);
// 	if (g_task_return_error_if_cancelled(G_TASK(task)))
// 	{
// 		g_object_unref(task);
// 		return;
// 	}
// 	g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_ANALYZE_TIMEOUT, "Analyze timeout "));
// 	g_object_unref(task);
// }

static void analyzeIntegrationDone(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	// UltraActionAnalyze *analyze = ULTRA_ACTION_ANALYZE(g_task_get_cancellable(task));
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
}

static void analyzeJustificationDoneCallback(UltraAction *action,UltraActionAnalyze *analyze){
	GList *l = m_UltraActionGroupGetActions(ULTRA_ACTION_GROUP(analyze->priv->intgGroup));
	for (; l != NULL; l = l->next)
	{
		m_UltraActionIntegrationStopJustification(ULTRA_ACTION_INTEGRATION(l->data));
		// m_UltraActionIntegrationRunIntegration(ULTRA_ACTION_INTEGRATION(l->data));
	}
}

static void  analyzeInjectionDoneCallback(UltraAction *action,UltraActionAnalyze *analyze){
	GList *l = m_UltraActionGroupGetActions(ULTRA_ACTION_GROUP(analyze->priv->intgGroup));
	for (; l != NULL; l = l->next)
	{
		m_UltraActionIntegrationRunIntegration(ULTRA_ACTION_INTEGRATION(l->data));
	}
}


gboolean ultraltraActionAnalyzeRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{

	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_ANALYZE(action), FALSE);
	UltraActionAnalyze *analyze = ULTRA_ACTION_ANALYZE(action);
	GTask *task = g_task_new(analyze, cancellable, callback, userData);
	if (analyze->priv->channels == NULL || g_list_length(analyze->priv->channels) == 0)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_NO_CHANNELS, "Analyze no channels "));
		g_object_unref(task);
		return FALSE;
	}
	analyze->priv->mainGroup = UltraActionGroupNew();
	m_UltraActionSetName(analyze->priv->mainGroup,"group-main");

	analyze->priv->intgGroup = UltraActionGroupNew();
	m_UltraActionSetName(analyze->priv->intgGroup,"group-integration");
	GList *l = analyze->priv->channels;
	for (; l != NULL; l = l->next)
	{
		if (m_UltraBusChannelIsTIC(ULTRA_BUS_CHANNEL(l->data)))
		{
			analyze->priv->isTic = TRUE;
		}
		UltraAction *integration= UltraActionIntegrationNew(ULTRA_BUS_CHANNEL(l->data),analyze->priv->isTic);
		gchar *name = g_strdup_printf("%s integration",m_UltraBusChannelGetName(ULTRA_BUS_CHANNEL(l->data)));
		m_UltraActionSetName(integration,name);
		m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(analyze->priv->intgGroup),integration);
		g_free(name);
	}
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(analyze->priv->mainGroup),g_object_ref(analyze->priv->intgGroup));
	// analyze->priv->timeout = UltraActionTimerNew(800 * G_TIME_SPAN_SECOND);
	// m_UltraActionSetName(analyze->priv->timeout,"Analyse timeout");
	UltraAction *processGroup = UltraActionGroupSyncNew();
	m_UltraActionSetName(processGroup,"process-injection");
	gdouble jsTime = UltradeviceGetJustificationTime(ConfigureDevice());
	g_debug("Justification time %f",jsTime);
	if(jsTime < G_TIME_SPAN_SECOND)jsTime = G_TIME_SPAN_SECOND*jsTime; 
	UltraAction *justificationTimer = UltraActionTimerNew(jsTime);
	m_UltraActionSetName(justificationTimer,"justification-timer");
	g_signal_connect(justificationTimer,"action-done",G_CALLBACK(analyzeJustificationDoneCallback),analyze);
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(processGroup),justificationTimer);

	UltraAction *injGroup = UltraActionGroupNew();
	m_UltraActionSetName(injGroup,"group-injection");
	gboolean codo = m_UltraBusStreamIsCODoInjection(analyze->priv->stream);
	UltraAction *injection = UltraActionInjectionNew(codo,analyze->priv->isTic);
	m_UltraActionSetName(injection,"injection");
	g_signal_connect(injection,"action-done",G_CALLBACK(analyzeInjectionDoneCallback),analyze);
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(injGroup),injection);
	UltraAction *airAnalyse = UltraActionInjairNew(analyze->priv->stream);

	m_UltraActionSetName(airAnalyse,"air-analyze");
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(injGroup),airAnalyse);
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(processGroup),injGroup);
	guint count=1;
	if(m_UltraBusStreamIsAfterRinsing(analyze->priv->stream))
		count = m_UltraBusStreamGetAfterRinsingCount(analyze->priv->stream);
	const gchar *drainVessel = m_UltraBusStreamGetDrainVessel(analyze->priv->stream);
	UltraAction *rinsing = UltraActionRinsingNew(drainVessel,400,count);
	m_UltraActionSetName(rinsing,"rinsing");
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(processGroup),rinsing);
	m_UltraActionGroupAddAction(ULTRA_ACTION_GROUP(analyze->priv->mainGroup),processGroup);
	// m_UltraActionRun(analyze->priv->timeout, cancellable, analyzeTimeoutCallback, task);
	m_UltraActionRun(analyze->priv->mainGroup, cancellable, analyzeIntegrationDone, task);
	return TRUE;
}

GList *UltraActionAnalyseChanels(UltraActionAnalyze *analyze)
{
	g_return_val_if_fail(analyze != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_ANALYZE(analyze), NULL);
	return analyze->priv->channels;
}