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

#ifndef _GL_MODULE_H_
#define _GL_MODULE_H_

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include "gl-plugin.h"

G_BEGIN_DECLS

#define GL_TYPE_MODULE             (gl_module_get_type ())
#define GL_MODULE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MODULE, GlModule))
#define GL_MODULE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MODULE, GlModuleClass))
#define GL_IS_MODULE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MODULE))
#define GL_IS_MODULE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MODULE))
#define GL_MODULE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_MODULE, GlModuleClass))

typedef struct _GlModuleClass      GlModuleClass;
typedef struct _GlModule           GlModule;
typedef struct _GlModulePrivate    GlModulePrivate;

struct _GlModuleClass
{
	GTypeModuleClass     parent_class;
};

struct _GlModule
{
	GTypeModule          parent_instance;
	GlModulePrivate     *priv;
	void              (* load   )       (GlModule *module );
	void              (* unload )       (GlModule *module );

};

GType                           gl_module_get_type              ( void ) G_GNUC_CONST;

void                            gl_lar_module_load              ( GlModule *module );
void                            gl_lar_module_unload            ( GlModule *module );

GlModule*                       gl_module_new                   ( const gchar *filepath );

GlPlugin*                       gl_module_get_plugin            ( GlModule *module );

gchar*                          gl_module_get_name_from_path    ( const gchar *patch );
G_END_DECLS

#endif /* _GL_MODULE_H_ */
