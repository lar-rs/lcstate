/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-translation.c
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 * 
 * gl-translation.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-translation.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_TRANSLATION_H_
#define _GL_TRANSLATION_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "mkt-atom.h"
#include "market-translation.h"

G_BEGIN_DECLS

#define GL_TYPE_TRANSLATION             (gl_translation_get_type ())
#define GL_TRANSLATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_TRANSLATION, GlTranslation))
#define GL_TRANSLATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_TRANSLATION, GlTranslationClass))
#define GL_IS_TRANSLATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_TRANSLATION))
#define GL_IS_TRANSLATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_TRANSLATION))
#define GL_TRANSLATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_TRANSLATION, GlTranslationClass))

typedef struct _GlTranslationClass GlTranslationClass;
typedef struct _GlTranslation GlTranslation;
typedef struct _GlTranslationPrivate GlTranslationPrivate;

struct _GlTranslationClass
{
	MktAtomClass                  parent_class;
	void                        (*translate  )                     ( GlTranslation *translation );
};

struct _GlTranslation
{
	MktAtom                        parent_instance;
	GlTranslationPrivate          *priv;
};

GType                       gl_translation_get_type             ( void ) G_GNUC_CONST;
GlTranslation*              gl_translation_new                  ( const gchar *id , const gchar *path );
GtkWidget*                  gl_translation_get_combobox         ( GlTranslation *translation );

G_END_DECLS

#endif /* _GL_TRANSLATION_H_ */
