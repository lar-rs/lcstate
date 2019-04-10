/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 *
 * Author: A. Smolkov
 *
 */

#include <errno.h>
#include <glib/gprintf.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


#include <gio/gio.h>
#include <glib/gprintf.h>

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>

#include "ultra-state.h"

/*
 * Can Bus states:
 *    10 : device not found  ( not ready for use )
 *    20 : configuration file failed ( not ready for use )
 *    30 : Bus module load error ( not ready for use )
 *	  50 : Open can device.
 *	  70 : Init all nodes
 *    100 : operation state ( ready for use )
 *
 *
 *
 *
 */


int main (int argc, char** argv)
{
	GApplication* app;
	int           status;
  GOptionContext* context = NULL;
  GError* error = NULL;
  gboolean version = FALSE;
  GOptionEntry main_entries[] = {
      { "version", 0, 0, G_OPTION_ARG_NONE, &version, "Show program version" },
      { NULL }
  };

  context = g_option_context_new ("- my command line tool");
  g_option_context_add_main_entries (context, main_entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error))
  {
    g_printerr ("%s\n", error->message);
    return EXIT_FAILURE;
  }

  if (version)
  {
    g_printf ("%s\n", PACKAGE_VERSION);
    return EXIT_SUCCESS;
  }
  // g_printf ("%s.main[%d]: app = can_manager_app_new (CAN_SERVICE_NAME, G_APPLICATION_IS_SERVICE);\n", __FILE__, __LINE__), fflush(stdout);
  app = ultra_state_new (ULTIMATE_DBUS_NAME, G_APPLICATION_IS_SERVICE);
  // g_printf ("%s.main[%d]: g_application_set_inactivity_timeout (app, 1000000);\n", __FILE__, __LINE__), fflush(stdout);
  g_application_set_inactivity_timeout (app, 1000000);

	// g_signal_connect   (app, "activate", G_CALLBACK (activate), NULL);

	// Wenn das 'g_application_hold' an dieser Stelle entfernt wird, so beendet
	// sich die Applikation sofort.
	g_application_hold (app);

	status = g_application_run (app, argc, argv);

	g_object_unref (app);

	return status;
}
