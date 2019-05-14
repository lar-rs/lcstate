/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup AxisAxisObject
 * @{
 * @file  move-axis-object.c
 * @brief This is AXIS model object description.
 *
 *  Copyright (C) LAR  2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "axis-object.h"
#include "d3go-object.h"
#include "d3sensor-object.h"
#include "move-axis.h"
#include "move-object.h"
#include <gio/gio.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#if GLIB_CHECK_VERSION(2, 31, 7)
static GRecMutex init_rmutex;
#define MUTEX_LOCK() g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)
#else
static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
#define MUTEX_LOCK() g_static_rec_mutex_lock(&init_mutex)
#define MUTEX_UNLOCK() g_static_rec_mutex_unlock(&init_mutex)
#endif

/* signals */

enum { AXIS_MOVE_DONE, LAST_SIGNAL };

// static guint move_axis_signals[LAST_SIGNAL];

static void move_axis_base_init(gpointer g_iface) {
    static gboolean is_axis_initialized = FALSE;
    MUTEX_LOCK();
    if (!is_axis_initialized) {
        g_object_interface_install_property(
            g_iface, g_param_spec_object("node-object", "Axis reverse parameter", "Set get reverse parameter", NODES_TYPE_OBJECT, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
        g_object_interface_install_property(
            g_iface, g_param_spec_object("axis-object", "Moved Axis object", "Set get moved axis ", AXIS_TYPE_OBJECT, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
        g_object_interface_install_property(g_iface, g_param_spec_string("message", "message", "message", "", G_PARAM_READABLE | G_PARAM_WRITABLE));
        g_object_interface_install_property(g_iface, g_param_spec_uint("part", "doppelmotor3 part", "doppelmotor3 part", 1, 2, 2, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

        is_axis_initialized = TRUE;
    }
    MUTEX_UNLOCK();
}

GType move_axis_get_type(void) {
    static GType iface_type = 0;
    if (iface_type == 0) {
        static const GTypeInfo info = {sizeof(MoveAxisInterface), (GBaseInitFunc)move_axis_base_init, (GBaseFinalizeFunc)NULL, (GClassInitFunc)NULL, NULL, NULL, 0, 0, (GInstanceInitFunc)NULL, 0};
        MUTEX_LOCK();
        if (iface_type == 0) {
            iface_type = g_type_register_static(G_TYPE_INTERFACE, "AxisAxisInterface", &info, 0);
            // g_type_interface_add_prerequisite (iface_type, MKT_TYPE_MODEL);
        }
        MUTEX_UNLOCK();
    }
    return iface_type;
}

/** @} */
