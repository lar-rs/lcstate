/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-level-notebook.c
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 * 
 * gl-level-notebook.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-level-notebook.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_LEVEL_NOTEBOOK_H_
#define _GL_LEVEL_NOTEBOOK_H_

#include <gtk/gtk.h>
#include "mkt-window.h"



#define GL_LEVEL_NOTEBOOK_MAX_X 8
#define GL_LEVEL_NOTEBOOK_MAX_Y 5


G_BEGIN_DECLS

#define GL_TYPE_LEVEL_NOTEBOOK             (gl_level_notebook_get_type ())
#define GL_LEVEL_NOTEBOOK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LEVEL_NOTEBOOK, GlLevelNotebook))
#define GL_LEVEL_NOTEBOOK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_LEVEL_NOTEBOOK,  GlLevelNotebookClass))
#define GL_IS_LEVEL_NOTEBOOK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LEVEL_NOTEBOOK))
#define GL_IS_LEVEL_NOTEBOOK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_LEVEL_NOTEBOOK))
#define GL_LEVEL_NOTEBOOK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_LEVEL_NOTEBOOK, GlLevelNotebookClass))

typedef struct _GlLevelNotebookClass        GlLevelNotebookClass;
typedef struct _GlLevelNotebook             GlLevelNotebook;
typedef struct _GlLevelNotebookPrivate      GlLevelNotebookPrivate;

struct _GlLevelNotebookClass
{
	MktWindowClass                parent_class;
};

struct _GlLevelNotebook
{
	MktWindow                     parent_instance;
	GlLevelNotebookPrivate       *priv;
};

GType                 gl_level_notebook_get_type           ( void ) G_GNUC_CONST;

MktAtom*              gl_level_notebook_new                ( const gchar *id );
MktAtom*              gl_level_notebook_new_full           ( const gchar *id , guint x , guint y , guint width , guint height );

void                  gl_level_notebook_set_search         ( GlLevelNotebook *notebook, const gchar *muster );

void                  gl_level_notebook_show               ( GlLevelNotebook *notebook );

void                  gl_level_notebook_search_signal_cb   ( MktWindow * window, const gchar *text , GlLevelNotebook *lnotebook );

//void                  gl_level_notebook_start_plugin       ( GlLevelNotebook *notebook , GlPlugin *plugin );
//void                  gl_level_notebook_stop_plugin        ( GlLevelNotebook *notebook , GlPlugin *plugin );




G_END_DECLS

#endif /* _GL_LEVEL_NOTEBOOK_H_ */
