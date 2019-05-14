/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultrabusstream.c
 * Copyright (C) LAR 2017
 * autor ASmolkov
 *
 */

#ifndef _ULTRA_ACTION_MEASUREMENT_H_
#define _ULTRA_ACTION_MEASUREMENT_H_

#include <glib-object.h>
#include <mktbus.h>
#include "ultraaction.h"
#include "ultraactioninterval.h"
#include "ultrabusstream.h"
#include "actioninterfaces.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_ACTION_MEASUREMENT (ultra_action_measurement_get_type())
#define ULTRA_ACTION_MEASUREMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_MEASUREMENT, UltraActionMeasurement))
#define ULTRA_ACTION_MEASUREMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_MEASUREMENT, UltraActionMeasurementClass))
#define ULTRA_IS_ACTION_MEASUREMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_MEASUREMENT))
#define ULTRA_IS_ACTION_MEASUREMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_MEASUREMENT))
#define ULTRA_ACTION_MEASUREMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_MEASUREMENT, UltraActionMeasurementClass))

typedef struct _UltraActionMeasurementClass UltraActionMeasurementClass;
typedef struct _UltraActionMeasurement UltraActionMeasurement;
typedef struct _UltraActionMeasurementPrivate UltraActionMeasurementPrivate;

GType ultra_action_measurement_get_type(void) G_GNUC_CONST;

struct _UltraActionMeasurementClass
{
	UltraActionClass parent_class;
};

struct _UltraActionMeasurement
{
	UltraAction parent_instance;
	UltraActionMeasurementPrivate *priv;
};

UltraAction *UltraActionMeasurementNew(UltraBusStream *stream);
UltraAction *UltraActionMeasurementNewCopy(UltraActionMeasurement *measurement);
UltraAction *UltraActionMeasurementOnlineNew(UltraBusStream *stream);
UltraAction *UltraActionMeasurementCheckNew(UltraBusStream *stream);

void m_UltraActionMeasurementSetPump(UltraActionMeasurement *measurement, const gchar *pump);
void m_UltraActionMeasurementNoFill(UltraActionMeasurement *measurement);
void m_UltraActionMeasurementSetVessel(UltraActionMeasurement *measurement, const gchar *vessel);
void m_UltraActionMeasurementNoPreRinsing(UltraActionMeasurement *measurement);
UltraBusStream *m_UltraActionMeasurementGetStream(UltraActionMeasurement *measurement);



#define ULTRA_TYPE_ACTION_MEAS_ONLINE (ultra_action_meas_online_get_type())
#define ULTRA_ACTION_MEAS_ONLINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_MEAS_ONLINE, UltraActionMeasOnline))
#define ULTRA_ACTION_MEAS_ONLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_MEAS_ONLINE, UltraActionMeasOnlineClass))
#define ULTRA_IS_ACTION_MEAS_ONLINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_MEAS_ONLINE))
#define ULTRA_IS_ACTION_MEAS_ONLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_MEAS_ONLINE))
#define ULTRA_ACTION_MEAS_ONLINE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_MEAS_ONLINE, UltraActionMeasOnlineClass))

typedef struct _UltraActionMeasOnlineClass UltraActionMeasOnlineClass;
typedef struct _UltraActionMeasOnline UltraActionMeasOnline;

struct _UltraActionMeasOnlineClass
{
	UltraActionMeasurementClass parent_class;
};

struct _UltraActionMeasOnline
{
	UltraActionMeasurement parent_instance;
};

GType ultra_action_meas_online_get_type(void) G_GNUC_CONST;


#define ULTRA_TYPE_ACTION_MEAS_CHECK (ultra_action_meas_check_get_type())
#define ULTRA_ACTION_MEAS_CHECK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_ACTION_MEAS_CHECK, UltraActionMeasCheck))
#define ULTRA_ACTION_MEAS_CHECK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_ACTION_MEAS_CHECK, UltraActionMeasCheckClass))
#define ULTRA_IS_ACTION_MEAS_CHECK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_ACTION_MEAS_CHECK))
#define ULTRA_IS_ACTION_MEAS_CHECK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_ACTION_MEAS_CHECK))
#define ULTRA_ACTION_MEAS_CHECK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_ACTION_MEAS_CHECK, UltraActionMeasCheckClass))

typedef struct _UltraActionMeasCheckClass UltraActionMeasCheckClass;
typedef struct _UltraActionMeasCheck UltraActionMeasCheck;

struct _UltraActionMeasCheckClass
{
	UltraActionMeasurementClass parent_class;
};

struct _UltraActionMeasCheck
{
	UltraActionMeasurement parent_instance;
};

GType ultra_action_meas_check_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif /* _ULTRA_ACTION_MEAS_ONLINE_H_ */

/** @} */
