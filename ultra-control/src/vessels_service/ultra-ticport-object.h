/**
 * @defgroup Ultra
 * @defgroup UltraTicportObject
 * @ingroup  UltraTicportObject
 * @{
 * @file  ultra-ticport-object.h	Ticport object header
 * @brief        Ticport object header file.
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author        A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _ULTRA_TICPORT_OBJECT_H_
#define _ULTRA_TICPORT_OBJECT_H_

#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define TICPORT_IS_OPEN_bit (8)   /* Y6S1 */
#define TICPORT_IS_CLOSED_bit (9) /* Y6S2 */
// TIC DC-motor open and close

#define TICPORT_OPEN_bit (6)
#define TICPORT_CLOSE_bit (7)

#define ULTRA_TYPE_TICPORT_OBJECT (ultra_ticport_object_get_type())
#define ULTRA_TICPORT_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_TICPORT_OBJECT, UltraTicportObject))
#define ULTRA_TICPORT_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_TICPORT_OBJECT, UltraTicportObjectClass))
#define ULTRA_IS_TICPORT_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_TICPORT_OBJECT))
#define ULTRA_IS_TICPORT_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_TICPORT_OBJECT))
#define ULTRA_TICPORT_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_TICPORT_OBJECT, UltraTicportObjectClass))

typedef struct _UltraTicportObjectClass   UltraTicportObjectClass;
typedef struct _UltraTicportObject        UltraTicportObject;
typedef struct _UltraTicportObjectPrivate UltraTicportObjectPrivate;

struct _UltraTicportObjectClass {
    VesselsObjectSkeletonClass parent_class;
};

struct _UltraTicportObject {
    VesselsObjectSkeleton      parent_instance;
    UltraTicportObjectPrivate *priv;
};

GType           ultra_ticport_object_get_type(void) G_GNUC_CONST;
NodesDigital16 *TICPORT_DIGITAL1(UltraTicportObject *ticport);
NodesDigital16 *TICPORT_DIGITAL2(UltraTicportObject *ticport);
void TICPORT_SET_OPEN(UltraTicportObject *object, gboolean value);
void TICPORT_SET_BUSY(UltraTicportObject *object, gboolean value);
G_END_DECLS

#endif /* _ULTRA_TICPORT_OBJECT_H_ */

/** @} */
