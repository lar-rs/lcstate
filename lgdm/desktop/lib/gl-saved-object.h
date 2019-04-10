/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 * 
mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_SAVED_OBJECT_H_
#define _GL_SAVED_OBJECT_H_

#include "mkt-model.h"
#include "mkt-item.h"
#include "mkt-item-object.h"
#include "gl-saved.h"


G_BEGIN_DECLS

#define GL_TYPE_SAVED_OBJECT             (gl_saved_object_get_type ())
#define GL_SAVED_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_SAVED_OBJECT, GlSavedObject))
#define GL_SAVED_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_SAVED_OBJECT, GlSavedObjectClass))
#define GL_IS_SAVED_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_SAVED_OBJECT))
#define GL_IS_SAVED_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_SAVED_OBJECT))
#define GL_SAVED_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_SAVED_OBJECT, GlSavedObjectClass))

typedef struct _GlSavedObjectClass   GlSavedObjectClass;
typedef struct _GlSavedObject        GlSavedObject;
typedef struct _GlSavedObjectPrivate GlSavedObjectPrivate;


struct _GlSavedObjectClass
{
	MktItemObjectClass    parent_class;
};

struct _GlSavedObject
{
	MktItemObject         parent_instance;
	GlSavedObjectPrivate *priv;
};

GType                     gl_saved_object_get_type                 (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _GL_SAVED_OBJECT_H_ */
