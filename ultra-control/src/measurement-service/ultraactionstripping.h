/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_STRIPPING_H_
#define _ULTRA_ACTION_STRIPPING_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_STRIPPING             (ultra_action_stripping_get_type ())
#define ULTRA_ACTION_STRIPPING(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_STRIPPING, UltraActionStripping))
#define ULTRA_ACTION_STRIPPING_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_STRIPPING, UltraActionStrippingClass))
#define ULTRA_IS_ACTION_STRIPPING(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_STRIPPING))
#define ULTRA_IS_ACTION_STRIPPING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_STRIPPING))
#define ULTRA_ACTION_STRIPPING_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_STRIPPING, UltraActionStrippingClass))

typedef struct _UltraActionStrippingClass     UltraActionStrippingClass;
typedef struct _UltraActionStripping          UltraActionStripping;
typedef struct _UltraActionStrippingPrivate   UltraActionStrippingPrivate;

GType ultra_action_stripping_get_type (void) G_GNUC_CONST;


struct _UltraActionStrippingClass
{
	UltraActionClass                      parent_class;
};

struct _UltraActionStripping
{
	UltraAction                              parent_instance;
	UltraActionStrippingPrivate            *priv;
};

UltraAction *UltraActionStrippingNew(gint64 interval);




G_END_DECLS

#endif /* _ULTRA_ACTION_STRIPPING_H_ */

/** @} */
