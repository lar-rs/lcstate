/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_CALIBRATION_H_
#define _ULTRA_ACTION_CALIBRATION_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultraactioninterval.h"
#include "ultrabusstream.h"
#include "actioninterfaces.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_CALIBRATION (ultra_action_calibration_get_type())
#define ULTRA_ACTION_CALIBRATION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_CALIBRATION, UltraActionCalibration))
#define ULTRA_ACTION_CALIBRATION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_CALIBRATION, UltraActionCalibrationClass))
#define ULTRA_IS_ACTION_CALIBRATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_CALIBRATION))
#define ULTRA_IS_ACTION_CALIBRATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_CALIBRATION))
#define ULTRA_ACTION_CALIBRATION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_CALIBRATION, UltraActionCalibrationClass))

typedef struct _UltraActionCalibrationClass UltraActionCalibrationClass;
typedef struct _UltraActionCalibration UltraActionCalibration;
typedef struct _UltraActionCalibrationPrivate UltraActionCalibrationPrivate;

GType ultra_action_calibration_get_type(void) G_GNUC_CONST;

struct _UltraActionCalibrationClass
{
	UltraActionClass parent_class;
};

struct _UltraActionCalibration
{
	UltraAction parent_instance;
	UltraActionCalibrationPrivate *priv;
};

UltraAction *UltraActionCalibrationNew(UltraBusStream *stream);
UltraAction *UltraActionCalibrationNewCopy(UltraActionCalibration *calibration);
UltraAction *UltraActionAutocalibrationNew(UltraBusStream *stream);

void m_UltraActionCalibrationSetPump(UltraActionCalibration *calibration, const gchar *pump);
void m_UltraActionCalibrationSetVessel(UltraActionCalibration *calibration, const gchar *vessel);
UltraBusStream *m_UltraActionCalibrationGetStream(UltraActionCalibration *calibration);



#define ULTRA_TYPE_ACTION_CAL_ONLINE (ultra_action_cal_online_get_type())
#define ULTRA_ACTION_CAL_ONLINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_CAL_ONLINE, UltraActionCalOnline))
#define ULTRA_ACTION_CAL_ONLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_CAL_ONLINE, UltraActionCalOnlineClass))
#define ULTRA_IS_ACTION_CAL_ONLINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_CAL_ONLINE))
#define ULTRA_IS_ACTION_CAL_ONLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_CAL_ONLINE))
#define ULTRA_ACTION_CAL_ONLINE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_CAL_ONLINE, UltraActionCalOnlineClass))

typedef struct _UltraActionCalOnlineClass UltraActionCalOnlineClass;
typedef struct _UltraActionCalOnline UltraActionCalOnline;

struct _UltraActionCalOnlineClass
{
	UltraActionCalibrationClass parent_class;
};

struct _UltraActionCalOnline
{
	UltraActionCalibration parent_instance;
};

GType ultra_action_cal_online_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif /* _ULTRA_ACTION_CAL_ONLINE_H_ */

/** @} */
