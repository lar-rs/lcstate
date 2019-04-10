/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-tree-data.c
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
gl-tree-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-tree-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_TREE_DATA_H_
#define _GL_TREE_DATA_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define GL_TYPE_TREE_DATA             (gl_tree_data_get_type ())
#define GL_TREE_DATA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_TREE_DATA, GlTreeData))
#define GL_TREE_DATA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_TREE_DATA, GlTreeDataClass))
#define GL_IS_TREE_DATA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_TREE_DATA))
#define GL_IS_TREE_DATA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_TREE_DATA))
#define GL_TREE_DATA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_TREE_DATA, GlTreeDataClass))

typedef struct _GlTreeDataClass GlTreeDataClass;
typedef struct _GlTreeData GlTreeData;

struct _GlTreeDataClass
{
	GObjectClass parent_class;
};

struct _GlTreeData
{
	GObject parent_instance;
};

GType gl_tree_data_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _GL_TREE_DATA_H_ */
