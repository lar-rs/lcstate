/**
 * @file node.c	node CAN interface stub - LAR market stub component
 *
 * (c) 2008 - 2011 LAR Process Analysers AG - All rights reserved.
 *
 * @author A.Smolkov
 *
 **/

// include standard header files

#include "node-analog-object.h"
#include "node-analogext-object.h"
#include "node-control-app-object.h"
#include "node-device-object.h"
#include "node-digital-object.h"
#include "node-index-object.h"
#include "node-motor-object.h"
#include "node-object.h"
#include <errno.h>
#include <glib/gprintf.h>
#include <mktbus.h>
#include <mktlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

/*
 * Can Bus states:
 *    10 : device not found  ( not ready for use )
 *    20 : configuration file failed ( not ready for use )
 *    30 : Bus module load error ( not ready for use )
 *	  50 : Open can device.
 *	  70 : Init all nodes
 *    100 : operation state ( ready for use )
 */

static void critical_message_dummy(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    // gchar *msg = g_strdup_printf("critical - %s",message);
    ServiceSimple *simple = tera_service_get_simple();
    if (simple) {
        service_simple_set_warning_msg(simple, message);
        guint i = service_simple_get_warning(simple);
        service_simple_set_warning(simple, i + 1);
    }
    return;
}

static void warning_message_dummy(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) {

    ServiceSimple *simple = tera_service_get_simple();
    if (simple) {
        service_simple_set_warning_msg(simple, message);
        guint i = service_simple_get_warning(simple);
        service_simple_set_warning(simple, i + 1);
    }
    return;
}

static void node_bus_service_activated_callback (TeraServiceObject* service, gpointer data)
{
	node_control_app_new (tera_service_dbus_connection());
}

int main (int argc, char** argv)
{
    tera_service_new_system             (CAN_SERVICE_NAME);
    tera_service_add_activated_callback (G_CALLBACK(node_bus_service_activated_callback), NULL);
    g_log_set_handler                   (NULL, G_LOG_LEVEL_CRITICAL, critical_message_dummy, tera_service_get_default());
    g_log_set_handler                   (NULL, G_LOG_LEVEL_WARNING,  warning_message_dummy,  tera_service_get_default());
    tera_service_run();
    return 0;
}
