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

#ifndef _NODE_OBJECT_H_
#define _NODE_OBJECT_H_

#include "node-index.h"


G_BEGIN_DECLS

#define NODE_TYPE_OBJECT             (node_object_get_type ())
#define NODE_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NODE_TYPE_OBJECT, NodeObject))
#define NODE_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  NODE_TYPE_OBJECT, NodeObjectClass))
#define NODE_IS_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NODE_TYPE_OBJECT))
#define NODE_IS_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  NODE_TYPE_OBJECT))
#define NODE_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  NODE_TYPE_OBJECT, NodeObjectClass))

typedef struct _NodeObjectClass   NodeObjectClass;
typedef struct _NodeObject        NodeObject;
typedef struct _NodeObjectPrivate NodeObjectPrivate;


struct _NodeObjectClass
{
	GObjectClass parent_class;
};

struct _NodeObject
{
	GObect parent_instance;
	NodeObjectPrivate              *priv;
};

GType                   node_object_get_type                 (void) G_GNUC_CONST;

// GList*                  node_object_childs_index             ( NodeObject *object );
GValue*                 node_object_read_value               ( NodeObject *object, const gchar *index_id);
GValue*                 node_object_read_value_type_transform( NodeObject *object, const gchar *index_id , GType type);
gboolean                node_object_write_value              ( NodeObject *object, const gchar *index_id , const GValue *value);
NodeIndex*              node_object_lookup_index             ( NodeObject *object, const gchar *index_id );
void                    node_object_reseted                  ( NodeObject *object );
const gchar*            node_object_get_eds_path             ( NodeObject *object );

// Wurde nur in 'node_device_object_look_for_eds' aufgerufen. Die Funktion wurde
// gestrichen.
// void                    node_object_dup_to_file              ( NodeObject *object, const gchar *adjust, gboolean done  );

G_END_DECLS

#endif /* _NODE_OBJECT_H_ */
