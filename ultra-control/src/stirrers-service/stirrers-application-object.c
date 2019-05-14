/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stirrers-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum {
    PROP_0,
};

struct _StirrersApplicationObjectPrivate {
    GDBusObjectManagerServer *stirrers_manager;
};

G_DEFINE_TYPE_WITH_PRIVATE(StirrersApplicationObject, stirrers_application_object, TERA_TYPE_SERVICE_OBJECT);


static void ultra_hardware_initialize_stirrers(StirrersApplicationObject *stirrers_application) {
    MktStirrerObject *stirrer = NULL;
    stirrer                   = MKT_STIRRER_OBJECT(g_object_new(MKT_TYPE_STIRRER_OBJECT, "g-object-path", ULTRA_STIRRERS_1_PATH, "stirrer-node-address", "/com/lar/nodes/Doppelmotor1", NULL));
    StirrersSimple *  simple  = stirrers_object_get_simple(STIRRERS_OBJECT(stirrer));
    if(simple){
        stirrers_simple_set_count(simple, 1);
        g_object_unref(simple);
    }
    g_dbus_object_manager_server_export(stirrers_application->priv->stirrers_manager, G_DBUS_OBJECT_SKELETON(stirrer));
    service_simple_set_done(tera_service_get_simple(), TRUE);
    service_simple_emit_initialized(tera_service_get_simple(), TRUE);
    g_object_unref(stirrer);
}

static void stirrers_application_object_init(StirrersApplicationObject *stirrers_application_object) {
    StirrersApplicationObjectPrivate *priv = stirrers_application_object_get_instance_private(stirrers_application_object);
    priv->stirrers_manager                 = NULL;
    stirrers_application_object->priv      = priv;
}

static void stirrers_application_object_finalize(GObject *object) {
    StirrersApplicationObject *stirrers_application = STIRRERS_APPLICATION_OBJECT(object);
    if (stirrers_application->priv->stirrers_manager) g_object_unref(stirrers_application->priv->stirrers_manager);
    G_OBJECT_CLASS(stirrers_application_object_parent_class)->finalize(object);
}

static void stirrers_application_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(STIRRERS_IS_APPLICATION_OBJECT(object));
    // StirrersApplicationObject *data = STIRRERS_APPLICATION_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void stirrers_application_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(STIRRERS_IS_APPLICATION_OBJECT(object));
    // StirrersApplicationObject *data = STIRRERS_APPLICATION_OBJECT(object);
    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void stirrers_application_object_activated(TeraServiceObject *service) {
    StirrersApplicationObject *stirrers_application = STIRRERS_APPLICATION_OBJECT(service);
    stirrers_application->priv->stirrers_manager    = g_dbus_object_manager_server_new(ULTRA_STIRRERS_MANAGER);
    g_dbus_object_manager_server_set_connection(stirrers_application->priv->stirrers_manager, tera_service_dbus_connection());
    ultra_hardware_initialize_stirrers(stirrers_application);
}

static void stirrers_application_object_class_init(StirrersApplicationObjectClass *klass) {
    GObjectClass *          object_class = G_OBJECT_CLASS(klass);
    TeraServiceObjectClass *app_class    = TERA_SERVICE_OBJECT_CLASS(klass);
    object_class->finalize               = stirrers_application_object_finalize;
    object_class->set_property           = stirrers_application_object_set_property;
    object_class->get_property           = stirrers_application_object_get_property;
    app_class->activated                 = stirrers_application_object_activated;
}
