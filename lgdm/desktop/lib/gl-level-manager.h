/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-level-manager.c
 * Copyright (C) Sascha 2011 <sascha@sascha-desktop>
 * 
gl-level-manager.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-level-manager.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_LEVEL_MANAGER_H_
#define _GL_LEVEL_MANAGER_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "mkt-atom.h"

G_BEGIN_DECLS

#define GL_TYPE_LEVEL_MANAGER             (gl_level_manager_get_type ())
#define GL_LEVEL_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LEVEL_MANAGER, GlLevelManager))
#define GL_LEVEL_MANAGER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LEVEL_MANAGER, GlLevelManagerClass))
#define GL_IS_LEVEL_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LEVEL_MANAGER))
#define GL_IS_LEVEL_MANAGER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LEVEL_MANAGER))
#define GL_LEVEL_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_LEVEL_MANAGER, GlLevelManagerClass))

typedef enum
{
	GUI_USER_DEVICE_TYPE         = 0 ,
	GUI_USER_OPERATOR_TYPE       = 1,
	GUI_USER_SUPER_OPERATOR_TYPE = 2,
	GUI_USER_SERVICE_TYPE        = 3,
	GUI_USER_ROOT_TYPE           = 4,
	GUI_USER_LAST_TYPE           = 5
}GlLevelManagerUserType;


typedef struct _GlLevelManagerClass        GlLevelManagerClass;
typedef struct _GlLevelManager             GlLevelManager;
typedef struct _GlLevelManagerPrivate      GlLevelManagerPrivate;

struct _GlLevelManagerClass
{
	MktAtomClass    parent_class;

	void           (*change_gui_level )     (GlLevelManager * manager);
	void           (*mount_usb)             (GlLevelManager * manager);
	void           (*umount_usb)            (GlLevelManager * manager);
	void           (*key_open  )            (GlLevelManager * manager);
	void           (*key_close )            (GlLevelManager * manager);
};

struct _GlLevelManager
{
	MktAtom                  parent_instance;
	GlLevelManagerPrivate   *priv;
};

GType               gl_level_manager_get_type                   ( void ) G_GNUC_CONST;

GlLevelManager*     gl_level_manager_new                        ( );

GlLevelManager*     gl_level_manager_get_static                 ( );

void                gl_level_manager_load                       ( );

gboolean            gl_level_manager_is_tru_user                ( GlLevelManagerUserType user_type);

gboolean            gl_level_manager_key_open_close             (  );

guint               gl_level_manager_get_file_level_type        ( const gchar *filename );

guint               gl_level_manager_get_level_type_from_name   ( gchar *level_name );

void                gl_level_manager_set_load_module_fraction   ( gdouble fraction );

gchar*              gl_level_manager_get_level_name_from_id     ( guint level );
gchar*              gl_level_manager_get_name_for_noob_from_id  ( guint level );

void                gl_level_manager_system_signal_handler      ( int signum );

gchar*              gl_level_manager_get_level_name_for_noob    ( );


void                gl_level_manager_mount_usb_force            ( );
void                gl_level_manager_umount_usb_force           ( );


G_END_DECLS

#endif /* _GL_LEVEL_MANAGER_H_ */
