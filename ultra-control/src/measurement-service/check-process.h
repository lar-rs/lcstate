/**
 * @defgroup CheckBusLibrary
 * @defgroup CheckProcess
 * @ingroup  CheckProcess
 * @{
 * @file  check-process.h	Process object header
 * @brief   Process object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _CHECK_PROCESS_H_
#define _CHECK_PROCESS_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultimate-process-object.h"

G_BEGIN_DECLS

#define CHECK_TYPE_PROCESS             (check_process_get_type ())
#define CHECK_PROCESS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHECK_TYPE_PROCESS, CheckProcess))
#define CHECK_PROCESS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CHECK_TYPE_PROCESS, CheckProcessClass))
#define CHECK_IS_PROCESS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHECK_TYPE_PROCESS))
#define CHECK_IS_PROCESS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHECK_TYPE_PROCESS))
#define CHECK_PROCESS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CHECK_TYPE_PROCESS, CheckProcessClass))

typedef struct _CheckProcessClass     CheckProcessClass;
typedef struct _CheckProcess          CheckProcess;
typedef struct _CheckProcessPrivate   CheckProcessPrivate;

GType check_process_get_type (void) G_GNUC_CONST;


struct _CheckProcessClass
{
	UltimateProcessObjectClass            parent_class;
};

struct _CheckProcess
{
	UltimateProcessObject                 parent_instance;
	CheckProcessPrivate                 *priv;
};




G_END_DECLS

#endif /* _CHECK_PROCESS_H_ */

/** @} */
