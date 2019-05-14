/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraactionholdxy.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#include "ultraactionholdxy.h"
#include "ultraerrors.h"
#include "ultraconfig.h"


#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _UltraActionHoldXYPrivate
{
	gchar *vessel;
	gint volume;
	// StreamsObject       *stream;
};

enum
{
	PROP0,
	PROP_VESSEL,
	PROP_VOLUME,
};

#define ULTRA_ACTION_HOLDXY_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_ACTION_HOLDXY, UltraActionHoldXYPrivate))

G_DEFINE_TYPE(UltraActionHoldXY, ultra_action_holdxy, ULTRA_TYPE_ACTION)

static void
ultra_action_holdxy_init(UltraActionHoldXY *holdxy)
{
	holdxy->priv = ULTRA_ACTION_HOLDXY_PRIVATE(holdxy);
	holdxy->priv->volume = 100;
	holdxy->priv->vessel = NULL;
}

static void
ultra_action_holdxy_finalize(GObject *object)
{
	// g_debug("Ultra action finalize 2\n");
	UltraActionHoldXY *holdxy = ULTRA_ACTION_HOLDXY(object);
	if (holdxy->priv->vessel)
		g_free(holdxy->priv->vessel);
	G_OBJECT_CLASS(ultra_action_holdxy_parent_class)->finalize(object);
}

static gboolean ultraltraActionHoldXYRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData);

static void
ultra_action_holdxy_class_init(UltraActionHoldXYClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(UltraActionHoldXYClass));
	UltraActionClass *aclass = ULTRA_ACTION_CLASS(klass);
	object_class->finalize = ultra_action_holdxy_finalize;
	aclass->run = ultraltraActionHoldXYRun;
}

UltraAction *UltraActionHoldXYXYNew()
{
	UltraAction *action = ULTRA_ACTION(g_object_new(ULTRA_TYPE_ACTION_HOLDXY, NULL));
	return action;
}

static void holdxyXYFinishCallback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GTask *subTask = G_TASK(user_data);
	if (g_task_return_error_if_cancelled(subTask))
	{
		g_object_unref(subTask);
		return;
	}
	gboolean result = FALSE;
	GError *error = NULL;
	if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error))
	{
		if (error)
			g_dbus_error_strip_remote_error(error);
		mkt_task_object_set_status(MKT_TASK_OBJECT(subTask), _("ptp - operation failed - %s"), error != NULL ? error->message : "unknown");
		g_task_return_error(subTask, error);
		g_object_unref(subTask); // TODO:Add movement error
		return;
	}
	g_task_return_boolean(subTask, TRUE);
	g_object_unref(subTask);
}

static void isFurnaseClosed_GoYHoldXY_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GTask *subTask = G_TASK(user_data);
	if (g_task_return_error_if_cancelled(subTask))
	{
		g_object_unref(subTask);
		return;
	}
	gboolean result = FALSE;
	GError *error = NULL;
	if (!vessels_furnace_call_close_finish(vessels_object_get_furnace(ULTRA_FURNACE()), &result, res, &error))
	{
		if (error)
			g_dbus_error_strip_remote_error(error);
		mkt_task_object_set_status(MKT_TASK_OBJECT(subTask), "furnace close failed - %s", error != NULL ? error->message : "unknown");
		g_task_return_error(subTask, error);
		g_object_unref(subTask);
	}
	else
	{
		GString *commands = g_string_new("");
		g_string_append_printf(commands, "MoveY(1,1,1);SensorY();HoldXYY(1,1);");
		g_string_append_printf(commands, "MoveX(1,1,1);SensorX();HoldXYX(1,1);");
		g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 30000);
		tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, g_task_get_cancellable(subTask), holdxyXYFinishCallback, subTask);
		g_string_free(commands, TRUE);
	}
}
// Extra rinsing process

static void closeFurnaseAfterRinsing(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GTask *subTask = G_TASK(user_data);
	if (g_task_return_error_if_cancelled(subTask))
	{
		g_object_unref(subTask);
		return;
	}
	gboolean result = FALSE;
	GError *error = NULL;
	if (!tera_xysystem_call_ptprun_finish(TERA_XYSYSTEM(source_object), &result, res, &error))
	{
		if (error)
			g_dbus_error_strip_remote_error(error);
		g_task_return_error(subTask, error);
		g_object_unref(subTask);
		return;
	}
	VesselsFurnace *furnace = vessels_object_get_furnace(ULTRA_FURNACE());
	vessels_furnace_call_close(furnace, g_task_get_cancellable(subTask), isFurnaseClosed_GoYHoldXY_callback, subTask);
}

gboolean ultraltraActionHoldXYRun(UltraAction *action, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer userData)
{
	g_return_val_if_fail(action != NULL, FALSE);
	g_return_val_if_fail(ULTRA_IS_ACTION_HOLDXY(action), FALSE);
	UltraActionHoldXY *holdxy = ULTRA_ACTION_HOLDXY(action);
	GTask *task = g_task_new(holdxy, cancellable, callback, userData);
	if (ConfigureIsTestMode())
	{
		g_task_return_boolean(task, TRUE);
		g_object_unref(task);
		return TRUE;
	}
	VesselsFurnace *furnace = vessels_object_get_furnace(ULTRA_FURNACE());
	guint needle_pos = vessels_furnace_get_needle_pos(furnace);
	GString *commands = g_string_new("");
	g_string_append_printf(commands, "MoveY(%d,1,0);", needle_pos);
	g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(XY_SYSTEM()), 50000);
	tera_xysystem_call_ptprun(XY_SYSTEM(), commands->str, cancellable, closeFurnaseAfterRinsing, task);
	g_string_free(commands, TRUE);
	g_object_unref(furnace);
	return TRUE;
}