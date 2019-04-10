/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mktlibrary
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 * 
mktlibrary is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * mktlibrary is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NODE_MOTOR_OBJECT_H_
#define _NODE_MOTOR_OBJECT_H_

#include "node-object.h"
#include <mktbus.h>



G_BEGIN_DECLS

#define NODE_TYPE_MOTOR_OBJECT             (node_motor_object_get_type ())
#define NODE_MOTOR_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NODE_TYPE_MOTOR_OBJECT,  NodeMotorObject))
#define NODE_MOTOR_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  NODE_TYPE_MOTOR_OBJECT,  NodeMotorObjectClass))
#define NODE_IS_MOTOR_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NODE_TYPE_MOTOR_OBJECT))
#define NODE_IS_MOTOR_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  NODE_TYPE_MOTOR_OBJECT))
#define NODE_MOTOR_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  NODE_TYPE_MOTOR_OBJECT,  NodeMotorObjectClass))

typedef struct _NodeMotorObjectClass   NodeMotorObjectClass;
typedef struct _NodeMotorObject        NodeMotorObject;
typedef struct _NodeMotorObjectPrivate NodeMotorObjectPrivate;


struct _NodeMotorObjectClass
{
	NodeObjectClass         parent_class;
};

struct _NodeMotorObject
{
	NodeObject              parent_instance;
    NodeMotorObjectPrivate *priv;
};

GType                         node_motor_object_get_type           ( void ) G_GNUC_CONST;


G_END_DECLS

#endif /* _NODE_MOTOR_OBJECT_H_ */
