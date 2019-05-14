/**
 * @defgroup RinsingBusLibrary
 * @defgroup RinsingTask
 * @ingroup  RinsingTask
 * @{
 * @file  rinsing-task.h	Task object header
 * @brief   Task object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef __RINSING_TASK_H__
#define __RINSING_TASK_H__

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define RINSING_TYPE_TASK             (rinsing_task_get_type ())
#define RINSING_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RINSING_TYPE_TASK, RinsingTask))
#define RINSING_TASK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RINSING_TYPE_TASK, RinsingTaskClass))
#define RINSING_IS_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RINSING_TYPE_TASK))
#define RINSING_IS_TASK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RINSING_TYPE_TASK))
#define RINSING_TASK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RINSING_TYPE_TASK, RinsingTaskClass))

typedef struct _RinsingTaskClass     RinsingTaskClass;
typedef struct _RinsingTask          RinsingTask;
typedef struct _RinsingTaskPrivate   RinsingTaskPrivate;

GType rinsing_task_get_type (void) G_GNUC_CONST;


struct _RinsingTaskClass
{
	MktTaskClass          parent_class;
};

struct _RinsingTask
{
	MktTask               parent_instance;
	RinsingTaskPrivate   *priv;
};

void               rinsing_task_set_channels    ( RinsingTask *task , GList *channels );




gboolean           rinsing_task_is_tic          ( RinsingTask *task );

G_END_DECLS

#endif /* _RINSING_TASK_H_ */

/** @} */
