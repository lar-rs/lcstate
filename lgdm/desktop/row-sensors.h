/**
 * @defgroup LgdmLibrary
 * @defgroup RowSensors
 * @ingroup  RowSensors
 * @{
 * @file  row-channel-info.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef ROW_SENSORS_H_
#define ROW_SENSORS_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gl-layout.h>
#include "row-service.h"


G_BEGIN_DECLS


#define ROW_TYPE_SENSORS    			           (row_sensors_get_type())
#define ROW_SENSORS(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),ROW_TYPE_SENSORS, RowSensors))
#define ROW_SENSORS_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,ROW_TYPE_SENSORS, RowSensorsClass))
#define ROW_IS_SENSORS(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),ROW_TYPE_SENSORS))
#define ROW_IS_SENSORS_CLASS(klass)               (G_TYPE_CHECK_CLASS_TYPE((klass) ,ROW_TYPE_SENSORS))

typedef struct _RowSensors			        RowSensors;
typedef struct _RowSensorsClass		        RowSensorsClass;
typedef struct _RowSensorsPrivate             RowSensorsPrivate;

struct _RowSensorsClass
{
	RowServiceClass                           parent_class;
};

struct _RowSensors
{
	RowService                                 parent;
	RowSensorsPrivate                       *priv;
};


GType 		         row_sensors_get_type                ( void );


G_END_DECLS
#endif /* ROW_SENSORS_H_ */

/** @} */
