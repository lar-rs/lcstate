/**
pr * @defgroup PrepareBusLibrary
 * @defgroup PrepareTask
 * @ingroup  PrepareTask
 * @{
 * @file  prepare-task.h	Task object header
 * @brief   Task object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _PREPARE_TASK_H_
#define _PREPARE_TASK_H_

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define PREPARE_TYPE_TASK (prepare_task_get_type())
#define PREPARE_TASK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PREPARE_TYPE_TASK, PrepareTask))
#define PREPARE_TASK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), PREPARE_TYPE_TASK, PrepareTaskClass))
#define PREPARE_IS_TASK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), PREPARE_TYPE_TASK))
#define PREPARE_IS_TASK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PREPARE_TYPE_TASK))
#define PREPARE_TASK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), PREPARE_TYPE_TASK, PrepareTaskClass))

typedef struct _PrepareTaskClass   PrepareTaskClass;
typedef struct _PrepareTask        PrepareTask;
typedef struct _PrepareTaskPrivate PrepareTaskPrivate;

GType prepare_task_get_type(void) G_GNUC_CONST;

struct _PrepareTaskClass {
    MktTaskObjectClass parent_class;
};

struct _PrepareTask {
    MktTaskObject       parent_instance;
    PrepareTaskPrivate *priv;
};

G_END_DECLS

#endif /* _PREPARE_TASK_H_ */

/** @} */
