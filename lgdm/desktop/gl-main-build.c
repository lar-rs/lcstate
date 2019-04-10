/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ProOptKa
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 *
 * ProOptKa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ProOptKa is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-main-build.h"
#include "gllib.h"
#include <gtk/gtk.h>
#include <string.h>


#include <mkt-value.h>

#include "../config.h"
#include <glib/gi18n-lib.h>


#define IS_DEBUG
#include "gl-update.h"

enum
{
	GL_MAIN_BUILD_CONTROLL_BOTTOM,
	GL_MAIN_BUILD_CONTROLL_RIGHT,
	GL_MAIN_BUILD_CONTROLL_LAST
};

enum
{
	GL_MAIN_BUILD_CENTRAL_PLACE,
	GL_MAIN_BUILD_BOTTOM_PLACE,
	GL_MAIN_BUILD_TOP_PLACE,
	GL_MAIN_BUILD_RIGHT_PLACE,
	GL_MAIN_BUILD_SMALL_PLACE,
	GL_MAIN_BUILD_LAST_PLACE

};


static const GEnumValue gl_main_build_places[] = {
    { GL_MAIN_BUILD_CENTRAL_PLACE, "gl_main_intarface", "level_notebook" },
    { GL_MAIN_BUILD_BOTTOM_PLACE,  "gl_hbox_main_control_box_bottom",  "control_bottom" },
    { GL_MAIN_BUILD_TOP_PLACE,     "gl_main_statusbar",     "status"         },
    { GL_MAIN_BUILD_RIGHT_PLACE,   "gl_hbox_main_control_box_right",   "control_right"  },
    { GL_MAIN_BUILD_SMALL_PLACE,   "gl_hbox_main_control_small_box",   "small_box"  },
    { GL_MAIN_BUILD_LAST_PLACE, NULL, NULL }
  };

struct _GlMainBuildPrivate
{
	GList            *modules;
	gchar            *action_path;
	gchar            *module_path;
	gchar            *wizards_path;
	gchar            *domane;
	gchar            *glade_path;
	GList            *managers;

	gdouble           progress;
	gdouble           step;
	GList            *mod_dirs;
	GList            *load_list;
	GList            *actions_list;
	guint             a_stream;
	MktDbusObject    *dbus;

};


#define GL_MAIN_BUILD_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_MAIN_BUILD, GlMainBuildPrivate))

enum
{
	PROP_0,
	PROP_ACTIONS_PATH,
	PROP_MODULE_PATH,
	PROP_DOMANE,
	PROP_GLADE_PATH,
	PROP_LEVEL
};

enum
{
	ADD_MODULE,
	LAST_SIGNAL
};


static guint main_build_signals[LAST_SIGNAL] = { 0 };

gint
gl_build_main_sort_list_with_string(gconstpointer a,gconstpointer b)
{
	gchar *vn,*nn;
	vn = (gchar*)a;
	nn = (gchar*)b;
	if( (vn==NULL) || (nn == NULL) ) return 0;
	return strcmp(vn,nn);
}


gboolean
gl_main_build_change_level (GlLevelManager *manager , gpointer data)
{
	//TEST:g_debug("gl_main_build_change_level");
	return FALSE;
}


G_DEFINE_TYPE (GlMainBuild, gl_main_build, MKT_TYPE_ATOM);

static void
gl_main_build_init (GlMainBuild *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_MAIN_BUILD,GlMainBuildPrivate);

	object->priv->module_path  = g_strdup("/lar/gui/test");
	object->priv->modules      = NULL;
	object->priv->mod_dirs     = NULL;
	object->priv->load_list    = NULL;
	object->priv->a_stream     = 0;
	object->priv->dbus         = MKT_DBUS_OBJECT(g_object_new(MKT_TYPE_DBUS_OBJECT,
										"dbus-name","org.freedesktop.DBus",
										"dbus-path","/org/freedesktop/DBus",
										"dbus-server",FALSE,NULL));

}

gboolean gl_main_build_modul_close (GtkButton *button, gpointer data)
{
	g_return_val_if_fail(data  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_MAIN_BUILD(data),FALSE);
	return TRUE;
}


gboolean gtk_lar_main_click_control_button(GtkWidget *widget, GlMainBuild *main)
{
	g_return_val_if_fail(main  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_MAIN_BUILD(main),FALSE);
	//GList *tm = main->moduls;
	//GtkControlBox* cBox = GTK_CONTROL_BOX(widget);
	return TRUE;
}


gboolean
gl_main_build_load(GlMainBuild *object)
{
	g_return_val_if_fail (GL_IS_MAIN_BUILD (object),FALSE);
	return TRUE;

}


gboolean
gl_main_build_add_module_path  ( GlMainBuild *object,const gchar *modulpath )
{
	g_return_val_if_fail(object  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_MAIN_BUILD(object),FALSE);
	if(g_file_test(modulpath,G_FILE_TEST_IS_DIR))
	{
//		g_debug("ADD MODUL PATH %s",modulpath);
		object->priv->mod_dirs  = g_list_append ( object->priv->mod_dirs , (gpointer ) (modulpath) );
	}
	return TRUE;
}

gint
__equal_modul_patch    (gconstpointer  a, gconstpointer  b)
{
	g_return_val_if_fail(a!=NULL,1);
	g_return_val_if_fail(a!=NULL,-1);

	return g_strcmp0((const gchar*)a,(const gchar*)b);
}

gboolean
gl_main_build_load_module_path (GlMainBuild *object,const gchar *path)
{
	//TEST:
	g_return_val_if_fail(GL_IS_MAIN_BUILD(object),FALSE);
	if(g_file_test( path,G_FILE_TEST_IS_DIR))
	{
		if(!g_list_find_custom(object->priv->mod_dirs,(gpointer ) (path),__equal_modul_patch))
		{
//			g_debug("ADD MODUL PATH %s",path);
			object->priv->mod_dirs  = g_list_append ( object->priv->mod_dirs , (gpointer ) (g_strdup(path)) );
		}
	}
	object->priv->mod_dirs = g_list_sort(object->priv->mod_dirs,gl_build_main_sort_list_with_string);
	return TRUE;
}

gboolean
gl_main_build_load_directory ( GlMainBuild *object , const gchar *directory)
{
	g_return_val_if_fail(GL_IS_MAIN_BUILD(object),FALSE);
	GDir *dir = NULL;
	GError *error = NULL;
	dir = g_dir_open ( directory, 0, &error );
	if(dir== NULL || error != NULL)
	{
		g_critical("%s", error->message );
		return FALSE;
	}
	const gchar *name = NULL;
	while( (name = g_dir_read_name (dir)) )
	{
		if(2 < strlen ( name ))
		{
			gchar *module_name      = g_build_path("/",directory,name,NULL);
//			g_debug("Test path %s",module_name);
			if(g_file_test(module_name,G_FILE_TEST_IS_DIR))
			{
				gl_main_build_load_module_path(object,module_name);

			}
			g_free(module_name);
		}
	}
	g_dir_close (dir);
	return TRUE;
}

gboolean
gl_main_build_load_module ( GlMainBuild *object,const gchar *modulpath )
{
	//TEST:
	g_return_val_if_fail(GL_IS_MAIN_BUILD(object),FALSE);
	if(!g_file_test(modulpath,G_FILE_TEST_IS_DIR))
	{
		g_warning("module %s not exists : load module \n",modulpath);
		return FALSE;
	}
	mkt_module_manager_load_directory(modulpath);
	mkt_collector_load_atoms_from_directory( modulpath );

	g_signal_emit (object,main_build_signals[ADD_MODULE],0);
	return TRUE;
}

gboolean
gl_main_build_destroy_module    ( GlMainBuild* object )
{
	GList *l = NULL ;
	for(l=object->priv->mod_dirs;l!=NULL;l=l->next)
	{
		if(l->data)
		{
			mkt_collector_remove_atoms_directory((const gchar*)l->data);
			mkt_module_manager_destroy_directory((const gchar*)l->data);
		}
	}
	return TRUE;
}


static void
gl_main_build_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	g_return_if_fail (GL_IS_MAIN_BUILD (object));
	GlMainBuild* build = GL_MAIN_BUILD(object);
	g_free(build->priv->domane);
	g_free(build->priv->glade_path);
	if(build->priv->module_path)g_free(build->priv->module_path);
	//gtk_widget_destroy(build->priv->window);
	//g_object_unref(build->priv->xml);
	GList *l = NULL ;
	for(l=build->priv->mod_dirs;l!=NULL;l=l->next)
	{
		if(l->data)
		{
			mkt_collector_remove_atoms_directory((const gchar*)l->data);
			mkt_module_manager_destroy_directory((const gchar*)l->data);
		}
	}
	if(build->priv->mod_dirs)     mkt_list_free_full(build->priv->mod_dirs,g_free);
	g_object_unref(build->priv->dbus);
	G_OBJECT_CLASS (gl_main_build_parent_class)->finalize (object);
}


static void
gl_main_build_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_MAIN_BUILD (object));
	GlMainBuild* build = GL_MAIN_BUILD(object);
	switch (prop_id)
	{
	case PROP_DOMANE:
		build->priv->domane  = g_value_dup_string (value);
		break;
	case PROP_MODULE_PATH:
		if( build->priv->module_path ) g_free ( build->priv->module_path );
		build->priv->module_path  = g_value_dup_string ( value );
		gl_main_build_load_module_path ( build ,build->priv->module_path);
		break;
	case PROP_GLADE_PATH:
		build->priv->glade_path = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_main_build_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_MAIN_BUILD (object));
	GlMainBuild* build = GL_MAIN_BUILD(object);
	switch (prop_id)
	{
	case PROP_DOMANE:
		/* TODO: Add setter for "domane" property here */
		 g_value_set_string (value, build->priv->domane);
		break;
	case PROP_GLADE_PATH:
		/* TODO: Add setter for "glade_path" property here */
		 g_value_set_string (value, build->priv->glade_path);
		break;
	case PROP_MODULE_PATH:
		g_value_set_string  ( value, build->priv->module_path );
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_main_build_add_module_signal (GlMainBuild* build,gpointer data)
{
	/* TODO: Add default signal handler implementation here */
}

static void
gl_main_build_class_init (GlMainBuildClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (GlMainBuildPrivate));

	object_class->finalize     = gl_main_build_finalize;
	object_class->set_property = gl_main_build_set_property;
	object_class->get_property = gl_main_build_get_property;

	klass->add_module = gl_main_build_add_module_signal;

	GParamSpec *pspec;
	pspec = g_param_spec_string ("domane_name",
			"domane name",
			"Set domane name",
			"noname",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_DOMANE,pspec);
	pspec = g_param_spec_string ("module_path",
				"Module path",
				"Set|Get module path",
				"/lar/gui/modules",
				G_PARAM_READABLE | G_PARAM_WRITABLE );
		g_object_class_install_property (object_class,
				PROP_MODULE_PATH,pspec);

	pspec = g_param_spec_string ("glade_path",
			"glade path",
			"Set glade path",
			"/lar/var/gui.glade",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_GLADE_PATH,pspec);

//---------------------------signals---------------------------------
	main_build_signals[ADD_MODULE] =
			g_signal_new ("add_module",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlMainBuildClass, add_module),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static gboolean GUI_BUILD_MODULE_LOADED = FALSE;


gboolean
gl_main_build_initialise_wait(gpointer data)
{
	//
	g_return_val_if_fail (data != NULL,FALSE);
	g_return_val_if_fail (GL_IS_MAIN_BUILD(data),FALSE);
	GlMainBuild *bmain = GL_MAIN_BUILD(data);
	if( !GUI_BUILD_MODULE_LOADED )
	{
		if(bmain->priv->progress  >= 1.5)
		{
			GObject *app = mkt_dbus_lookup_object(MKT_DBUS(bmain->priv->dbus),"/com/lar/GDM/Control");
			if(app && MKT_IS_APP(app))
				mkt_app_change_state(MKT_DBUS(app),"loading external plug-in");
			bmain->priv->progress  = 0.00;
			bmain->priv->step      = 1.0;
			bmain->priv->load_list = bmain->priv->mod_dirs;
			if(bmain->priv->load_list != NULL )
			{
				bmain->priv->step  = (gdouble) 1./g_list_length(bmain->priv->load_list);
				gl_level_manager_set_load_module_fraction(bmain->priv->step);
			}
		}
		if( bmain->priv->load_list )
		{
			bmain->priv->progress += bmain->priv->step;
			gl_level_manager_set_load_module_fraction(bmain->priv->progress);
			if(bmain->priv->load_list->data)
			{
				const gchar *path = (const gchar*)bmain->priv->load_list->data;
				gl_main_build_load_module(bmain,path);
			}
			bmain->priv->load_list = bmain->priv->load_list ->next;
		}
		else
		{
			GUI_BUILD_MODULE_LOADED = TRUE;
			bmain->priv->progress = -1.0;
			gl_level_manager_set_load_module_fraction(bmain->priv->progress);
		}
	}
	else if(GUI_BUILD_MODULE_LOADED)
	{
		g_list_free(bmain->priv->mod_dirs);
		bmain->priv->mod_dirs = NULL;
		GObject *app = mkt_dbus_lookup_object(MKT_DBUS(bmain->priv->dbus),"/com/lar/GDM/Control");
		if(app && MKT_IS_APP(app))
			mkt_app_change_state(MKT_DBUS(app),"load_module_done");
		return FALSE;
	}


/*	// TEST 28 -> 0 start status
#ifdef GUI_DEBUG
	if( !GUI_BUILD_MODULE_LOADED ) // TODO : test..
#else
	if( (mkIget(control_subscription__internalStatus) >= 28 )&&(mkIget(control_subscription__internalStatus) < 35 ) ) // TODO : test..
#endif
	{
		if(bmain->priv->progress  > 1.5)
		{
			bmain->priv->progress  = 0.00;
			bmain->priv->step      = 1.0;
			bmain->priv->load_list = bmain->priv->mod_dirs;
			if(bmain->priv->load_list != NULL )
			{
				bmain->priv->step  = (gdouble) 1./g_list_length(bmain->priv->load_list);
			}
		}
		if( bmain->priv->load_list )
		{
			bmain->priv->progress+=bmain->priv->step;
			GString *name =  (GString *)bmain->priv->load_list->data;
			if(name != NULL && bmain->priv->load_path != NULL)
			{
				//g_debug("Test create %s",name->str);
				gchar *module_path = NULL;
				module_path = g_build_filename (bmain->priv->load_path,name->str, NULL);
				gl_main_build_load_module(bmain,module_path);
				g_free (module_path);
			}
			gl_level_manager_set_load_module_fraction(bmain->priv->level_manager,bmain->priv->progress);
			bmain->priv->load_list = bmain->priv->load_list ->next;
			//if(main->priv->load_list == NULL ) g_debug("TEST ....");
		}
		else
		{

			GUI_BUILD_MODULE_LOADED = TRUE;
			bmain->priv->progress = 1.5;
			gl_level_manager_set_load_module_fraction(bmain->priv->level_manager,bmain->priv->progress);
#ifndef GUI_DEBUG
			mkIset(gui_controlControl__control,35);
#endif

			//g_debug("\nSET_CONTROL 35\n");
			//gl_main_build_start_last_plugin( main ); Weg war nicht gewünscht
			return FALSE;
		}

	}*/
	return TRUE;
}


void
gl_main_build_initialise_wait_destroy( gpointer data )
{

}

void
gl_main_build_load_module_async_waite_market ( GlMainBuild* object )
{
	 g_timeout_add_full(G_PRIORITY_DEFAULT,300,gl_main_build_initialise_wait,object,gl_main_build_initialise_wait_destroy);
}

void
gl_main_build_run    ( GlMainBuild* object )
{
	g_return_if_fail ( object != NULL );
	g_return_if_fail ( GL_IS_MAIN_BUILD(object) );
	g_object_set(G_OBJECT(gl_level_manager_get_static()),"level",GUI_USER_DEVICE_TYPE,NULL);
	gl_level_manager_key_open_close ();
	object->priv->progress = 2.;
	gl_main_build_load_module_async_waite_market ( object );
}
