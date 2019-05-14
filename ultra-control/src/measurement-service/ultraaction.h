/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_H_
#define _ULTRA_ACTION_H_

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION (ultra_action_get_type())
#define ULTRA_ACTION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION, UltraAction))
#define ULTRA_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION, UltraActionClass))
#define ULTRA_IS_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION))
#define ULTRA_IS_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION))
#define ULTRA_ACTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION, UltraActionClass))

typedef struct _UltraActionClass UltraActionClass;
typedef struct _UltraAction UltraAction;
typedef struct _UltraActionPrivate UltraActionPrivate;

GType ultra_action_get_type(void) G_GNUC_CONST;

GQuark UltraActionErrorQuark(void);

struct _UltraActionClass
{
	GObjectClass parent_class;
	gboolean (*run)(UltraAction *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
	gboolean (*finish)(UltraAction *self);
	guint    (*get_id)(UltraAction *self);
};

struct _UltraAction
{
	GObject parent_instance;
	UltraActionPrivate *priv;
};

gboolean m_UltraActionRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);
gboolean m_UltraActionFinish(UltraAction *action, GAsyncResult *result);
gboolean m_UltraActionRunned(UltraAction *action);
gboolean m_UltraActionDone(UltraAction *action);
gboolean m_UltraActionComplited(UltraAction *action);
GError *m_UltraActionError(UltraAction *action);
guint m_UltraActionGetId(UltraAction *action);
// void m_UltraActionSetID(UltraAction *action, guint id );
void m_UltraActionSetName(UltraAction *action, const gchar *name );
const gchar *m_UltraActionGetName(UltraAction *action);
const gchar *m_UltraActionGetStatus(UltraAction *action);
gint UltraActionCompareFunc(gconstpointer a, gconstpointer b);





//Testing optionen 
void actions_set_testing_mod();
gboolean actions_get_testing_mod();

G_END_DECLS

#endif /* _ULTRA_ACTION_H_ */

/** @} */
