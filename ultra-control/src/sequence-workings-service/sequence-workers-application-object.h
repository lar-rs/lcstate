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

#ifndef __SEQUENCE_WORKERS_APPLICATION_OBJECT_H_
#define __SEQUENCE_WORKERS_APPLICATION_OBJECT_H_


#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>

G_BEGIN_DECLS



#define SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT             (sequence_application_workers_object_get_type ())
#define SEQUENCE_WORKERS_APPLICATION_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT, SequenceApplicationWorkersObject))
#define SEQUENCE_WORKERS_APPLICATION_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT, SequenceApplicationWorkersObjectClass))
#define SEQUENCE_IS_WORKERS_APPLICATION_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT))
#define SEQUENCE_IS_WORKERS_APPLICATION_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT))
#define SEQUENCE_WORKERS_APPLICATION_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT, SequenceApplicationWorkersObjectClass))

typedef struct _SequenceApplicationWorkersObjectClass         SequenceApplicationWorkersObjectClass;
typedef struct _SequenceApplicationWorkersObject              SequenceApplicationWorkersObject;
typedef struct _SequenceApplicationWorkersObjectPrivate       SequenceApplicationWorkersObjectPrivate;


struct _SequenceApplicationWorkersObjectClass
{
	TeraServiceObjectClass                                          parent_class;

};

struct _SequenceApplicationWorkersObject
{
	TeraServiceObject                                                parent_instance;
	SequenceApplicationWorkersObjectPrivate                    *priv;
};


GType            sequence_application_workers_object_get_type                  ( void );


GCancellable*    sequence_application_cancelable                               ( void );
void             sequence_application_cancelable_init                          ( void );
void             sequence_application_cancelable_cancel                        ( void );




G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */
