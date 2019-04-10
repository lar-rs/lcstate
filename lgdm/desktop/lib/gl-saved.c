/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup GlItem
 * @{
 * @file  mkt-saved.c	Item model interface
 * @brief This is Item model interface description.
 *
 *
 *  Copyright (C) A.Smolkov 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 */


#include "gl-saved.h"
#include "mkt-value.h"
#include "market-time.h"

#if GLIB_CHECK_VERSION(2,31,7)
static GRecMutex init_rmutex;
#define MUTEX_LOCK() g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)
#else
static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
#define MUTEX_LOCK() g_static_rec_mutex_lock(&init_mutex)
#define MUTEX_UNLOCK() g_static_rec_mutex_unlock(&init_mutex)
#endif

static void
gl_saved_base_init (gpointer g_iface)
{
	static gboolean is_saved_initialized = FALSE;
	MUTEX_LOCK();
	if (!is_saved_initialized)
	{

		g_object_interface_install_property (g_iface,
				g_param_spec_string  ("saved-tree-path",
						"Item object data type property",
						"Set get saved object data type property",
						"void",
						G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_MODEL_DB_PROP));

		g_object_interface_install_property (g_iface,
				g_param_spec_string  ("saved-window",
						"Item object data type property",
						"Set get saved object data type property",
						"void",
						G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_MODEL_DB_PROP));

		g_object_interface_install_property (g_iface,
				g_param_spec_string  ("saved-widget",
						"Item object data type property",
						"Set get saved object data type property",
						"void",
						G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_MODEL_DB_PROP));

		is_saved_initialized = TRUE;
	}
	MUTEX_UNLOCK();
}

GType
gl_saved_get_type (void)
{
	static GType iface_type = 0;
	if (iface_type == 0)
	{
		static const GTypeInfo info = {
				sizeof (GlSavedInterface),
				(GBaseInitFunc) gl_saved_base_init,
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
			iface_type = g_type_register_static (G_TYPE_INTERFACE, "GlItemInterface",&info, 0);
		}
		MUTEX_UNLOCK();
	}
	return iface_type;
}


const gchar*
gl_saved_get_path                          ( GlSaved *saved )
{
	g_return_val_if_fail(saved != NULL ,  NULL);
	g_return_val_if_fail(GL_IS_SAVED(saved) ,  NULL);
	if(GL_SAVED_GET_INTERFACE(saved)->saved_path )
		return GL_SAVED_GET_INTERFACE(saved)->saved_path(saved);
	return NULL;
}

const gchar*
gl_saved_get_window                        ( GlSaved *saved )
{
	g_return_val_if_fail(saved != NULL ,  NULL);
	g_return_val_if_fail(GL_IS_SAVED(saved) ,  NULL);
	if(GL_SAVED_GET_INTERFACE(saved)->saved_window )
		return GL_SAVED_GET_INTERFACE(saved)->saved_window(saved);
	return NULL;
}

const gchar*
gl_saved_get_widget                        ( GlSaved *saved )
{
	g_return_val_if_fail(saved != NULL ,  NULL);
	g_return_val_if_fail(GL_IS_SAVED(saved) ,  NULL);
	if(GL_SAVED_GET_INTERFACE(saved)->saved_widget )
		return GL_SAVED_GET_INTERFACE(saved)->saved_widget(saved);
	return NULL;
}
