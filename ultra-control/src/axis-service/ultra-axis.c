/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup UltraAxisObject
 * @{
 * @file  ultra-axis-object.c
 * @brief This is AXIS model object description.
 *
 *  Copyright (C) LAR  2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "ultra-axis.h"
#include <gio/gio.h>



#include "../../config.h"
#include <glib/gi18n-lib.h>

#if GLIB_CHECK_VERSION(2,31,7)
static GRecMutex       init_rmutex;
#define MUTEX_LOCK()   g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)
#else
static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
#define MUTEX_LOCK()   g_static_rec_mutex_lock(&init_mutex)
#define MUTEX_UNLOCK() g_static_rec_mutex_unlock(&init_mutex)
#endif


static GCancellable* Z_CANCELLABLE = NULL;

static GCancellable* MOVE_CANCELLABLE = NULL;
static guint WAIT_TAG = 0;

void
ultra_axis_remove_tag                      ( void )
{
	if(WAIT_TAG)g_source_remove(WAIT_TAG);
	WAIT_TAG = 0;
}

void
ultra_axis_wait_tag (guint tag )
{
	WAIT_TAG = tag;
}

GCancellable*
ultra_axis_cancellable                     ( void )
{
	if(MOVE_CANCELLABLE == NULL)MOVE_CANCELLABLE = g_cancellable_new();
	return MOVE_CANCELLABLE;
}

GCancellable*
ultra_axisZ_cancellable                    ( void )
{
	if(Z_CANCELLABLE == NULL)Z_CANCELLABLE = g_cancellable_new();
	return Z_CANCELLABLE;

}

gboolean
ultra_axis_is_canceled ( void )
{

	return g_cancellable_is_cancelled(MOVE_CANCELLABLE);
}


gboolean
ultra_axisZ_is_canceled ( void )
{

	return g_cancellable_is_cancelled(Z_CANCELLABLE);
}

void
ultra_axis_cancellable_init                ( void )
{
	if(MOVE_CANCELLABLE == NULL)MOVE_CANCELLABLE = g_cancellable_new();
	ultra_axis_remove_tag();
	if(Z_CANCELLABLE)g_cancellable_cancel(Z_CANCELLABLE);
	g_cancellable_cancel(MOVE_CANCELLABLE);
	g_cancellable_reset(MOVE_CANCELLABLE);
}

void
ultra_axisZ_cancellable_init                ( void )
{
	if(MOVE_CANCELLABLE == NULL)Z_CANCELLABLE = g_cancellable_new();
	ultra_axis_remove_tag();
	if(Z_CANCELLABLE)g_cancellable_cancel(Z_CANCELLABLE);
	if(MOVE_CANCELLABLE)g_cancellable_cancel(MOVE_CANCELLABLE);
	g_cancellable_reset(Z_CANCELLABLE);
}


void
ultra_axis_operation_cancel( void )
{
	if(MOVE_CANCELLABLE == NULL)MOVE_CANCELLABLE = g_cancellable_new();
	ultra_axis_remove_tag();
	g_cancellable_cancel(MOVE_CANCELLABLE);
	if(Z_CANCELLABLE)g_cancellable_cancel(Z_CANCELLABLE);
}


static void
ultra_axis_base_init (gpointer g_iface)
{
	static gboolean is_axis_initialized = FALSE;
	MUTEX_LOCK();
	if (!is_axis_initialized)
	{


		is_axis_initialized = TRUE;
	}
	MUTEX_UNLOCK();
}

GType
ultra_axis_get_type (void)
{
	static GType iface_type = 0;
	if (iface_type == 0)
	{
		static const GTypeInfo info = {
				sizeof (UltraAxisInterface),
				(GBaseInitFunc) ultra_axis_base_init,
				(GBaseFinalizeFunc) NULL,
				(GClassInitFunc) NULL,
				NULL,
				NULL,
				0,
				0,
				(GInstanceInitFunc) NULL,
				0
		};
		MUTEX_LOCK();
		if (iface_type == 0)
		{
			iface_type = g_type_register_static (G_TYPE_INTERFACE, "UltraAxisInterface",&info, 0);
			//g_type_interface_add_prerequisite (iface_type, MKT_TYPE_MODEL);
		}
		MUTEX_UNLOCK();
	}
	return iface_type;
}



gboolean
ultra_axis_set_busy                  ( UltraAxis *axis , gboolean value )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
	{
		achsen_achse_set_is_busy(achse,value);
		return TRUE;
	}
	return FALSE;
}

gboolean
ultra_axis_is_busy                  ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,TRUE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),TRUE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_is_busy(achse);
	return TRUE;
}

gboolean
ultra_axis_is_init                        ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,TRUE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),TRUE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_is_init(achse);
	return TRUE;
}

gboolean
ultra_axis_set_init                        ( UltraAxis *axis , gboolean value )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
	{
		achsen_achse_set_is_init(achse,value);
		return TRUE;
	}
	return FALSE;
}

gboolean
ultra_axis_set_initialized                 ( UltraAxis *axis , gboolean value )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
	{
		achsen_achse_set_initialized(achse,value);
		return TRUE;
	}
	return FALSE;
}

gboolean
ultra_axis_is_initialized                  ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,TRUE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),TRUE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_initialized(achse);
	return TRUE;
}

gboolean
ultra_axis_set_position                    ( UltraAxis *axis , guint position )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
	{
		achsen_achse_set_position(achse,position);
		return TRUE;
	}
	return FALSE;
}

guint
ultra_axis_get_position                    ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,0);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),0);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_position(achse);
	return 0;
}

gboolean
ultra_axis_set_parameter                   ( UltraAxis *axis , guint  value )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
	{
		achsen_achse_set_parameter(achse,value);
		return TRUE;
	}
	return FALSE;
}

guint
ultra_axis_get_parameter                   ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,0);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),0);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_parameter(achse);
	return 0;
}

guint
ultra_axis_get_hold                        ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,0);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),0);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_hold(achse);
	return 0;
}

gboolean
ultra_axis_set_go_position                    ( UltraAxis *axis , guint position )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
	{
		achsen_achse_set_go_to_pos(achse,position);
		return TRUE;
	}
	return FALSE;
}

guint
ultra_axis_get_go_position                 ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,0);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),0);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		return achsen_achse_get_go_to_pos(achse);
	return 0;
}

void
ultra_axis_change_status                   ( UltraAxis *axis  ,const gchar *format,...)
{
	g_return_if_fail(axis!=NULL);
	g_return_if_fail(ACHSEN_IS_OBJECT(axis));
	va_list args;
	gchar *new_status;
	va_start (args, format);
	new_status =g_strdup_vprintf (format, args);
	va_end (args);
	AchsenAchse *achse = achsen_object_get_achse(ACHSEN_OBJECT(axis));
	if(achse)
		achsen_achse_set_status(achse,new_status);

	g_free(new_status);

}
gboolean
ultra_axis_init_parameter                  ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	if(ULTRA_AXIS_GET_INTERFACE(axis)->init_parameter)
		return ULTRA_AXIS_GET_INTERFACE(axis)->init_parameter(axis);
	return FALSE;
}


gboolean
ultra_axis_go_hold                         ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	if(ULTRA_AXIS_GET_INTERFACE(axis)->go_hold)
		return ULTRA_AXIS_GET_INTERFACE(axis)->go_hold(axis);
	return FALSE;
}

gboolean
ultra_axis_go_sensor                       ( UltraAxis *axis )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	if(ULTRA_AXIS_GET_INTERFACE(axis)->go_sensor)
		return ULTRA_AXIS_GET_INTERFACE(axis)->go_sensor(axis);
	return FALSE;
}

gboolean
ultra_axis_go_position                     ( UltraAxis *axis , guint position )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	if(ULTRA_AXIS_GET_INTERFACE(axis)->go_hold)
		return ULTRA_AXIS_GET_INTERFACE(axis)->go_position(axis,position);
	return FALSE;
}

gboolean
ultra_axis_is_position                     ( UltraAxis *axis , guint position )
{
	g_return_val_if_fail(axis!=NULL,FALSE);
	g_return_val_if_fail(ACHSEN_IS_OBJECT(axis),FALSE);
	if(ULTRA_AXIS_GET_INTERFACE(axis)->is_position)
		return ULTRA_AXIS_GET_INTERFACE(axis)->is_position(axis,position);
	return FALSE;
}


/** @} */
