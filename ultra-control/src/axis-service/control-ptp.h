/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup XYSystem
 * @ingroup  XYSystem
 * @{
 * @file  ControlPtp-object.h	ControlPtp object header
 * @brief This is ControlPtp object header file.
 * 	 Copyright (C) LAR 2016
 *
 * @author A.Smolkov  <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _CONTROL_PTP_H_
#define _CONTROL_PTP_H_

#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define CONTROL_TYPE_PTP (control_ptp_get_type())
#define CONTROL_PTP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CONTROL_TYPE_PTP, ControlPtp))
#define CONTROL_PTP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), CONTROL_TYPE_PTP, ControlPtpClass))
#define CONTROL_IS_PTP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CONTROL_TYPE_PTP))
#define CONTROL_IS_PTP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CONTROL_TYPE_PTP))
#define CONTROL_PTP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), CONTROL_TYPE_PTP, ControlPtpClass))

typedef struct _ControlPtpClass   ControlPtpClass;
typedef struct _ControlPtp        ControlPtp;
typedef struct _ControlPtpPrivate ControlPtpPrivate;

struct _ControlPtpClass {
    GObjectClass parent_class;
    void (*run)(ControlPtp *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
};

struct _ControlPtp {
    GObject            parent_instance;
    ControlPtpPrivate *priv;
};

GType control_ptp_get_type() G_GNUC_CONST;

void cotrol_ptp_run(ControlPtp *control, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);

G_END_DECLS

#endif /* _CONTROL_PTP_H_ */
/** @} */
