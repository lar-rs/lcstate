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

#ifndef _GL_MAIN_BUILD_H_
#define _GL_MAIN_BUILD_H_

#include <glib-object.h>
#include "mkt-lib.h"
#include "gllib.h"


G_BEGIN_DECLS

#define GL_TYPE_MAIN_BUILD             (gl_main_build_get_type ())
#define GL_MAIN_BUILD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MAIN_BUILD, GlMainBuild))
#define GL_MAIN_BUILD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MAIN_BUILD, GlMainBuildClass))
#define GL_IS_MAIN_BUILD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MAIN_BUILD))
#define GL_IS_MAIN_BUILD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MAIN_BUILD))
#define GL_MAIN_BUILD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_MAIN_BUILD, GlMainBuildClass))

typedef struct _GlMainBuildClass   GlMainBuildClass;
typedef struct _GlMainBuild        GlMainBuild;
typedef struct _GlMainBuildPrivate GlMainBuildPrivate;

struct _GlMainBuildClass
{
	MktAtomClass           parent_class;

	/* Signals */
	void (* add_module) (GlMainBuild* build,gpointer user_data);
};

struct _GlMainBuild
{
	MktAtom                 parent_instance;
	GlMainBuildPrivate     *priv;
};

GType                gl_main_build_get_type                    ( void ) G_GNUC_CONST;

gboolean             gl_main_build_add_module_path             ( GlMainBuild *object,const gchar *modulpath );
gboolean             gl_main_build_load                        ( GlMainBuild *object );

void                 gl_main_build_run                         ( GlMainBuild* object );
gboolean             gl_main_build_load_module                 ( GlMainBuild *object,const gchar *modulpath );

gboolean             gl_main_build_destroy_module              ( GlMainBuild* object );



G_END_DECLS

#endif /* _GL_MAIN_BUILD_H_ */
