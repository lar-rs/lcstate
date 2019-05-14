/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup UltimateLibrary
 * @defgroup UltraAxisObject
 * @ingroup  UltraAxisObject
 * @{
 * @file  ultra-axis-object.h	AXIS object header
 * @brief This is AXIS object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef _ULTRA_AXIS_INTERFACE_H_
#define _ULTRA_AXIS_INTERFACE_H_

#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_AXIS (ultra_axis_get_type())
#define ULTRA_AXIS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_AXIS, UltraAxis))
#define ULTRA_IS_AXIS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_AXIS))
#define ULTRA_AXIS_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), ULTRA_TYPE_AXIS, UltraAxisInterface))

typedef struct _UltraAxisInterface UltraAxisInterface;
typedef struct _UltraAxis          UltraAxis;

struct _UltraAxisInterface {
    GTypeInterface parent_iface;
    gboolean (*go_hold)(UltraAxis *axis);
    gboolean (*go_sensor)(UltraAxis *axis);
    gboolean (*init_parameter)(UltraAxis *axis);
    gboolean (*go_position)(UltraAxis *axis, guint pos);
    gboolean (*is_position)(UltraAxis *axis, guint pos);
    guint (*curr_position)(UltraAxis *axis);
};

GType ultra_axis_get_type(void) G_GNUC_CONST;

GCancellable *ultra_axis_cancellable(void);
GCancellable *ultra_axisZ_cancellable(void);
void          ultra_axis_cancellable_init(void);
void          ultra_axisZ_cancellable_init(void);
void          ultra_axis_remove_tag(void);
void ultra_axis_wait_tag(guint tag);
gboolean ultra_axis_is_canceled(void);
gboolean ultra_axisZ_is_canceled(void);
void     ultra_axis_operation_cancel(void);

gboolean ultra_axis_is_busy(UltraAxis *axis);
gboolean ultra_axis_set_busy(UltraAxis *axis, gboolean value);
gboolean ultra_axis_set_init(UltraAxis *axis, gboolean value);
gboolean ultra_axis_is_init(UltraAxis *axis);

gboolean ultra_axis_set_initialized(UltraAxis *axis, gboolean value);
gboolean ultra_axis_is_initialized(UltraAxis *axis);

gboolean ultra_axis_set_parameter(UltraAxis *axis, guint value);
guint ultra_axis_get_parameter(UltraAxis *axis);

gboolean ultra_axis_set_position(UltraAxis *axis, guint position);
guint ultra_axis_get_position(UltraAxis *axis);

guint ultra_axis_get_hold(UltraAxis *axis);
guint ultra_axis_get_go_position(UltraAxis *axis);
gboolean ultra_axis_set_go_position(UltraAxis *axis, guint position);

void ultra_axis_change_status(UltraAxis *axis, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

gboolean ultra_axis_init_parameter(UltraAxis *axis);
gboolean ultra_axis_go_hold(UltraAxis *axis);
gboolean ultra_axis_go_sensor(UltraAxis *axis);
gboolean ultra_axis_go_position(UltraAxis *axis, guint position);
gboolean ultra_axis_is_position(UltraAxis *axis, guint position);
gboolean ultra_axis_fast_speed(UltraAxis *axis);
gboolean ultra_axis_normal_speed(UltraAxis *axis);
gboolean ultra_axis_slow_speed(UltraAxis *axis);

void ultra_axis_emit_timeout(UltraAxis *axis);
void ultra_axis_emit_on_position(UltraAxis *axis, gboolean value);
void ultra_axis_emit_on_sensor(UltraAxis *axis, gboolean value);

GCancellable *ultra_axis_get_move_cancellable();

G_END_DECLS

#endif /* _ULTRA_AXIS_INTERFACE_H_ */
/** @} */
