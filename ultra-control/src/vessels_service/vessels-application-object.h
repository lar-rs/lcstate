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

#ifndef __VESSELS_APPLICATION_OBJECT_H_
#define __VESSELS_APPLICATION_OBJECT_H_


#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>


G_BEGIN_DECLS



#define VESSELS_TYPE_APPLICATION_OBJECT             (vessels_application_object_get_type ())
#define VESSELS_APPLICATION_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VESSELS_TYPE_APPLICATION_OBJECT, VesselsApplicationObject))
#define VESSELS_APPLICATION_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  VESSELS_TYPE_APPLICATION_OBJECT, VesselsApplicationObjectClass))
#define VESSELS_IS_APPLICATION_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VESSELS_TYPE_APPLICATION_OBJECT))
#define VESSELS_IS_APPLICATION_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  VESSELS_TYPE_APPLICATION_OBJECT))
#define VESSELS_APPLICATION_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  VESSELS_TYPE_APPLICATION_OBJECT, VesselsApplicationObjectClass))

typedef struct _VesselsApplicationObjectClass         VesselsApplicationObjectClass;
typedef struct _VesselsApplicationObject              VesselsApplicationObject;
typedef struct _VesselsApplicationObjectPrivate       VesselsApplicationObjectPrivate;


struct _VesselsApplicationObjectClass
{
	TeraServiceObjectClass                                   parent_class;

};

struct _VesselsApplicationObject
{
	TeraServiceObject                                        parent_instance;
	VesselsApplicationObjectPrivate                         *priv;
};


GType         vessels_application_object_get_type                  ( void );




G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */
