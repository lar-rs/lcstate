/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-security-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
mkt-security-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-security-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SECURITY_APPLICATION_OBJECT_H_
#define __SECURITY_APPLICATION_OBJECT_H_


#include <mktlib.h>





#define SECURITY_TYPE_APPLICATION_OBJECT             (security_application_object_get_type ())
#define SECURITY_APPLICATION_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SECURITY_TYPE_APPLICATION_OBJECT, SecurityApplicationObject))
#define SECURITY_APPLICATION_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  SECURITY_TYPE_APPLICATION_OBJECT, SecurityApplicationObjectClass))
#define SECURITY_IS_APPLICATION_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SECURITY_TYPE_APPLICATION_OBJECT))
#define SECURITY_IS_APPLICATION_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  SECURITY_TYPE_APPLICATION_OBJECT))
#define SECURITY_APPLICATION_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  SECURITY_TYPE_APPLICATION_OBJECT, SecurityApplicationObjectClass))

typedef struct _SecurityApplicationObjectClass         SecurityApplicationObjectClass;
typedef struct _SecurityApplicationObject              SecurityApplicationObject;
typedef struct _SecurityApplicationObjectPrivate       SecurityApplicationObjectPrivate;


struct _SecurityApplicationObjectClass
{
	TeraServiceObjectClass                               parent_class;

};

struct _SecurityApplicationObject
{
	TeraServiceObject                                    parent_instance;
	SecurityApplicationObjectPrivate                    *priv;
};


GType                         security_application_object_get_type                  ( void );


G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */
