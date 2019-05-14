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

#include "stirrers-application-object.h"





int main(int argc,char **argv)
{
//	g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, "/usr/share/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

#if GLIB_CHECK_VERSION(2,33,7)
#else
	g_type_init();
#endif

	mkt_library_init();
	TeraClientObject *client = NULL;
	tera_service_new_user_session(STIRRERS_TYPE_APPLICATION_OBJECT,ULTRA_STIRRERS_NAME);
	client = mkt_can_manager_client_new();
	tera_service_add_watch_client(client);
	client = tera_security_manager_client_new();
	tera_service_add_watch_client(client);
	tera_service_run();
	return 0;
}


