/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup XYSystem
 * @ingroup  XYSystem
 * @{
 * @file  axis-object.h	AXIS object header
 * @brief This is AXIS object header file.
 * 	 Copyright (C) LAR 2016
 *
 * @author A.Smolkov  <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */
#ifndef _AXIS_OBJECT_H_
#define _AXIS_OBJECT_H_

#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define AXIS_TYPE_OBJECT (axis_object_get_type())
#define AXIS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), AXIS_TYPE_OBJECT, AxisObject))
#define AXIS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), AXIS_TYPE_OBJECT, AxisObjectClass))
#define AXIS_IS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), AXIS_TYPE_OBJECT))
#define AXIS_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), AXIS_TYPE_OBJECT))
#define AXIS_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AXIS_TYPE_OBJECT, AxisObjectClass))

typedef struct _AxisObjectClass   AxisObjectClass;
typedef struct _AxisObject        AxisObject;
typedef struct _AxisObjectPrivate AxisObjectPrivate;

struct _AxisObjectClass {
    AchsenObjectSkeletonClass parent_class;
};

struct _AxisObject {
    AchsenObjectSkeleton parent_instance;
    AxisObjectPrivate *  priv;
};

GType axis_object_get_type(void) G_GNUC_CONST;

NodesObject *axis_node_object(AxisObject *axis_object);

void axis_change_status(AxisObject *axis_object, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
const gchar *axis_object_get_name(AxisObject *axis_object);
guint axis_object_get_part(AxisObject *axis_object);
void axis_init_parameter(AxisObject *axis_object);
void axis_object_set_error_number(AxisObject *axis_object, MktErrorsNumbers number);
MktErrorsNumbers axis_object_get_error_number(AxisObject *axis_object);
gboolean axis_is_activate_stall_guard(AxisObject *axis_object);
void axis_activate_stall_guard(AxisObject *axis_object);

void axis_position_done(GObject *source_object, GAsyncResult *res, gpointer user_data);

gboolean axis_object_go_position_callback(AchsenAchse *interface, GDBusMethodInvocation *invocation, guint position, gpointer user_data);

G_END_DECLS

#endif /* _AXIS_OBJECT_H_ */
/** @} */
