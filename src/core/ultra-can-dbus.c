/*
 * brief Ultra can api dbus server
 * copyright	 Copyright (C) LAR 2019
 * author A.Smolkov  <asmolkov@lar.com>
 *
 */

#include "ultra-can-dbus.h"
#include "ultra-ams-dbus.h"


#define ULTRA_AMS_CAN_PATH             "/lar/ams/ultra/can0"
#define ULTRA_AMS_NODE_ANALOG1         "/lar/ams/ultra/analog1"
#define ULTRA_AMS_NODE_DOPPELMOTOR1    "/lar/ams/ultra/doppelmotor1"
#define ULTRA_AMS_NODE_DOPPELMOTOR2    "/lar/ams/ultra/doppelmotor2"
#define ULTRA_AMS_NODE_DIGITAL1        "/lar/ams/ultra/digital1"
#define ULTRA_AMS_NODE_DIGITAL2        "/lar/ams/ultra/digital2"
#define ULTRA_AMS_NODE_ANALOGOUTS      "/lar/ams/ultra/analogouts"


static UltraCan0               *can0           = NULL;
static UltraNodeAnalog1        *analog2        = NULL;
static UltraNodeDoppelmotor3   *doppelmotor12  = NULL;
static UltraNodeDoppelmotor3   *doppelmotor14  = NULL;
static UltraNodeDigital16      *digital18      = NULL;
static UltraNodeDigital16      *digital19      = NULL;
static UltraNodeAnalogouts     *analogouts     = NULL;


void  ultra_can_dbus_state          ( GDBusConnection *connection){
    g_return_if_fail(connection!=NULL);
    if(can0 == NULL){
        can0 = ultra_can0_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(can0),connection ,ULTRA_AMS_CAN_PATH , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(analog2 == NULL){
        analog2 = ultra_node_analog1_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(analog2),connection ,ULTRA_AMS_NODE_ANALOG1 , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(doppelmotor12== NULL){
        doppelmotor12 = ultra_node_doppelmotor3_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(doppelmotor12),connection ,ULTRA_AMS_NODE_DOPPELMOTOR1 , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(doppelmotor14== NULL){
        doppelmotor14 = ultra_node_doppelmotor3_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(doppelmotor14),connection ,ULTRA_AMS_NODE_DOPPELMOTOR2 , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(digital18== NULL){
        digital18= ultra_node_digital16_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(digital18),connection ,ULTRA_AMS_NODE_DIGITAL1 , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(digital19== NULL){
        digital19= ultra_node_digital16_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(digital19),connection ,ULTRA_AMS_NODE_DIGITAL2 , &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
    if(analogouts== NULL){
        analogouts= ultra_node_analogouts_skeleton_new();
        GError *error = NULL;
        if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(analogouts),connection ,ULTRA_AMS_NODE_ANALOGOUTS, &error))
        {
            //TODO: write error in to logbook
            if(error) {
                g_error_free(error);
            }
        }
    }
}


// Client server functions

UltraCan0*
ultra_can_dbus(GDBusConnection *connection) {
    if( can0 == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }

        can0 = ultra_can0_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_CAN_PATH,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return can0;
}


UltraNodeAnalog1*
ultra_node_analog2(GDBusConnection *connection){
    if( analog2 == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }

        g_return_val_if_fail(conn,NULL);
        analog2 = ultra_node_analog1_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_NODE_ANALOG1,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return analog2;
}

UltraNodeDoppelmotor3*
ultra_node_doppelmotor12(GDBusConnection *connection){
    if( doppelmotor12 == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }

        g_return_val_if_fail(connection,NULL);
        doppelmotor12 = ultra_node_doppelmotor3_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_NODE_DOPPELMOTOR1,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return doppelmotor12;
}

UltraNodeDoppelmotor3*
ultra_node_doppelmoter14(GDBusConnection *connection){
    if( doppelmotor14 == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }
        doppelmotor14 = ultra_node_doppelmotor3_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_NODE_DOPPELMOTOR2,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return doppelmotor14;

}
UltraNodeDigital16*
ultra_node_digital18(GDBusConnection *connection){
    if( digital18 == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }

        g_return_val_if_fail(connection,NULL);
        digital18 = ultra_node_digital16_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_NODE_DIGITAL1,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return digital18;


}
UltraNodeDigital16*
ultra_node_digital19(GDBusConnection *connection){
    if( digital19 == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }

        g_return_val_if_fail(connection,NULL);
        digital19 = ultra_node_digital16_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_NODE_DIGITAL2,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return digital19;
}
UltraNodeAnalogouts*
ultra_node_analogouts(GDBusConnection *connection){

    if( analogouts == NULL ) {
        GError *error;
        GDBusConnection *conn = NULL;
        if(connection != NULL) {
            conn = connection;
        }else {
            conn = ultra_ams_dbus_connection();
        }

        g_return_val_if_fail(connection,NULL);
        analogouts = ultra_node_analogouts_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE,ultra_ams_dbus_name(),ULTRA_AMS_NODE_ANALOGOUTS,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return analogouts;
}
