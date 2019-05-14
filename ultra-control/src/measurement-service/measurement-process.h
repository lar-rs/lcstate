/**
 * @defgroup MeasurementBusLibrary
 * @defgroup MeasurementProcess
 * @ingroup  MeasurementProcess
 * @{
 * @file  measurement-process.h	Process object header
 * @brief   Process object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _MEASUREMENT_PROCESS_H_
#define _MEASUREMENT_PROCESS_H_

#include "ultimate-process-object.h"
#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define MEASUREMENT_TYPE_PROCESS (measurement_process_get_type())
#define MEASUREMENT_PROCESS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MEASUREMENT_TYPE_PROCESS, MeasurementProcess))
#define MEASUREMENT_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MEASUREMENT_TYPE_PROCESS, MeasurementProcessClass))
#define MEASUREMENT_IS_PROCESS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MEASUREMENT_TYPE_PROCESS))
#define MEASUREMENT_IS_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MEASUREMENT_TYPE_PROCESS))
#define MEASUREMENT_PROCESS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MEASUREMENT_TYPE_PROCESS, MeasurementProcessClass))

typedef struct _MeasurementProcessClass   MeasurementProcessClass;
typedef struct _MeasurementProcess        MeasurementProcess;
typedef struct _MeasurementProcessPrivate MeasurementProcessPrivate;

GType measurement_process_get_type(void) G_GNUC_CONST;

struct _MeasurementProcessClass {
    UltimateProcessObjectClass parent_class;
};

struct _MeasurementProcess {
    UltimateProcessObject      parent_instance;
    MeasurementProcessPrivate *priv;
};

gboolean measurement_process_check_range(MeasurementProcess *process, gdouble limit, const gchar *channel, gboolean up);

G_END_DECLS

#endif /* _MEASUREMENT_PROCESS_H_ */

/** @} */
