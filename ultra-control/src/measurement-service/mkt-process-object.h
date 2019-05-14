/**
 * @copyright	 Copyright (C) LAR 2015
 * @brief   ProcessObject object header file.
 * @author       A.Smolkov <asmolkov@lar.com>
 */

#ifndef _MKT_PROCESS_OBJECT_H_
#define _MKT_PROCESS_OBJECT_H_

#include "channels-generated-code.h"
#include "mkt-task-object.h"
#include "mkt-task.h"
#include "process-generated-code.h"
#include <glib-object.h>
#include <glib.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define MKT_TYPE_PROCESS_OBJECT (mkt_process_object_get_type())
#define MKT_PROCESS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MKT_TYPE_PROCESS_OBJECT, MktProcessObject))
#define MKT_PROCESS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MKT_TYPE_PROCESS_OBJECT, MktProcessObjectClass))
#define MKT_IS_PROCESS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MKT_TYPE_PROCESS_OBJECT))
#define MKT_IS_PROCESS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MKT_TYPE_PROCESS_OBJECT))
#define MKT_PROCESS_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MKT_TYPE_PROCESS_OBJECT, MktProcessObjectClass))

typedef struct _MktProcessObjectClass   MktProcessObjectClass;
typedef struct _MktProcessObject        MktProcessObject;
typedef struct _MktProcessObjectPrivate MktProcessObjectPrivate;

typedef void (*MktProcessTaskDoneCallback)(MktProcessObject *object, MktTask *task);

struct _MktProcessObjectClass {
    ProcessObjectSkeletonClass parent_class;
    gboolean (*start)(MktProcessObject *process);
    void (*run)(MktProcessObject *process, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
    gboolean (*finish)(MktProcessObject *process, GTask *task, GError **error);
    void (*online)(MktProcessObject *process);
    void (*offline)(MktProcessObject *process);
    void (*interval_trigger)(MktProcessObject *process,gdouble duration);
};

struct _MktProcessObject {
    ProcessObjectSkeleton    parent_instance;
    MktProcessObjectPrivate *priv;
};

GType mkt_process_object_get_type(void) G_GNUC_CONST;

gboolean mkt_process_object_start(MktProcessObject *process);
void mkt_process_object_break(MktProcessObject *process);

guint mkt_process_object_get_status(MktProcessObject *process);
guint mkt_process_object_get_state(MktProcessObject *process);

gboolean mkt_process_object_run(MktProcessObject *process, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean mkt_process_object_finish(MktProcessObject *process, GAsyncResult *res, GError **error);

void mkt_process_object_add_channel(MktProcessObject *process, ChannelsObject *channel);
GList *mkt_process_object_channels(MktProcessObject *process);
void mkt_process_object_user_next(MktProcessObject *process);

gboolean mkt_process_object_is_activate(MktProcessObject *process);
void mkt_process_object_activate(MktProcessObject *process);
void mkt_process_object_deactivate(MktProcessObject *process);
void mkt_process_object_update_start_time(MktProcessObject *process);
void mkt_process_object_update_stop_time(MktProcessObject *process);
MktProcess *mkt_process_object_get_original(MktProcessObject *process);
guint64 mkt_process_object_ref_id(MktProcessObject *process);
void mkt_process_object_save(MktProcessObject *process);

gboolean mkt_process_object_online(MktProcessObject *process);
void mkt_process_object_offline(MktProcessObject *process);

gboolean mkt_process_calculate_measurement_statistic(MktProcessObject *process, ChannelsObject *channel);

void mkt_process_object_status(MktProcessObject *process, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void mkt_process_object_message(MktProcessObject *process, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void mkt_process_object_critical(MktProcessObject *process, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

void mkt_process_bind_task_status(MktProcessObject *process, MktTaskObject *task);

void mkt_process_object_trace(gpointer process, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

G_END_DECLS

#endif /* _MKT_PROCESS_OBJECT_H_ */

/** @} */
