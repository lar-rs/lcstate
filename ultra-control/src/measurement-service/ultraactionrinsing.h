/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionrinsing.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_RINSING_H_
#define _ULTRA_ACTION_RINSING_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_RINSING             (ultra_action_rinsing_get_type ())
#define ULTRA_ACTION_RINSING(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_RINSING, UltraActionRinsing))
#define ULTRA_ACTION_RINSING_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_RINSING, UltraActionRinsingClass))
#define ULTRA_IS_ACTION_RINSING(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_RINSING))
#define ULTRA_IS_ACTION_RINSING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_RINSING))
#define ULTRA_ACTION_RINSING_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_RINSING, UltraActionRinsingClass))

typedef struct _UltraActionRinsingClass     UltraActionRinsingClass;
typedef struct _UltraActionRinsing          UltraActionRinsing;
typedef struct _UltraActionRinsingPrivate   UltraActionRinsingPrivate;

GType ultra_action_rinsing_get_type (void) G_GNUC_CONST;


struct _UltraActionRinsingClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionRinsing
{
	UltraAction                         parent_instance;
	UltraActionRinsingPrivate            *priv;
};

UltraAction *UltraActionRinsingNew(const gchar *vessel, guint volume, guint repeat);



G_END_DECLS

#endif /* _ULTRA_ACTION_RINSING_H_ */

/** @} */
