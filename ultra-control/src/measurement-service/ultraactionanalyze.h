/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_ANALYZE_H_
#define _ULTRA_ACTION_ANALYZE_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultrabusstream.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_ANALYZE             (ultra_action_analyze_get_type ())
#define ULTRA_ACTION_ANALYZE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_ANALYZE, UltraActionAnalyze))
#define ULTRA_ACTION_ANALYZE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_ANALYZE, UltraActionAnalyzeClass))
#define ULTRA_IS_ACTION_ANALYZE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_ANALYZE))
#define ULTRA_IS_ACTION_ANALYZE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_ANALYZE))
#define ULTRA_ACTION_ANALYZE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_ANALYZE, UltraActionAnalyzeClass))

typedef struct _UltraActionAnalyzeClass     UltraActionAnalyzeClass;
typedef struct _UltraActionAnalyze          UltraActionAnalyze;
typedef struct _UltraActionAnalyzePrivate   UltraActionAnalyzePrivate;

GType ultra_action_analyze_get_type (void) G_GNUC_CONST;


struct _UltraActionAnalyzeClass
{
	UltraActionClass                      parent_class;
};

struct _UltraActionAnalyze
{
	UltraAction                              parent_instance;
	UltraActionAnalyzePrivate            *priv;
};

UltraAction *UltraActionAnalyzeNew(UltraBusStream *stream, GList *channels);





G_END_DECLS

#endif /* _ULTRA_ACTION_ANALYZE_H_ */

