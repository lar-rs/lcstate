/**
 * @defgroup Ultra
 * @defgroup UltraFurnaceObject
 * @ingroup  UltraFurnaceObject
 * @{
 * @file  ultra-furnace-object.h	Furnace object header
 * @brief        Furnace object header file.
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author        A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _ULTRA_FURNACE_OBJECT_H_
#define _ULTRA_FURNACE_OBJECT_H_

#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

G_BEGIN_DECLS

#define FURNACE_IS_OPEN_bit ((guint)1)         /* Y6S1 */
#define FURNACE_IS_CLOSED_bit ((guint)2)       /* Y6S2 */
#define FURNACE_IS_OUT_OF_RANGE_bit ((guint)7) /* N2 */
#define FURNACE_IS_DEAD_bit ((guint)8)         /* N2 */
#define COOLER_IS_OUT_OF_RANGE_bit ((guint)4)  /* EC */

#define FURNACE_OPEN_bit ((guint)1)
#define FURNACE_CLOSE_bit ((guint)2)
#define FURNACE_ON_bit ((guint)6)

#define CHECK_FUNC_WAIT_SECONDS ((guint)10)

#define DIGITAL1_PATH "/com/lar/nodes/Digital1"

#define ULTRA_TYPE_FURNACE_OBJECT (ultra_furnace_object_get_type())
#define ULTRA_FURNACE_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_FURNACE_OBJECT, UltraFurnaceObject))
#define ULTRA_FURNACE_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_FURNACE_OBJECT, UltraFurnaceObjectClass))
#define ULTRA_IS_FURNACE_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_FURNACE_OBJECT))
#define ULTRA_IS_FURNACE_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_FURNACE_OBJECT))
#define ULTRA_FURNACE_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_FURNACE_OBJECT, UltraFurnaceObjectClass))

typedef struct _UltraFurnaceObjectClass   UltraFurnaceObjectClass;
typedef struct _UltraFurnaceObject        UltraFurnaceObject;
typedef struct _UltraFurnaceObjectPrivate UltraFurnaceObjectPrivate;

struct _UltraFurnaceObjectClass {
    VesselsObjectSkeletonClass parent_class;
};

struct _UltraFurnaceObject {
    VesselsObjectSkeleton      parent_instance;
    UltraFurnaceObjectPrivate *priv;
};

GType ultra_furnace_object_get_type(void) G_GNUC_CONST;

NodesDigital16 *FURNACE_DIGITAL1(UltraFurnaceObject *furnace);
void FURNACE_SET_BUSY(UltraFurnaceObject *furnace, gboolean is_busy);
void FURNACE_SET_OPEN(UltraFurnaceObject *furnace, gboolean value);
G_END_DECLS

#endif /* _ULTRA_FURNACE_OBJECT_H_ */

/** @} */
