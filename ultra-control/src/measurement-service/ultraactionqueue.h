/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraacrionlist.h
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_QUEUE_H_
#define _ULTRA_ACTION_QUEUE_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_QUEUE (ultra_action_queue_get_type())
#define ULTRA_ACTION_QUEUE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_QUEUE, UltraActionQueue))
#define ULTRA_ACTION_QUEUE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_QUEUE, UltraActionQueueClass))
#define ULTRA_IS_ACTION_QUEUE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_QUEUE))
#define ULTRA_IS_ACTION_QUEUE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_QUEUE))
#define ULTRA_ACTION_QUEUE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_QUEUE, UltraActionQueueClass))

typedef struct _UltraActionQueueClass UltraActionQueueClass;
typedef struct _UltraActionQueue UltraActionQueue;
typedef struct _UltraActionQueuePrivate UltraActionQueuePrivate;

GType ultra_action_queue_get_type(void) G_GNUC_CONST;

struct _UltraActionQueueClass
{
	GObjectClass parent_class;
};

struct _UltraActionQueue
{
	GObject parent_instance;
	UltraActionQueuePrivate *priv;
};

typedef enum {
	ACTION_QUEUE_ERROR,
	ACTION_QUEUE_BUSY,
	ACTION_QUEUE_RUNNED
}QueueStatus;

UltraActionQueue *UltraActionQueueNew();

gboolean m_UltraActionQueueAddAction(UltraActionQueue *list, UltraAction *action);
gboolean m_UltraActionQueuePushAction(UltraActionQueue *list, UltraAction *action);
guint m_UltraActionQueueLength(UltraActionQueue *list);
QueueStatus m_UltraActionQueueRun(UltraActionQueue *list, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);
UltraAction* m_UltraActionQueueFinish(UltraActionQueue *list, GAsyncResult *result);

G_END_DECLS

#endif /* _ULTRA_ACTION_QUEUE_H_ */

/** @} */
