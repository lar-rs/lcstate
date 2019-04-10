/**
 * @brief Ultra can api dbus server
 * @copyright	 Copyright (C) LAR 2019
 *
 * @author A.Smolkov  <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef  _ULTRA_CAN_DBUS_
#define  _ULTRA_CAN_DBUS_



#include <glib.h>
#include <gio/gio.h>

#include <ultra-can-generated-code.h>
#include <ultra-ams-dbus.h>


void                     ultra_can_dbus_state     ( GDBusConnection *connection );
UltraCan0 *              ultra_can_dbus           ( GDBusConnection *connection );
UltraNodeAnalog1*        ultra_node_analog2       ( GDBusConnection *connection );
UltraNodeDoppelmotor3*   ultra_node_doppelmotor12 ( GDBusConnection *connection );
UltraNodeDoppelmotor3*   ultra_node_doppelmoter14 ( GDBusConnection *connection );
UltraNodeDigital16*      ultra_node_digital18     ( GDBusConnection *connection );
UltraNodeDigital16*      ultra_node_digital19     ( GDBusConnection *connection );
UltraNodeAnalogouts*     ultra_node_analogouts    ( GDBusConnection *connection );


#endif

