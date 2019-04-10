/*
 * gtk-foobar.c
 * Copyright (C) 2014 doseus <doseus@doseus-ThinkPad-T430s>
 *
 */
#include "gl-application-object.h"

#include <gdk/gdkkeysyms.h>
#include "ldm-uiresources.h"
#include <gio/gdesktopappinfo.h>


#include "../config.h"
#include <glib/gi18n-lib.h>

/*Window            application->priv->WinID = 0;

static GOptionEntry entries[] =
	{
	  { "window-id", 'w', 0, G_OPTION_ARG_INT64, &application->priv->WinID, "Set Socket window id for application", "WId" },
	  { NULL }
	};
*/
/* signals */
enum {
	SHOW,
	HIDE,
	OPENED,
	CLOSED,
	CHANGE_THEME,
	LAST_SIGNAL
};


static guint gl_application_signals[LAST_SIGNAL];


/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/ui/gl_application.ui" */

G_DEFINE_TYPE (GlApplication, gl_application, GTK_TYPE_APPLICATION);

/* ANJUTA: Macro GL_APPLICATION gets GlApplication - DO NOT REMOVE */
struct _GlApplicationPrivate
{
	GtkWidget                *plug;
	GtkStyleProvider         *provider;
	GtkIconTheme             *icontheme;
	GlLayoutManager          *layout_manager;
	GDBusObjectManager       *client;
	GDesktopAppInfo          *appinfo;
	gchar                    *object_path;
	LgdmObject               *app_object;
	GuiAppGapp               *gapp;
	GDBusObjectManager       *desktop_manager;
	DesktopObject            *desktop_object;
	GList                    *need_service;
	GList                    *processed;
	GHashTable               *service_table;


	/* ANJUTA: Widgets declaration for gl_application.ui - DO NOT REMOVE */
};


static void
apply_css (GtkWidget *widget, GtkStyleProvider *provider)
{

  gtk_style_context_add_provider (gtk_widget_get_style_context (widget), provider, G_MAXUINT);
  if (GTK_IS_CONTAINER (widget))
    gtk_container_forall (GTK_CONTAINER (widget), (GtkCallback) apply_css, provider);
}


void
gl_application_apply_container_css (GlApplication *application , GtkWidget *container )
{
	if( application->priv->provider == NULL )
	{
		apply_css(container,GTK_STYLE_PROVIDER(gtk_css_provider_get_default()));
	}
	else
	{
		apply_css(container,application->priv->provider);
	}
}

gboolean
gl_application_apply_css           ( GlApplication *application )
{
	GList *wins = gtk_application_get_windows(GTK_APPLICATION(application));
	GList *l = NULL;
	for(l=wins;l!=NULL;l=l->next)
	{
		if(l->data && GTK_IS_WINDOW(l->data))
			gl_application_apply_container_css(application,GTK_WIDGET(l->data));
	}
	return TRUE;
}

GlLayoutManager*
gl_application_get_layout_manager                     ( GlApplication *application )
{
	return application->priv->layout_manager;
}

static void
gl_application_start_callback( LgdmApp *ldmapp , const gchar *address, GlApplication *application )
{
	if(!gl_layout_manager_default_activate_named(address))
	{
		gl_layout_manager_default_activate_root(ldmapp);
	}
}

gboolean
gl_application_started_idle_callback ( gpointer data )
{
	GlApplication *app = GL_APPLICATION(data);
	LgdmObject *object = LGDM_OBJECT(g_dbus_object_manager_get_object(app->priv->client,app->priv->object_path));
	if(object)
	{
		LgdmApp *ldmapp = lgdm_object_get_app(object) ;
		lgdm_app_set_started(ldmapp,TRUE);
	}
	return FALSE;
}

static void
gl_application_activate_callback( LgdmApp *ldmapp , GParamSpec *pspec , GlApplication *application )
{
	if(lgdm_app_get_activated(ldmapp))
	{
		if(application->priv->plug)
			gtk_widget_show_all(application->priv->plug);
		else
			gtk_widget_show_all(GTK_WIDGET(gtk_application_get_active_window(GTK_APPLICATION(application))));
	}
	else
	{
		if(application->priv->plug)
			gtk_widget_hide(application->priv->plug);
		else
			gtk_widget_show_all(GTK_WIDGET(gtk_application_get_active_window(GTK_APPLICATION(application))));
	}

}



static void application_check_client_service ( GlApplication *application);



static void
application_check_service_callback ( TeraClientObject *object , gboolean done , GlApplication *application)
{
	if(application->priv->processed)
	{
		if( (gpointer) application->priv->processed->data == (gpointer)object)
		{
			if(tera_client_is_critical(TERA_CLIENT_OBJECT(application->priv->processed->data)))

;
			application->priv->processed = application->priv->processed->next;
			application_check_client_service(application);
		}
	}
}

static void
application_check_service_name_lost_callback ( TeraClientObject *object ,GlApplication *application)
{
	// g_error(_("Dependence service name %s critical error - %s"),tera_client_id(object),tera_client_critical_message(object));
}



void application_check_client_service ( GlApplication *application )
{
	if(application->priv->processed == NULL)
	{
		if(GL_APPLICATION_GET_CLASS(application)->load_window)
			{
				GL_APPLICATION_GET_CLASS(application)->load_window(application);
			}
	}
	else
	{
		tera_client_run(TERA_CLIENT_OBJECT(application->priv->processed->data));
		if(tera_client_initialized(TERA_CLIENT_OBJECT(application->priv->processed->data)))
		{
			if(tera_client_is_critical(TERA_CLIENT_OBJECT(application->priv->processed->data)))
				// g_error(_("Dependence service name %s critical error - %s"),tera_client_id(application->priv->processed->data),tera_client_critical_message(application->priv->processed->data));

			application->priv->processed = application->priv->processed->next;
			application_check_client_service(application);
		}
		else
		{
			g_signal_connect(application->priv->processed->data,"client-done",G_CALLBACK(application_check_service_callback),application);
		}
	}
}

static void
application_start_method_cb (LgdmApp *dapp ,GDBusMethodInvocation *invocation, const gchar *layout_name ,GlApplication *application )
{
	if(!gl_layout_manager_default_activate_named(layout_name))
	{
		gl_layout_manager_default_activate_root(layout_name);
	}
}


static void
gl_application_reload_in_socket ( GlApplication *application)
{
	if(application->priv->desktop_manager != NULL)
	{
		if(application->priv->plug)   g_object_unref(application->priv->plug);
		application->priv->plug = gtk_plug_new(0);

		application->priv->layout_manager = GL_LAYOUT_MANAGER(gl_layout_manager_new(application->priv->desktop_object));
		gtk_container_add(GTK_CONTAINER(application->priv->plug),GTK_WIDGET(application->priv->layout_manager));
		gtk_window_set_application (GTK_WINDOW (application->priv->plug), GTK_APPLICATION (application));
		gchar *desktop_id = g_strdup_printf("%s.desktop",g_application_get_application_id(G_APPLICATION(application)));
		application->priv->appinfo = g_desktop_app_info_new(desktop_id);
		g_free(desktop_id);
		if(g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(application->priv->appinfo)))
		{

			if(application->priv->appinfo!= NULL )
			{
				if(application->priv->object_path)g_free(application->priv->object_path);

			/*	g_message("App Info Id = %s Dateiname = %s WmClass = %s",g_app_info_get_id(G_APP_INFO(application->priv->appinfo)),
						g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(application->priv->appinfo)),
						g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(application->priv->appinfo)));*/
				application->priv->object_path = g_strdup_printf("/com/lar/lgdm/app/%s",g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(application->priv->appinfo)));
				application->priv->app_object = LGDM_OBJECT(g_dbus_object_manager_get_object(application->priv->client,application->priv->object_path));
				if(application->priv->app_object)
				{
					LgdmApp *ldmapp = lgdm_object_get_app(application->priv->app_object) ;
					if(ldmapp)
					{
						//TEST:	g_debug("TEST APPLICATION START SET  PLUG ID = %d",gtk_plug_get_id(GTK_PLUG(application->priv->plug)));
						lgdm_app_set_plug_id(ldmapp,gtk_plug_get_id(GTK_PLUG(application->priv->plug)));
						g_signal_connect(ldmapp,"start",G_CALLBACK(gl_application_start_callback),application);
						g_signal_connect(ldmapp,"notify::activated",G_CALLBACK(gl_application_activate_callback),application);
					}

				}
			}
		}
		gui_app_gapp_set_plug_id(application->priv->gapp,gtk_plug_get_id(GTK_PLUG(application->priv->plug)));
		g_signal_connect(application->priv->gapp,"handle-start",G_CALLBACK(application_start_method_cb),application);

		application->priv->processed = application->priv->need_service;
		application_check_client_service(application);

		gtk_widget_show_all (GTK_WIDGET (application->priv->plug));
	}
	else
	{
		GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		application->priv->layout_manager = GL_LAYOUT_MANAGER(gl_layout_manager_new(NULL));
		gtk_window_set_default_size(GTK_WINDOW(window),800,600);
		gtk_container_add(GTK_CONTAINER(window),GTK_WIDGET(application->priv->layout_manager));
		gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (application));
		gchar *desktop_id = g_strdup_printf("%s.desktop",g_application_get_application_id(G_APPLICATION(application)));
		application->priv->appinfo = g_desktop_app_info_new(desktop_id);
		g_free(desktop_id);
		if(g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(application->priv->appinfo)))
		{

			if(application->priv->appinfo!= NULL )
			{
				if(application->priv->object_path)g_free(application->priv->object_path);

				/*g_message("App Info Id = %s Dateiname = %s WmClass = %s",g_app_info_get_id(G_APP_INFO(application->priv->appinfo)),
						g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(application->priv->appinfo)),
						g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(application->priv->appinfo)));*/
				application->priv->object_path = g_strdup_printf("/com/lar/lgdm/app/%s",g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO(application->priv->appinfo)));
				application->priv->app_object = LGDM_OBJECT(g_dbus_object_manager_get_object(application->priv->client,application->priv->object_path));
				if(application->priv->app_object)
				{
					LgdmApp *ldmapp = lgdm_object_get_app(application->priv->app_object) ;
					if(ldmapp)
					{
						lgdm_app_set_plug_id(ldmapp,0);
						g_signal_connect(ldmapp,"start",G_CALLBACK(gl_application_start_callback),application);
						g_signal_connect(ldmapp,"notify::activated",G_CALLBACK(gl_application_activate_callback),application);
					}

				}
			}
		}
		gui_app_gapp_set_plug_id(application->priv->gapp,0);
		g_signal_connect(application->priv->gapp,"handle-start",G_CALLBACK(application_start_method_cb),application);
		application->priv->processed = application->priv->need_service;
		application_check_client_service(application);
		gtk_widget_show_all (GTK_WIDGET (window));
	}
	g_timeout_add(500,gl_application_started_idle_callback,application);
}

static void
gl_application_new_window (GApplication *app,
                           GFile        *file)
{
	ldm_get_resource();
	gl_application_reload_in_socket(GL_APPLICATION(app));
}


static void
gl_application_init_action (GlApplication *app  )
{
	// g_signal_connect (action, "activate", G_CALLBACK (gl_application_activate_action_test), app);


}

static void
gl_application_activate (GApplication *application)
{
	GlApplication *app = GL_APPLICATION(application);
	GError *error = NULL;
	app->priv->client = lgdm_object_manager_client_new_for_bus_sync (G_BUS_TYPE_SESSION,
			G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
			"com.lar.LGDM",
			"/com/lar/lgdm/app",
			NULL, /* GCancellable */
			&error);
	if (app->priv->client == NULL)
	{
		g_printerr ("Error getting object manager client: %s", error->message);
		g_error_free (error);
		return;
	}
	gl_application_new_window(application,NULL);
	G_APPLICATION_CLASS(gl_application_parent_class)->activate (application);
}

/* GApplication implementation */
static void
gl_application_startup (GApplication *application)
{
	//
	gl_application_init_action(GL_APPLICATION(application));
	G_APPLICATION_CLASS(gl_application_parent_class)->startup (application);
}

static void
gl_application_open (GApplication  *application,
                     GFile        **files,
                     gint           n_files,
                     const gchar   *hint)
{
	//gint i;
	/*for (i = 0; i < n_files; i++)
		gl_application_new_window (application, files[i]);*/
}

static void
gl_application_init (GlApplication *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object, GL_TYPE_APPLICATION, GlApplicationPrivate);
	object->priv->icontheme      = NULL;
	object->priv->provider       = NULL;
	object->priv->layout_manager = NULL;
	object->priv->plug           = NULL;
	object->priv->desktop_manager= NULL;
	object->priv->service_table  =  g_hash_table_new(g_str_hash,g_str_equal);




	//object->priv->larglob    = g_settings_new ("com.lar.Global-schema");

/*	GSimpleAction *action;
	action = g_simple_action_new ("hide", NULL);
	g_signal_connect (action, "activate", G_CALLBACK (gl_application_activate_action_hide), object);
	g_action_map_add_action (G_ACTION_MAP (object), G_ACTION (action));
	g_object_unref (action);

	action = g_simple_action_new ("show", NULL);
	g_signal_connect (action, "activate", G_CALLBACK (gl_application_activate_action_show), object);
	g_action_map_add_action (G_ACTION_MAP (object), G_ACTION (action));
	g_object_unref (action);
	action = g_simple_action_new ("close", NULL);
	g_signal_connect (action, "activate", G_CALLBACK (gl_application_activate_action_close), object);
	g_action_map_add_action (G_ACTION_MAP (object), G_ACTION (action));
	g_object_unref (action);*/



//	g_signal_connect ( mkt_collector_get_static(),"new-atom",G_CALLBACK(gl_application_new_atom_cb),object);
	/*action = g_simple_action_new_stateful ("toggle-action", NULL,
	                                         g_variant_new_boolean (FALSE));
	g_signal_connect (action, "activate", G_CALLBACK (activate_toggle_action), app);
	g_action_map_add_action (G_ACTION_MAP (object), G_ACTION (action));
	g_object_unref (action);
	g_signal_connect (object, "activate", G_CALLBACK (gl_application_activate_action), NULL);*/
}

static void
gl_application_finalize (GObject *object)
{
	GlApplication *app = GL_APPLICATION(object);
	if(app->priv->icontheme) g_object_unref(app->priv->icontheme);
	if(app->priv->provider) g_object_unref(app->priv->provider);
	if(app->priv->client) g_object_unref(app->priv->client);
	if(app->priv->appinfo) g_object_unref(app->priv->appinfo);
	if(app->priv->object_path) g_object_unref(app->priv->object_path);
	G_OBJECT_CLASS (gl_application_parent_class)->finalize (object);
}

static gboolean
gl_application_dbus_register (GApplication    *application,
		                      GDBusConnection *connection,
		                      const gchar     *object_path,
		                      GError         **error)
{
	/*app->priv->bus_id = g_bus_own_name (G_BUS_TYPE_SESSION,
	                       "org.gtk.GDBus.Examples.ObjectManager",
	                       G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
	                       G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                       on_bus_acquired,
	                       on_name_acquired,
	                       on_name_lost,
	                       loop,
	                       NULL);*/
	GlApplication *app = GL_APPLICATION(application);
	GError *desktop_error = NULL;
	app->priv->desktop_manager = desktop_object_manager_client_new_for_bus_sync (G_BUS_TYPE_SESSION,
			G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_DO_NOT_AUTO_START,
			"com.lar.LGDM",
			"/com/lar/lgdm/desktop",
			NULL,
			&desktop_error);
	if(desktop_error)
	{
		g_printerr ("Error getting desktop on com.lar.LGDM /com/lar/lgdm/desktop/main - %s",desktop_error->message);
		g_error_free(desktop_error);
		app->priv->desktop_manager = NULL;
	}

	if (app->priv->desktop_manager  != NULL )
	{
		app->priv->desktop_object = DESKTOP_OBJECT(g_dbus_object_manager_get_object(app->priv->desktop_manager,"/com/lar/lgdm/desktop/main"));
		if(app->priv->desktop_object == NULL)
		{
			g_printerr ("Error getting desktop on com.lar.LGDM /com/lar/lgdm/desktop/main\n");
			app->priv->desktop_manager = NULL;
			//g_assert_nonnull(app->priv->desktop_manager);
		}
	}
	else
	{
		g_message( "No LAR desktop manager found - debug mode activated");
	}
	app->priv->gapp = gui_app_gapp_skeleton_new();
	desktop_error = NULL;
	if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (app->priv->gapp),
			connection,
			"/tera/gapp",
			&desktop_error))
	{
		// g_error("Service interface create fail");
	}
	if (!G_APPLICATION_CLASS (gl_application_parent_class)->dbus_register (application,
			connection,
			object_path,
			error))
		return FALSE;
	return TRUE;
}

static void
gl_application_dbus_unregister (GApplication    *application,
		                        GDBusConnection *connection,
	                        	const gchar     *object_path)
{

	//GlApplication *app = GL_APPLICATION(application);
	//g_bus_unown_name (app->priv->bus_id);
	/* Lastly, we must chain up to the parent class */
	G_APPLICATION_CLASS (gl_application_parent_class)->dbus_unregister (application,
			connection,
			object_path);
}



static void
gl_application_class_init (GlApplicationClass *klass)
{

	G_APPLICATION_CLASS (klass)->startup         = gl_application_startup;
	G_APPLICATION_CLASS (klass)->activate        = gl_application_activate;
	G_APPLICATION_CLASS (klass)->open            = gl_application_open;
	G_OBJECT_CLASS (klass)->finalize             = gl_application_finalize;

	g_type_class_add_private (klass, sizeof (GlApplicationPrivate));

	G_APPLICATION_CLASS (klass)->dbus_register   = gl_application_dbus_register;
	G_APPLICATION_CLASS (klass)->dbus_unregister = gl_application_dbus_unregister;


	klass->load_window   = NULL;


}


static GlApplication * __application__ = NULL;

GlApplication *
gl_application_get ( )
{
	return __application__;
}

GlLayoutManager*
gl_application_default_layout_manager                 ( void )
{
	if(__application__!=NULL) 	return __application__->priv->layout_manager;
	return NULL;
}

GDBusObjectManager*
gl_application_get_lgdm_manager                       ( void )
{
	if(__application__!=NULL) 	return __application__->priv->client;
	return NULL;
}

LgdmObject*
gl_application_get_lgdm_object                        ( void )
{
	if(__application__==NULL) 	return NULL;
	return LGDM_OBJECT(g_dbus_object_manager_get_object(__application__->priv->client,__application__->priv->object_path));
}

GuiAppGapp*
gl_application_get_gapp                               ( void )
{
	if(__application__==NULL) 	return NULL;
	return __application__->priv->gapp;
}


gint
gl_application_command_line (GApplication            *application,
               GApplicationCommandLine *command_line,
               gpointer                 user_data)
{
	return 0;
}

GlApplication *
gl_application_new (GType app_type ,const gchar *application_id , GApplicationFlags flag )
{
	if(!g_type_is_a(app_type,GL_TYPE_APPLICATION))
	{
		g_critical("Unknown appliaton type ");
		return NULL;
	}
	if(__application__)
	{
		g_critical ( "Es ist möglich nur eine Anwendung starten");
		return __application__;
	}
	GlApplication *app =  GL_APPLICATION(g_object_new (app_type,
	                     "application-id", application_id,
	                     "flags", flag,
	                     NULL));
	g_application_set_default(G_APPLICATION(app));
	//g_application_add_main_option_entries (G_APPLICATION(app),entries);
	__application__ = app;
	TeraClientObject *client = tera_security_manager_client_new();
	gl_application_add_watch_client(client);
	return app;
}


void
gl_application_add_watch_client ( TeraClientObject *object )
{
	if(__application__ == NULL )
	{
		// g_error("Main server session no init");
	}
	if(__application__->priv->need_service == NULL || NULL==g_list_find(__application__->priv->need_service,object))
	{
		__application__->priv->need_service = g_list_append(__application__->priv->need_service,object);
		g_hash_table_insert(__application__->priv->service_table,(gpointer)tera_client_id(object),(gpointer)object);
		g_signal_connect(object,"client-lost",G_CALLBACK(application_check_service_name_lost_callback),__application__);
	}
}

gboolean
gl_application_has_descktop                           ( void )
{
	g_return_val_if_fail(__application__!=NULL,FALSE);
	return __application__->priv->desktop_manager  != NULL;
}


TeraClientObject*
gl_application_get_client ( const gchar *id )
{
	g_return_val_if_fail(__application__!=NULL,NULL);
	TeraClientObject *client = TERA_CLIENT_OBJECT(g_hash_table_lookup(__application__->priv->service_table,(gconstpointer)id));
	return client;
}


gboolean
gl_application_show            ( GlApplication *application )
{
	g_return_val_if_fail(application!=NULL,FALSE);
	g_signal_emit(application,gl_application_signals[SHOW],0);
	g_application_hold (G_APPLICATION(application));
	g_application_release (G_APPLICATION(application));
	return TRUE;
}
gboolean
gl_application_hide          ( GlApplication *application )
{
	g_return_val_if_fail(application!=NULL,FALSE);
	g_signal_emit(application,gl_application_signals[HIDE],0);
	g_application_hold (G_APPLICATION(application));
	g_application_release (G_APPLICATION(application));
	return TRUE;
}

LgdmObject*
gl_application_get_object          ( GlApplication *application )
{
	g_return_val_if_fail(application!=NULL,NULL);
	return application->priv->app_object;
}

