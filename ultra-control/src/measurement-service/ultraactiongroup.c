/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactiongroup.h"
#include "ultraactiontimer.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionGroupPrivate
{
	gboolean async;
	GList *actions;
	GList *current;
	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_ASYNC,
};

#define ULTRA_ACTION_GROUP_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_GROUP, UltraActionGroupPrivate))

G_DEFINE_TYPE(UltraActionGroup, ultra_action_group, ULTRA_TYPE_ACTION)

static void
ultra_action_group_init(UltraActionGroup *ultra_action_group)
{
	//tera_pumps_manager_client_new();
	//ultra_vessels_manager_client_new();
	ultra_action_group->priv = ULTRA_ACTION_GROUP_PRIVATE(ultra_action_group);
	/* TODO: Add initialization code here */
}

static void
ultra_action_group_finalize(GObject *object)
{
	/* TODO: Add deinitalization code here */
	UltraActionGroup *group = ULTRA_ACTION_GROUP(object);
	if (group->priv->actions)
		g_list_free_full(group->priv->actions, g_object_unref);
	G_OBJECT_CLASS(ultra_action_group_parent_class)->finalize(object);
	//g_debug("Ultra action finalize 2");
}

static void
ultra_action_group_set_property(GObject *object,
								guint prop_id,
								const GValue *value,
								GParamSpec *pspec)
{
	UltraActionGroup *action = ULTRA_ACTION_GROUP(object);
	switch (prop_id)
	{
	case PROP_ASYNC:
		action->priv->async = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_group_get_property(GObject *object,
								guint prop_id,
								GValue *value,
								GParamSpec *pspec)
{
	UltraActionGroup *action = ULTRA_ACTION_GROUP(object);
	switch (prop_id)
	{
	case PROP_ASYNC:
		g_value_set_boolean(value, action->priv->async);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean ultraltraActionGroupRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_group_class_init(UltraActionGroupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionGroupClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	//object_class->dispose              = ultimate_atom_dispose;
	object_class->finalize = ultra_action_group_finalize;
	object_class->set_property = ultra_action_group_set_property;
	object_class->get_property = ultra_action_group_get_property;
	aclass->run = ultraltraActionGroupRun;
	g_object_class_install_property(object_class, PROP_ASYNC,
									g_param_spec_boolean("async", "Action is async", "Action async", TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
}

void runNextAction(GTask *task);

void ultraActionNextFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
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
		if (!m_UltraActionFinish(action, res))
		{
			GError *error = m_UltraActionError(action);
			g_task_return_error(task, g_error_new(UltraActionErrorQuark(), error ? error->code : 6, "action fail - %s", error ? error->message : "unknown"));
			g_object_unref(task);
			return;
		}
	}
	else
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), 2, "Incorrect action in the actions list"));
		g_object_unref(task);
		return;
	}
	UltraActionGroup *group = ULTRA_ACTION_GROUP(g_task_get_source_object(task));
	group->priv->current = group->priv->current->next;
	if (group->priv->current == NULL)
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return;
	}
	runNextAction(task);
}

gboolean checkIsComplited(UltraActionGroup *group)
{
	gboolean allDone = TRUE;
	GList *l = group->priv->actions;
	for (; l != NULL; l = l->next)
	{
		if (!m_UltraActionComplited(ULTRA_ACTION(l->data)))
			allDone = FALSE;
	}
	return allDone;
}
void ultraActionWaitFinishCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData)
{
	UltraAction *action = ULTRA_ACTION(sourceObject);
	GTask *task = G_TASK(userData);
	m_UltraActionFinish(action, res);
	UltraActionGroup *group = ULTRA_ACTION_GROUP(g_task_get_source_object(task));
	if (checkIsComplited(group))
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
	}
}

void runNextAction(GTask *task)
{
	if (g_task_return_error_if_cancelled(task))
	{
		g_object_unref(task);
		return;
	}
	UltraActionGroup *group = ULTRA_ACTION_GROUP(g_task_get_source_object(task));
	if (group->priv->current)
	{
		UltraAction *action = ULTRA_ACTION(group->priv->current->data);
		m_UltraActionRun(action, g_task_get_cancellable(task), ultraActionNextFinishCallback, task);
	}
	else
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), 1, "Empty action list"));
		g_object_unref(task);
	}
}

void runAsyncActions(GTask *task)
{
	if (g_task_return_error_if_cancelled(task))
	{
		g_object_unref(task);
		return;
	}
	UltraActionGroup *group = ULTRA_ACTION_GROUP(g_task_get_source_object(task));
	if (group->priv->actions)
	{
		GList *l = group->priv->actions;
		for (; l != NULL; l = l->next)
		{
			m_UltraActionRun(ULTRA_ACTION(l->data), g_task_get_cancellable(task), ultraActionWaitFinishCallback, task);
		}
	}
	else
	{
		g_task_return_error(task, g_error_new(UltraActionErrorQuark(), 1, "Empty action list"));
		g_object_unref(task);
	}
}

gboolean ultraltraActionGroupRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_GROUP(action), FALSE);
	UltraActionGroup *group = ULTRA_ACTION_GROUP(action);
	GTask *task = g_task_new(group, cancellable, callback, userData);
	if (group->priv->actions == NULL || g_list_length(group->priv->actions) == 0)
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	group->priv->current = group->priv->actions;
	if (group->priv->async)
	{
		runAsyncActions(task);
	}
	else
	{
		runNextAction(task);
	}

	return TRUE;
}

/**
 * UltraActionGroupNew:
 *
 * created a new #UltraActionGroup async
 *
 * Return:a new #UltraActionGroup
 */
UltraAction *UltraActionGroupNew()
{
	UltraAction *group = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_GROUP, NULL));
	return group;
}
/**
 * UltraActionGroupSyncNew:
 *
 * created a new #UltraActionGroup sync 
 *
 * Return:a new #UltraActionGroup
 */
UltraAction *UltraActionGroupSyncNew()
{
	UltraAction *group = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_GROUP, "async", FALSE, NULL));
	return group;
}

/**
 * m_UltraActionGroupAddAction:
 * @group: a #UltraActionGroup
 * @action: a #UltraAction
 *
 * result gboolean add a new action to the group 
 *
 * Return:TRUE if the action successfully added
 */

gboolean m_UltraActionGroupAddAction(UltraActionGroup *group, UltraAction *action)
{
	g_return_val_if_fail(group != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_GROUP(group), FALSE);
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	group->priv->actions = g_list_append(group->priv->actions, action);
	return TRUE;
}

/**
 * m_UltraActionGroupAddAction:
 * @group: a #UltraActionGroup
 *
 * result action list
 *
 * Return:GList of a #UltraAction
 */
GList *m_UltraActionGroupGetActions(UltraActionGroup *group)
{
	g_return_val_if_fail(group != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION_GROUP(group), NULL);
	return group->priv->actions;
}