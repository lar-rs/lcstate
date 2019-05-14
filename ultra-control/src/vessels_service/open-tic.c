/*
 * @coryright Copyright (C) LAR  2017
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include "open-tic.h"

static void open_furnace_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error      = NULL;
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    gboolean result = g_task_propagate_boolean(G_TASK(res), &error);
    if (error) {
        g_dbus_method_invocation_return_error(invocation,ERROR_QUARK,E1821,"Furnace open destination was not reached %s", error != NULL ? error->message : "unknown error");
        g_error_free(error);
    } else {
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", result));
    }
    TICPORT_SET_BUSY(ULTRA_TICPORT_OBJECT(g_task_get_source_object(G_TASK(res))), FALSE);
      TICPORT_SET_OPEN(ULTRA_TICPORT_OBJECT(g_task_get_source_object(G_TASK(res))),TRUE);
    g_object_unref(invocation);
}


static void START_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void OUT_CLOSE_BIT_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void OUT_OPEN_BIT_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void IN_OPEN_BIT_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void WAITE_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void OUT_OPEN_BIT_CLOSE_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);
static void OUT_CLOSE_BIT_OPEN_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);

void open_tic_operation(UltraTicportObject *tic, GCancellable *cancellable, GDBusMethodInvocation *invocation) {
    GTask * task  = g_task_new(tic, cancellable, open_furnace_done, g_object_ref(invocation));
    GTimer *timer = g_timer_new();
    g_timer_start(timer);
    g_task_set_task_data(task, timer, (GDestroyNotify)g_timer_destroy);
    TICPORT_SET_BUSY(tic, TRUE);
    lar_timer_default_run(g_task_get_cancellable(task), START_timeout_callback, 0.20, task);
}

void START_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    UltraTicportObject *furnace = ULTRA_TICPORT_OBJECT(g_task_get_source_object(task));
    nodes_digital16_call_set_digital_out(TICPORT_DIGITAL2(furnace), TICPORT_CLOSE_bit, FALSE, g_task_get_cancellable(task), OUT_CLOSE_BIT_callback, task);
}

void OUT_CLOSE_BIT_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    UltraTicportObject *furnace = ULTRA_TICPORT_OBJECT(g_task_get_source_object(task));
    gboolean            result  = FALSE;
    GError *            error   = NULL;
    if (!nodes_digital16_call_set_digital_out_finish(NODES_DIGITAL16(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    nodes_digital16_call_set_digital_out(TICPORT_DIGITAL2(furnace), TICPORT_OPEN_bit, TRUE, g_task_get_cancellable(task), OUT_OPEN_BIT_callback, task);
}

void OUT_OPEN_BIT_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    gboolean result = FALSE;
    GError * error  = NULL;
    if (!nodes_digital16_call_set_digital_out_finish(NODES_DIGITAL16(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    nodes_digital16_call_get_digital_in(NODES_DIGITAL16(source_object), TICPORT_IS_OPEN_bit, g_task_get_cancellable(task), IN_OPEN_BIT_callback, task);
}

void IN_OPEN_BIT_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    UltraTicportObject *furnace = ULTRA_TICPORT_OBJECT(g_task_get_source_object(task));
    gboolean            result  = FALSE;
    GError *            error   = NULL;
    if (!nodes_digital16_call_get_digital_in_finish(NODES_DIGITAL16(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    } else if (result == TRUE) {
        g_task_return_boolean(task, TRUE);
        g_object_unref(task);
    } else {
        GTimer *timer = (GTimer *)g_task_get_task_data(task);
        if (g_timer_elapsed(timer, NULL) > 5.0 && g_timer_elapsed(timer, NULL) < 5.5) {
            nodes_digital16_call_set_digital_out(TICPORT_DIGITAL2(furnace), TICPORT_OPEN_bit, FALSE, g_task_get_cancellable(task), OUT_OPEN_BIT_CLOSE_callback, task);

        } else if (g_timer_elapsed(timer, NULL) > 12.0) {
            g_task_return_error(task, g_error_new(ERROR_QUARK,E1821,"(at time)"));
            g_object_unref(task);

        } else {
            lar_timer_default_run(g_task_get_cancellable(task), WAITE_timeout_callback, 0.10, task);
        }
    }
}

void WAITE_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    UltraTicportObject *furnace = ULTRA_TICPORT_OBJECT(g_task_get_source_object(task));
    nodes_digital16_call_get_digital_in(TICPORT_DIGITAL2(furnace), TICPORT_IS_OPEN_bit, g_task_get_cancellable(task), IN_OPEN_BIT_callback, task);
}

void OUT_OPEN_BIT_CLOSE_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    UltraTicportObject *furnace = ULTRA_TICPORT_OBJECT(g_task_get_source_object(task));
    gboolean            result  = FALSE;
    GError *            error   = NULL;
    if (!nodes_digital16_call_set_digital_out_finish(NODES_DIGITAL16(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    nodes_digital16_call_set_digital_out(TICPORT_DIGITAL2(furnace), TICPORT_CLOSE_bit, TRUE, g_task_get_cancellable(task), OUT_CLOSE_BIT_OPEN_callback, task);
}

void OUT_CLOSE_BIT_OPEN_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    lar_timer_default_run(g_task_get_cancellable(task), START_timeout_callback, 0.50, task);
}
