/*
 * file  gl-desktop.c	LGDM desktop
 * brief LGDM desktop
 *
 *
 * Copyright (C) LAR 2014-2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */
#include <mkt-log.h>

#include "lgdm-state.h"

#include <gtk/gtkx.h>
#include <string.h>

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>

LgdmState* lgdm_state() {
    static LgdmState *state = NULL;
    if(state)return state;
    state = lgdm_state_skeleton_new();
    GError *error = NULL;
    GDBusConnection *connection = g_application_get_dbus_connection(g_application_get_default());
    if(!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(state),connection ,GUI_DBUS_NAME , &error)) {
        if(error) {
            mkt_log_error_message("UI state binding - %s",error->message);
            g_error_free(error);
        }
        g_object_unref(state);
        exit(1);
    }
    return state;
}

GDBusObjectManagerServer* lgdm_state_app_manager  ( void )
{
	static GDBusObjectManagerServer *ldm_server_manager = NULL;
	if(ldm_server_manager!= NULL)return ldm_server_manager;
	ldm_server_manager = g_dbus_object_manager_server_new ("/lgdm/state");
    g_dbus_object_manager_server_set_connection (ldm_server_manager, g_application_get_dbus_connection(g_application_get_default()));
	return ldm_server_manager;
}



// Client
LgdmState*  lgdm_state_client(void) {
    static LgdmState *client = NULL;
    if(client)return client;
    GError *error = NULL;
    GDBusConnection *connection = g_application_get_dbus_connection(g_application_get_default());
    client = lgdm_state_proxy_new_sync(connection,G_DBUS_PROXY_FLAGS_NONE,GUI_DBUS_NAME,"lar/ams/lgdm/state",NULL,&error);
    //TODO: write client error in to logbook
    if(error) {
        mkt_log_error_message("Client ui binding - %s",error->message);
        g_error_free(error);
    }
    return client;
}

void lgdm_state_login() {

}
gboolean  lgdm_state_login_password(guint level,const gchar *password){
    return FALSE;
}

const gchar*      lgdm_state_user_name(){
    switch(lgdm_state_get_level(lgdm_state())){
        case 1:return _("User");
        case 2:return _("Operator");
        case 3:return _("Anvance");
        case 4:return _("Service");
        case 5:return _("Root");
        default:return _("User");
    }
}
