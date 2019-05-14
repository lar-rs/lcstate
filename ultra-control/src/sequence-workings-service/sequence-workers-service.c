/**
 * @file node.c	node CAN interface stub - LAR market stub component
 *
 * (c) 2008 - 2011 LAR Process Analysers AG - All rights reserved.
 *
 * @author A.Smolkov
 *
 **/

// include standard header files

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <locale.h>
#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "sequence-workers-application-object.h"

static void message_critical(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)

{
    // gchar *msg = g_strdup_printf("critical - %s",message);
    mkt_log_error_message_sync("Sequence critical - %s", message);
   // tera_service_exit();
    return;
}
static void message_warning(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)

{
    mkt_log_error_message_sync("Sequence warning - %s", message);

    // g_error("Control service error - %s", message);
}




int main(int argc,char **argv)
{
	// g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, "/usr/share/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

#if GLIB_CHECK_VERSION(2,33,7)
#else
	g_type_init();
#endif

	mkt_library_init();
	mkt_errors_init(FALSE);
	mkt_errors_clean(E1740);

	TeraClientObject *client = NULL;
	tera_service_new_user_session(SEQUENCE_TYPE_WORKERS_APPLICATION_OBJECT,ULTRA_SEQUENCE_WORKERS_NAME);
	client = mkt_can_manager_client_new();
	tera_service_add_watch_client(client);
	client = tera_security_manager_client_new();
	tera_service_add_watch_client(client);
	client = ultra_axis_manager_client_new();
	tera_service_add_watch_client(client);
	client = ultra_stirrers_manager_client_new();
	tera_service_add_watch_client(client);

	client = ultra_valves_manager_client_new();
	tera_service_add_watch_client(client);
	client = tera_pumps_manager_client_new();
	tera_service_add_watch_client(client);
	client = ultra_vessels_manager_client_new();
	tera_service_add_watch_client(client);

	g_log_set_handler(NULL, G_LOG_LEVEL_CRITICAL,message_critical,NULL);
    g_log_set_handler(NULL, G_LOG_LEVEL_WARNING, message_warning,NULL);
  
	tera_service_run();
	return 0;

}
