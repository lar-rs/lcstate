/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * node-index.h
 * Copyright (C) 2014 doseus <doseus@doseus-ThinkPad-T430s>
 *
 * largdm is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * largdm is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NODE_INDEX_OBJECT_H_
#define _NODE_INDEX_OBJECT_H_

#include <glib-object.h>
#include "node-index.h"

G_BEGIN_DECLS

#define NODE_TYPE_INDEX_OBJECT             (node_index_object_get_type ())
#define NODE_INDEX_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NODE_TYPE_INDEX_OBJECT, NodeIndexObject))
#define NODE_INDEX_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NODE_TYPE_INDEX_OBJECT, NodeIndexObjectClass))
#define NODE_IS_INDEX_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NODE_TYPE_INDEX_OBJECT))
#define NODE_IS_INDEX_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NODE_TYPE_INDEX_OBJECT))
#define NODE_INDEX_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NODE_TYPE_INDEX_OBJECT, NodeIndexObjectClass))

typedef struct _NodeIndexObjectClass   NodeIndexObjectClass;
typedef struct _NodeIndexObject        NodeIndexObject;
typedef struct _NodeIndexObjectPrivate NodeIndexObjectPrivate;


struct _NodeIndexObjectClass
{
	GObjectClass parent_class;
};

struct _NodeIndexObject
{
	GObject parent_instance;

	NodeIndexObjectPrivate *priv;
};

GType node_index_object_get_type (void) G_GNUC_CONST;


gchar*              node_index_object_dup_str  ( NodeIndexObject *index );
void                node_object_test_print     ( NodeIndexObject *index );
gboolean            node_index_object_toggle   ( NodeIndexObject *index, unsigned char *rdata, unsigned char *wdata  );
gchar*              node_index_dup_value       ( NodeIndexObject *index );
// Wurde nur in 'node_device_object_look_for_eds' benutzt. Diese Funktion ist
// aber gestrichen.
// void                node_index_object_status   ( NodeIndexObject *index, const gchar *format, ... )G_GNUC_PRINTF (2, 3);

G_END_DECLS

#endif /* _NODE_INDEX_OBJECT_H_ */

