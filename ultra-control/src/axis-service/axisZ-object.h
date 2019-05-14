/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup UltimateLibrary
 * @defgroup UltraAxisZObject
 * @ingroup  UltraAxisZObject
 * @{
 * @file  ultra-axisZ-object.h	AXIS object header
 * @brief This is AXIS object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef _ULTRA_AXISZ_OBJECT_H_
#define _ULTRA_AXISZ_OBJECT_H_

#include <mktlib.h>
#include <mktbus.h>
#include "axis-object.h"






G_BEGIN_DECLS

#define ULTRA_TYPE_AXISZ_OBJECT             (ultra_axisZ_object_get_type ())
#define ULTRA_AXISZ_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_AXISZ_OBJECT, UltraAxisZObject))
#define ULTRA_AXISZ_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_AXISZ_OBJECT, UltraAxisZObjectClass))
#define ULTRA_IS_AXISZ_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_AXISZ_OBJECT))
#define ULTRA_IS_AXISZ_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_AXISZ_OBJECT))
#define ULTRA_AXISZ_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_AXISZ_OBJECT, UltraAxisZObjectClass))

typedef struct _UltraAxisZObjectClass         UltraAxisZObjectClass;
typedef struct _UltraAxisZObject              UltraAxisZObject;
typedef struct _UltraAxisZObjectPrivate       UltraAxisZObjectPrivate;


struct _UltraAxisZObjectClass
{
	AxisObjectClass                            parent_class;
};

struct _UltraAxisZObject
{
	AxisObject                                 parent_instance;
	UltraAxisZObjectPrivate                   *priv;
};

GType                   ultra_axisZ_object_get_type                 ( void ) G_GNUC_CONST;

G_END_DECLS

#endif /* _ULTRA_AXISZ_OBJECT_H_ */
/** @} */
