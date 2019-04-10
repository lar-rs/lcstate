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


#include <glib.h>
#include "gl-plugin.h"
#include "gl-draganddrop.h"
#include "gl-widget-option.h"
#include "gl-action-widget.h"
#include "gl-xkbd.h"
#include "gl-translation.h"
//#include "gl-tree-data.h"
#include "../../config.h"

#include <mkt-collector.h>

#include <gio/gio.h>
#include <string.h>
#include "../lgdm-status.h"


typedef struct _GlPluginStatus GlPluginStatus;

struct _GlPluginStatus
{
	gboolean   is_status;
	GtkWidget  *status;
	GtkWidget  *image;
	GtkWidget  *label;

	GtkWidget  *window;
	GtkBuilder *builder;
	GList      *info_widgets;
};


struct _GlPluginPrivate
{
	gchar           *name;
	//gchar           *schema_path;
	gchar           *autor;
	gchar           *device;
	guint            level;
	guint            move_level;
	gchar           *image_path;
	gboolean         start_up;
	gchar           *muster;

	GList           *plugin_action;
	GList           *controlled_widgets;

	GKeyFile        *config;
	GFile           *cfile;

	GlLevelManager  *level_manager;
	GlConnection    *connection;
	GlPluginStatus   status;
};

#define GL_PLUGIN_CONFIGURATION_FILE "module.cfg"


#define GL_PLUGIN_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_PLUGIN, GlPluginPrivate))



#define GL_PLUGIN_MODULE_GROUP   "Module Group"

#define GL_PLUGIN_STATUS_WINDOW  "gl_module_statuswindow"

enum {
	PROP_PLUGIN_NULL,
	PROP_PLUGIN_PATH,
	PROP_PLUGIN_NAME,
	PROP_PLUGIN_AUTOR,
	PROP_PLUGIN_DEVICE,
	PROP_PLUGIN_LEVEL,
	PROP_PLUGIN_DEFAULT_LEVEL,
	PROP_PLUGIN_MOVE_LEVEL,
	PROP_PLUGIN_CONST_LEVEL,
	PROP_PLUGIN_MENU,
	PROP_PLUGIN_MENU_PLACE,
	PROP_PLUGIN_MENU_XML,
	PROP_PLUGIN_IMAGE,
	PROP_PLUGIN_MUSTER,
	PROP_PLUGIN_START_UP,
};


enum
{
	GL_PLUGIN_START,
	GL_PLUGIN_STOP,
	GL_PLUGIN_START_MENU,
	GL_PLUGIN_STOP_MENU,
	GL_PLUGIN_ADD_INDICATE,
	GL_PLUGIN_DESTROY_INDICATE,
	LAST_SIGNAL
};


static guint gl_plugin_signals[LAST_SIGNAL] = { 0 };



G_DEFINE_TYPE (GlPlugin, gl_plugin, MKT_TYPE_WINDOW);


static gboolean            gl_plugin_open_click_signal             (GlActionWidget *widget , gpointer data);


#define GL_PLUGIN_SET_ALL_ACTION(plugin,prop,value)G_STMT_START{      \
        if(plugin != NULL || prop != NULL || GL_IS_PLUGIN(plugin)) {  \
      	  GList *__cr_a_w_ = plugin->priv->plugin_action;             \
          while(__cr_a_w_) { mktAPSet(__cr_a_w_->data,prop,value);__cr_a_w_=__cr_a_w_->next;}; } \
        else {  g_critical("GL_PLUGIN_SET_PARAM(plig,prop,val) parameter (plig != NULL || prop != NULL || GL_IS_PLUGIN(plig)) fail");} \
}G_STMT_END

gboolean
gl_plugin_create_new_starter (GtkWidget *widget , gpointer data)
{
	g_return_val_if_fail ( data != NULL,FALSE);
	g_return_val_if_fail ( GL_IS_PLUGIN (data),FALSE);
	//TEST:g_debug ( "plug in %s create starter" ,mktAPGet(data,"name",string) );
	return TRUE;
}

static gboolean
gl_plugin_status_window_close(GlPlugin * plugin )
{
	if( plugin->priv->status.window != NULL )
	{
		gtk_widget_set_sensitive(plugin->priv->status.window,FALSE);
		gtk_widget_hide(plugin->priv->status.window);
	}
	return TRUE;
}

static gboolean
gl_plugin_status_window_open(GlPlugin * plugin )
{
	if( plugin->priv->status.window != NULL )
	{
		gtk_widget_set_sensitive(plugin->priv->status.window,TRUE);
		gtk_widget_show(plugin->priv->status.window);
	}
	return TRUE;
}

gboolean
gl_plugin_show_info_press_button_signal (GtkWidget *widget, GdkEvent  *event , gpointer data)
{
	g_debug("gl_plugin_show_info_press_button_signal");
	g_return_val_if_fail ( data != NULL,FALSE);
	g_return_val_if_fail ( GL_IS_PLUGIN (data),FALSE);
	GlPlugin *plugin =  GL_PLUGIN(data);
	if(gl_level_manager_is_tru_user(2))
	{
		g_debug("gl_plugin_show_info_press_button_signal 1");
		if( plugin->priv->status.window != NULL )
		{
			g_debug("gl_plugin_show_info_press_button_signal 2");
			if(GTK_WIDGET_IS_SENSITIVE(plugin->priv->status.window))
			{
				gl_plugin_status_window_close(plugin);
			}
			else
			{
				gl_plugin_status_window_open( plugin );
				gint x_root = 0,y_root = 0;
				gdk_window_get_root_origin(widget->window,&x_root,&y_root);
				gtk_window_move(GTK_WINDOW (plugin->priv->status.window),(x_root+200),(y_root+50));
			}
		}
	}
	return TRUE;
}

static void
gl_plugin_realize_status_window ( GlPlugin *plug )
{
	g_debug("gl_plugin_realize_status_window");
	if(plug->priv->status.window)
	{
		g_object_unref(plug->priv->status.window);
		plug->priv->status.window = NULL;
	}

	if(plug->priv->status.builder)
	{
		g_object_unref(plug->priv->status.builder);
	}
	GError      *error      = NULL;
	gchar *status_ui =  g_strdup_printf("/usr/local/share/%s/pluginstatus.glade",PACKAGE);
	g_debug("xmlPath = %s\n",status_ui);
	plug->priv->status.builder = gtk_builder_new();
	if(!gtk_builder_add_from_file(plug->priv->status.builder,status_ui,&error))
	{
		g_critical("%s", error->message);
		g_error_free(error);
	}
	else
	{
		GSList *slist = gtk_builder_get_objects(plug->priv->status.builder);
		GSList *tL = slist;
		while(tL != NULL)
		{
			GObject *widget = NULL;
			if(( GTK_IS_WIDGET(tL->data ) )&&((widget = G_OBJECT(tL->data))!= NULL) )
			{
				if(GTK_IS_WIDGET(widget))
				{
					if(GTK_IS_WINDOW(widget))
					{
						if(plug->priv->status.window != NULL )
						{
							g_critical("Status XML has more than one windows");
						}
						else
						{
							plug->priv->status.window = GTK_WIDGET(widget);
						}
					}

					if(GTK_IS_LABEL(widget))
					{
						if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__name"))
						{
							gtk_label_set_text(GTK_LABEL(widget),mkt_atom_get_id(MKT_ATOM(plug)));
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__author"))
						{
							gtk_label_set_text(GTK_LABEL(widget),plug->priv->autor);
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__device"))
						{
							gtk_label_set_text(GTK_LABEL(widget),plug->priv->device);
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__level"))
						{
							g_debug("PLUG_IN_LEVEL=%d",mkt_window_get_level(MKT_WINDOW(plug)));
							gtk_label_set_text(GTK_LABEL(widget),gl_level_manager_get_name_for_noob_from_id(mkt_window_get_level(MKT_WINDOW(plug))));
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__level_move"))
						{
							gtk_label_set_text(GTK_LABEL(widget),gl_level_manager_get_name_for_noob_from_id(mkt_window_get_default_level(MKT_WINDOW(plug))));
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__package"))
						{
							gtk_label_set_text(GTK_LABEL(widget),gl_plugin_get_package(plug));
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"Module__version"))
						{
							gtk_label_set_text(GTK_LABEL(widget),gl_plugin_get_version(plug));
						}
					}
					else if(GTK_IS_TEXT_VIEW(widget))
					{

						if(0==g_strcmp0(gl_widget_option_get_name(widget),"textview_copyright"))
						{
							gchar *fpath =  g_strdup_printf("/usr/local/share/doc/%s/COPYING",gl_plugin_get_package(plug));
							if(g_file_test(fpath,G_FILE_TEST_EXISTS) )
							{
								GError *error = NULL;
								gchar  *content = NULL;
								gsize   len = 0;
								if(!g_file_get_contents(fpath,&content,&len,&error))
								{
									g_warning("Plugin %s COPYING file load failed: %s",mkt_atom_get_id(MKT_ATOM(plug)),error?error->message:"unknown");
									if(error) g_error_free(error);
								}
								else
								{
									GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
									GtkTextIter iter;
									gtk_text_buffer_create_tag(buffer, "test", NULL);
									gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
									gtk_text_buffer_insert(buffer, &iter, content, -1);
									g_free(content);
								}
							}
							else
							{
								g_warning("File %s not found",fpath);
							}
							g_free(fpath);
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"textview_news"))
						{
							gchar *fpath =  g_strdup_printf("/usr/local/share/doc/%s/NEWS",gl_plugin_get_package(plug));
							if( g_file_test(fpath,G_FILE_TEST_EXISTS) )
							{
								GError *error = NULL;
								gchar *content = NULL;
								gsize len = 0;
								if(!g_file_get_contents(fpath,&content,&len,&error))
								{
									g_warning("Plugin %s NEWS file load failed: %s",mkt_atom_get_id(MKT_ATOM(plug)),error?error->message:"unknown");
									if(error) g_error_free(error);
								}
								else
								{
									GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
									GtkTextIter iter;
									gtk_text_buffer_create_tag(buffer, "test", NULL);
									gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
									gtk_text_buffer_insert(buffer, &iter, content, -1);
									g_free(content);
								}
							}
							else
							{
								g_warning("File %s not found",fpath);
							}
							g_free(fpath);
						}
						else if(0==g_strcmp0(gl_widget_option_get_name(widget),"textview_changelog"))
						{
							gchar *fpath =  g_strdup_printf("/usr/local/share/doc/%s/ChangeLog",gl_plugin_get_package(plug));
							if( g_file_test(fpath,G_FILE_TEST_EXISTS) )
							{
								GError *error = NULL;
								gchar *content = NULL;
								gsize len = 0;
								if(!g_file_get_contents(fpath,&content,&len,&error))
								{
									g_warning("Plugin %s ChangeLog file load failed: %s",mkt_atom_get_id(MKT_ATOM(plug)),error?error->message:"unknown");
									if(error) g_error_free(error);
								}
								else
								{
									GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
									GtkTextIter iter;
									gtk_text_buffer_create_tag(buffer, "test", NULL);
									gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
									gtk_text_buffer_insert(buffer, &iter, content, -1);
									g_free(content);
								}
							}
							else
							{
								g_warning("File %s not found",fpath);
							}
							g_free(fpath);
						}
					}
				}
				gl_binding_insert_object ( widget );
			}
			tL = tL -> next;
		}
		gtk_builder_connect_signals( plug->priv->status.builder, plug );
		g_slist_free(slist);
	}
	g_free(status_ui);
}

static void
gl_plugin_create_status ( GlPlugin * plugin )
{
	g_debug("gl_plugin_create_status");
	g_return_if_fail ( GL_IS_PLUGIN (plugin));
	if( plugin->priv->status.status != NULL  )
	{
		gtk_widget_destroy(plugin->priv->status.status);
	}
	plugin->priv->status.status = gtk_event_box_new();
	gtk_widget_ref(plugin->priv->status.status);
	GtkWidget *hbox = gtk_hbox_new(FALSE,1);
	gtk_container_add(GTK_CONTAINER(plugin->priv->status.status),hbox);
	gtk_widget_add_events(GTK_WIDGET(plugin->priv->status.status),GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	plugin->priv->status.image = gtk_image_new();
	gchar *icon = (gchar*) mktAPGet(plugin,"icon",string);
	if( ! g_file_test(icon,G_FILE_TEST_EXISTS) ) icon = "/lar/gui/utilities-log-viewer.png";
	gtk_image_set_from_file(GTK_IMAGE(plugin->priv->status.image),icon);
	plugin->priv->status.label = gtk_label_new(mkt_atom_get_id(MKT_ATOM(plugin)));
	gchar status_name[256] = "";
	g_snprintf(status_name,sizeof(status_name),"TRANSLATE_label_Name_%s",mkt_atom_get_id(MKT_ATOM(plugin)));
	//g_debug("Translate Statusname %s",status_name);
	gl_widget_option_set_name(G_OBJECT(plugin->priv->status.label),status_name);
	gl_binding_insert_widget(plugin->priv->status.label);
	gtk_misc_set_alignment(GTK_MISC(plugin->priv->status.label),0.01,0.5);
	gtk_widget_modify_font(GTK_WIDGET(plugin->priv->status.label),pango_font_description_from_string("Oreal 6"));
	gtk_box_pack_start(GTK_BOX(hbox),plugin->priv->status.image,FALSE,FALSE,1);
	gtk_box_pack_start(GTK_BOX(hbox),plugin->priv->status.label,FALSE,FALSE,1);
	gtk_widget_hide( plugin->priv->status.image );
	gtk_widget_hide( plugin->priv->status.label );
	gtk_widget_show( hbox);
	gtk_widget_show( plugin->priv->status.status);
	g_signal_connect ( plugin->priv->status.status, "button-press-event" , G_CALLBACK(gl_plugin_show_info_press_button_signal), plugin );
	gtk_widget_show( plugin->priv->status.label );
	gtk_widget_show( plugin->priv->status.image );
	gtk_widget_show( hbox);
	g_debug("package - %s",PACKAGE);
	gl_plugin_realize_status_window(plugin);


}

GtkWidget*
gl_plugin_get_gtk_status ( GlPlugin *plugin )
{
	g_return_val_if_fail(plugin != NULL , NULL );
	g_return_val_if_fail(GL_IS_PLUGIN(plugin) , NULL );
	return plugin->priv->status.status;
}

static void
gl_plugin_change_status_icon (  GlPlugin *plugin )
{
	if(!mkt_window_is_realized(MKT_WINDOW(plugin))) return;
	g_return_if_fail(plugin->priv->status.image != NULL);
	gchar *icon = mkt_atom_build_path(MKT_ATOM(plugin),mktAPGet(plugin,"icon",string));
	if( ! g_file_test(icon,G_FILE_TEST_EXISTS) ) icon = g_strdup_printf("/usr/local/share/%s/utilities-log-viewer.png",PACKAGE);
	gtk_image_set_from_file(GTK_IMAGE(plugin->priv->status.image),icon);
	g_free(icon);
}

static void
gl_plugin_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_PLUGIN (object));
	GlPlugin* plugin = GL_PLUGIN(object);
	//g_debug("Set (GL_PLUGIN) property %d - %s",prop_id,g_param_spec_get_name(pspec));
	switch (prop_id)
	{
	case PROP_PLUGIN_AUTOR:
		if(plugin->priv->autor) g_free(plugin->priv->autor);
		plugin->priv->autor = g_value_dup_string(value);
		break;
	case PROP_PLUGIN_DEVICE:
		if(plugin->priv->device) g_free(plugin->priv->device);
		plugin->priv->device = g_value_dup_string(value);
		break;
	case PROP_PLUGIN_MOVE_LEVEL:
		plugin->priv->move_level    = g_value_get_uint(value);
		//g_debug("CHANGE_LEVEL Plugin %s move Level %d....0",mkt_atom_get_id(MKT_ATOM(object)),plugin->priv->move_level);
		break;
	case PROP_PLUGIN_IMAGE:
		if(plugin->priv->image_path) g_free(plugin->priv->image_path);
		plugin->priv->image_path = mkt_atom_build_path(MKT_ATOM(plugin),g_value_get_string(value));
		gl_plugin_change_status_icon(plugin);
		break;
	case PROP_PLUGIN_START_UP:
		plugin->priv->start_up   = g_value_get_boolean(value);
		break;
	case PROP_PLUGIN_MUSTER:
		if(plugin->priv->muster ) g_free(plugin->priv->muster);
		plugin->priv->muster = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_plugin_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_PLUGIN (object));
	GlPlugin* plugin = GL_PLUGIN(object);
	//g_debug("Get (GL_PLUGIN) property %s - %d",g_param_spec_get_name(pspec) , prop_id);
	switch (prop_id)
	{
	case PROP_PLUGIN_AUTOR:
		g_value_set_string(value,plugin->priv->autor);
		break;
	case PROP_PLUGIN_DEVICE:
		g_value_set_string(value,plugin->priv->device);
		break;
	case PROP_PLUGIN_MOVE_LEVEL:
		g_value_set_uint(value,plugin->priv->move_level);
		break;
	case PROP_PLUGIN_IMAGE:
		g_value_set_string(value,plugin->priv->image_path);
		break;
	case PROP_PLUGIN_START_UP:
		g_value_set_boolean(value,plugin->priv->start_up);
		break;
	case PROP_PLUGIN_MUSTER:
		g_value_set_string(value,plugin->priv->muster);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



// ----------------------------------------END Drag and Drop
static gboolean
gl_plugin_open_click_signal(GlActionWidget *widget,gpointer data)
{
	g_return_val_if_fail(GL_IS_PLUGIN(data),FALSE);
	GlPlugin *plugin = GL_PLUGIN(data);
	gl_plugin_open(plugin);
	return TRUE;
}

static gboolean
gl_plugin_init_menu(GlPlugin *object )
{

	return TRUE;
}

void
gl_plugin_child_change_level_notify (MktAtom *child ,  GParamSpec *pspec , MktAtom *plugin )
{
	mktAPSet(plugin,"level",mkt_window_get_level(MKT_WINDOW(child)));
}

void
gl_plugin_add_child ( MktAtom *parent ,MktAtom *child , gpointer user_data )
{
	//TEST:g_debug("PlugIN %s add child %s",mkt_atom_get_id(parent),mkt_atom_get_id(child));
	g_return_if_fail (parent!= NULL);
	g_return_if_fail (GL_IS_PLUGIN (parent));
	if(GL_IS_ACTION_WIDGET(child))
	{
		g_signal_connect(child,"start_action",G_CALLBACK(gl_plugin_open_click_signal),(gpointer)parent);
		guint def_level      = mktAPGet(parent, "default-level",uint);
		gboolean const_level = mktAPGet(parent, "const-level",boolean);
		mkt_atom_object_set(child,"level",mkt_window_get_level(MKT_WINDOW(child)),"default-level",def_level , "const-level" ,const_level , NULL);
		mktAPSet(child,"level",mktAPGet(parent,"level",uint));
		g_signal_connect (child, "notify::level", G_CALLBACK (gl_plugin_child_change_level_notify), parent);
		GL_PLUGIN(parent)->priv->plugin_action = g_list_append(GL_PLUGIN(parent)->priv->plugin_action,child);
	}
}

static void
gl_plugin_window_close( MktWindow *win , gpointer data )
{
	g_return_if_fail ( win != NULL );
	g_return_if_fail ( MKT_IS_WINDOW(win) );
	GlPlugin *plugin = GL_PLUGIN(win);
	gl_plugin_status_window_close(plugin);
}



void
gl_plugin_init (GlPlugin *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_PLUGIN,GlPluginPrivate);
	g_return_if_fail (GL_IS_PLUGIN (object));
	/* TODO: Add initialization code here */
	object->isLoad                   = FALSE;
	object->priv->autor              = g_strdup("unknown author");
	object->priv->device             = g_strdup("unknown device");
	object->priv->image_path         = g_strdup("/lar/gui/utilities-log-viewer.png");
	object->priv->controlled_widgets = NULL;
	object->priv->status.status      = NULL;
	object->priv->plugin_action      = NULL;
	object->priv->muster             = g_strdup("---");

	g_signal_connect ( MKT_WINDOW(object), "window_hide",G_CALLBACK(gl_plugin_window_close),object);
	g_signal_connect(object,"add-child",G_CALLBACK(gl_plugin_add_child),NULL);

//  plugin menu and widget initialize -----------------------------

}

void
gl_plugin_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlPlugin* plugin = GL_PLUGIN(object);
	g_free(plugin->priv->autor);
	g_free(plugin->priv->device);
	g_free(plugin->priv->image_path);
	g_free(plugin->priv->muster);
	gtk_widget_destroy(plugin->priv->status.status);
	if(NULL != plugin->priv->status.window) gtk_widget_destroy( plugin->priv->status.window);
	// FIX : free actions and widget !!!
	G_OBJECT_CLASS (gl_plugin_parent_class)->finalize (object);
}

#include <string.h>

static gboolean
gl_plugin_static_load (GlPlugin *object )
{
	g_return_val_if_fail (GL_IS_PLUGIN (object),FALSE);

	if(GL_PLUGIN_GET_CLASS(object)->plugin_init_menu)
	{
		GL_PLUGIN_GET_CLASS(object)->plugin_init_menu (object);
	}

	if(GL_PLUGIN_GET_CLASS(object)->plugin_load_menu)
	{
		GL_PLUGIN_GET_CLASS(object)->plugin_load_menu (object);
	}

// PlugIn create menu ---------------------------------------------
	/*if(mktAPGet(object,"menu",boolean))
	{

		// PlugIn menu init  ---------------------------------------------

		if(GL_PLUGIN_GET_CLASS(object)->plugin_load_menu)
		{
			GL_PLUGIN_GET_CLASS(object)->plugin_load_menu (object,object->priv->menu);
		}
	}*/
	//gl_plugin_static_change_level(object,object->priv->level_manager);
	return object->isLoad;
}

static void
gl_plugin_static_open (GlPlugin *object )
{
//	g_debug("PLUG_IN  gl_plugin_static_open\n");
}
static void
gl_plugin_static_close (GlPlugin *object )
{
	if(!gl_plugin_status_window_close(object))
	{
		g_warning ( "plig-in status window can not closed");
	}
}
static void
gl_plugin_static_open_menu (GlPlugin *object )
{
	mktAPSet(object,"startup",TRUE);
}
static void
gl_plugin_static_close_menu (GlPlugin *object )
{
	mktAPSet(object,"startup",FALSE);
}


void
gl_plugin_expose ( MktWindow *win )
{
	g_return_if_fail (win!= NULL);
	g_return_if_fail (GL_IS_PLUGIN (win));
	gl_plugin_create_status( GL_PLUGIN(win) );
	gl_plugin_load ( GL_PLUGIN(win) );
}

static void
gl_plugin_class_init (GlPluginClass *klass)
{
	GObjectClass*   object_class = G_OBJECT_CLASS (klass);
	MktWindowClass* parent_class = MKT_WINDOW_CLASS (klass);
	//MktAtomClass*   atom_class   = MKT_ATOM_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlPluginPrivate));

	object_class -> finalize                          =  gl_plugin_finalize;
	object_class -> set_property                      =  gl_plugin_set_property;
	object_class -> get_property                      =  gl_plugin_get_property;
	parent_class -> expose                            =  gl_plugin_expose;


	klass        -> init_plugin                       =  NULL;   //Wird vor aufgerufen
	klass        -> plugin_init_menu                  =  gl_plugin_init_menu;
	klass        -> plugin_load_menu                  =  NULL;


	klass        -> load_plugin                       =  gl_plugin_static_load;

	klass        -> open_plugin                       =  gl_plugin_static_open;
	klass        -> close_plugin                      =  gl_plugin_static_close;
	klass        -> open_menu                         =  gl_plugin_static_open_menu;
	klass        -> close_menu                        =  gl_plugin_static_close_menu;

	klass        -> load_plugin_reserved1             =  NULL;
	klass        -> load_plugin_reserved2             =  NULL;
	klass        -> load_plugin_reserved3             =  NULL;

	klass        -> version_string                    =  NULL;
	klass        -> package_string                    =  NULL;

	GParamSpec *pspec;
	pspec = g_param_spec_string ("author",
			"Plug in author",
			"Get plug in author",
			"unknown author",
			G_PARAM_READWRITE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_PLUGIN_AUTOR,pspec);

	pspec = g_param_spec_string ("device",
			"Plug in device",
			"Get plug in device",
			"unknown device",
			G_PARAM_READWRITE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_PLUGIN_DEVICE,pspec);

	pspec = g_param_spec_uint("level-move",
			"Plugin level",
			"Set/Get plugin level",
			GUI_USER_DEVICE_TYPE,GUI_USER_LAST_TYPE,GUI_USER_DEVICE_TYPE,
			G_PARAM_READWRITE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_PLUGIN_MOVE_LEVEL,pspec);

	pspec = g_param_spec_string ("icon",
			"Plugin status icon",
			"Set/Get module status icon",
			"/lar/gui/utilities-log-viewer.png",
			G_PARAM_READWRITE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_PLUGIN_IMAGE,pspec);

	pspec = g_param_spec_boolean("startup",
			"Plugin start up",
			"Set/Get plugin start up",
			FALSE,
			G_PARAM_READWRITE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_PLUGIN_START_UP,pspec);

	pspec = g_param_spec_string ("muster",
				"Plugin muster text",
				"Set/Get module status icon",
				"---",
				G_PARAM_READWRITE | MKT_PARAM_SAVE );
		g_object_class_install_property (object_class,
				PROP_PLUGIN_MUSTER,pspec);


	gl_plugin_signals[GL_PLUGIN_START] =
			g_signal_new ("plugin_start",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlPluginClass, open_plugin),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_plugin_signals[GL_PLUGIN_STOP] =
			g_signal_new ("plugin_stop",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlPluginClass, close_plugin),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_plugin_signals[GL_PLUGIN_START_MENU] =
			g_signal_new ("plugin_start_menu",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlPluginClass, open_menu),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_plugin_signals[GL_PLUGIN_STOP_MENU] =
			g_signal_new ("plugin_stop_menu",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlPluginClass, close_menu),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

GlPlugin* gl_plugin_new ( const  gchar *path )
{
	g_debug("gl_plugin_new\n");
	GlPlugin *plugin = NULL;
	plugin = GL_PLUGIN(g_object_new(GL_TYPE_PLUGIN ,"plugin_path",path,NULL));
	return plugin;
}


//-----------------------PlugIn initialize -----------------------------
gboolean  gl_plugin_initialize ( GlPlugin *plugin )
{
	g_return_val_if_fail (GL_IS_PLUGIN (plugin),FALSE);
	//TEST:g_debug("gl_plugin_initialize 11");
	if(GL_PLUGIN_GET_CLASS(plugin)->init_plugin)
		return GL_PLUGIN_GET_CLASS(plugin)->init_plugin (plugin);
	//TEST:g_debug("gl_plugin_initialize 22");
	return TRUE;
}

gboolean  gl_plugin_load ( GlPlugin *plugin )
{
	g_return_val_if_fail (GL_IS_PLUGIN (plugin),FALSE);
	if(GL_PLUGIN_GET_CLASS(plugin)->load_plugin)
		return GL_PLUGIN_GET_CLASS(plugin)->load_plugin (plugin);
	return FALSE;
}

void  gl_plugin_load_reserved ( GlPlugin *plugin )
{
	if( GL_PLUGIN_GET_CLASS(plugin)->load_plugin_reserved1 )
		GL_PLUGIN_GET_CLASS(plugin)->load_plugin_reserved1 (plugin);
	if( GL_PLUGIN_GET_CLASS(plugin)->load_plugin_reserved2 )
		GL_PLUGIN_GET_CLASS(plugin)->load_plugin_reserved2 (plugin);
	if( GL_PLUGIN_GET_CLASS(plugin)->load_plugin_reserved3 )
		GL_PLUGIN_GET_CLASS(plugin)->load_plugin_reserved3 (plugin);
}


const gchar*
gl_plugin_get_version                       ( GlPlugin *plugin )
{
	g_return_val_if_fail (GL_IS_PLUGIN (plugin),FALSE);
	if(GL_PLUGIN_GET_CLASS(plugin)->version_string)
		return GL_PLUGIN_GET_CLASS(plugin)->version_string(plugin);
	return "not version";
}

const gchar*
gl_plugin_get_package                       ( GlPlugin *plugin )
{
	g_return_val_if_fail (GL_IS_PLUGIN (plugin),FALSE);
	if(GL_PLUGIN_GET_CLASS(plugin)->package_string)
		return GL_PLUGIN_GET_CLASS(plugin)->package_string(plugin);
	return "not package";
}


//----------------------- PlugIn initialize end -----------------------------


//----------------------- PlugIn signature ----------------------------------

void
gl_plugin_open ( GlPlugin *plugin )
{
	//TEST:	g_debug("plugin %s open..",mkt_atom_get_id(MKT_ATOM(plugin)));
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	if(gl_level_manager_is_tru_user(mktAPGet(plugin,"level",uint)))
	{
		g_signal_emit(plugin,gl_plugin_signals[GL_PLUGIN_START],0);
		gl_plugin_open_menu(plugin);
	}
}
void
gl_plugin_close ( GlPlugin *plugin )
{
	//TEST:	g_debug("plugin %s close..",mkt_atom_get_id(MKT_ATOM(plugin)));
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	g_signal_emit(plugin,gl_plugin_signals[GL_PLUGIN_STOP],0);
	gl_plugin_close_menu(plugin);
}

void
gl_plugin_open_menu  ( GlPlugin *plugin )
{
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	g_signal_emit(plugin,gl_plugin_signals[GL_PLUGIN_START_MENU],0);
	GlStatus *status = GL_STATUS(mkt_collector_get_atom_static("com.lar.GlStatus.ultra-status"));
	gl_status_close_status_window(status);
	mkt_window_show(MKT_WINDOW(plugin));
	gl_xkbd_stop();
}
void
gl_plugin_close_menu  ( GlPlugin *plugin )
{
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	g_signal_emit(plugin,gl_plugin_signals[GL_PLUGIN_STOP_MENU],0);
	mkt_window_hide(MKT_WINDOW(plugin));
	gl_plugin_status_window_close(plugin);
	gl_xkbd_stop();
}

void
gl_plugin_change_level ( GlPlugin *plugin )
{
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	//TEST:g_debug ( "gl_plugin_change_level");
}


void
gl_plugin_save_configuration( GlPlugin *plugin )
{
	g_return_if_fail( plugin != NULL);
	g_return_if_fail( GL_IS_PLUGIN(plugin));
	//gl_system_save_key_file( plugin->priv->config ,plugin->priv->cfile);
	//g_printf("|%s|\n",data);
}

gboolean
gl_plugin_remove_action     (  GlPlugin *plugin ,GlActionWidget *widget )
{
	g_return_val_if_fail (plugin!=NULL,FALSE);
	g_return_val_if_fail (GL_IS_PLUGIN (plugin),FALSE);
	plugin->priv->plugin_action = g_list_remove(plugin->priv->plugin_action,widget);
	g_object_unref(widget);
	return TRUE;
}

MktAtom*
gl_plugin_create_action   ( GlPlugin *plugin, const gchar *nick, gboolean active )
{
	g_return_val_if_fail (GL_IS_PLUGIN (plugin),NULL);

	return NULL;
}

void
gl_plugin_reinit_actions                   ( GlPlugin *plugin )
{
	g_return_if_fail(plugin != NULL);
	g_return_if_fail(GL_IS_PLUGIN(plugin));

}

void
gl_plugin_reload_menu                      ( GlPlugin *plugin )
{
	g_return_if_fail(plugin != NULL);
	g_return_if_fail(GL_IS_PLUGIN(plugin));
}


void
gl_plugin_reinit ( GlPlugin *plugin )
{
	g_return_if_fail(plugin != NULL);
	g_return_if_fail(GL_IS_PLUGIN(plugin));
//	gl_plugin_reinit_widget  ( plugin ) ;
//	gl_plugin_reinit_menu    ( plugin ) ;
}


gboolean
gl_plugin_change_level_signal (GlLevelManager *manager , gpointer data)
{
	//TEST:g_debug("gl_plugin_change_level_signal");
	g_return_val_if_fail   ( GL_IS_PLUGIN (data),FALSE );
	gl_plugin_change_level ( GL_PLUGIN(data) );
	return TRUE;
}

gboolean
gl_plugin_is_level_accept ( GlPlugin *object )
{
	g_return_val_if_fail (object != NULL,FALSE);
	g_return_val_if_fail (GL_IS_PLUGIN (object),FALSE);
	return gl_level_manager_is_tru_user(mktAPGet(object,"level",uint));
}


gboolean
gl_plugin_is_search_muster   ( GlPlugin *object ,const  gchar *muster )
{
	g_return_val_if_fail (object != NULL,FALSE);
	g_return_val_if_fail (GL_IS_PLUGIN (object),FALSE);
	g_return_val_if_fail (muster != NULL,FALSE);
	gboolean ret = FALSE;
	const gchar* pmust = mktAPGet(object,"muster",string);
	//TEST:g_debug("Plugin muster=%s",pmust);
	gchar **mar = g_strsplit_set(pmust,";",-1);
	int i;
	for (i=0;(mar != NULL)  && (mar[i] != NULL);i++)
	{
		if(g_str_has_prefix(mar[i],muster)) ret = TRUE;
	}
	if(mar)g_strfreev(mar);
	return ret;
}



void
gl_plugin_set_action_widget_icon            ( GlPlugin *plugin , const gchar *icon )
{
	g_return_if_fail (plugin != NULL);
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	g_return_if_fail (icon != NULL);
	GL_PLUGIN_SET_ALL_ACTION(plugin,"icon",icon);
}

void
gl_plugin_set_action_widget_sensetive      ( GlPlugin *plugin , gboolean sens )
{
	g_return_if_fail (plugin != NULL);
	g_return_if_fail (GL_IS_PLUGIN (plugin));
	GL_PLUGIN_SET_ALL_ACTION(plugin,"sensetive",sens);
}

