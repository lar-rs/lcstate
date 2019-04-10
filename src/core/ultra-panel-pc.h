/**
 * file  ultra-panel-pc.h
 * brief pc device apt network client.
 * Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * author A.Smolkov
 *
 */

#ifndef _ULTRA_PANEL_PC_CLIENT_H_
#define  _ULTRA_PANEL_PC_CLIENT_H_



#include <glib.h>
#include <gio/gio.h>

#include "ultra-pc-generated-code.h"


// Default watch name "com.lar.service.pc"
// Es steht allerding überall "com.lar.service.device" und damit funktioniert es auch.

#define  ULTRA_AMS_PC_PATH                          "/ultra/pc"





UltraPc*                             ultra_ams_panel_pc                                    ( GDBusConnection *conn );
UltraApt*                            ultra_ams_panel_pc_apt                                ( GDBusConnection *conn );
UltraNetEth0*                        ultra_ams_panel_pc_eth0                               ( GDBusConnection *conn );
UltraNetWlan0*                       ultra_ams_panel_pc_wlan0                              ( GDBusConnection *conn );
void                                 ultra_ams_panel_pc_state                              ( GDBusConnection *conn );

#endif /* _MKT_ERRORS_MANAGER_CLIENT_H_ */


/** @} */

