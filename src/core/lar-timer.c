/* -*- Mo

de: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lar-mongo-timer.c
 * Copyright (C) 2017 sascha.smolkov <saschasmolkov@saschasmolkov>
 *
 * lar-mongo-timer.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * lar-mongo-timer.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lar-timer.h"

struct _LarTimerPrivate {
    gdouble  seconds;
    GTimer * timer;
    gpointer userdata;
};

enum {
    PROP_0,
    PROP_PORT,
    PROP_ADRESS,
    PROP_DBNAME,
};

G_DEFINE_TYPE(LarTimer, lar_timer, G_TYPE_OBJECT);

static void lar_timer_init(LarTimer *lar_timer) {
    lar_timer->priv          = G_TYPE_INSTANCE_GET_PRIVATE(lar_timer, LAR_TYPE_TIMER, LarTimerPrivate);
    lar_timer->priv->timer   = g_timer_new();
    lar_timer->priv->seconds = 1.0;

    /* TODO: Add initialization code here */
}

static void lar_timer_finalize(GObject *object) {
    LarTimer *timer = LAR_TIMER(object);
    if (timer->priv->timer) g_timer_destroy(timer->priv->timer);
    G_OBJECT_CLASS(lar_timer_parent_class)->finalize(object);
}

static void lar_timer_constructed(GObject *object) {
    // LarTimer *timer = LAR_TIMER(object);
    G_OBJECT_CLASS(lar_timer_parent_class)->constructed(object);
}

static void lar_timer_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(LAR_IS_TIMER(object));
    LarTimer *timer = LAR_TIMER(object);
    switch (prop_id) {
    case PROP_PORT:
        timer->priv->seconds = g_value_get_double(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void lar_timer_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(LAR_IS_TIMER(object));
    LarTimer *timer = LAR_TIMER(object);
    switch (prop_id) {
    case PROP_PORT:
        g_value_set_double(value, timer->priv->seconds);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void lar_timer_class_init(LarTimerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(LarTimerPrivate));

    object_class->finalize    = lar_timer_finalize;
    object_class->constructed = lar_timer_constructed;

    object_class->set_property = lar_timer_set_property;
    object_class->get_property = lar_timer_get_property;

    g_object_class_install_property(object_class, PROP_PORT, g_param_spec_double("timeout", "timeout", "timeout", -G_MAXDOUBLE, G_MAXDOUBLE, 0.0, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void timer_thread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable) {
    LarTimer *timeout = LAR_TIMER(source_object);
    while (TRUE) {
        if (g_task_return_error_if_cancelled(task)) return;
        if (g_timer_elapsed(timeout->priv->timer, NULL) > timeout->priv->seconds) {
            g_task_return_boolean(task, TRUE);
            return;
        }
        g_usleep(500);
    }
}

gboolean lar_timer_default_finish(GAsyncResult *result, GError **error) { return g_task_propagate_boolean(G_TASK(result), error); }

gboolean lar_timer_default_run(GCancellable *cancellable, GAsyncReadyCallback callback, gdouble seconds, gpointer user_data) {
    LarTimer *timer = g_object_new(LAR_TYPE_TIMER, "timeout", seconds, NULL);
    GTask *   task  = g_task_new(timer, cancellable, callback, user_data);
    g_task_run_in_thread(task, timer_thread);
    g_object_unref(timer);
    g_object_unref(task);
    return TRUE;
}

LarTimer *lar_timer_new(gdouble seconds) {
    LarTimer *timer = g_object_new(LAR_TYPE_TIMER, "timeout", seconds, NULL);
    return timer;
}

gboolean lar_timer_run(LarTimer *timer, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *task = g_task_new(timer, cancellable, callback, user_data);
    g_task_run_in_thread(task, timer_thread);
    g_object_unref(task);
    return TRUE;
}
