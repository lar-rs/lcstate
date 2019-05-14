/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactioninjection.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_INJECTION_H_
#define _ULTRA_ACTION_INJECTION_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_INJECTION             (ultra_action_injection_get_type ())
#define ULTRA_ACTION_INJECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_INJECTION, UltraActionInjection))
#define ULTRA_ACTION_INJECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_INJECTION, UltraActionInjectionClass))
#define ULTRA_IS_ACTION_INJECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_INJECTION))
#define ULTRA_IS_ACTION_INJECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_INJECTION))
#define ULTRA_ACTION_INJECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_INJECTION, UltraActionInjectionClass))

typedef struct _UltraActionInjectionClass     UltraActionInjectionClass;
typedef struct _UltraActionInjection          UltraActionInjection;
typedef struct _UltraActionInjectionPrivate   UltraActionInjectionPrivate;

GType ultra_action_injection_get_type (void) G_GNUC_CONST;


struct _UltraActionInjectionClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionInjection
{
	UltraAction                         parent_instance;
	UltraActionInjectionPrivate            *priv;
};

UltraAction *UltraActionInjectionNew(gboolean codo,gboolean tic);


G_END_DECLS

#endif /* _ULTRA_ACTION_INJECTION_H_ */

/** @} */
