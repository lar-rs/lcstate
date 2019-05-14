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

#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "sequence-workers-application-object.h"
#include "ultra-sequence-process.h"
enum {
    PROP_0,
};

struct _UltraSequenceProcessPrivate {
    GDBusMethodInvocation *invocation;
};

GCancellable *SEQUENCE_CANCELLABLE;

enum {
    STOP_STATE,
    GO_X_DRAIN,
    ON_Y_INJ,
    ON_Z_HOLD,
    ON_Z_SEQUENCE,
};

#define ULTRA_SEQUENCE_PROCESS_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), ULTRA_TYPE_SEQUENCE_PROCESS, UltraSequenceProcessPrivate))

G_DEFINE_TYPE(UltraSequenceProcess, ultra_sequence_process, SEQUENCE_TYPE_OBJECT_SKELETON)

void sequence_workers_process_change_status(SequenceWorkersProcess *sequence_worker, const gchar *format, ...) {
    g_return_if_fail(sequence_worker != NULL);
    g_return_if_fail(SEQUENCE_IS_WORKERS_PROCESS(sequence_worker));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    sequence_workers_process_set_status(sequence_worker, new_status);
    g_free(new_status);
}

void sequence_workers_process_change_status_error(SequenceWorkersProcess *sequence_worker, const gchar *format, ...) {
    g_return_if_fail(sequence_worker != NULL);
    g_return_if_fail(SEQUENCE_IS_WORKERS_PROCESS(sequence_worker));
    va_list args;
    gchar * new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    sequence_workers_process_set_status(sequence_worker, new_status);
    g_free(new_status);
}


static void ultra_sequence_process_done_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *               error            = NULL;
    UltraSequenceProcess * sequence_process = ULTRA_SEQUENCE_PROCESS(source_object);
    GDBusMethodInvocation *invocation       = G_DBUS_METHOD_INVOCATION(user_data);
    g_signal_handlers_disconnect_by_data(g_task_get_cancellable(G_TASK(res)), G_TASK(res));
    sequence_workers_process_set_busy(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), FALSE);
    sequence_workers_process_set_run(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), FALSE);
    if (ULTRA_SEQUENCE_PROCESS_GET_CLASS(sequence_process)->cancel) ULTRA_SEQUENCE_PROCESS_GET_CLASS(sequence_process)->cancel(sequence_process);

    if (g_dbus_connection_is_closed(g_dbus_method_invocation_get_connection(invocation))) return;
    if (g_cancellable_is_cancelled(g_task_get_cancellable(G_TASK(res)))) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Sequence %s cancelled", g_dbus_object_get_object_path(G_DBUS_OBJECT(sequence_process)));
    } else {
        if (!g_task_propagate_boolean(G_TASK(res), &error)) {
            sequence_workers_process_set_success(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), FALSE);
            if (error) g_dbus_error_strip_remote_error(error);
            g_dbus_method_invocation_return_error(invocation, ERROR_QUARK, error ? error->code : 500, "%s", error != NULL ? error->message : "unknown error");
            if (error == NULL || !mkt_errors_report(error->code, "%s", error->message)){
                mkt_log_error_message("Sequense %s error - %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(sequence_process)), error != NULL ? error->message : "unknown error");
            }
        } else {
            sequence_workers_process_set_success(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), TRUE);

            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", TRUE));
        }
    }
    g_object_unref(res);
    g_object_unref(invocation);
}

gboolean ultra_sequence_process_run_callback(SequenceWorkersProcess *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraSequenceProcess *sequence_process = ULTRA_SEQUENCE_PROCESS(user_data);
    if (SEQUENCE_CANCELLABLE) {
        g_cancellable_cancel(SEQUENCE_CANCELLABLE);
        g_object_unref(SEQUENCE_CANCELLABLE);
    }
    SEQUENCE_CANCELLABLE = g_cancellable_new();
    GTask *task          = g_task_new(sequence_process, SEQUENCE_CANCELLABLE, ultra_sequence_process_done_callback, g_object_ref(invocation));
    sequence_workers_process_set_busy(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), TRUE);
    sequence_workers_process_set_run(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), TRUE);
    sequence_workers_process_set_success(sequence_object_get_workers_process(SEQUENCE_OBJECT(sequence_process)), FALSE);
    sequence_workers_process_change_status(interface, "Start");
    if (ULTRA_SEQUENCE_PROCESS_GET_CLASS(sequence_process)->run) ULTRA_SEQUENCE_PROCESS_GET_CLASS(sequence_process)->run(sequence_process, task);
    return TRUE;
}

static gboolean ultra_allhold_process_stop_callback(SequenceWorkersProcess *interface, GDBusMethodInvocation *invocation, gpointer user_data) {
    UltraSequenceProcess *sequence_process = ULTRA_SEQUENCE_PROCESS(user_data);
    sequence_workers_process_change_status(interface,"Stop");
    if (SEQUENCE_CANCELLABLE) {
        g_cancellable_cancel(SEQUENCE_CANCELLABLE);
        g_object_unref(SEQUENCE_CANCELLABLE);
    }
    SEQUENCE_CANCELLABLE                                              = NULL;
    gboolean ret                                                      = TRUE;
    if (ULTRA_SEQUENCE_PROCESS_GET_CLASS(sequence_process)->stop) ret = ULTRA_SEQUENCE_PROCESS_GET_CLASS(sequence_process)->stop(sequence_process);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", ret));
    return TRUE;
}

static void ultra_sequence_process_init(UltraSequenceProcess *ultra_sequence_process) {
    UltraSequenceProcessPrivate *priv = ULTRA_SEQUENCE_PROCESS_PRIVATE(ultra_sequence_process);
    ultra_sequence_process->priv      = priv;
    /// com/lar/valves/TICVentile
}

static void ultra_sequence_process_constructed(GObject *object) {
    UltraSequenceProcess *  sequence_process = ULTRA_SEQUENCE_PROCESS(object);
    SequenceWorkersProcess *worker           = sequence_workers_process_skeleton_new();
    sequence_object_skeleton_set_workers_process(SEQUENCE_OBJECT_SKELETON(sequence_process), worker);
    g_signal_connect(worker, "handle-run", G_CALLBACK(ultra_sequence_process_run_callback), sequence_process);
    g_signal_connect(worker, "handle-stop", G_CALLBACK(ultra_allhold_process_stop_callback), sequence_process);
    g_object_unref(worker);
    if (G_OBJECT_CLASS(ultra_sequence_process_parent_class)->constructed) G_OBJECT_CLASS(ultra_sequence_process_parent_class)->constructed(object);
}

static void ultra_sequence_process_finalize(GObject *object) {
    UltraSequenceProcess *sequence = ULTRA_SEQUENCE_PROCESS(object);
    if (sequence->priv->invocation) g_object_unref(sequence->priv->invocation);
    if (SEQUENCE_CANCELLABLE) {
        g_signal_handlers_disconnect_by_data(SEQUENCE_CANCELLABLE, sequence);
    }
    G_OBJECT_CLASS(ultra_sequence_process_parent_class)->finalize(object);
}

static void ultra_sequence_process_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_SEQUENCE_PROCESS(object));
    // UltraSequenceProcess *data = ULTRA_SEQUENCE_PROCESS(object);
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_sequence_process_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(ULTRA_IS_SEQUENCE_PROCESS(object));
    // UltraSequenceProcess *data = ULTRA_SEQUENCE_PROCESS(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void ultra_sequence_process_class_init(UltraSequenceProcessClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(UltraSequenceProcessPrivate));

    object_class->finalize     = ultra_sequence_process_finalize;
    object_class->set_property = ultra_sequence_process_set_property;
    object_class->get_property = ultra_sequence_process_get_property;
    object_class->constructed  = ultra_sequence_process_constructed;
    klass->run                 = NULL;
    klass->stop                = NULL;
}
