/*
 * @file  ultra-panel-pc.c	Client binding to pc apt net interfaces
 * @brief Functions :
 *  - update software
 *  - install package
 *  - install licinse
 *  - mount usb
 *  - vnc server info
 *  - network settings
 *
 *
 *  Copyright (C) LAR  2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "ultra-panel-pc.h"
#include "ultra-ams-dbus.h"


static UltraPc  *amspanslpc = NULL;
static UltraApt *amsapt = NULL;
static UltraNetEth0 *neteth0= NULL;
static UltraNetWlan0 *netwlan0 = NULL;



UltraPc*
ultra_ams_panel_pc            ( GDBusConnection *conn ) {
    if(amspanslpc== NULL) {
        GError *error =NULL;
        GDBusConnection *c = ultra_ams_dbus_connection();
        if(conn != NULL) {
            c= conn;
        }
        amspanslpc = ultra_pc_proxy_new_sync(c,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_PC_PATH,NULL,&error);
        //TODO: write client error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return amspanslpc;
}

UltraApt*                            ultra_ams_panel_pc_apt                                ( GDBusConnection *conn ) {
    if(amsapt == NULL) {
        GError *error =NULL;
        GDBusConnection *c = ultra_ams_dbus_connection();
        if(conn != NULL) {
            c= conn;
        }
        amsapt = ultra_apt_proxy_new_sync(c,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_PC_PATH,NULL,&error);
        //TODO: write client error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return amsapt;
}

UltraNetEth0*                        ultra_ams_panel_pc_eth0                               ( GDBusConnection *conn ) {
    if(neteth0== NULL) {
        GError *error = NULL;
        GDBusConnection *c = ultra_ams_dbus_connection();
        if(conn != NULL) {
            c= conn;
        }
        neteth0 = ultra_net_eth0_proxy_new_sync(c,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_PC_PATH,NULL,&error);
        //TODO: write client error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return neteth0;
}

UltraNetWlan0*                       ultra_ams_panel_pc_wlan0                              ( GDBusConnection *conn ) {
    GError *error = NULL;
    if(netwlan0== NULL) {
        GDBusConnection *c = ultra_ams_dbus_connection();
        if(conn != NULL) {
            c= conn;
        }
        netwlan0 = ultra_net_wlan0_proxy_new_sync(c,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_PC_PATH,NULL,&error);
        //TODO: write client error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return netwlan0;
}


// Panel pc server
static void ultra_ams_pc_state(GDBusConnection *connection) {
    if(amspanslpc==NULL) {
        amspanslpc = ultra_pc_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(amspanslpc),connection ,ULTRA_AMS_PC_PATH , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
}

static void ultra_ams_apt_state(GDBusConnection *connection) {
    if(amsapt==NULL) {
        amsapt = ultra_apt_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(amsapt),connection ,ULTRA_AMS_PC_PATH , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
}
static void ultra_ams_net_state(GDBusConnection *connection) {
    if(neteth0==NULL) {
        neteth0 = ultra_net_eth0_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(neteth0),connection ,ULTRA_AMS_PC_PATH , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(netwlan0==NULL) {
        netwlan0 = ultra_net_wlan0_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(netwlan0),connection ,ULTRA_AMS_PC_PATH , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
}
void ultra_ams_panel_pc_state(GDBusConnection *connection) {
    ultra_ams_pc_state(connection);
    ultra_ams_apt_state(connection);
    ultra_ams_net_state(connection);
}

