/*
 *  Copyright (C) LAR  2019
 *
 *  author A.Smolkov <asmolkov@lar.com>
 *
 */


#include "ultimate-config.h"
#include <ultra-ams-dbus.h>
#include <ultra-errors-generated-code.h>
#include <ultra-level-generated-code.h>


// #define  ULTRA_AMS_DBUS_NAME            "lar.ams.ultra"

// static guint watch_id = 0;


const gchar*       ultra_ams_dbus_name         ( void ){
    return ULTIMATE_DBUS_NAME;
}
GBusType           ultra_ams_dbus_type         ( void ){
    return G_BUS_TYPE_SESSION;
}


static UltraControl     *ultracontrol = NULL;
static UltraStirrer     *ultrastirrer = NULL;
static UltraTemperatur  *ultratemperatur = NULL;
// static UltraErrors      *errors = NULL;
// static UltraAirflow         *airflow = NULL;

// static UltraAirport         *airport = NULL;
// static UltraAxisX           *xaxis = NULL;
// static UltraAxisY           *yaxis = NULL;
// static UltraXysystem        *xysys = NULL;
// static UltraInjection       *injection = NULL;
static UltraAmsMeasurement measurement1  =  {1,FALSE,FALSE,ULTRA_AMS_MEASUREMENT1_PATH,"#01",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static UltraAmsMeasurement measurement2  =  {2,FALSE,FALSE,ULTRA_AMS_MEASUREMENT2_PATH,"#02",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static UltraAmsMeasurement measurement3  =  {3,FALSE,FALSE,ULTRA_AMS_MEASUREMENT3_PATH,"#03",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static UltraAmsMeasurement measurement4  =  {4,FALSE,FALSE,ULTRA_AMS_MEASUREMENT4_PATH,"#04",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static UltraAmsMeasurement measurement5  =  {5,FALSE,FALSE,ULTRA_AMS_MEASUREMENT5_PATH,"#05",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static UltraAmsMeasurement measurement6  =  {6,FALSE,FALSE,ULTRA_AMS_MEASUREMENT6_PATH,"#06",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

static gchar *dbus_remote_address = NULL;
static GBusType dbustype = G_BUS_TYPE_STARTER;
static guint dconn_watch_id   = 0;

#define checkerror(e) do{                                  \
        if(e){                                             \
           /*TODO: write error message in logbook*/   \
            g_error_free(error);                           \
            error = NULL;                                  \
        }                                                  \
   }while(0)

void ultra_ams_dbus_remote(gchar *ip,guint port) {
    g_return_if_fail(dconn_watch_id!=0);
    if(dbus_remote_address) {
        g_free(dbus_remote_address);
    }
    dbus_remote_address = g_strdup_printf("tcp:host=%s,port=%d",ip,port);
    g_message("AMS switch dbus to remote bus:%s", dbus_remote_address);
}

void ultra_ams_dbus_local(){
     g_return_if_fail(dconn_watch_id!=0);
    if(dbus_remote_address) {
        g_free(dbus_remote_address);
        dbus_remote_address = NULL;
    }
    g_message("AMS switch dbus to local");
}
const gchar* ultra_ams_dbus_address() {
    return dbus_remote_address;
}
static UltraAmsMeasurement* get_measurement( guint number ) {
    switch (number)
   {
       case ULTRA_AMS_MEASUREMENT1:
           return &measurement1;
       case ULTRA_AMS_MEASUREMENT2:
           return &measurement2;
       case ULTRA_AMS_MEASUREMENT3:
           return &measurement3;
       case ULTRA_AMS_MEASUREMENT4:
           return &measurement4;
       case ULTRA_AMS_MEASUREMENT5:
           return &measurement5;
       case ULTRA_AMS_MEASUREMENT6:
           return &measurement6;
       default:
           return &measurement1;
   }
}

    // UltraStream            *stream;
    // UltraProcessOnline      *online;
    // UltraProcessSingle      *single;
    // UltraProcessCalibration *calibration;
    // UltraProcessCheck       *check;
    // UltraChannelTc          *tc;
    // UltraChannelTic         *tic;
    // UltraChannelToc         *toc;
    // UltraChannelCodo        *codo;
    // UltraChannelTnb         *tnb;

static void remove_measurement(guint number) {
    UltraAmsMeasurement *measurement  = get_measurement(number);
    g_return_if_fail(measurement!=NULL);
    if(measurement->stream) {
        g_object_unref(measurement->stream);
        measurement->stream = NULL;
    }
    if(measurement->tc){
        g_object_unref(measurement->tc);
        measurement->tc = NULL;
    }
    if(measurement->tic){
        g_object_unref(measurement->tic);
        measurement->tic = NULL;
    }
    if(measurement->toc){
        g_object_unref(measurement->toc);
        measurement->toc = NULL;
    }
    if(measurement->codo){
        g_object_unref(measurement->codo);
        measurement->codo = NULL;
    }
    if(measurement->tnb){
        g_object_unref(measurement->tnb);
        measurement->tnb = NULL;
    }
    if(measurement->online){
        g_object_unref(measurement->online);
        measurement->online = NULL;
    }
    if(measurement->single){
        g_object_unref(measurement->single);
        measurement->single= NULL;
    }
    if(measurement->calibration){
        g_object_unref(measurement->calibration);
        measurement->calibration = NULL;
    }
    if(measurement->check){
        g_object_unref(measurement->check);
        measurement->check= NULL;
    }
}

// static void remove_connected_objects() {
//     if(ultracontrol) {
//         g_object_unref(ultracontrol);
//     }
//     gint i =0;
//     for(i=0;i<=ULTRA_AMS_MEASUREMENT6;i++) {
//         remove_measurement(i);
//     }
// }

static void
on_ams_name_appeared (GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data)
{
	g_print ("Name %s on %s is owned by %s\n",
			name,
			"the system bus",
			name_owner);
}

static void
on_ams_name_vanished (GDBusConnection *connection, const gchar     *name, gpointer         user_data)
{
  g_print ("Name %s does not exist on %s\n",
           name,
          "the system bus");
}

static GDBusConnection* client_connection() {
    static GDBusConnection * default_conn= NULL;
    GError *error = NULL;
    if(default_conn == NULL){
       if(dconn_watch_id) {
            default_conn = g_dbus_connection_new_for_address_sync(dbus_remote_address, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,NULL, NULL,&error);
       } else {
            default_conn = g_application_get_dbus_connection(g_application_get_default());
            if(default_conn == NULL)
                default_conn = g_bus_get_sync(dbustype,NULL,&error);
        }
        dconn_watch_id =  g_bus_watch_name_on_connection(default_conn,ultra_ams_dbus_name(), G_BUS_NAME_WATCHER_FLAGS_AUTO_START, on_ams_name_appeared, on_ams_name_vanished,NULL,NULL);
    }
    return default_conn;
}


GDBusConnection * dbus_connection(GDBusConnection *connection) {
    if(connection) return connection;
    return client_connection();
}


UltraControl* ultra_ams_control( GDBusConnection *connection ){
    if(ultracontrol == NULL) {
        GError *error = NULL;
        ultracontrol = ultra_control_proxy_new_sync(dbus_connection(connection), G_DBUS_PROXY_FLAGS_NONE,
                                                ultra_ams_dbus_name(),
                                                ULTRA_AMS_CONTROL_PATH, NULL, &error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return ultracontrol;
}


void channels_connect(UltraAmsMeasurement *measurement) {
    g_return_if_fail(measurement!=NULL);
    GError *error = NULL;
    if(measurement->tc) g_object_unref(measurement->tc);
    measurement->tc = ultra_channel_tc_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
    if(measurement->tic) g_object_unref(measurement->tc);
   measurement->tic = ultra_channel_tic_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
    if(measurement->toc) g_object_unref(measurement->toc);
   measurement->toc = ultra_channel_toc_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
    if(measurement->tnb) g_object_unref(measurement->tnb);
   measurement->tnb = ultra_channel_tnb_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
    if(measurement->codo) g_object_unref(measurement->codo);
   measurement->codo = ultra_channel_codo_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
}

static void process_connect(UltraAmsMeasurement *measurement) {
    g_return_if_fail(measurement!=NULL);
    GError *error = NULL;
    if(measurement->online) g_object_unref(measurement->online);
    measurement->online = ultra_process_online_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
    checkerror(error);

    if(measurement->single) g_object_unref(measurement->single);
    measurement->single = ultra_process_single_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
    if(measurement->calibration) g_object_unref(measurement->calibration);
    measurement-> calibration = ultra_process_calibration_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);
    if(measurement->check) g_object_unref(measurement->check);
    measurement->check = ultra_process_check_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
   checkerror(error);



}


static void stream_connect(UltraAmsMeasurement *measurement) {
    g_return_if_fail(measurement!=NULL);
    GError *error = NULL;
    if(measurement->stream!=NULL) g_object_unref(measurement->stream);
    measurement->stream = ultra_stream_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE,
                                                            ultra_ams_dbus_name(),measurement->path,
                                                            NULL,&error);
    checkerror(error);
}

UltraAmsMeasurement* ultra_ams_measurement(guint number) {
    UltraAmsMeasurement *measurement = get_measurement(number);
    if(!measurement->initialized) {
        ultra_ams_control(NULL);
        stream_connect(measurement);
        channels_connect(measurement);
        process_connect(measurement);
        measurement->initialized = TRUE;
    }
    return measurement;
}

GList*   ultra_ams_measurements_list  ( ) {
    GList *list = NULL;
    gint i=1;
    for(i=1;i<7;i++){
        list = g_list_append(list,ultra_ams_measurement(i));
    }
    return list;
}

UltraAmsMeasurement*   ultra_ams_measurement1  ( void ){
    return ultra_ams_measurement(ULTRA_AMS_MEASUREMENT1);
}
UltraAmsMeasurement*   ultra_ams_measurement2  ( void ){
    return ultra_ams_measurement(ULTRA_AMS_MEASUREMENT2);
}
UltraAmsMeasurement*   ultra_ams_measurement3  ( void ){
    return ultra_ams_measurement(ULTRA_AMS_MEASUREMENT3);
}
UltraAmsMeasurement*   ultra_ams_measurement4  ( void ){
    return ultra_ams_measurement(ULTRA_AMS_MEASUREMENT4);
}
UltraAmsMeasurement*   ultra_ams_measurement5  ( void ){
    return ultra_ams_measurement(ULTRA_AMS_MEASUREMENT5);
}
UltraAmsMeasurement*   ultra_ams_measurement6  ( void ){
    return ultra_ams_measurement(ULTRA_AMS_MEASUREMENT6);
}

UltraTemperatur* ultra_ams_temperatur()
{
    if(ultratemperatur == NULL) {
        GError *error = NULL;
        ultratemperatur=ultra_temperatur_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE, ultra_ams_dbus_name(),ULTRA_AMS_CONTROL_PATH, NULL,&error);
        checkerror(error);
    }
	return ultratemperatur;
}

UltraStirrer*
ultra_ams_stirrer()
{
    if(ultrastirrer== NULL) {
        GError *error = NULL;
        ultrastirrer=ultra_stirrer_proxy_new_sync(client_connection(),G_DBUS_PROXY_FLAGS_NONE, ultra_ams_dbus_name(),ULTRA_AMS_CONTROL_PATH, NULL,&error);
        checkerror(error);
    }
	return ultrastirrer;
}

GDBusConnection *
ultra_ams_dbus_connection() {
    return client_connection();
}
// Ultra level system
static UltraLevel  *ultralevel   = NULL;

UltraLevel*
ultra_ams_level ( GDBusConnection *connection ) {
   if(ultralevel == NULL) {
       GError *error = NULL;
        GDBusConnection *conn = NULL;
        if(connection){ conn = connection;}
        else { conn = client_connection();}
        ultralevel = ultra_level_proxy_new_sync(conn,G_DBUS_PROXY_FLAGS_NONE, ultra_ams_dbus_name(),ULTRA_AMS_LEVEL_PATH ,NULL,&error);
        //TODO: write error in to logbook
        if(error) {
            g_error_free(error);
        }
    }
    return ultralevel;
}
static GDBusObjectManager *errorsmanager = NULL;


GDBusObjectManager*
ultra_ams_errors                                     (GDBusConnection *connection) {
    if(errorsmanager == NULL) {
        GError *error = NULL;
        GDBusConnection *conn = NULL;
        if(connection){ conn = connection;}
        else { conn = client_connection();}
        //TODO: write error in to logbook
        errorsmanager = monitoring_object_manager_client_new_sync(conn,G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, ULTIMATE_DBUS_NAME, ULTRA_AMS_ERRORS_PATH, NULL, &error);
        if(error) {
            g_error_free(error);
        }
     }
    return errorsmanager;
}
