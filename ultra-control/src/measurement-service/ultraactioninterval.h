/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_INTERVAL_H_
#define _ULTRA_ACTION_INTERVAL_H_

#include <glib-object.h>
#include <mktlib.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultrabusstream.h"


G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_INTERVAL             (ultra_action_interval_get_type ())
#define ULTRA_ACTION_INTERVAL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_INTERVAL, UltraActionInterval))
#define ULTRA_ACTION_INTERVAL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_INTERVAL, UltraActionIntervalClass))
#define ULTRA_IS_ACTION_INTERVAL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_INTERVAL))
#define ULTRA_IS_ACTION_INTERVAL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_INTERVAL))
#define ULTRA_ACTION_INTERVAL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_INTERVAL, UltraActionIntervalClass))

typedef struct _UltraActionIntervalClass     UltraActionIntervalClass;
typedef struct _UltraActionInterval          UltraActionInterval;
typedef struct _UltraActionIntervalPrivate   UltraActionIntervalPrivate;

GType ultra_action_interval_get_type (void) G_GNUC_CONST;


struct _UltraActionIntervalClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionInterval
{
	UltraAction                         parent_instance;
	UltraActionIntervalPrivate            *priv;
};

UltraAction *UltraActionIntervalNew(UltraAction *action,gint64 sec);
UltraAction* m_UltraActionIntervalGetNextRun(UltraActionInterval *interval);




G_END_DECLS

#endif /* _ULTRA_ACTION_INTERVAL_H_ */

/** @} */
