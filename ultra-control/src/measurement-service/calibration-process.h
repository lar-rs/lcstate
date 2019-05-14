/**
 * @defgroup CalibrationBusLibrary
 * @defgroup CalibrationProcess
 * @ingroup  CalibrationProcess
 * @{
 * @file  single-process.h	Process object header
 * @brief   Process object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _CALIBRATION_PROCESS_H_
#define _CALIBRATION_PROCESS_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultimate-process-object.h"

G_BEGIN_DECLS

#define CALIBRATION_TYPE_PROCESS             (calibration_process_get_type ())
#define CALIBRATION_PROCESS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CALIBRATION_TYPE_PROCESS, CalibrationProcess))
#define CALIBRATION_PROCESS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CALIBRATION_TYPE_PROCESS, CalibrationProcessClass))
#define CALIBRATION_IS_PROCESS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CALIBRATION_TYPE_PROCESS))
#define CALIBRATION_IS_PROCESS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CALIBRATION_TYPE_PROCESS))
#define CALIBRATION_PROCESS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CALIBRATION_TYPE_PROCESS, CalibrationProcessClass))

typedef struct _CalibrationProcessClass     CalibrationProcessClass;
typedef struct _CalibrationProcess          CalibrationProcess;
typedef struct _CalibrationProcessPrivate   CalibrationProcessPrivate;

GType calibration_process_get_type (void) G_GNUC_CONST;


struct _CalibrationProcessClass
{
	UltimateProcessObjectClass                  parent_class;
};

struct _CalibrationProcess
{
	UltimateProcessObject                       parent_instance;
	CalibrationProcessPrivate                  *priv;
};




G_END_DECLS

#endif /* _CALIBRATION_PROCESS_H_ */

/** @} */
