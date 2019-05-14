/**
 * @defgroup PSAnalyseBusLibrary
 * @defgroup PSAnalyseTask
 * @ingroup  PSAnalyseTask
 * @{
 * @file  analyze-task.h	Task object header
 * @brief   Task object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef __PSANALYSE_TASK_H__
#define __PSANALYSE_TASK_H__

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define PSANALYSE_TYPE_TASK (psanalyse_task_get_type())
#define PSANALYSE_TASK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PSANALYSE_TYPE_TASK, PSAnalyseTask))
#define PSANALYSE_TASK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), PSANALYSE_TYPE_TASK, PSAnalyseTaskClass))
#define PSANALYSE_IS_TASK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), PSANALYSE_TYPE_TASK))
#define PSANALYSE_IS_TASK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PSANALYSE_TYPE_TASK))
#define PSANALYSE_TASK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), PSANALYSE_TYPE_TASK, PSAnalyseTaskClass))

typedef struct _PSAnalyseTaskClass   PSAnalyseTaskClass;
typedef struct _PSAnalyseTask        PSAnalyseTask;
typedef struct _PSAnalyseTaskPrivate PSAnalyseTaskPrivate;

GType psanalyse_task_get_type(void) G_GNUC_CONST;

struct _PSAnalyseTaskClass {
    MktTaskObjectClass parent_class;
};

struct _PSAnalyseTask {
    MktTaskObject       parent_instance;
    PSAnalyseTaskPrivate *priv;
};

void psanalyse_task_set_channels(PSAnalyseTask *task, GList *channels);
gdouble psanalyse_justification_airflow(PSAnalyseTask *task);
gboolean psanalyse_task_is_tic(PSAnalyseTask *task);

G_END_DECLS

#endif /* _PSANALYSE_TASK_H_ */

/** @} */
