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
#include <glib/gprintf.h>

#include <gtk/gtk.h>
// #include <mkt-utils.h>
// #include <mkt-error-message.h>


#include <locale.h>

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>
#include "lgdm-resources.h"

#include "lgdm-desktop.h"

/*
#include "gl-xkbd.h"
#include "gl-translation.h"
#include "gl-level-manager.h"
#include "mkt-window-manager.h"
#include "gl-controlbox.h"
#include "gl-level-notebook.h"
#include "lgdm-status.h"
#include "gl-connection.h"
#include "gl-main-build.h"
*/

/*
 * lar desktop manager application states:
 * state:10   - initial waiting
 * state:30   - reload modules.
 * state:50   - loading modules.
 * state:100  - operational.
 *
 */
/*


    LgdmDesktopApp *gdm     = LGDM_DESKTOP_APP(application);

    // g_signal_connect(pcClient, "client-done", G_CALLBACK(pc_sevice_ready), login);



	// gtk_widget_show_all(GTK_WIDGET(LGDM_LOGIN()));
	// gtk_application_add_window(GTK_APPLICATION(application),GTK_WINDOW(LGDM_LOGIN()));
	// g_signal_connect(LGDM_LOGIN(),"notify::is-booted",G_CALLBACK(lgdm_desktop_system_booted_callback),gdm);
}
*/


static void
on_activate (GtkApplication *app)
{
    g_debug("desktop activate");
    //mkt_log_message(MKT_LOG_STATE_SYSTEM, "UI start");

    LgdmDesktop *desktop = lgdm_desktop_local_get();

    gtk_application_add_window(GTK_APPLICATION(app),GTK_WINDOW(desktop));
    gtk_widget_show_all(GTK_WIDGET(desktop));
    gtk_window_set_default_size(GTK_WINDOW(desktop),800,600);
    gtk_window_set_resizable(GTK_WINDOW(desktop),FALSE);
    gtk_window_present (GTK_WINDOW(desktop));

  /* It's good practice to check your parameters at the beginning of the
	 * function. It helps catch errors early and in development instead of
	 * by your users.
	 */
	// g_assert (GTK_IS_APPLICATION (app));

	/* Get the current window or create one if necessary. */
	// window = gtk_application_get_active_window (app);
	// if (window == NULL)
	// 	window = g_object_new (ULTIMATE_TYPE_WINDOW,
	// 	                       "application", app,
	// 	                       "default-width", 600,
	// 	                       "default-height", 300,
	// 	                       NULL);

	/* Ask the window manager/compositor to present the window. */
}

int
main (int   argc,
      char *argv[])
{
	int ret;
	GtkApplication *app = NULL;

	/* Set up gettext translations */
	g_setenv ("G_MESSAGES_DEBUG","all",TRUE);
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	lgdm_get_resource();

	/*
	 * Create a new GtkApplication. The application manages our main loop,
	 * application windows, integration with the window manager/compositor, and
	 * desktop features such as file opening and single-instance applications.
	 */
	app = gtk_application_new (GUI_DBUS_NAME, G_APPLICATION_FLAGS_NONE);

	/*
	 * We connect to the activate signal to create a window when the application
	 * has been lauched. Additionally, this signal notifies us when the user
	 * tries to launch a "second instance" of the application. When they try
	 * to do that, we'll just present any existing window.
	 *
	 * Because we can't pass a pointer to any function type, we have to cast
	 * our "on_activate" function to a GCallback.
	 */
	g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

	/*
	 * Run the application. This function will block until the applicaiton
	 * exits. Upon return, we have our exit code to return to the shell. (This
	 * is the code you see when you do `echo $?` after running a command in a
	 * terminal.
	 *
	 * Since GtkApplication inherits from GApplication, we use the parent class
	 * method "run". But we need to cast, which is what the "G_APPLICATION()"
	 * macro does.
	 */
	ret = g_application_run (G_APPLICATION (app), argc, argv);

	return ret;
}
