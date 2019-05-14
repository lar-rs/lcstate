/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_ONLINE_H_
#define _ULTRA_ACTION_ONLINE_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultrabusstream.h"



G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_ONLINE             (ultra_action_online_get_type ())
#define ULTRA_ACTION_ONLINE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_ONLINE, UltraActionOnline))
#define ULTRA_ACTION_ONLINE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_ONLINE, UltraActionOnlineClass))
#define ULTRA_IS_ACTION_ONLINE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_ONLINE))
#define ULTRA_IS_ACTION_ONLINE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_ONLINE))
#define ULTRA_ACTION_ONLINE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_ONLINE, UltraActionOnlineClass))

typedef struct _UltraActionOnlineClass     UltraActionOnlineClass;
typedef struct _UltraActionOnline          UltraActionOnline;
typedef struct _UltraActionOnlinePrivate   UltraActionOnlinePrivate;

GType ultra_action_online_get_type (void) G_GNUC_CONST;


struct _UltraActionOnlineClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionOnline
{
	UltraAction                         parent_instance;
	UltraActionOnlinePrivate                       *priv;
};


UltraAction* ultra_action_online_new();


G_END_DECLS

#endif /* _ULTRA_ACTION_ONLINE_H_ */

/** @} */
