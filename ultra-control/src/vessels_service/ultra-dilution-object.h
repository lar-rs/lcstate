/**
 * @defgroup UltraControl
 * @defgroup UltraDilutionObject
 * @ingroup  UltraDilutionObject
 * @{
 * @file  ultra-vessel-object.h	Vessel object header
 * @brief Vessel object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */

#ifndef __ULTRA_DILUTION_OBJECT_H_
#define __ULTRA_DILUTION_OBJECT_H_



#include <glib.h>
#include <gio/gio.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_DILUTION_OBJECT             (ultra_dilution_object_get_type ())
#define ULTRA_DILUTION_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_DILUTION_OBJECT, UltraDilutionObject))
#define ULTRA_DILUTION_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_DILUTION_OBJECT, UltraDilutionObjectClass))
#define ULTRA_IS_DILUTION_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_DILUTION_OBJECT))
#define ULTRA_IS_DILUTION_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_DILUTION_OBJECT))
#define ULTRA_DILUTION_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_DILUTION_OBJECT, UltraDilutionObjectClass))

typedef struct _UltraDilutionObjectClass                      UltraDilutionObjectClass;
typedef struct _UltraDilutionObject                           UltraDilutionObject;
typedef struct _UltraDilutionObjectPrivate                    UltraDilutionObjectPrivate;


struct _UltraDilutionObjectClass
{
	VesselsObjectSkeletonClass                                  parent_class;
};

struct _UltraDilutionObject
{
	VesselsObjectSkeleton                                       parent_instance;
	UltraDilutionObjectPrivate                                *priv;
};

GType                                     ultra_dilution_object_get_type                  ( void ) G_GNUC_CONST;



G_END_DECLS

#endif /* _ULTRA_DILUTION_OBJECT_H_ */


/** @} */

