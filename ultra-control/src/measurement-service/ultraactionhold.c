/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionhold.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionhold.h"
#include "ultraactiontimer.h"
#include "ultraerrors.h"
#include "ultraconfig.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionHoldPrivate
{
	gboolean reserved;
	// gchar *vessel;
	// gint volume;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
};

#define ULTRA_ACTION_HOLD_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_HOLD, UltraActionHoldPrivate))

G_DEFINE_TYPE(UltraActionHold, ultra_action_hold, ULTRA_TYPE_ACTION)

static void
ultra_action_hold_init(UltraActionHold *hold)
{
	hold->priv = ULTRA_ACTION_HOLD_PRIVATE(hold);
}

static void
ultra_action_hold_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	// UltraActionHold *hold = ULTRA_ACTION_HOLD(object);
	// if (hold->priv->vessel)
	// 	g_free(hold->priv->vessel);
	G_OBJECT_CLASS(ultra_action_hold_parent_class)->finalize(object);
}



static gboolean ultraltraActionHoldRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_hold_class_init(UltraActionHoldClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionHoldClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_hold_finalize;
	aclass->run = ultraltraActionHoldRun;
}

UltraAction *UltraActionHoldNew()
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_HOLD, NULL));
	return action;
}

static void holdFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
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

gboolean ultraltraActionHoldRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_HOLD(action), FALSE);
	UltraActionHold *hold = ULTRA_ACTION_HOLD(action);
	GTask *task = g_task_new(hold, cancellable, callback, userData);
	if (ConfigureIsTestMode())
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	SequenceObject *worker = ULTRA_HOLD_SEQUENCE_WORKER();
	if (worker == NULL)
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), UERROR_WORKER_NOT_FOUND, "Hold start error - hold worker not found"));
		g_object_unref(task);
		return FALSE;
	}
	SequenceWorkersProcess *workerProcess = sequence_object_get_workers_process(worker);
	sequence_workers_process_call_run(workerProcess, cancellable, holdFinishCallback, task);
	g_object_unref(workerProcess);
	return TRUE;
}