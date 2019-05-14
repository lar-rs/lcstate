/**
 * @file node.c	node CAN interface stub - LAR market stub component
 *
 * (c) 2008 - 2011 LAR Process Analysers AG - All rights reserved.
 *
 * @author A.Smolkov
 *
 **/

// include standard header files

#include <errno.h>
#include <mktbus.h>
#include <mktlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "axis-application-object.h"
#include <locale.h>
/*
 * Ultimate control Bus states:
 *
 *
 */
#ifndef DATA_DIR
#define DATA_DIR "test"
#endif

#ifndef PACKAGE
#define PACKAGE "ultimatecontrol"
#endif

static void axis_critical_message_dummy(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) {
    mkt_log_error_message_sync("XY-System error - %s", message);
    //tera_service_exit();
    return;
}

static void
message_warning(const gchar *log_domain,
                     GLogLevelFlags log_level,
                     const gchar *message,
                     gpointer user_data )

{
    mkt_log_error_message_sync("XY-System warning - %s", message);
    // g_error("Control service error - %s", message);
    return;
}


int main(int argc, char **argv) {
//    g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, "/usr/share/locale");
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    mkt_library_init();
    mkt_errors_init(FALSE);
    mkt_error_set_service("com.lar.tera.axis");
    mkt_errors_clean(E1710);
    mkt_errors_clean(E1715);
    mkt_errors_clean(E1720);
    mkt_errors_clean(E1730);
    mkt_errors_clean(E1740);

    TeraClientObject *client = NULL;
    tera_service_new_user_session(AXIS_TYPE_APPLICATION_OBJECT, ULTRA_AXIS_NAME);
    client = mkt_can_manager_client_new();
    mkt_can_manager_add_watch_node("Digital1");
    mkt_can_manager_add_watch_node("Doppelmotor1");
    mkt_can_manager_add_watch_node("Doppelmotor2");
    tera_service_add_watch_client(client);
    g_log_set_handler(NULL, G_LOG_LEVEL_CRITICAL, axis_critical_message_dummy, tera_service_get_default());
    g_log_set_handler(NULL, G_LOG_LEVEL_WARNING,message_warning, tera_service_get_default());
    tera_service_run();
    return 0;
}
