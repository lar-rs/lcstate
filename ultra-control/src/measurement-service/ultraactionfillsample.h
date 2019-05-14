/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_FILL_SAMPLE_H_
#define _ULTRA_ACTION_FILL_SAMPLE_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_FILL_SAMPLE             (ultra_action_fill_sample_get_type ())
#define ULTRA_ACTION_FILL_SAMPLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_FILL_SAMPLE, UltraActionFillSample))
#define ULTRA_ACTION_FILL_SAMPLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_FILL_SAMPLE, UltraActionFillSampleClass))
#define ULTRA_IS_ACTION_FILL_SAMPLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_FILL_SAMPLE))
#define ULTRA_IS_ACTION_FILL_SAMPLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_FILL_SAMPLE))
#define ULTRA_ACTION_FILL_SAMPLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_FILL_SAMPLE, UltraActionFillSampleClass))

typedef struct _UltraActionFillSampleClass     UltraActionFillSampleClass;
typedef struct _UltraActionFillSample          UltraActionFillSample;
typedef struct _UltraActionFillSamplePrivate   UltraActionFillSamplePrivate;

GType ultra_action_fill_sample_get_type (void) G_GNUC_CONST;


struct _UltraActionFillSampleClass
{
	UltraActionClass                      parent_class;
};

struct _UltraActionFillSample
{
	UltraAction                              parent_instance;
	UltraActionFillSamplePrivate            *priv;
};

UltraAction *UltraActionFillSampleNew(const gchar *pump, gint64 interval);




G_END_DECLS

#endif /* _ULTRA_ACTION_FILL_SAMPLE_H_ */

/** @} */
