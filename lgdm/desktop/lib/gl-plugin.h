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

#ifndef _GL_PLUGIN_H_
#define _GL_PLUGIN_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "gl-level-manager.h"
#include "gl-connection.h"
#include "gl-indicate.h"
#include "mkt-window.h"


G_BEGIN_DECLS

#define GL_TYPE_PLUGIN             (gl_plugin_get_type ())
#define GL_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PLUGIN, GlPlugin))
#define GL_PLUGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PLUGIN, GlPluginClass))
#define GL_IS_PLUGIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PLUGIN))
#define GL_IS_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PLUGIN))
#define GL_PLUGIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_PLUGIN, GlPluginClass))

typedef struct _GlPluginClass             GlPluginClass;
typedef struct _GlPlugin                  GlPlugin;
typedef struct _GlPluginPrivate           GlPluginPrivate;
typedef struct _GlPluginWidgetOptions     GlPluginWidgetOptions;

#define GL_PLUGIN_DEFAULT_SIZE   "default"
#define GL_PLUGIN_SMALL_SIZE     "small"
#define GL_PLUGIN_LARGE_SIZE     "large"
#define GL_PLUGIN_IS_SIZE(s,size)  (s?0==g_strcmp(s,size):FALSE)


#define GL_PLUGIN_IS_LOAD(plugin)  (GL_IS_PLUGIN(plugin)?plugin->isLoad:FALSE)





typedef enum
{
	GL_PLUGIN_MODULE_GROUP_NAME,
	GL_PLUGIN_MODULE_GROUP_SETTING,
	GL_PLUGIN_MAIN_LEVEL,
	GL_PLUGIN_MAIN_LEVEL_CONST,
	GL_PLUGIN_WIDGET_POSITION,
	GL_PLUGIN_WIDGET_POSITION_TYPE,
	GL_PLUGIN_WIDGET_SIZE,
	GL_PLUGIN_MENU_POSITION_TYPE,
}GlPluginWidgetProperty;


#define GL_PLUGIN_KEY_FILE_WIDGET = "widget"

struct _GlPluginClass
{
	MktWindowClass          parent_class;

	gboolean   (*plugin_init_menu       )            ( GlPlugin *plugin );
	gboolean   (*plugin_load_menu       )            ( GlPlugin *plugin );
	gboolean   (*plugin_reload_menu     )            ( GlPlugin *plugin );

//	gboolean   (*plugin_resize_widget)               (GlPlugin *plugin,GtkWidget *widget);
//	gboolean   (*plugin_resize_menu)                 (GlPlugin *plugin,GtkWidget *widget);

	gboolean   (*init_plugin  )                      ( GlPlugin *plugin );
	gboolean   (*load_plugin  )                      ( GlPlugin *plugin );
	void       (*open_plugin  )                      ( GlPlugin *plugin );
	void       (*close_plugin )                      ( GlPlugin *plugin );
	void       (*open_menu    )                      ( GlPlugin *plugin );
	void       (*close_menu   )                      ( GlPlugin *plugin );


	const gchar*     (*version_string)                     ( GlPlugin *plugin );
	const gchar*     (*package_string)                     ( GlPlugin *plugin );


	void       (*load_plugin_reserved1 )  (GlPlugin *plugin);
	void       (*load_plugin_reserved2 )  (GlPlugin *plugin);
	void       (*load_plugin_reserved3 )  (GlPlugin *plugin);
	//gboolean   (*start_plugin) (GlPlugin *plugin);

};


struct _GlPlugin
{
	MktWindow                 parent_instance;
	GlPluginPrivate          *priv;
	gboolean                  isLoad;
};

GType                 gl_plugin_get_type                          ( void ) G_GNUC_CONST;

const GValue*         gl_plugin_get_parameter                     ( GlPlugin *plugin , const gchar *parameter );
GParamSpec*           gl_plugin_get_param_spec                    ( GlPlugin *plugin , const gchar *parameter );

GlPlugin*             gl_plugin_new                               ( const gchar *path );

gboolean              gl_plugin_initialize                        ( GlPlugin *plugin );
gboolean              gl_plugin_load                              ( GlPlugin *plugin );
void                  gl_plugin_load_reserved                     ( GlPlugin *plugin );

void                  gl_plugin_open                              ( GlPlugin *plugin );
void                  gl_plugin_close                             ( GlPlugin *plugin );
void                  gl_plugin_open_menu                         ( GlPlugin *plugin );
void                  gl_plugin_close_menu                        ( GlPlugin *plugin );
void                  gl_plugin_change_level                      ( GlPlugin *plugin );

MktAtom  *            gl_plugin_create_action                     ( GlPlugin *plugin, const gchar *nick, gboolean active );

void                  gl_plugin_save_configuration                ( GlPlugin *plugin );


void                  gl_plugin_add_level_manager                 ( GlPlugin *object, GlLevelManager *manager    );

void                  gl_plugin_add_connection                    ( GlPlugin *object, GlConnection   *connection );

gboolean              gl_plugin_free_collection                   ( GlPlugin *plugin );

gboolean              gl_plugin_is_level_accept                   ( GlPlugin *plugin );

gboolean              gl_plugin_is_search_muster                  ( GlPlugin *plugin , const gchar *muster );

void                  gl_plugin_set_action_widget_icon            ( GlPlugin *plugin , const gchar *icon );
void                  gl_plugin_set_action_widget_sensetive       ( GlPlugin *plugin , gboolean sens );


const gchar*          gl_plugin_get_version                       ( GlPlugin *plugin );
const gchar*          gl_plugin_get_package                       ( GlPlugin *plugin );


// Gtk-Library  functions

GtkWidget*            gl_plugin_get_gtk_status                    ( GlPlugin *plugin );


//GtkWidget*            gl_plugin_get_status_bar                    ( GlPlugin *object )  ;
//GtkWidget*            gl_plugin_get_status_full                   ( GlPlugin *object )  ;


//void         gl_plugin_add_status_bar_referenz          ( GlPlugin *status_bar,GtkWidget *status_bar);





G_END_DECLS

#endif /* _GL_PLUGIN_H_ */
