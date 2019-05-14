/**
 * @defgroup CleanDilutionBusLibrary
 * @defgroup CleanDilutionTask
 * @ingroup  CleanDilutionTask
 * @{
 * @file  clean_dilution-task.h	Task object header
 * @brief   Task object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef __CLEAN_DILUTION_TASK_H__
#define __CLEAN_DILUTION_TASK_H__

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define CLEAN_DILUTION_TYPE_TASK             (clean_dilution_task_get_type ())
#define CLEAN_DILUTION_TASK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLEAN_DILUTION_TYPE_TASK, CleanDilutionTask))
#define CLEAN_DILUTION_TASK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CLEAN_DILUTION_TYPE_TASK, CleanDilutionTaskClass))
#define CLEAN_DILUTION_IS_TASK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLEAN_DILUTION_TYPE_TASK))
#define CLEAN_DILUTION_IS_TASK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CLEAN_DILUTION_TYPE_TASK))
#define CLEAN_DILUTION_TASK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CLEAN_DILUTION_TYPE_TASK, CleanDilutionTaskClass))

typedef struct _CleanDilutionTaskClass     CleanDilutionTaskClass;
typedef struct _CleanDilutionTask          CleanDilutionTask;
typedef struct _CleanDilutionTaskPrivate   CleanDilutionTaskPrivate;

GType clean_dilution_task_get_type (void) G_GNUC_CONST;


struct _CleanDilutionTaskClass
{
	MktTaskClass          parent_class;
};

struct _CleanDilutionTask
{
	MktTask               parent_instance;
	CleanDilutionTaskPrivate   *priv;
};

void               clean_dilution_task_set_channels    ( CleanDilutionTask *task , GList *channels );




gboolean           clean_dilution_task_is_tic          ( CleanDilutionTask *task );

G_END_DECLS

#endif /* _CLEAN_DILUTION_TASK_H_ */

/** @} */
