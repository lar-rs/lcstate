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

#ifndef __AXIS_APPLICATION_OBJECT_H_
#define __AXIS_APPLICATION_OBJECT_H_

#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

G_BEGIN_DECLS

#define AXIS_TYPE_APPLICATION_OBJECT (axis_application_object_get_type())
#define AXIS_APPLICATION_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), AXIS_TYPE_APPLICATION_OBJECT, AxisApplicationObject))
#define AXIS_APPLICATION_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), AXIS_TYPE_APPLICATION_OBJECT, AxisApplicationObjectClass))
#define AXIS_IS_APPLICATION_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), AXIS_TYPE_APPLICATION_OBJECT))
#define AXIS_IS_APPLICATION_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), AXIS_TYPE_APPLICATION_OBJECT))
#define AXIS_APPLICATION_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AXIS_TYPE_APPLICATION_OBJECT, AxisApplicationObjectClass))

typedef struct _AxisApplicationObjectClass   AxisApplicationObjectClass;
typedef struct _AxisApplicationObject        AxisApplicationObject;
typedef struct _AxisApplicationObjectPrivate AxisApplicationObjectPrivate;

struct _AxisApplicationObjectClass {
    TeraServiceObjectClass parent_class;
};

struct _AxisApplicationObject {
    TeraServiceObject             parent_instance;
    AxisApplicationObjectPrivate *priv;
};
GQuark domain_error_quark(void);

GType axis_application_object_get_type(void);

GDBusObjectManager *axis_application_get_object_manajer();

G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */
