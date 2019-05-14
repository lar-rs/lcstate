/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusprocess.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraaction.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionPrivate
{
	gboolean complited;
	gboolean done;
	gboolean runned;
	GError *error;
	gchar *name;
	gchar *status;

};

enum
{
	ULTRA_PROP0,
	PROP_ID,
	PROP_NAME,
	PROP_STATUS,
};

GQuark UltraActionErrorQuark(void)
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string("action-error");
	return error;
}

#define ULTRA_ACTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION, UltraActionPrivate))

G_DEFINE_TYPE(UltraAction, ultra_action, G_TYPE_OBJECT)

static void
ultra_action_init(UltraAction *action)
{
	action->priv = ULTRA_ACTION_PRIVATE(action);
	action->priv->name = NULL;
	action->priv->status = NULL;
	action->priv->error = NULL;
}

static void
ultra_action_finalize(GObject *object)
{
	UltraAction *action = ULTRA_ACTION(object);
	if (action->priv->error)
		g_error_free(action->priv->error);
	if (action->priv->name)
		g_free(action->priv->name);
	if (action->priv->status)
		g_free(action->priv->status);
	G_OBJECT_CLASS(ultra_action_parent_class)->finalize(object);
}

static void
ultra_action_set_property(GObject *object,
						  guint prop_id,
						  const GValue *value,
						  GParamSpec *pspec)
{
	UltraAction *action = ULTRA_ACTION(object);
	switch (prop_id)
	{
	case PROP_NAME:
		if (action->priv->name)
			g_free(action->priv->name);
		action->priv->name = g_value_dup_string(value);
		break;
	case PROP_STATUS:
		if (action->priv->status)
			g_free(action->priv->status);
		action->priv->status = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ultra_action_get_property(GObject *object,
						  guint prop_id,
						  GValue *value,
						  GParamSpec *pspec)
{
	UltraAction *action = ULTRA_ACTION(object);
	switch (prop_id)
	{
	case PROP_NAME:
		g_value_set_string(value, action->priv->name);
		break;
	case PROP_STATUS:
		g_value_set_string(value, action->priv->status);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static guint ultra_action_get_id_real(UltraAction *action){
	return g_str_hash(G_OBJECT_TYPE_NAME(action));
}

static void
ultra_action_class_init(UltraActionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionClass));
	object_class->set_property = ultra_action_set_property;
	object_class->get_property = ultra_action_get_property;

	object_class->finalize = ultra_action_finalize;
	klass->run    = NULL;
	klass->get_id = ultra_action_get_id_real;
	klass->finish = NULL;
	g_object_class_install_property(object_class, PROP_NAME,
									g_param_spec_string("name", "name", "name", NULL, G_PARAM_WRITABLE | G_PARAM_READABLE));
	g_object_class_install_property(object_class, PROP_STATUS,
									g_param_spec_string("status", "status", "status", NULL, G_PARAM_WRITABLE | G_PARAM_READABLE));

	g_signal_new("action-done", G_TYPE_FROM_CLASS(klass),
				 G_SIGNAL_RUN_LAST,
				 0,
				 NULL, NULL,
				 g_cclosure_marshal_VOID__VOID,
				 G_TYPE_NONE,
				 0,
				 G_TYPE_NONE);
}

gboolean m_UltraActionRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	if (ULTRA_ACTION_GET_CLASS(action)->run)
		action->priv->runned = ULTRA_ACTION_GET_CLASS(action)->run(action, cancellable, callback, userData);
	return action->priv->runned;
}

gboolean m_UltraActionFinish(UltraAction *action, GAsyncResult *result)
{
	if(action->priv->complited)return action->priv->done;
	if (action->priv->error)
		g_error_free(action->priv->error);
	action->priv->error = NULL;
	action->priv->done = g_task_propagate_boolean(G_TASK(result), &action->priv->error);
	action->priv->complited = TRUE;
	if (ULTRA_ACTION_GET_CLASS(action)->finish)
		action->priv->done = ULTRA_ACTION_GET_CLASS(action)->finish(action);
	if (action->priv->done){
		g_signal_emit_by_name(action, "action-done");
	}
	return action->priv->done;
}
gboolean m_UltraActionRunned(UltraAction *action)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	return action->priv->runned;
}
gboolean m_UltraActionDone(UltraAction *action)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	return action->priv->done;
}
gboolean m_UltraActionComplited(UltraAction *action)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), FALSE);
	return action->priv->complited;
}
GError *m_UltraActionError(UltraAction *action)
{
	g_return_val_if_fail(action != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), NULL);
	return action->priv->error;
}

void m_UltraActionSetName(UltraAction *action, const gchar *name ){
	g_return_if_fail(ULTRA_IS_ACTION(action));
	g_object_set(action,"name",name,NULL);
}
const gchar *m_UltraActionGetName(UltraAction *action)
{
	g_return_val_if_fail(action != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), NULL);
	return action->priv->name;
}
const gchar *m_UltraActionGetStatus(UltraAction *action)
{
	g_return_val_if_fail(action != NULL, NULL);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), NULL);
	return action->priv->status;
}
guint m_UltraActionGetId(UltraAction *action){
	g_return_val_if_fail(action != NULL,0);
	g_return_val_if_fail(ULTRA_IS_ACTION(action), 0);
	g_return_val_if_fail(ULTRA_ACTION_GET_CLASS(action)->get_id!=NULL,0);
	return	ULTRA_ACTION_GET_CLASS(action)->get_id(action);	
}


gint UltraActionCompareFunc(gconstpointer a, gconstpointer b){
	if(a == b) return 0;
	if(!ULTRA_IS_ACTION(a))return 1;
	if(!ULTRA_IS_ACTION(b))return 1;
	return m_UltraActionGetId(ULTRA_ACTION(a))-m_UltraActionGetId(ULTRA_ACTION(b));
}

static gboolean _testing_mode = FALSE;


void actions_set_testing_mod(){
	_testing_mode = TRUE && TEST_MODE == 1; 
}

gboolean actions_get_testing_mod(){
	return _testing_mode && TEST_MODE;
}