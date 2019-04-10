/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-manager.c
 * Copyright (C) Sascha 2011 <sascha@sascha-desktop>
 * 
gl-manager.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-manager.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_MANAGER_H_
#define _GL_MANAGER_H_

#include <glib-object.h>

#include "gl-level-manager.h"
#include "gl-connection.h"
#include "gl-plugin.h"
#include "gl-action-widget.h"

G_BEGIN_DECLS

#define GL_TYPE_MANAGER             (gl_manager_get_type ())
#define GL_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MANAGER, GlManager))
#define GL_MANAGER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MANAGER, GlManagerClass))
#define GL_IS_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MANAGER))
#define GL_IS_MANAGER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MANAGER))
#define GL_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_MANAGER, GlManagerClass))

typedef struct _GlManagerClass    GlManagerClass;
typedef struct _GlManager         GlManager;
typedef struct _GlManagerPrivate  GlManagerPrivate;

struct _GlManagerClass
{
	GtkEventBoxClass    parent_class;
	gboolean            (*pack_plugin)       (  GlManager *manager , GlPlugin *plugin , gboolean user_move);
	gboolean            (*remove_plugin)     (  GlManager *manager , GlPlugin *plugin , gboolean user_move);
	void                (*start_plugin)      (  GlManager *manager , GlPlugin *plugin);
	void                (*stop_plugin)       (  GlManager *manager , GlPlugin *plugin);
	void                (*start_plugin_menu) (  GlManager *manager , GlPlugin *plugin);
	void                (*stop_plugin_menu)  (  GlManager *manager , GlPlugin *plugin);

	void                (*add_connection )   (  GlManager *manager , GlConnection   *connection );
	void                (*add_level_manager) (  GlManager *manager , GlLevelManager *level);
	void                (*change_level ) 	 (  GlManager *manager , GlLevelManager *level);

};

struct _GlManager
{
	GtkEventBox           parent_instance;
	GlManagerPrivate     *priv;
};

GType           gl_manager_get_type                  (void) G_GNUC_CONST;

GlManager*      gl_manager_new                       ( gchar *place );

void            gl_manager_add_level_manager         ( GlManager *manager ,  GlLevelManager *level      );
GlLevelManager* gl_manager_get_level_manager         ( GlManager *manager );
void            gl_manager_add_connection            ( GlManager *manager ,  GlConnection   *connection );
GlConnection*   gl_manager_get_connection            ( GlManager *manager );

gboolean        gl_manager_pack_plugin_start         ( GlManager *manager ,  GlPlugin        *plugin , gboolean user_move );
gboolean        gl_manager_remove_plugin             ( GlManager *manager ,  GlPlugin        *plugin , gboolean user_move );

gboolean        gl_manager_is_plugin_active          ( GlManager *manager );

GlPlugin*       gl_manager_get_active_plugin         ( GlManager *manager );
GlPlugin*       gl_manager_get_active_plugin_menu    ( GlManager *manager );


const gchar*    gl_manager_get_place                 ( GlManager *manager );


G_END_DECLS

#endif /* _GL_MANAGER_H_ */
