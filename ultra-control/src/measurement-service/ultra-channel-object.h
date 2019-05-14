/**
 * @defgroup Ultra
 * @defgroup UltraChannelObject
 * @ingroup  UltraChannelObject
 * @{
 * @file  ultra-channel-object.h	Valve object header
 * @brief Pump object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */

#ifndef _ULTRA_CHANNEL_OBJECT_H_
#define _ULTRA_CHANNEL_OBJECT_H_



#include <glib.h>
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>
#include "ultimate-channel.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_CHANNEL_OBJECT             (ultra_channel_object_get_type ())
#define ULTRA_CHANNEL_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_CHANNEL_OBJECT, UltraChannelObject))
#define ULTRA_CHANNEL_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_CHANNEL_OBJECT, UltraChannelObjectClass))
#define ULTRA_IS_CHANNEL_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_CHANNEL_OBJECT))
#define ULTRA_IS_CHANNEL_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_CHANNEL_OBJECT))
#define ULTRA_CHANNEL_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_CHANNEL_OBJECT, UltraChannelObjectClass))

typedef struct _UltraChannelObjectClass         UltraChannelObjectClass;
typedef struct _UltraChannelObject              UltraChannelObject;
typedef struct _UltraChannelObjectPrivate       UltraChannelObjectPrivate;

struct _UltraChannelObjectClass
{
	ChannelsObjectSkeletonClass                                 parent_class;

};

struct _UltraChannelObject
{
	ChannelsObjectSkeleton                                      parent_instance;
	UltraChannelObjectPrivate                                  *priv;
};

GType                                                   ultra_channel_object_get_type                  ( void ) G_GNUC_CONST;



gboolean                                                ultra_channel_object_activate_autocal          ( UltraChannelObject *object );

gboolean                                                ultra_channel_object_can_activate_autocal      ( UltraChannelObject *object );
gboolean                                                ultra_channel_object_activate_last             ( UltraChannelObject *object );
gboolean                                                ultra_channel_object_recalculate_calibration   ( UltraChannelObject *channel_object);

gint                                                    ultra_channel_start_calibration                ( UltraChannelObject *channel_object, MktProcessObject *process );
gboolean                                                ultra_channel_calibration_calculate_statistic  ( UltraChannelObject *channel_object, MktProcessObject *process );
MktCalibration*                                         ultra_channel_calibration_model                ( UltraChannelObject *channel_object );
MktCalPoint*                                            ultra_channel_calibration_current_point        ( UltraChannelObject *channel_object );
gboolean                                                ultra_channel_calibration_next_point           ( UltraChannelObject *channel_object );
gboolean                                                ultra_channel_transmit_integration             ( UltraChannelObject *channel_object, MktProcessObject *process );
gboolean                                                ultra_channel_activate_calibration             ( UltraChannelObject *channel_object, gboolean main );


G_END_DECLS

#endif /* _ULTRA_CHANNEL_OBJECT_H_ */


/** @} */

