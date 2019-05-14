/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup XYSystem
 * @ingroup  XYSystem
 * @{
 * @file  D3SENSOR-object.h	D3SENSOR object header
 * @brief This is D3SENSOR object header file.
 * 	 Copyright (C) LAR 2016
 *
 * @author A.Smolkov  <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */
#ifndef _D3SENSOR_OBJECT_H_
#define _D3SENSOR_OBJECT_H_

#include "move-object.h"
#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define D3SENSOR_TYPE (D3SENSOR_get_type())
#define D3SENSOR_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), D3SENSOR_TYPE, D3SENSOR))
#define D3SENSOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), D3SENSOR_TYPE, D3SENSORClass))
#define D3SENSOR_IS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), D3SENSOR_TYPE))
#define D3SENSOR_IS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), D3SENSOR_TYPE))
#define D3SENSOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), D3SENSOR_TYPE, D3SENSORClass))

typedef struct _D3SENSORClass   D3SENSORClass;
typedef struct _D3SENSOR        D3SENSOR;
typedef struct _D3SENSORPrivate D3SENSORPrivate;

struct _D3SENSORClass {
    MOVEClass parent_class;
};

struct _D3SENSOR {
    MOVE             parent_instance;
    D3SENSORPrivate *priv;
};

GType D3SENSOR_get_type(void) G_GNUC_CONST;

MOVE *d3sensor_new(AxisObject *axis, guint part);
void d3sensor_run(D3SENSOR *d3sensor, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);

G_END_DECLS

#endif /* _D3SENSOR_OBJECT_H_ */
/** @} */
