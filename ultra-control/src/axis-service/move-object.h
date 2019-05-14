/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup XYSystem
 * @ingroup  XYSystem
 * @{
 * @file  MOVE-object.h	MOVE object header
 * @brief This is MOVE object header file.
 * 	 Copyright (C) LAR 2016
 *
 * @author A.Smolkov  <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */
#ifndef _MOVE_OBJECT_H_
#define _MOVE_OBJECT_H_

#include <mktbus.h>
#include <mktlib.h>

#include "move-axis.h"

G_BEGIN_DECLS

#define MOVE_TYPE (MOVE_get_type())
#define MOVE_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MOVE_TYPE, MOVE))
#define MOVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MOVE_TYPE, MOVEClass))
#define MOVE_IS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MOVE_TYPE))
#define MOVE_IS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MOVE_TYPE))
#define MOVE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MOVE_TYPE, MOVEClass))

typedef struct _MOVEClass   MOVEClass;
typedef struct _MOVE        MOVE;
typedef struct _MOVEPrivate MOVEPrivate;

struct _MOVEClass {
    MktTaskObjectClass parent_class;
};

struct _MOVE {
    MktTaskObject parent_instance;
    MOVEPrivate * priv;
};

GQuark move_error_quark(void);

GType MOVE_get_type(void) G_GNUC_CONST;

gboolean MOVE_is_timeout(MOVE *move);
void MOVE_timer_reset(MOVE *move);
void MOVE_timer_start(MOVE *move);
gdouble MOVE_timeout(MOVE *move);
guint MOVE_get_part(MOVE *move);

void MOVE_message(MOVE *MOVE_object, const gchar *format, ...);
void MOVE_object_done(MOVE *MOVE_object, const gchar *format, ...);
void MOVE_object_error(MOVE *MOVE_object, const gchar *format, ...);
NodesObject *MOVE_node(MOVE *move);
const gchar *MOVE_node_name(MOVE *move);
MktErrorsNumbers MOVE_error_number(MOVE *move);
AxisObject *MOVE_axis(MOVE *move);

void MOVE_set_timeout(MOVE *move, gdouble timeout);
void MOVE_set_position(MOVE *move, guint position);
void MOVE_set_old_position(MOVE *move, guint position);

guint MOVE_result(MOVE *move);

void          MOVE_cancel();
GCancellable *MOVE_cancellable();
void          MOVE_cancellable_unref();

gboolean MOVE_parameter_sync(MOVE *move, guint parameter, gboolean *out, GCancellable *cancellable, GError **error);
gboolean MOVE_stepper_go_pos(MOVE *move, guint move_to, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean MOVE_go_pos_finish(MOVE *move, gboolean *result, GAsyncResult *res, GError **error);
gboolean MOVE_current_position(MOVE *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean MOVE_current_position_finish(MOVE *move, guint *result, GAsyncResult *res, GError **error);
gboolean MOVE_position_old_sync(MOVE *move, guint *result, GCancellable *cancellable, GError **error);
gboolean MOVE_command_status_sync(MOVE *move, guint command, gboolean *out_done, GCancellable *cancellable, GError **error);
gboolean MOVE_command_status(MOVE *move, guint command, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean MOVE_command_status_finish(MOVE *move, gboolean *result, GAsyncResult *res, GError **error);
gboolean MOVE_final_position(MOVE *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean MOVE_final_position_finish(MOVE *move, gboolean *result, GAsyncResult *res, GError **error);

gboolean MOVE_get_stall_guard(MOVE *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean MOVE_get_stall_guard_finish(MOVE *move, guint *result, GAsyncResult *res, GError **error);

G_END_DECLS

#endif /* _MOVE_OBJECT_H_ */
/** @} */
