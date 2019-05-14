/**
 * @defgroup MktBusLibrary
 * @defgroup MktStirrerObject
 * @ingroup  MktStirrerObject
 * @{
 * @file  mkt-stirrer-object.h	Valve object header
 * @brief Stirrer object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */

#ifndef _MKT_STIRRER_OBJECT_H_
#define _MKT_STIRRER_OBJECT_H_



#include <glib.h>
#include <gio/gio.h>
#include "stirrers-generated-code.h"

G_BEGIN_DECLS

#define MKT_TYPE_STIRRER_OBJECT             (mkt_stirrer_object_get_type ())
#define MKT_STIRRER_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MKT_TYPE_STIRRER_OBJECT, MktStirrerObject))
#define MKT_STIRRER_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  MKT_TYPE_STIRRER_OBJECT, MktStirrerObjectClass))
#define MKT_IS_STIRRER_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MKT_TYPE_STIRRER_OBJECT))
#define MKT_IS_STIRRER_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  MKT_TYPE_STIRRER_OBJECT))
#define MKT_STIRRER_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  MKT_TYPE_STIRRER_OBJECT, MktStirrerObjectClass))

typedef struct _MktStirrerObjectClass         MktStirrerObjectClass;
typedef struct _MktStirrerObject              MktStirrerObject;
typedef struct _MktStirrerObjectPrivate       MktStirrerObjectPrivate;


struct _MktStirrerObjectClass
{
	StirrersObjectSkeletonClass               parent_class;
	gboolean                                (*initialize)                             ( MktStirrerObject *self );

};

struct _MktStirrerObject
{
	StirrersObjectSkeleton                              parent_instance;
	MktStirrerObjectPrivate                            *priv;
};

GType                                    mkt_stirrer_object_get_type                  ( void ) G_GNUC_CONST;
gboolean                                 mkt_stirrer_object_reset                     ( MktStirrerObject *stirrer_object );

G_END_DECLS

#endif /* _MKT_STIRRER_OBJECT_H_ */


/** @} */

