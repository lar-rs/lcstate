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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_TREE_DATA             (gl_tree_data_get_type ())
#define GL_TREE_DATA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_TREE_DATA, GlTreeData))
#define GL_TREE_DATA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_TREE_DATA, GlTreeDataClass))
#define GL_IS_TREE_DATA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_TREE_DATA))
#define GL_IS_TREE_DATA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_TREE_DATA))
#define GL_TREE_DATA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_TREE_DATA, GlTreeDataClass))

typedef struct _GlTreeDataClass     GlTreeDataClass;
typedef struct _GlTreeData          GlTreeData;
typedef struct _GlTreeDataPrivate   GlTreeDataPrivate;

enum {
	GL_TREE_DATA_TYPE_TEXT,
	GL_TREE_DATA_TYPE_TOGGLE
};

struct _GlTreeDataClass
{
	GObjectClass                parent_class;
	void          (*focus_in)    ( GlTreeData *data , GtkEntry *entry );
	void          (*changed )    ( GlTreeData *data );
	void          (*destroy )    ( GlTreeData *data );
};

struct _GlTreeData
{
	GObject                    parent_instance;
	GlTreeDataPrivate         *priv;
};

GType             gl_tree_data_get_type          (void) G_GNUC_CONST;

GlTreeData*       gl_tree_data_new               (GtkTreeView *tree,GtkCellRenderer   *renderer,GtkTreeIter *iter ,guint colum);
GlTreeData*       gl_tree_data_new_full          (GtkTreeView *tree,GtkCellRenderer   *renderer, GtkTreeIter *iter,guint colum ,gboolean editable);

const gchar*      gl_tree_data_get_text          (GlTreeData *data);
gboolean          gl_tree_data_set_text          (GlTreeData *data ,gchar *text);
gchar*            gl_tree_data_get_name          (GlTreeData *data);
void              gl_tree_data_set_name          (GlTreeData *data ,const gchar *name);
GtkWidget*        gl_tree_data_get_tree          (GlTreeData *data);
guint             gl_tree_data_get_cell_type     (GlTreeData *data);


G_END_DECLS

#endif /* _GL_TREE_DATA_H_ */
