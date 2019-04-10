/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup GlApp
 * @{
 * @file  gl-app.c	Pc model interface
 * @brief This is APP model interface description.
 *
 *
 *  Copyright (C) A.Smolkov 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 */

#include "gl-app.h"
//#include "gl-app-object.h"
#include <math.h>
#include "gl-app-object.h"
#include <mkt-model.h>



#if GLIB_CHECK_VERSION(2,31,7)
static GRecMutex init_rmutex;
#define MUTEX_LOCK() g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)
#else
static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
#define MUTEX_LOCK() g_static_rec_mutex_lock(&init_mutex)
#define MUTEX_UNLOCK() g_static_rec_mutex_unlock(&init_mutex)
#endif

/* signals */
enum {
	OPENED,
	CLOSED,
	LAST_SIGNAL
};

static guint gl_app_iface_signals[LAST_SIGNAL];

static gboolean
gl_app_open              ( GlApp *app, gboolean *value, GError** error )
{
	g_return_val_if_fail(app!=NULL,FALSE);
	if(GL_APP_GET_INTERFACE(app)->open )
	{
		*value = GL_APP_GET_INTERFACE(app)->open(app);
		return TRUE;
	}
	return FALSE;
}

static gboolean
gl_app_close           ( GlApp *app, gboolean *valueOut, GError** error )
{
	g_return_val_if_fail(app!=NULL,FALSE);
	if(GL_APP_GET_INTERFACE(app)->close )
	{
		*valueOut = GL_APP_GET_INTERFACE(app)->close(app);
		return TRUE;
	}
	return FALSE;
}

static gboolean
gl_app_release           ( GlApp *app, gboolean *valueOut, GError** error )
{
	g_return_val_if_fail(app!=NULL,FALSE);
	if(GL_APP_GET_INTERFACE(app)->release )
	{
		*valueOut = GL_APP_GET_INTERFACE(app)->release(app);
		return TRUE;
	}
	return FALSE;
}

static gboolean
gl_app_hold            ( GlApp *app, gboolean *valueOut, GError** error )
{
	g_return_val_if_fail(app!=NULL,FALSE);
	if(GL_APP_GET_INTERFACE(app)->hold )
	{
		*valueOut = GL_APP_GET_INTERFACE(app)->hold(app);
		return TRUE;
	}
	return FALSE;
}

#include "dbus-interface/gl_app-server-interface.h"




static void
gl_app_base_init (gpointer g_iface)
{
	static gboolean is_initialized = FALSE;
	MUTEX_LOCK();
	GType iface_type = G_TYPE_FROM_INTERFACE (g_iface);

	if (!is_initialized)
	{
		//GParamSpec *pspec;

		/*pspec = g_app_spec_string ("app-timestamp",
				"App timestamp prop",
				"Set|Get Subscriprion's app name",
				"NOW()",
				G_APP_READWRITE |  GL_MODEL_DB_PROP | GL_MODEL_DB_TIMESTAMP  );
				g_object_interface_install_property (g_iface, pspec); */



		gl_app_iface_signals[OPENED] =
				g_signal_new ("opened",
						iface_type,
						G_SIGNAL_RUN_LAST,
						0,//G_STRUCT_OFFSET (TestHelloIface, greetings),
						NULL, NULL,
						g_cclosure_marshal_VOID__BOOLEAN,
						G_TYPE_NONE,
						1,
						G_TYPE_BOOLEAN);
		gl_app_iface_signals[CLOSED] =
				g_signal_new ("closed",
						iface_type,
						G_SIGNAL_RUN_LAST,
						0,//G_STRUCT_OFFSET (TestHelloIface, greetings),
						NULL, NULL,
						g_cclosure_marshal_VOID__BOOLEAN,
						G_TYPE_NONE,
						1,
						G_TYPE_BOOLEAN);

		dbus_g_object_type_install_info (iface_type, &dbus_glib_gl_app_object_info);
		is_initialized = TRUE;
	}

	MUTEX_UNLOCK();
}

GType
gl_app_get_type (void)
{
	static GType iface_type = 0;
	if (iface_type == 0)
	{
		static const GTypeInfo info = {
				sizeof (GlAppInterface),
				(GBaseInitFunc) gl_app_base_init,
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
			iface_type = g_type_register_static (G_TYPE_INTERFACE, "GlAppInterface",&info, 0);
		}
		MUTEX_UNLOCK();
	}
	return iface_type;
}


gboolean
gl_app_start                                             ( MktDbus *app )
{
	g_return_val_if_fail(app!=NULL,0);
	gboolean result = FALSE;
	if(GL_IS_APP(app))
	{
		if(!gl_app_open(GL_APP(app),&result,NULL))
		{
			g_warning ( "GApp open method call error");
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		GError *error = NULL;
		gboolean accept = TRUE;
		DBusGProxy* proxy = dbus_g_proxy_new_for_name(mkt_dbus_get_connection(app),mkt_dbus_get_name(app),mkt_dbus_get_path(app),"com.lar.GAppInterface");
		if(!com_lar_GAppInterface_open(proxy,&result,&error))
		{
			accept = FALSE;
			g_debug("GApp interface open error = %s",error->message);
			g_error_free(error);
		}
		g_object_unref(proxy);
		dbus_g_connection_flush(mkt_dbus_get_connection(app));
		return accept;
	}
	return FALSE;
}

gboolean
gl_app_stop                                              ( MktDbus *app )
{
	g_return_val_if_fail(app!=NULL,0);
	gboolean result = FALSE;
	if(GL_IS_APP(app))
	{
		if(!gl_app_close(GL_APP(app),&result,NULL))
		{
			g_warning ("GApp close method call error");
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		GError *error = NULL;
		gboolean accept = TRUE;
		DBusGProxy* proxy = dbus_g_proxy_new_for_name(mkt_dbus_get_connection(app),mkt_dbus_get_name(app),mkt_dbus_get_path(app),"com.lar.GAppInterface");
		if(!com_lar_GAppInterface_close(proxy,&result,&error))
		{
			accept = FALSE;
			g_debug("GApp interface close error = %s",error->message);
			g_error_free(error);
		}
		g_object_unref(proxy);
		dbus_g_connection_flush(mkt_dbus_get_connection(app));
		return accept;
	}
	return FALSE;
}

gboolean
gl_app_activate                                              ( MktDbus *app )
{
	g_return_val_if_fail(app!=NULL,0);
	gboolean result = FALSE;
	if(GL_IS_APP(app))
	{
		if(!gl_app_release(GL_APP(app),&result,NULL))
		{
			g_warning ("GApp hold method call error");
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		GError *error = NULL;
		gboolean accept = TRUE;
		DBusGProxy* proxy = dbus_g_proxy_new_for_name(mkt_dbus_get_connection(app),mkt_dbus_get_name(app),mkt_dbus_get_path(app),"com.lar.GAppInterface");
		if(!com_lar_GAppInterface_release(proxy,&result,&error))
		{
			accept = FALSE;
			g_debug("GApp interface exit error = %s",error->message);
			g_error_free(error);
		}
		g_object_unref(proxy);
		dbus_g_connection_flush(mkt_dbus_get_connection(app));
		return accept;
	}
	return FALSE;
}

gboolean
gl_app_deactivate                                              ( MktDbus *app )
{
	g_return_val_if_fail(app!=NULL,0);
	gboolean result = FALSE;
	if(GL_IS_APP(app))
	{
		if(!gl_app_hold(GL_APP(app),&result,NULL))
		{
			g_warning ("GApp hold method call error");
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		GError *error = NULL;
		gboolean accept = TRUE;
		DBusGProxy* proxy = dbus_g_proxy_new_for_name(mkt_dbus_get_connection(app),mkt_dbus_get_name(app),mkt_dbus_get_path(app),"com.lar.GAppInterface");
		if(!com_lar_GAppInterface_hold(proxy,&result,&error))
		{
			accept = FALSE;
			g_debug("GApp interface hold error = %s",error->message);
			g_error_free(error);
		}
		g_object_unref(proxy);
		dbus_g_connection_flush(mkt_dbus_get_connection(app));
		return accept;
	}
	return FALSE;
}


/** @} */
