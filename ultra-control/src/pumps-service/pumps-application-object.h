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

#ifndef __PUMPS_APPLICATION_OBJECT_H_
#define __PUMPS_APPLICATION_OBJECT_H_


#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>


G_BEGIN_DECLS



#define PUMPS_TYPE_APPLICATION_OBJECT             (pumps_application_object_get_type ())
#define PUMPS_APPLICATION_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PUMPS_TYPE_APPLICATION_OBJECT, PumpsApplicationObject))
#define PUMPS_APPLICATION_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  PUMPS_TYPE_APPLICATION_OBJECT, PumpsApplicationObjectClass))
#define PUMPS_IS_APPLICATION_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PUMPS_TYPE_APPLICATION_OBJECT))
#define PUMPS_IS_APPLICATION_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  PUMPS_TYPE_APPLICATION_OBJECT))
#define PUMPS_APPLICATION_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  PUMPS_TYPE_APPLICATION_OBJECT, PumpsApplicationObjectClass))

typedef struct _PumpsApplicationObjectClass         PumpsApplicationObjectClass;
typedef struct _PumpsApplicationObject              PumpsApplicationObject;
typedef struct _PumpsApplicationObjectPrivate       PumpsApplicationObjectPrivate;


struct _PumpsApplicationObjectClass
{
	TeraServiceObjectClass                                parent_class;

};

struct _PumpsApplicationObject
{
	TeraServiceObject                                        parent_instance;
	PumpsApplicationObjectPrivate                    *priv;
};


GType         pumps_application_object_get_type                  ( void );




G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */
