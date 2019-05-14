/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionremote.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraerrors.h"
#include "ultrabusstream.h"
#include "ultraactionremote.h"
#include "ultraactionqueue.h"
#include "ultraactionmeasurement.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionRemotePrivate
{
	UltraBusStream *stream;
	UltraActionQueue *queue;
	GDateTime *last_signal;
	GList *connected_node;

	// StreamsObject       *stream;
};

enum
{
	ULTRA_PROP0,
	PROP_STATE,
};

#define ULTRA_ACTION_REMOTE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_REMOTE, UltraActionRemotePrivate))

G_DEFINE_TYPE(UltraActionRemote, ultra_action_remote, ULTRA_TYPE_ACTION)

static void
ultra_action_remote_init(UltraActionRemote *remote)
{
	remote->priv = ULTRA_ACTION_REMOTE_PRIVATE(remote);
	remote->priv->last_signal = NULL;
	remote->priv->stream = NULL;
	remote->priv->queue = NULL;
	remote->priv->connected_node = NULL;
}

static void
ultra_action_remote_finalize(GObject *object)
{
	UltraActionRemote *remote = ULTRA_ACTION_REMOTE(object);
	if (remote->priv->connected_node)
	{
		GList *signals = remote->priv->connected_node;
		for (; signals != NULL; signals = signals->next)
		{
			g_signal_handlers_disconnect_by_data(signals->data, remote);
		}
		g_list_free_full(remote->priv->connected_node, g_object_unref);
	}
	if (remote->priv->last_signal)
		g_date_time_unref(remote->priv->last_signal);
	if (remote->priv->stream)
		g_object_unref(remote->priv->stream);
	if (remote->priv->queue)
		g_object_unref(remote->priv->queue);
	G_OBJECT_CLASS(ultra_action_remote_parent_class)->finalize(object);
}

static void
ultra_action_remote_set_property(GObject *object,
								 guint prop_id,
								 const GValue *value,
								 GParamSpec *pspec)
{
	// 	UltraActionRemote *action = ULTRA_ACTION_REMOTE(object);
	// 	switch (prop_id)
	// 	{
	// 	case PROP_STATE:
	// 		action->priv->state = g_value_get_gtype(value);
	// 		break;
	// 	default:
	// 		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	// 		break;
	// 	}
}

static void
ultra_action_remote_get_property(GObject *object,
								 guint prop_id,
								 GValue *value,
								 GParamSpec *pspec)
{
	// UltraActionRemote *action = ULTRA_ACTION_REMOTE(object);
	// switch (prop_id)
	// {
	// case PROP_STATE:
	// 	g_value_set_uint(value, action->priv->state);
	// 	break;

	// default:
	// 	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	// 	break;
	// }
}

void remoteControlSignalChanged(NodesDigital16 *digin, GParamSpec *pspec, UltraActionRemote *remote)
{
	gboolean remote_signal = FALSE;
	if (pspec->value_type == G_TYPE_BOOLEAN)
		g_object_get(digin, pspec->name, &remote_signal, NULL);
	if(remote_signal){
	    UltraAction *remote_measurement =  UltraActionMeasurementOnlineNew(ULTRA_BUS_STREAM(remote->priv->stream));
		m_UltraActionQueueAddAction(remote->priv->queue,remote_measurement);
		g_object_unref(remote_measurement);
	}
}

static void remote_cancelled(GCancellable *cancel, GTask *task)
{
	g_task_return_boolean(task, TRUE);
	g_object_unref(task);
}

static gboolean ultraltraActionRemoteRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_REMOTE(action), FALSE);
	UltraActionRemote *remote = ULTRA_ACTION_REMOTE(action);
	GTask *task = g_task_new(remote, cancellable, callback, userData);
	if (remote->priv->stream == NULL)
	{
		g_task_return_error(task, g_error_new(UltraErrorsQuark(), UERROR_EMPTY_STREAM, "Streams is empty"));
		return FALSE;
	}
	if (remote->priv->queue== NULL)
	{
		g_task_return_error(task, g_error_new(UltraErrorsQuark(), UERROR_EMPTY_STREAM, "Queue is empty"));
		return FALSE;
	}
	if(cancellable)	g_cancellable_connect(cancellable, G_CALLBACK(remote_cancelled), g_object_ref(task), g_object_unref);
	NodesObject *node = NULL;
	NodesDigital16 *remote_node = NULL;
	if (m_UltraBusStreamGetNumber(ULTRA_BUS_STREAM(remote->priv->stream)) > 3)
		node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
	else node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
	if (node != NULL)
	{
		remote_node = nodes_object_get_digital16(node);
		if (remote_node)
		{
			gint in = m_UltraBusStreamGetNumber(ULTRA_BUS_STREAM(remote->priv->stream));
			gchar *connect_prop = g_strdup_printf("in%02d", in);
			if (g_signal_connect(remote_node, connect_prop, G_CALLBACK(remoteControlSignalChanged), remote) > 0)
			{
				remote->priv->connected_node = g_list_append(remote->priv->connected_node, g_object_ref(remote_node));
			}
			g_object_unref(remote_node);
		}
		g_object_unref(node);
	}
	return TRUE;
}

static void
ultra_action_remote_class_init(UltraActionRemoteClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionRemoteClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_remote_finalize;
	object_class->set_property = ultra_action_remote_set_property;
	object_class->get_property = ultra_action_remote_get_property;
	aclass->run = ultraltraActionRemoteRun;
}

UltraAction *UltraActionRemoteNew(UltraActionQueue *queue, UltraBusStream *stream)
{
	UltraActionRemote *remote = ULTRA_ACTION_REMOTE(g_object_new(ULTRA_TYPE_ACTION_REMOTE, NULL));
	remote->priv->stream = g_object_ref(stream);
	remote->priv->queue = g_object_ref(queue);
	return ULTRA_ACTION(remote);
}
