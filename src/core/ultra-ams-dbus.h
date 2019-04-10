/**
 * Copyright (C) LAR 2018
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef _ULTRA_CONTROL_CLIENT_H_
#define _ULTRA_CONTROL_CLIENT_H_



#include <glib.h>
#include <gio/gio.h>

#include <ultra-ams-generated-code.h>
#include <ultra-measurement-generated-code.h>
#include <ultra-pc-generated-code.h>
#include <ultra-level-generated-code.h>


typedef struct _UltraAmsMeasurement UltraAmsMeasurement;

struct _UltraAmsMeasurement{
    guint                    number;
    gboolean                 initialized;
    gboolean                 activated;
    const gchar             *path;
    const gchar             *id;
    UltraStream             *stream;
    UltraProcessOnline      *online;
    UltraProcessSingle      *single;
    UltraProcessCalibration *calibration;
    UltraProcessCheck       *check;
    UltraChannelTc          *tc;
    UltraChannelTic         *tic;
    UltraChannelToc         *toc;
    UltraChannelCodo        *codo;
    UltraChannelTnb         *tnb;
};

enum
{
	ULTRA_AMS_MEASUREMENT1,
	ULTRA_AMS_MEASUREMENT2,
	ULTRA_AMS_MEASUREMENT3,
	ULTRA_AMS_MEASUREMENT4,
	ULTRA_AMS_MEASUREMENT5,
	ULTRA_AMS_MEASUREMENT6,
};
// Ultra objetces  dbus path
#define  ULTRA_AMS_CONTROL_PATH                                 "/ultra/control"
#define  ULTRA_AMS_ERRORS_PATH                                  "/ultra/errors"
#define  ULTRA_AMS_MEASUREMENT_FORMAT                           "/ultra/measurment/%d"
#define  ULTRA_AMS_MEASUREMENT1_PATH                            "/ultra/measurment/1"
#define  ULTRA_AMS_MEASUREMENT2_PATH                            "/ultra/measurment/2"
#define  ULTRA_AMS_MEASUREMENT3_PATH                            "/ultra/measurment/3"
#define  ULTRA_AMS_MEASUREMENT4_PATH                            "/ultra/measurment/4"
#define  ULTRA_AMS_MEASUREMENT5_PATH                            "/ultra/measurment/5"
#define  ULTRA_AMS_MEASUREMENT6_PATH                            "/ultra/measurment/6"

#define  ULTRA_AMS_LEVEL_PATH                                   "/ultra/level"
// #define  ULTRA_AMS_MEASUREMENT_CHANNEL_MANAGER               "/ultra/measurment/"
// #define  ULTRA_AMS_MEASUREMENT_CHANNEL_OBJECT_FORMAT         "/ultra/measurment/channels/%d/%s"
// #define  ULTRA_PROCESS_MANAGER                               "/ultra/measurment/%d/%s"

#define  ULTRA_CONTROL_PROCESS_SINGLE                           "Single"
#define  ULTRA_CONTROL_PROCESS_ONLINE                           "Measurement"
#define  ULTRA_CONTROL_PROCESS_CALIBRATION                      "Calibration"
#define  ULTRA_CONTROL_PROCESS_CHECK                            "Check"
#define  ULTRA_CONTROL_PROCESS_AUTOCALIBRATION                  "Autocalibration"
enum
{
	ULTIMATE_SIGNAL_ANALYSE_DATA       = 21,
	ULTIMATE_SIGNAL_JUSTIFICATION_DATA = 22,
	ULTIMATE_SIGNAL_JUSTIFICATED       = 23,
	ULTIMATE_SIGNAL_INTEGRATION_DATA   = 24,
	ULTIMATE_SIGNAL_INTEGRATION_RUN    = 25,
	ULTIMATE_SIGNAL_INTEGRATION_START  = 27,
	ULTIMATE_SIGNAL_INTEGRATION_STOP   = 28,
};


enum
{
	ULTIMATE_SENSOR_KIND_UNKNOWN       = 0,
	ULTIMATE_SENSOR_KIND_NDIR          = 1,
	ULTIMATE_SENSOR_KIND_CODo          = 2,
	ULTIMATE_SENSOR_KIND_TNb           = 3,
};


enum
{
	ULTIMATE_PROCESS_CALIBRATION       = 1,
	ULTIMATE_PROCESS_SINGLE            = 2,
	ULTIMATE_PROCESS_MEASUREMENT       = 3,
	ULTIMATE_PROCESS_CHECK             = 4,
	ULTIMATE_PROCESS_AUTOCALIBRATION   = 5
};


#define  TERA_INTEGRATION_MANAGER       "/com/lar/tera/integrations"

guint                                    ultra_ams_streams_license                                       ( void );
UltraControl*                            ultra_ams_control                                               ( GDBusConnection *connection );
UltraAmsMeasurement*                     ultra_ams_measurement                                           ( guint number );
GList*                                   ultra_ams_measurements                                          ( void );
UltraAmsMeasurement*                     ultra_ams_measurement1                                          ( void );
UltraAmsMeasurement*                     ultra_ams_measurement2                                          ( void );
UltraAmsMeasurement*                     ultra_ams_measurement3                                          ( void );
UltraAmsMeasurement*                     ultra_ams_measurement4                                          ( void );
UltraAmsMeasurement*                     ultra_ams_measurement5                                          ( void );
UltraAmsMeasurement*                     ultra_ams_measurement6                                          ( void );

UltraLevel*                              ultra_ams_level                                                 ( GDBusConnection *connection );
GDBusObjectManager*                      ultra_ams_errors                                                ( GDBusConnection *connection );

GDBusConnection *                        ultra_ams_dbus_connection                                       ( void );
const gchar*                             ultra_ams_dbus_name                                             ( void );
GBusType                                 ultra_ams_dbus_type                                             ( void );




//DBus connection configuration functions

void ultra_ams_dbus_remote(gchar *ip,guint port);
void ultra_ams_dbus_local();
const gchar* ultra_ams_dbus_address();




#endif
