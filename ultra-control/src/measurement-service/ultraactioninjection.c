/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactioninjection.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactioninjection.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include "ultraconfig.h"


#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionInjectionPrivate
{
	gboolean codo;
	gboolean tic;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_CODO,
	PROP_TIC,
};

#define ULTRA_ACTION_INJECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_INJECTION, UltraActionInjectionPrivate))

G_DEFINE_TYPE(UltraActionInjection, ultra_action_injection, ULTRA_TYPE_ACTION)

static void
ultra_action_injection_init(UltraActionInjection *injection)
{
	injection->priv = ULTRA_ACTION_INJECTION_PRIVATE(injection);
	injection->priv->codo = FALSE;
	injection->priv->tic = FALSE;
}

static void
ultra_action_injection_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	// UltraActionInjection *injection = ULTRA_ACTION_INJECTION(object);
	G_OBJECT_CLASS(ultra_action_injection_parent_class)->finalize(object);
}

static void
ultra_action_injection_set_property(GObject *object,
								   guint prop_id,
								   const GValue *value,
								   GParamSpec *pspec)
{
	UltraActionInjection *injection = ULTRA_ACTION_INJECTION(object);
	switch (prop_id)
	{
	case PROP_CODO:
		injection->priv->codo = g_value_get_boolean(value);
		break;
	case PROP_TIC:
		injection->priv->tic = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_injection_get_property(GObject *object,
								   guint prop_id,
								   GValue *value,
								   GParamSpec *pspec)
{
	UltraActionInjection *injection = ULTRA_ACTION_INJECTION(object);
	switch (prop_id)
	{
	case PROP_CODO:
		g_value_set_boolean(value, injection->priv->codo);
		break;
	case PROP_TIC:
		g_value_set_boolean(value, injection->priv->tic);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionInjectionRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_injection_class_init(UltraActionInjectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionInjectionClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_injection_finalize;
	object_class->set_property = ultra_action_injection_set_property;
	object_class->get_property = ultra_action_injection_get_property;
	aclass->run = ultraltraActionInjectionRun;
	g_object_class_install_property(object_class, PROP_CODO,
									g_param_spec_boolean("codo", "codo", "codo",FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property(object_class, PROP_TIC,
									g_param_spec_boolean("tic", "tic", "tic",FALSE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

UltraAction *UltraActionInjectionNew(gboolean codo, gboolean tic)
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_INJECTION, "codo", codo, "tic", tic, NULL));
	return action;
}

static void injectionFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	if (g_task_return_error_if_cancelled(G_TASK(task)))
	{
		g_object_unref(task);
		return;
	}
	gboolean result = FALSE;
	GError *error = NULL;

	if (!sequence_workers_process_call_run_finish(SEQUENCE_WORKERS_PROCESS(sourceObject), &result, res, &error))
	{
		if (error)
			g_dbus_error_strip_remote_error(error);

		g_task_return_error(task, error);
		g_object_unref(task);
		return;
	}
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
}

void injectionTestingFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
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

gboolean ultraltraActionInjectionRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_INJECTION(action), FALSE);
	UltraActionInjection *injection = ULTRA_ACTION_INJECTION(action);
	GTask *task = g_task_new(injection, cancellable, callback, userData);
	if (ConfigureIsTestMode())
	{
		UltraAction *test_timer = UltraActionTimerNew(14*G_TIME_SPAN_SECOND);
		m_UltraActionSetName(test_timer,"I:test-injection");
		m_UltraActionRun(test_timer,cancellable,injectionTestingFinishCallback,task);
		g_object_unref(test_timer);
		return TRUE;
	}
	SequenceObject *worker = NULL;
    if (injection->priv->codo && !injection->priv->tic ) {
		worker = ULTRA_INJECTIONCOD_SEQUENCE_WORKER();
    } else if(injection->priv->tic) {
		worker = ULTRA_INJECTIONTIC_SEQUENCE_WORKER();
    } else {
		worker = ULTRA_INJECTION_SEQUENCE_WORKER();
    }
	if (worker == NULL) {
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_WORKER_NOT_FOUND, "Injection start error - injection worker not found"));
		g_object_unref(task);
		return FALSE;
	}
	SequenceWorkersProcess *workerProcess = sequence_object_get_workers_process(worker);
  	sequence_workers_process_call_run(workerProcess, cancellable, injectionFinishCallback, task);
	g_object_unref(workerProcess);
	return TRUE;
}