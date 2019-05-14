/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionqueue.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionqueue.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionQueuePrivate
{
	GList *actions;
	UltraAction *runned;
	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_ASYNC,
};

#define ULTRA_ACTION_QUEUE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_QUEUE, UltraActionQueuePrivate))

G_DEFINE_TYPE(UltraActionQueue, ultra_action_queue,G_TYPE_OBJECT)

static void
ultra_action_queue_init(UltraActionQueue *list)
{
	//tera_pumps_manager_client_new();
	//ultra_vessels_manager_client_new();
	list->priv = ULTRA_ACTION_QUEUE_PRIVATE(list);
	list->priv->actions = NULL;
	list->priv->runned= NULL;
	/* TODO: Add initialization code here */
}

static void
ultra_action_queue_finalize(GObject *object)
{
	/* TODO: Add deinitalization code here */
	UltraActionQueue *list = ULTRA_ACTION_QUEUE(object);
	if (list->priv->actions)
		g_list_free_full(list->priv->actions, g_object_unref);
	if (list->priv->runned)
		g_object_unref(list->priv->runned);
	G_OBJECT_CLASS(ultra_action_queue_parent_class)->finalize(object);
}

// static void
// ultra_action_queue_set_property(GObject *object,
// 								guint prop_id,
// 								const GValue *value,
// 								GParamSpec *pspec)
// {
// 	UltraActionQueue *action = ULTRA_ACTION_QUEUE(object);
// 	switch (prop_id)
// 	{
// 	case PROP_ASYNC:
// 		action->priv->async = g_value_get_boolean(value);
// 		break;
// 	default:
// 		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
// 		break;
// 	}
// }

// static void
// ultra_action_queue_get_property(GObject *object,
// 								guint prop_id,
// 								GValue *value,
// 								GParamSpec *pspec)
// {
// 	UltraActionQueue *action = ULTRA_ACTION_QUEUE(object);
// 	switch (prop_id)
// 	{
// 	case PROP_ASYNC:
// 		g_value_set_boolean(value, action->priv->async);
// 		break;

// 	default:
// 		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
// 		break;
// 	}
// }

static void
ultra_action_queue_class_init(UltraActionQueueClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionQueueClass));
	//object_class->dispose              = ultimate_atom_dispose;
	object_class->finalize = ultra_action_queue_finalize;
	// object_class->set_property = ultra_action_queue_set_property;
	// object_class->get_property = ultra_action_queue_get_property;
	// g_object_class_install_property(object_class, PROP_ASYNC,
	// 								g_param_spec_boolean("async", "Action is async", "Action async", TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
	g_signal_new("add", G_TYPE_FROM_CLASS(klass),
				 G_SIGNAL_RUN_LAST,
				 0,
				 NULL, NULL,
				 g_cclosure_marshal_VOID__OBJECT,
				 G_TYPE_NONE,
				 0,
				 G_TYPE_OBJECT);
	g_signal_new("push", G_TYPE_FROM_CLASS(klass),
				 G_SIGNAL_RUN_LAST,
				 0,
				 NULL, NULL,
				 g_cclosure_marshal_VOID__OBJECT,
				 G_TYPE_NONE,
				 0,
				 G_TYPE_OBJECT);
	g_signal_new("pop", G_TYPE_FROM_CLASS(klass),
				 G_SIGNAL_RUN_LAST,
				 0,
				 NULL, NULL,
				 g_cclosure_marshal_VOID__OBJECT,
				 G_TYPE_NONE,
				 0,
				 G_TYPE_OBJECT);
}

/**
 * UltraActionQueueNew:
 *
 * created a new #UltraActionQueue async
 *
 * Return:a new #UltraActionQueue
 */
UltraActionQueue *UltraActionQueueNew()
{
	UltraActionQueue *queue= ULTRA_ACTION_QUEUE(g_object_new(ULTRA_TYPE_ACTION_QUEUE, NULL));
	return queue;
}
/**
 * m_UltraActionQueuePushAction:
 * @list: a #UltraActionQueue
 * @action: a #UltraAction
 *
 * result gboolean tru action pushed in action list
 *
 * Return:TRUE if the action successfully added
 */

gboolean m_UltraActionQueuePushAction(UltraActionQueue *list, UltraAction *action)
{
	g_return_val_if_fail(list != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_QUEUE(list), FALSE);
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	GList *found = g_list_find_custom(list->priv->actions, action, UltraActionCompareFunc);
	if (found != NULL)
		return FALSE;
	list->priv->actions = g_list_prepend(list->priv->actions, g_object_ref(action));
	g_signal_emit_by_name(list, "push", action);
	return TRUE;
}
/**
 * m_UltraActionQueueAddAction:
 * @list: a #UltraActionQueue
 * @action: a #UltraAction
 *
 * result gboolean true action is added to the list.
 *
 * Return:TRUE if the action successfully added
 */

gboolean m_UltraActionQueueAddAction(UltraActionQueue *list, UltraAction *action)
{
	g_return_val_if_fail(list != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_QUEUE(list), FALSE);
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	GList *found = g_list_find_custom(list->priv->actions, action, UltraActionCompareFunc);
	if (found != NULL)
		return FALSE;
	list->priv->actions = g_list_append(list->priv->actions, g_object_ref(action));
	g_signal_emit_by_name(list, "add", action);
	return TRUE;
}

guint m_UltraActionQueueLength(UltraActionQueue *list)
{
	g_return_val_if_fail(list != NULL, 0);
	g_return_val_if_fail(ULTRA_IS_ACTION_QUEUE(list), 0);
	g_return_val_if_fail(list->priv->actions != NULL, 0);
	return g_list_length(list->priv->actions);
}

void actionFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	GTask *task = G_TASK(userData);
	UltraActionQueue *list = ULTRA_ACTION_QUEUE(g_task_get_source_object(task));
	list->priv->runned = NULL;
	m_UltraActionFinish(ULTRA_ACTION(sourceObject),res);
	g_task_return_pointer(task,sourceObject,g_object_unref);
	g_object_unref(task);
}
QueueStatus m_UltraActionQueueRun(UltraActionQueue *list, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
	g_return_val_if_fail(list != NULL,ACTION_QUEUE_ERROR);
	g_return_val_if_fail(ULTRA_IS_ACTION_QUEUE(list),ACTION_QUEUE_ERROR);
	if(list->priv->actions == NULL || g_list_length(list->priv->actions)==0)return ACTION_QUEUE_ERROR;

	if (list->priv->runned != NULL)
	{
		return ACTION_QUEUE_BUSY;
	}
	list->priv->runned = ULTRA_ACTION(list->priv->actions->data);
	list->priv->actions = g_list_remove_all(list->priv->actions, list->priv->runned);
	GTask *task = g_task_new(list, cancellable, callback, user_data);
	g_signal_emit_by_name(list, "pop", list->priv->runned);
	m_UltraActionRun(list->priv->runned, cancellable, actionFinishCallback, task);
	return ACTION_QUEUE_RUNNED;
}

UltraAction* m_UltraActionQueueFinish(UltraActionQueue *list, GAsyncResult *result)
{
	g_return_val_if_fail(list != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_QUEUE(list), NULL);
	UltraAction *action = g_task_propagate_pointer(G_TASK(result), NULL);
	return action;
}
