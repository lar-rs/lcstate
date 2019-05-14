/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_INTEGRATION_H_
#define _ULTRA_ACTION_INTEGRATION_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "lartestsensor.h"
#include "ultrabuschannel.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_INTEGRATION (ultra_action_integration_get_type())
#define ULTRA_ACTION_INTEGRATION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_INTEGRATION, UltraActionIntegration))
#define ULTRA_ACTION_INTEGRATION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_INTEGRATION, UltraActionIntegrationClass))
#define ULTRA_IS_ACTION_INTEGRATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_INTEGRATION))
#define ULTRA_IS_ACTION_INTEGRATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_INTEGRATION))
#define ULTRA_ACTION_INTEGRATION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_INTEGRATION, UltraActionIntegrationClass))

typedef struct _UltraActionIntegrationClass UltraActionIntegrationClass;
typedef struct _UltraActionIntegration UltraActionIntegration;
typedef struct _UltraActionIntegrationPrivate UltraActionIntegrationPrivate;

GType ultra_action_integration_get_type(void) G_GNUC_CONST;

struct _UltraActionIntegrationClass
{
	UltraActionClass parent_class;
};

struct _UltraActionIntegration
{
	UltraAction parent_instance;
	UltraActionIntegrationPrivate *priv;
};

UltraAction *UltraActionIntegrationNew(UltraBusChannel *channel, gboolean tic);

void m_UltraActionIntegrationRunJustification(UltraActionIntegration *integration);
void m_UltraActionIntegrationStopJustification(UltraActionIntegration *integration);
void m_UltraActionIntegrationRunIntegration(UltraActionIntegration *integration);
LarIntgrec* m_UltraActionIntegrationGetIntgrec(UltraActionIntegration *integration);
UltraBusChannel * m_UltraActionIntegrationGetChannel(UltraActionIntegration *integration);
G_END_DECLS

#endif /* _ULTRA_ACTION_INTEGRATION_H_ */

/** @} */
