/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionhold.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_HOLD_H_
#define _ULTRA_ACTION_HOLD_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_HOLD             (ultra_action_hold_get_type ())
#define ULTRA_ACTION_HOLD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_HOLD, UltraActionHold))
#define ULTRA_ACTION_HOLD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_HOLD, UltraActionHoldClass))
#define ULTRA_IS_ACTION_HOLD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_HOLD))
#define ULTRA_IS_ACTION_HOLD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_HOLD))
#define ULTRA_ACTION_HOLD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_HOLD, UltraActionHoldClass))

typedef struct _UltraActionHoldClass     UltraActionHoldClass;
typedef struct _UltraActionHold          UltraActionHold;
typedef struct _UltraActionHoldPrivate   UltraActionHoldPrivate;

GType ultra_action_hold_get_type (void) G_GNUC_CONST;


struct _UltraActionHoldClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionHold
{
	UltraAction                         parent_instance;
	UltraActionHoldPrivate            *priv;
};

UltraAction *UltraActionHoldNew();



G_END_DECLS

#endif /* _ULTRA_ACTION_HOLD_H_ */

/** @} */
