/**
 * @defgroup UltraControl
 * @defgroup UltraVesselObject
 * @ingroup  UltraVesselObject
 * @{
 * @file  ultra-vessel-object.h	Vessel object header
 * @brief Vessel object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */

#ifndef __ULTRA_VESSEL_OBJECT_H_
#define __ULTRA_VESSEL_OBJECT_H_



#include <glib.h>
#include <gio/gio.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_VESSEL_OBJECT             (ultra_vessel_object_get_type ())
#define ULTRA_VESSEL_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_VESSEL_OBJECT, UltraVesselObject))
#define ULTRA_VESSEL_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_VESSEL_OBJECT, UltraVesselObjectClass))
#define ULTRA_IS_VESSEL_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_VESSEL_OBJECT))
#define ULTRA_IS_VESSEL_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_VESSEL_OBJECT))
#define ULTRA_VESSEL_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_VESSEL_OBJECT, UltraVesselObjectClass))

typedef struct _UltraVesselObjectClass                      UltraVesselObjectClass;
typedef struct _UltraVesselObject                           UltraVesselObject;
typedef struct _UltraVesselObjectPrivate                    UltraVesselObjectPrivate;


struct _UltraVesselObjectClass
{
	VesselsObjectSkeletonClass                                  parent_class;
};

struct _UltraVesselObject
{
	VesselsObjectSkeleton                                       parent_instance;
	UltraVesselObjectPrivate                                *priv;
};

GType                                     ultra_vessel_object_get_type                  ( void ) G_GNUC_CONST;



G_END_DECLS

#endif /* _ULTRA_VESSEL_OBJECT_H_ */


/** @} */

