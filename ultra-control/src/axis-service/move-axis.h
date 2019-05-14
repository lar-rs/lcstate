/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup UltimateLibrary
 * @defgroup MoveAxisObject
 * @ingroup  MoveAxisObject
 * @{
 * @file  move-axis-object.h	AXIS object header
 * @brief This is AXIS object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef _MOVE_AXIS_INTERFACE_H_
#define _MOVE_AXIS_INTERFACE_H_

#include "axis-object.h"
#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define MOVE_TYPE_AXIS (move_axis_get_type())
#define MOVE_AXIS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MOVE_TYPE_AXIS, MoveAxis))
#define MOVE_IS_AXIS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MOVE_TYPE_AXIS))
#define MOVE_AXIS_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), MOVE_TYPE_AXIS, MoveAxisInterface))

typedef struct _MoveAxisInterface MoveAxisInterface;
typedef struct _MoveAxis          MoveAxis;
typedef gboolean (*MoveFunc)(MoveAxis *move, gpointer user_data);

#define MOVE_FUNCTION(f) ((MoveFunc)(f))

enum { MOVE_RESULT_FINAL_POSITION = 1, MOVE_RESULT_ON_POSITION = 2, MOVE_RESULT_STALL_GUARD = 3 };

struct _MoveAxisInterface {
    GTypeInterface parent_iface;
    // gboolean (*run)(MoveAxis *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
};

GType move_axis_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif /* _MOVE_AXIS_INTERFACE_H_ */
/** @} */
