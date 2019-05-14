/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionsampling.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_SAMPLING_H_
#define _ULTRA_ACTION_SAMPLING_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_SAMPLING             (ultra_action_sampling_get_type ())
#define ULTRA_ACTION_SAMPLING(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_SAMPLING, UltraActionSampling))
#define ULTRA_ACTION_SAMPLING_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_SAMPLING, UltraActionSamplingClass))
#define ULTRA_IS_ACTION_SAMPLING(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_SAMPLING))
#define ULTRA_IS_ACTION_SAMPLING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_SAMPLING))
#define ULTRA_ACTION_SAMPLING_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_SAMPLING, UltraActionSamplingClass))

typedef struct _UltraActionSamplingClass     UltraActionSamplingClass;
typedef struct _UltraActionSampling          UltraActionSampling;
typedef struct _UltraActionSamplingPrivate   UltraActionSamplingPrivate;

GType ultra_action_sampling_get_type (void) G_GNUC_CONST;


struct _UltraActionSamplingClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionSampling
{
	UltraAction                         parent_instance;
	UltraActionSamplingPrivate            *priv;
};

UltraAction *UltraActionSamplingNew(const gchar *vessel, guint volume,gboolean isCodo);



G_END_DECLS

#endif /* _ULTRA_ACTION_SAMPLING_H_ */

/** @} */
