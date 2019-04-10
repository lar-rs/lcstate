/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * can-manager-app.h
 *
 * Author: A. Smolkov / G. Stützer
 *
 */

#ifndef __ULTRA_STATE_OBJECT_H_
#define __ULTRA_STATE_OBJECT_H_


#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS


GApplication*       ultra_state_new             (const gchar* path, GApplicationFlags flags);
// void can_manager_app_new (GDBusConnection* connection);

// GDBusObjectManager* can_manager_app_device_manager (void);
// GDBusObjectManager* can_manager_app_nodes_manager  (void);


G_END_DECLS

#endif
