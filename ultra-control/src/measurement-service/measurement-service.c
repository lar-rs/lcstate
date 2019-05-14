/**
 * @file node.c	node CAN interface stub - LAR market stub component
 *
 * (c) 2008 - 2011 LAR Process Analysers AG - All rights reserved.
 *
 * @author A.Smolkov
 *
 **/

// include standard header files

#include "../../config.h"
#include <errno.h>
#include <glib/gi18n-lib.h>
#include <mktbus.h>
#include <mktlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ultimate-library.h>
#include "ultraconfig.h"
#include "measurement-application-object.h"
#include <locale.h>

#define __LOG_DOMAIN ((gchar *)"ultra.measurement")

// static void
// message_dummy(const gchar *log_domain,
//                      GLogLevelFlags log_level,
//                      const gchar *message,
//                      gpointer user_data )

//     gchar *path =
//     g_build_path("/",g_get_home_dir(),"ultra-measurement.log",NULL); FILE *f
//     = fopen(path,"a"); if(f!=NULL)
//     {
//             fprintf(f,"%s:%s\n",market_db_get_date_lar_format(market_db_time_now()),message);
//             fflush(f);
//             fclose(f);
//     }
//     g_free(path);
//     return;
// }

gboolean service_connection_vanished(gpointer data){
  gchar *name = (gchar*)  data;
  g_error("Control:service %s is vanish",name?name:"unknown");
  if(name)g_free(name);
}


static void message_warning(const gchar *log_domain, GLogLevelFlags log_level,
                            const gchar *message, gpointer user_data)

{
  mkt_log_error_message_sync("Control: warning - %s", message);
  return;
}

static void message_critical(const gchar *log_domain, GLogLevelFlags log_level,
                             const gchar *message, gpointer user_data)

{
  mkt_log_error_message_sync("Control:critical - %s", message);
  mkt_errors_come(E1700);
  return;
}

static void start_axis_service(TeraClientObject *client, gboolean done,
                               gpointer data) {

  if (!ConfigureAxis(client)) {
    tera_service_print_critical("control: service %s connection fail - %s",tera_client_id(client), tera_client_critical_message(client));
    mkt_log_error_message_sync("Control service watch %s error - configure service", tera_client_id(client));
    mkt_errors_come(E1700);
    g_timeout_add(500,service_connection_vanished,g_strdup("XY-System"));
    return;
  }
}

static void start_security_service(TeraClientObject *client, gboolean done,
                                   gpointer data) {
    ConfigureSecurity(client);
}

static void start_analogs_service(TeraClientObject *client, gboolean is_done,
                                  gpointer data) {

  GDBusObjectManager *manager = tera_client_get_manager(client, "/analogs");
  if (manager == NULL) {
    GError *error = NULL;
    manager = analogs_object_manager_client_new_sync(
        tera_client_dbus_connection(client),
        G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, tera_client_id(client),
        "/analogs", NULL, &error);

    if (manager) {
      tera_client_add_manager(client, manager);
    }
  }
  ConfigureAnalogs(client);
}

static void start_relay_service(TeraClientObject *client, gboolean done,
                                gpointer data) {
  ConfigureRelays(client);
}
static void start_humidity_service(TeraClientObject *client, gboolean done,
                                   gpointer data) {
  ConfigureHumidity(client);
}
static void start_pressure_service(TeraClientObject *client, gboolean done,
                                   gpointer data) {
ConfigurePressure(client);
}
static void start_vessels_service(TeraClientObject *client, gboolean done,
                                  gpointer data) {
ConfigureVessels(client);
}
static void start_airflow_service(TeraClientObject *client, gboolean done,
                                  gpointer data) {
  ConfigureAirflow(client);
}

static void start_sensor_service(TeraClientObject *client, gboolean done,
                                  gpointer data) {
  ConfigureSensors(client);
}
static void start_sequence_service(TeraClientObject *client, gboolean done,
                                   gpointer data) {
  ConfigureSequence(client);
}

static void control_watch_service_check(TeraClientObject *client, gboolean done,
                                        gpointer data) {

  if (tera_client_is_critical(client)) {

    tera_service_print_critical("control: %s service is vanish - %s",tera_client_id(client), tera_client_critical_message(client));
    mkt_log_error_message_sync("control: %s service is - %s",tera_client_id(client), tera_client_critical_message(client));
    mkt_errors_come(E1700);
    g_timeout_add(500,service_connection_vanished,g_strdup(tera_client_id(client)));
    return;
  }
}

static void control_watch_service_lost(TeraClientObject *client,
                                       gpointer data) {

    tera_service_print_critical("control: %s service is vanish - %s",tera_client_id(client), tera_client_critical_message(client));
    mkt_log_error_message_sync("control: %s service is - %s",tera_client_id(client), tera_client_critical_message(client));
    mkt_errors_come(E1700);
    g_timeout_add(500,service_connection_vanished,g_strdup(tera_client_id(client)));
}

int main(int argc, char **argv) {

  // g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, "/usr/share/locale");
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
  mkt_errors_init(TRUE);

  g_log_set_handler(NULL, G_LOG_LEVEL_CRITICAL, message_critical, __LOG_DOMAIN);
  g_log_set_handler(NULL, G_LOG_LEVEL_WARNING, message_warning, __LOG_DOMAIN);

#if GLIB_CHECK_VERSION(2, 33, 7)
#else
  g_type_init();
#endif
  mkt_library_autocheck_sync();
  mkt_library_init();
  ConfigureBindRedis();
  mkt_error_set_service(TERA_MEASUREMENT_NAME);
  TeraClientObject *client = NULL;
  tera_service_new_user_session(MEASUREMENT_TYPE_APPLICATION_OBJECT,
                                TERA_MEASUREMENT_NAME);

  client = mkt_can_manager_client_new();
  tera_service_add_watch_client(client);
  mkt_can_manager_add_watch_node("Digital1");
  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = mkt_pc_manager_client_new();
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);

  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = tera_security_manager_client_new();
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_security_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = TERA_CLIENT_OBJECT(g_object_new(TERA_TYPE_CLIENT_OBJECT, "server-id",
                                           "com.lar.analogs.out", NULL));
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_analogs_service),
                   NULL);
  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = TERA_CLIENT_OBJECT(tera_relays_manager_client_new());
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_relay_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = TERA_CLIENT_OBJECT(g_object_new(TERA_TYPE_CLIENT_OBJECT, "server-id",
                                           "com.lar.sensor.humidity", NULL));
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_humidity_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = TERA_CLIENT_OBJECT(g_object_new(TERA_TYPE_CLIENT_OBJECT, "server-id",
                                           "com.lar.sensor.pressure", NULL));
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_pressure_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = ultra_stirrers_manager_client_new();
  tera_service_add_watch_client(client);

  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = tera_pumps_manager_client_new();
  tera_service_add_watch_client(client);

  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = ultra_valves_manager_client_new();
  tera_service_add_watch_client(client);

  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = ultra_vessels_manager_client_new();
  tera_service_add_watch_client(client);

  g_signal_connect(client, "client-done", G_CALLBACK(start_vessels_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = ultra_axis_manager_client_new();
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_axis_service), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = ultra_airflow_manager_client_new();
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_airflow_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = TERA_CLIENT_OBJECT(g_object_new(TERA_TYPE_CLIENT_OBJECT, "server-id",
                                           SENSORS_PLUG_NAME, "client-timeout",
                                           50.0, NULL));
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = TERA_CLIENT_OBJECT(g_object_new(TERA_TYPE_CLIENT_OBJECT, "server-id",
                                           SENSORS_SIGNAL_NAME,
                                           "client-timeout", 50.0, NULL));
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done",
                   G_CALLBACK(start_sensor_service), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);

  client = ultra_sequence_workers_manager_client_new();
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done", G_CALLBACK(start_sequence_service),
                   NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);
  client = TERA_CLIENT_OBJECT(g_object_new(TERA_TYPE_CLIENT_OBJECT, "server-id",
                                           "com.lar.protocol.rs232", NULL));
  tera_service_add_watch_client(client);
  g_signal_connect(client, "client-done",
                   G_CALLBACK(control_watch_service_check), NULL);
  g_signal_connect(client, "client-lost",
                   G_CALLBACK(control_watch_service_lost), NULL);
  tera_service_run();
  return 0;
}
