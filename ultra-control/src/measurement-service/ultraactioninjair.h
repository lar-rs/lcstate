/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactioninjair.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_INJAIR_H_
#define _ULTRA_ACTION_INJAIR_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultrabusstream.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_INJAIR             (ultra_action_injair_get_type ())
#define ULTRA_ACTION_INJAIR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_INJAIR, UltraActionInjair))
#define ULTRA_ACTION_INJAIR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_INJAIR, UltraActionInjairClass))
#define ULTRA_IS_ACTION_INJAIR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_INJAIR))
#define ULTRA_IS_ACTION_INJAIR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_INJAIR))
#define ULTRA_ACTION_INJAIR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_INJAIR, UltraActionInjairClass))

typedef struct _UltraActionInjairClass     UltraActionInjairClass;
typedef struct _UltraActionInjair          UltraActionInjair;
typedef struct _UltraActionInjairPrivate   UltraActionInjairPrivate;

GType ultra_action_injair_get_type (void) G_GNUC_CONST;


struct _UltraActionInjairClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionInjair
{
	UltraAction                         parent_instance;
	UltraActionInjairPrivate            *priv;
};

UltraAction *UltraActionInjairNew(UltraBusStream *stream);


G_END_DECLS

#endif /* _ULTRA_ACTION_INJAIR_H_ */

/** @} */
