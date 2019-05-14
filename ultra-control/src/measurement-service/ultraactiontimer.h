/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_TIMER_H_
#define _ULTRA_ACTION_TIMER_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_TIMER             (ultra_action_timer_get_type ())
#define ULTRA_ACTION_TIMER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_TIMER, UltraActionTimer))
#define ULTRA_ACTION_TIMER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_TIMER, UltraActionTimerClass))
#define ULTRA_IS_ACTION_TIMER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_TIMER))
#define ULTRA_IS_ACTION_TIMER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_TIMER))
#define ULTRA_ACTION_TIMER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_TIMER, UltraActionTimerClass))

typedef struct _UltraActionTimerClass     UltraActionTimerClass;
typedef struct _UltraActionTimer          UltraActionTimer;
typedef struct _UltraActionTimerPrivate   UltraActionTimerPrivate;

GType ultra_action_timer_get_type (void) G_GNUC_CONST;


struct _UltraActionTimerClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionTimer
{
	UltraAction                         parent_instance;
	UltraActionTimerPrivate            *priv;
};

UltraAction* UltraActionTimerNew(gint64 interval);
UltraAction *UltraActionTimerNewWithName(gint64 interval,const gchar *name );
gint64 m_UltraActionTimerGetInterval(UltraActionTimer *timer);

GDateTime* rt_now_utc();
GDateTime* rt_now_local();

//Simulation 

void rt_run_simulation(GDateTime *dt, guint interval);
guint rt_interval();
gboolean rt_simulation();

G_END_DECLS

#endif /* _ULTRA_ACTION_TIMER_H_ */

