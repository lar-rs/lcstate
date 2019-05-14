/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup XYSystem
 * @{
 * @file  D3SENSOR-object.c
 * @brief This is D3SENSOR model object description.
 *
 *  Copyright (C) LAR  2016
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "axis-object.h"
#include "d3sensor-object.h"
#include "move-axis.h"
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>

enum {
    PROP_0,
    PROP_NODE_OBJECT,
    PROP_AXIS_OBJECT,
};

#include "../../config.h"
#include <glib/gi18n-lib.h>

/* signals */

enum { D3SENSOR_DONE, LAST_SIGNAL };

// static guint D3SENSOR_signals[LAST_SIGNAL];

struct _D3SENSORPrivate {

    NodesDoppelmotor3 *doppel_motor;
    guint              curr_position;
    gint               old_position;
    gboolean           final_positon;
    gulong             handler_id;
    GCancellable *     cancellable;
    guint              replicate;
    // GSetting   *settings;
};

#define D3SENSOR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), D3SENSOR_TYPE, D3SENSORPrivate))

// ------------------------------------- initialize Axen parameter parameter ------------------------------------------------

static void D3SENSOR_GO_SENSOR_final_position_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);

static void D3SENSOR_GO_SENSOR_comman_status_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data);

static void WAITE_distance_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    if (!MOVE_command_status(MOVE_OBJECT(d3sensor), 3, g_task_get_cancellable(task), D3SENSOR_GO_SENSOR_comman_status_async_callback, task)) {
        g_task_return_error(task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - internal error")));
        g_object_unref(task);
    }
}

void D3SENSOR_GO_POSITION_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    gboolean  result;
    GError *  error = NULL;
    if (!MOVE_go_pos_finish(MOVE_OBJECT(d3sensor), &result, res, &error)) {
        g_task_return_error(
            task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - %s"), error != NULL ? error->message : "unknown"));
        if (error) g_error_free(error);
        g_object_unref(task);
        return;
    }
    lar_timer_default_run(g_task_get_cancellable(task), WAITE_distance_timeout_callback, 0.50, task);
}



static void D3SENSOR_GO_SENSOR_current_pos_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    GError *  error    = NULL;
    if (!MOVE_current_position_finish(MOVE_OBJECT(d3sensor), &d3sensor->priv->curr_position, res, &error)) {
        g_task_return_error(
            task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - %s"), error != NULL ? error->message : "unknown"));
        if (error) g_error_free(error);
        g_object_unref(task);
    } else{
      if (d3sensor->priv->curr_position == d3sensor->priv->old_position) {
         if (!MOVE_stepper_go_pos(MOVE_OBJECT(d3sensor), d3sensor->priv->curr_position + 10, g_task_get_cancellable(task), D3SENSOR_GO_POSITION_async_callback, task)) {
              g_task_return_error(task, NULL);
              g_object_unref(task);
          }
          return;
      }
      if (!MOVE_final_position(MOVE_OBJECT(d3sensor), g_task_get_cancellable(task), D3SENSOR_GO_SENSOR_final_position_async_callback, task)) {
          g_task_return_error(task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - internal error")));
          g_object_unref(task);
      }
        d3sensor->priv->old_position = d3sensor->priv->curr_position;
    }
}
static void WAITE_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    if (!MOVE_current_position(MOVE_OBJECT(d3sensor), g_task_get_cancellable(task), D3SENSOR_GO_SENSOR_current_pos_async_callback, task)) {
        g_task_return_error(task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - internal error")));
        g_object_unref(task);
    }
}

void D3SENSOR_GO_SENSOR_final_position_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    GError *  error    = NULL;
    if (!MOVE_final_position_finish(MOVE_OBJECT(d3sensor), &d3sensor->priv->final_positon, res, &error)) {
        g_task_return_error(
            task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - %s"), error != NULL ? error->message : "unknown"));
        if (error) g_error_free(error);
        g_object_unref(task);
    } else if (d3sensor->priv->final_positon) {
        g_task_return_boolean(task, TRUE);
        g_object_unref(task);
    } else if (MOVE_is_timeout(MOVE_OBJECT(d3sensor))) {
        g_task_return_error(task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - timeout")));
        g_object_unref(task);
    } else  lar_timer_default_run(g_task_get_cancellable(task), WAITE_timeout_callback, 0.03, task);
}

void D3SENSOR_GO_SENSOR_comman_status_async_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    gboolean  result;
    GError *  error = NULL;
    if (!MOVE_command_status_finish(MOVE_OBJECT(d3sensor), &result, res, &error)) {
        g_task_return_error(
            task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - %s"), error != NULL ? error->message : "unknown"));
        if (error) g_error_free(error);
        g_object_unref(task);
    } else if (!MOVE_final_position(MOVE_OBJECT(d3sensor), g_task_get_cancellable(task), D3SENSOR_GO_SENSOR_final_position_async_callback, task)) {
        g_task_return_error(task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - internal error")));
        g_object_unref(task);
    }
}
static void START_timeout_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    MOVE_message(MOVE_OBJECT(d3sensor), _("%s run stepper 2 to sensor"), MOVE_node_name(MOVE_OBJECT(d3sensor)));
    if (!MOVE_command_status(MOVE_OBJECT(d3sensor), 3, g_task_get_cancellable(task), D3SENSOR_GO_SENSOR_comman_status_async_callback, task)) {
        g_task_return_error(task, g_error_new(ERROR_QUARK, axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3sensor))), _("sensor was not reached - internal error")));
        g_object_unref(task);
    }
}
static void D3SENSOR_cancelled(GCancellable *cancel, GTask *task) {
    D3SENSOR *d3sensor = D3SENSOR_OBJECT(g_task_get_source_object(task));
    if (d3sensor->priv->curr_position > 0 && !d3sensor->priv->final_positon) {
        MOVE_stepper_go_pos(MOVE_OBJECT(d3sensor), d3sensor->priv->curr_position, NULL, NULL, NULL);
    }
}

MOVE *d3sensor_new(AxisObject *axis, guint part) {
    MOVE *move = MOVE_OBJECT(g_object_new(D3SENSOR_TYPE, "axis-object", axis, "node-object", axis_node_object(axis), "part", part, NULL));
    return move;
}

void d3sensor_run(D3SENSOR *d3sensor, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *task                 = g_task_new(d3sensor, cancellable, callback, user_data);
    d3sensor->priv->cancellable = g_object_ref(cancellable);
    d3sensor->priv->handler_id  = g_cancellable_connect(g_task_get_cancellable(task), G_CALLBACK(D3SENSOR_cancelled), task, NULL);
    MOVE_timer_start(MOVE_OBJECT(d3sensor));
    lar_timer_default_run(g_task_get_cancellable(task), START_timeout_callback, 0.020, task);
}
void d3sensor_intern_run(MktTaskObject *move, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) { d3sensor_run(D3SENSOR_OBJECT(move), cancellable, callback, user_data); }

gboolean d3sensor_intern_finish(MktTaskObject *move, GTask *subTask, GError **error) {
    gboolean result = g_task_propagate_boolean(subTask, error);
    if (*error) {
        // D3SENSOR *d3sensor = D3SENSOR_OBJECT(move);
        MOVE_object_error(MOVE_OBJECT(move), "move to sensor error %s", (*error)->message);
    } else {
        // D3SENSOR *d3sensor = D3SENSOR_OBJECT(move);
        MOVE_object_done(MOVE_OBJECT(move), "move to psensor done");
    }
    return result;
}

static void D3SENSOR_init_axis_interface(MoveAxisInterface *iface) {}

G_DEFINE_TYPE_WITH_CODE(D3SENSOR, D3SENSOR, MOVE_TYPE, G_IMPLEMENT_INTERFACE(MOVE_TYPE_AXIS, D3SENSOR_init_axis_interface))

// -------------------------------------------- Help functions --------------------------------------------------------

static void D3SENSOR_constructed(GObject *object) {
    D3SENSOR *D3SENSOR_object           = D3SENSOR_OBJECT(object);
    D3SENSOR_object->priv->doppel_motor = nodes_object_get_doppelmotor3(MOVE_node(MOVE_OBJECT(D3SENSOR_object)));
    if (G_OBJECT_CLASS(D3SENSOR_parent_class)->constructed) G_OBJECT_CLASS(D3SENSOR_parent_class)->constructed(object);
}

static void D3SENSOR_init(D3SENSOR *D3SENSOR_object) {
    D3SENSORPrivate *priv = D3SENSOR_PRIVATE(D3SENSOR_object);
    priv->cancellable     = NULL;
    priv->doppel_motor    = NULL;
    priv->replicate       = 0;
    priv->old_position    = -1;
    D3SENSOR_object->priv = priv;

    // Settings property connection ...
    /* TODO: Add initialization code here */
}

static void D3SENSOR_finalize(GObject *object) {
    D3SENSOR *D3SENSOR_object = D3SENSOR_OBJECT(object);
    if (D3SENSOR_object->priv->cancellable) {
        g_cancellable_disconnect(D3SENSOR_object->priv->cancellable, D3SENSOR_object->priv->handler_id);
        g_object_unref(D3SENSOR_object->priv->cancellable);
    }
    G_OBJECT_CLASS(D3SENSOR_parent_class)->finalize(object);
}

static void D3SENSOR_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(D3SENSOR_IS(object));
    //	D3SENSOR *D3SENSOR_object = D3SENSOR_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void D3SENSOR_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(D3SENSOR_IS(object));
    // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.D3SENSORInterface",value,pspec)) return;
    //	D3SENSOR *D3sensor1 = D3SENSOR_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void D3SENSOR_class_init(D3SENSORClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(D3SENSORPrivate));
    object_class->finalize     = D3SENSOR_finalize;
    object_class->set_property = D3SENSOR_set_property;
    object_class->get_property = D3SENSOR_get_property;
    object_class->constructed  = D3SENSOR_constructed;
    MktTaskObjectClass *mclass = MKT_TASK_OBJECT_CLASS(klass);
    mclass->run                = d3sensor_intern_run;
    mclass->finish             = d3sensor_intern_finish;

    /*	klass->check_D3SENSORX        = NULL;
    klass->raw_value           = NULL;*/
}

/** @} */
