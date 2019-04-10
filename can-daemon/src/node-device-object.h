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

#ifndef _NODE_DEVICE_OBJECT_H_
#define _NODE_DEVICE_OBJECT_H_

#include "can4linux.h"
#include "node-index-object.h"
#include "node-index.h"
#include "node-object.h"
#include <mktbus.h>

/*
typedef enum {
    CAN_MSG_WRITE = 1 << 5,
    CAN_MSG_READ  = 1 << 6,
} NodeMsgType;
*/

G_BEGIN_DECLS

#define NODE_TYPE_DEVICE_OBJECT (node_device_object_get_type())
#define NODE_DEVICE_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), NODE_TYPE_DEVICE_OBJECT, NodeDeviceObject))
#define NODE_DEVICE_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), NODE_TYPE_DEVICE_OBJECT, NodeDeviceObjectClass))
#define NODE_IS_DEVICE_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), NODE_TYPE_DEVICE_OBJECT))
#define NODE_IS_DEVICE_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), NODE_TYPE_DEVICE_OBJECT))
#define NODE_DEVICE_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), NODE_TYPE_DEVICE_OBJECT, NodeDeviceObjectClass))

typedef struct _NodeDeviceObjectClass   NodeDeviceObjectClass;
typedef struct _NodeDeviceObject        NodeDeviceObject;
typedef struct _NodeDeviceObjectPrivate NodeDeviceObjectPrivate;

struct _NodeDeviceObjectClass {
    CandeviceObjectSkeletonClass parent_class;
};

struct _NodeDeviceObject {
    CandeviceObjectSkeleton  parent_instance;
    NodeDeviceObjectPrivate *priv;
};

GType node_device_object_get_type(void) G_GNUC_CONST;

gboolean node_device_object_open(NodeDeviceObject *device);
gboolean node_device_object_scan(NodeDeviceObject *device);
gboolean node_device_object_init_can(NodeDeviceObject *device);
gint node_device_object_read_index(NodeDeviceObject *device, NodeIndex *index);
gboolean node_device_object_write_index(NodeDeviceObject *device, NodeIndex *index);
gboolean node_device_object_insert_node(NodeDeviceObject *device, NodeObject *object);
void node_device_clean_report(NodeDeviceObject *device);
// gboolean node_device_object_scan (NodeDeviceObject* device); Wurde ersetzt durch 'node_device_init_ultra_nodes'
gboolean node_device_init_ultra_nodes(NodeDeviceObject *device);

G_END_DECLS

#endif /* _NODE_DEVICE_OBJECT_H_ */
