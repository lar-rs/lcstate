/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_REMOTE_H_
#define _ULTRA_ACTION_REMOTE_H_

#include <glib-object.h>
#include <mktlib.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultrabusstream.h"
#include "ultraactionqueue.h"


G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_REMOTE             (ultra_action_remote_get_type ())
#define ULTRA_ACTION_REMOTE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_REMOTE, UltraActionRemote))
#define ULTRA_ACTION_REMOTE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_REMOTE, UltraActionRemoteClass))
#define ULTRA_IS_ACTION_REMOTE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_REMOTE))
#define ULTRA_IS_ACTION_REMOTE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_REMOTE))
#define ULTRA_ACTION_REMOTE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_REMOTE, UltraActionRemoteClass))

typedef struct _UltraActionRemoteClass     UltraActionRemoteClass;
typedef struct _UltraActionRemote          UltraActionRemote;
typedef struct _UltraActionRemotePrivate   UltraActionRemotePrivate;

GType ultra_action_remote_get_type (void) G_GNUC_CONST;


struct _UltraActionRemoteClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionRemote
{
	UltraAction                         parent_instance;
	UltraActionRemotePrivate            *priv;
};

UltraAction *UltraActionRemoteNew(UltraActionQueue *queue, UltraBusStream *stream);


G_END_DECLS

#endif /* _ULTRA_ACTION_REMOTE_H_ */

/** @} */
