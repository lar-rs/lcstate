/**
 * @defgroup AnalyzeBusLibrary
 * @defgroup AnalyzeTask
 * @ingroup  AnalyzeTask
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

#ifndef __ANALYZE_TASK_H__
#define __ANALYZE_TASK_H__

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define ANALYZE_TYPE_TASK (analyze_task_get_type())
#define ANALYZE_TASK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ANALYZE_TYPE_TASK, AnalyzeTask))
#define ANALYZE_TASK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ANALYZE_TYPE_TASK, AnalyzeTaskClass))
#define ANALYZE_IS_TASK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ANALYZE_TYPE_TASK))
#define ANALYZE_IS_TASK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ANALYZE_TYPE_TASK))
#define ANALYZE_TASK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ANALYZE_TYPE_TASK, AnalyzeTaskClass))

typedef struct _AnalyzeTaskClass   AnalyzeTaskClass;
typedef struct _AnalyzeTask        AnalyzeTask;
typedef struct _AnalyzeTaskPrivate AnalyzeTaskPrivate;

GType analyze_task_get_type(void) G_GNUC_CONST;

struct _AnalyzeTaskClass {
    MktTaskObjectClass parent_class;
};

struct _AnalyzeTask {
    MktTaskObject       parent_instance;
    AnalyzeTaskPrivate *priv;
};

void analyze_task_set_channels(AnalyzeTask *task, GList *channels);
gdouble analyse_justification_airflow(AnalyzeTask *task);
gboolean analyze_task_is_tic(AnalyzeTask *task);

G_END_DECLS

#endif /* _ANALYZE_TASK_H_ */

/** @} */
