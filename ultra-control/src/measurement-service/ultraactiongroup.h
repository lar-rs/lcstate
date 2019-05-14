/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_GROUP_H_
#define _ULTRA_ACTION_GROUP_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_GROUP             (ultra_action_group_get_type ())
#define ULTRA_ACTION_GROUP(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ACTION_GROUP, UltraActionGroup))
#define ULTRA_ACTION_GROUP_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTRA_TYPE_ACTION_GROUP, UltraActionGroupClass))
#define ULTRA_IS_ACTION_GROUP(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ACTION_GROUP))
#define ULTRA_IS_ACTION_GROUP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTRA_TYPE_ACTION_GROUP))
#define ULTRA_ACTION_GROUP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTRA_TYPE_ACTION_GROUP, UltraActionGroupClass))

typedef struct _UltraActionGroupClass     UltraActionGroupClass;
typedef struct _UltraActionGroup          UltraActionGroup;
typedef struct _UltraActionGroupPrivate   UltraActionGroupPrivate;

GType ultra_action_group_get_type (void) G_GNUC_CONST;


struct _UltraActionGroupClass
{
	UltraActionClass                    parent_class;
};

struct _UltraActionGroup
{
	UltraAction                         parent_instance;
	UltraActionGroupPrivate                       *priv;
};


UltraAction* UltraActionGroupNew();
UltraAction* UltraActionGroupSyncNew();

gboolean m_UltraActionGroupAddAction(UltraActionGroup *group,UltraAction *action);
GList* m_UltraActionGroupGetActions(UltraActionGroup *group);


G_END_DECLS

#endif /* _ULTRA_ACTION_GROUP_H_ */

/** @} */
