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

#ifndef __MEASUREMENT_APPLICATION_OBJECT_H_
#define __MEASUREMENT_APPLICATION_OBJECT_H_


#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>


G_BEGIN_DECLS



#define MEASUREMENT_TYPE_APPLICATION_OBJECT             (measurement_application_object_get_type ())
#define MEASUREMENT_APPLICATION_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MEASUREMENT_TYPE_APPLICATION_OBJECT, MeasurementApplicationObject))
#define MEASUREMENT_APPLICATION_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  MEASUREMENT_TYPE_APPLICATION_OBJECT, MeasurementApplicationObjectClass))
#define MEASUREMENT_IS_APPLICATION_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MEASUREMENT_TYPE_APPLICATION_OBJECT))
#define MEASUREMENT_IS_APPLICATION_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  MEASUREMENT_TYPE_APPLICATION_OBJECT))
#define MEASUREMENT_APPLICATION_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  MEASUREMENT_TYPE_APPLICATION_OBJECT, MeasurementApplicationObjectClass))

typedef struct _MeasurementApplicationObjectClass         MeasurementApplicationObjectClass;
typedef struct _MeasurementApplicationObject              MeasurementApplicationObject;
typedef struct _MeasurementApplicationObjectPrivate       MeasurementApplicationObjectPrivate;


struct _MeasurementApplicationObjectClass
{
	TeraServiceObjectClass                                parent_class;

};

struct _MeasurementApplicationObject
{
	TeraServiceObject                                        parent_instance;
	MeasurementApplicationObjectPrivate                    *priv;
};


GType                         measurement_application_object_get_type                  ( void );


G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */
