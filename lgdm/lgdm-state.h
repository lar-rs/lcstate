/**
 * file  lgdm-desktop.h object header
 * Copyright (C) LAR 2013-2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef LGDM_STATE_H_
#define LGDM_STATE_H_
#include "lgdm-state-generated-code.h"
#include "lgdm-app-generated-code.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>


GDBusObjectManagerServer* lgdm_state_app_manager  ( void );
LgdmState*     lgdm_state(void);
void           lgdm_state_login();
void           lgdm_state_logout();
gboolean       lgdm_state_login_password(guint level,const gchar *password);
const gchar*   lgdm_state_user_name();


//Clien ist uberhaupt notig?
// LgdmState*     lgdm_state_client(void);

G_END_DECLS
#endif
