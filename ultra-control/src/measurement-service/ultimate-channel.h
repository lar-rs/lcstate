/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @file  ultimate-channel-object.h	CHANNEL object header
 * @brief This is CHANNEL object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef _ULTIMATE_CHANNEL_OBJECT_H_
#define _ULTIMATE_CHANNEL_OBJECT_H_

#include <mktlib.h>
#include <mktbus.h>
#include "mkt-process-object.h"





G_BEGIN_DECLS

#define ULTIMATE_TYPE_CHANNEL                  (ultimate_channel_get_type ())
#define ULTIMATE_CHANNEL(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj),ULTIMATE_TYPE_CHANNEL, UltimateChannel))
#define ULTIMATE_IS_CHANNEL(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj),ULTIMATE_TYPE_CHANNEL))
#define ULTIMATE_CHANNEL_GET_INTERFACE(inst)   (G_TYPE_INSTANCE_GET_INTERFACE ((inst), ULTIMATE_TYPE_CHANNEL, UltimateChannelInterface))


typedef struct _UltimateChannelInterface UltimateChannelInterface;
typedef struct _UltimateChannel          UltimateChannel;




struct _UltimateChannelInterface
{
	GTypeInterface               parent_iface;
	MktChannel*                  (*channel_model)                               ( UltimateChannel *channel );
	MktCalibration*              (*calibration_model)                           ( UltimateChannel *channel );

	gboolean                     (*start_measurement)                           ( UltimateChannel *channel );
	gboolean                     (*start_calibration)                           ( UltimateChannel *channel );
	gboolean                     (*start_solution_point)                        ( UltimateChannel *channel , guint solution);
	gboolean                     (*have_solution_value)                         ( UltimateChannel *channel , guint solution);
	gboolean                     (*next_measurement)                            ( UltimateChannel *channel );
	gboolean                     (*transmit_M_replicate)                        ( UltimateChannel *channel, MktProcessObject *process );
	gboolean                     (*transmit_M_result)                           ( UltimateChannel *channel, MktProcessObject *process );
	gboolean                     (*reset_measurement)                           ( UltimateChannel *channel );
	void                         (*calculate_error)                             ( UltimateChannel *channel );
	void                         (*amount_init )                                ( UltimateChannel *channel );
	void                         (*amount_transmit)                             ( UltimateChannel *channel, MktProcessObject *process );

	gboolean                     (*analyze_start)                               ( UltimateChannel *channel );
	void                         (*analyze_break)                               ( UltimateChannel *channel );

};

GType                            ultimate_channel_get_type                      ( void ) G_GNUC_CONST;
MktChannel*                      ultimate_channel_get_channel_model             ( UltimateChannel *channel );
MktCalibration*                  ultimate_channel_get_calibration_model         ( UltimateChannel *channel );

gboolean                         ultimate_channel_start_measurement             ( UltimateChannel *channel );
gboolean                         ultimate_channel_start_calibration             ( UltimateChannel *channel );
gboolean                         ultimate_channel_start_solution_point          ( UltimateChannel *channel , guint solution);
gboolean                         ultimate_channel_have_solution_value           ( UltimateChannel *channel , guint solution);
gboolean                         ultimate_channel_next_measurement              ( UltimateChannel *channel );
gboolean                         ultimate_channel_clean                         ( UltimateChannel *channel );


gboolean                         ultimate_channel_transmit_M_replicate          ( UltimateChannel *channel, MktProcessObject *process  );

gboolean                         ultimate_channel_transmit_M_result             ( UltimateChannel *channel, MktProcessObject *process );
gboolean                         ultimate_channel_transmit_C_result             ( UltimateChannel *channel );
void                             ultimate_channel_amount_init                   ( UltimateChannel *channel );
void                             ultimate_channel_transmit_amount               ( UltimateChannel *channel, MktProcessObject *process );
void                             ultimate_channel_transmit_last                 ( UltimateChannel *channel );

gboolean                         ultimate_channel_reset_measurement             ( UltimateChannel *channel );
gboolean                         ultimate_channel_recalculate_calibration       ( UltimateChannel *channel, gboolean automatical );
gboolean                         ultimate_channel_activate_calibration          ( UltimateChannel *channel );
void                             ultimate_channel_change_status                 ( UltimateChannel *channel, const gchar *format,...)G_GNUC_PRINTF (2, 3);


IntegrationObject*               ultimate_channel_get_integration               ( UltimateChannel *channel );
gboolean                         ultimate_channel_start_analyse                 ( UltimateChannel *channel );
gboolean                         ultimate_channel_integration_is_runned         ( UltimateChannel *channel );
void                             ultimate_channel_analyse_stop                  ( UltimateChannel *channel );
gboolean                         ultimate_channel_is_integrating                ( UltimateChannel *channel );
void                             ultimate_channel_justification                 ( UltimateChannel *channel );
void                             ultimate_channel_integration                   ( UltimateChannel *channel );
void                             ultimate_channel_calculate_justification       ( UltimateChannel *channel );
void                             ultimate_channel_calculate_integration         ( UltimateChannel *channel );
void                             ultimate_channel_check_limit                   ( UltimateChannel *channel );
void                             ultimate_channel_transmit_analog               ( UltimateChannel *channel );
void                             ultimate_channel_check_limit_check             ( UltimateChannel *channel );
void                             ultimate_channel_transmit_analog_check         ( UltimateChannel *channel );

void                             ultimate_channel_create_manager                ( GDBusConnection *connection );
GDBusObjectManagerServer*        ultimate_channel_get_manager                   ( void );
gboolean                         ultimate_channel_is_error                      ( UltimateChannel *channel );
void                             ultimate_channel_calculate_error               ( UltimateChannel *channel );


G_END_DECLS

#endif /* _ULTIMATE_CHANNEL_OBJECT_H_ */
