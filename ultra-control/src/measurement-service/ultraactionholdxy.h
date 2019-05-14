/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionholdxy.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_HOLDXY_H_
#define _ULTRA_ACTION_HOLDXY_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_HOLDXY             (ultra_action_holdxy_get_type ())
#define ULTRA_ACTION_HOLDXY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_HOLDXY, UltraActionHoldXY))
#define ULTRA_ACTION_HOLDXY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_HOLDXY, UltraActionHoldXYClass))
#define ULTRA_IS_ACTION_HOLDXY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_HOLDXY))
#define ULTRA_IS_ACTION_HOLDXY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_HOLDXY))
#define ULTRA_ACTION_HOLDXY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_HOLDXY, UltraActionHoldXYClass))

typedef struct _UltraActionHoldXYClass     UltraActionHoldXYClass;
typedef struct _UltraActionHoldXY          UltraActionHoldXY;
typedef struct _UltraActionHoldXYPrivate   UltraActionHoldXYPrivate;

GType ultra_action_holdxy_get_type (void) G_GNUC_CONST;


struct _UltraActionHoldXYClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionHoldXY
{
	UltraAction                         parent_instance;
	UltraActionHoldXYPrivate            *priv;
};

UltraAction *UltraActionHoldXYNew();



G_END_DECLS

#endif /* _ULTRA_ACTION_HOLDXY_H_ */

/** @} */
