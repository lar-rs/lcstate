/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup XYSystem
 * @{
 * @file  D3GO-object.c
 * @brief This is D3GO model object description.
 *
 *  Copyright (C) LAR  2016
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "axis-object.h"
#include "d3go-object.h"
#include "move-axis.h"
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>

enum { PROP_0, PROP_MOVE_POS, PROP_PARAMETER, PROP_REPEAT };

#include "../../config.h"
#include <glib/gi18n-lib.h>

/* signals */

enum { D3GO_DONE, LAST_SIGNAL };

// static guint D3GO_signals[LAST_SIGNAL];

struct _D3GOPrivate {
  guint move_to;
  guint curr_position;
  guint parameter;
  gboolean stal_guard;
  guint max_repeat;
  guint repeat;
  gulong handler_id;
  guint final_repeat;
  gboolean final_pos;
  GCancellable *cancellable;
  MktErrorsNumbers error;

  // GSetting   *settings;
};

#define D3GO_PRIVATE(o)                                                        \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), D3GO_TYPE, D3GOPrivate))

// ------------------------------------- initialize Axen parameter parameter
// ------------------------------------------------

static void D3GO_GO_POSITION_async_callback(GObject *source_object,
                                            GAsyncResult *res,
                                            gpointer user_data);
static void D3GO_GO_POSITION_current_pos_async_callback(GObject *source_object,
                                                        GAsyncResult *res,
                                                        gpointer user_data);

// static void stallguard_ERROR(AxisObject *axis) {
//     static MktError *E1701 = NULL;
//     if (E1701 == NULL) {
//         E1701 = mkt_error_find(MALFUNC_ULTRA_X_AXIS_MOVEMENT);
//         if (E1701 == NULL)
//             E1701 = MKT_ERROR(mkt_model_new(MKT_TYPE_ERROR_MESSAGE,
//             "error-number", MALFUNC_ULTRA_X_AXIS_MOVEMENT, "error-pending",
//             FALSE, "error-type", MKT_ERROR_WARNING, "error-description",
//                 _("XY-System: X-Axis movement error"), "error-service",
//                 mkt_error_service(), NULL));
//         mkt_error_set_pending(E1701, FALSE);
//     }
//     gchar *discription = g_strdup_printf("Stall guard flag detected axis %s",
//     g_dbus_object_get_object_path(G_DBUS_OBJECT(axis))); gchar *note        =
//     g_strdup_printf("E%d", mkt_error_number(E1701));
//     mkt_error_set_pending(E1701, TRUE);
//     mkt_error_set_description(E1701, "%s", discription);
//     g_free(note);
//     g_free(discription);
// }

void D3GO_GO_SENSOR_stall_guard_flag_finish_callback(GObject *source_object,
                                                     GAsyncResult *res,
                                                     gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  guint result;
  GError *error = NULL;
  if (!MOVE_get_stall_guard_finish(MOVE_OBJECT(d3go), &result, res, &error)) {
    g_task_return_error(task, error);
    g_object_unref(task);
  } else {
    if (result) {
      if (axis_is_activate_stall_guard(MOVE_axis(MOVE_OBJECT(d3go)))) {
        g_task_return_error(
            task,
            g_error_new(
                ERROR_QUARK,
                axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))) + 5,
                _("position deviation reported - %s"), "stallguard"));
        g_object_unref(task);
      } else {
        g_task_return_boolean(task, TRUE);
        mkt_log_error_message(
            "%s move to pos %d:%d - stall guard",
            axis_object_get_name(MOVE_axis(MOVE_OBJECT(d3go))),
            d3go->priv->move_to, d3go->priv->curr_position);
        g_object_unref(task);
      }
      // g_task_return_error(task, g_error_new(ERROR_QUARK, E1730, _("stall
      // guard is activated"))); g_object_unref(task);
    } else {
      g_task_return_boolean(task, TRUE);
      g_object_unref(task);
    }
  }
}

void D3GO_GO_SENSOR_final_position_async_callback(GObject *source_object,
                                                  GAsyncResult *res,
                                                  gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  GError *error = NULL;
  if (!MOVE_final_position_finish(MOVE_OBJECT(d3go), &d3go->priv->final_pos,
                                  res, &error)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - %s"),
                    error != NULL ? error->message : "unknown"));
    if (error)
      g_error_free(error);
    g_object_unref(task);
  } else if (d3go->priv->final_pos) {
    if (d3go->priv->repeat < d3go->priv->max_repeat) {
      gboolean out_done;
      d3go->priv->repeat++;
      if (!MOVE_parameter_sync(MOVE_OBJECT(d3go), d3go->priv->parameter,
                               &out_done, g_task_get_cancellable(task),
                               &error)) {
        g_task_return_error(
            task, g_error_new(ERROR_QUARK,
                              axis_object_get_error_number(
                                  MOVE_axis(MOVE_OBJECT(d3go))),
                              _("destination was not reached - %s"),
                              error != NULL ? error->message : "unknown"));
        if (error)
          g_error_free(error);
        g_object_unref(task);
      } else if (!MOVE_stepper_go_pos(MOVE_OBJECT(d3go), d3go->priv->move_to,
                                      g_task_get_cancellable(task),
                                      D3GO_GO_POSITION_async_callback, task)) {
        g_task_return_error(
            task,
            g_error_new(
                ERROR_QUARK,
                axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                _("destination was not reached - internal error")));
        g_object_unref(task);
      }
    } else if (d3go->priv->max_repeat > 0 && d3go->priv->move_to < 10) {
      g_task_return_boolean(task, TRUE);
      g_object_unref(task);
    } else {
      if (d3go->priv->final_repeat < 10) {
        d3go->priv->final_repeat++;
        if (!MOVE_current_position(
                MOVE_OBJECT(d3go), g_task_get_cancellable(task),
                D3GO_GO_POSITION_current_pos_async_callback, task)) {
          g_task_return_error(
              task,
              g_error_new(
                  ERROR_QUARK,
                  axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                  _("destination was not reached - internal error")));
          g_object_unref(task);
        }
        return;
      }
      g_task_return_error(
          task,
          g_error_new(
              ERROR_QUARK,
              axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
              _("destination  %d was not reached failed %d on final position"),
              d3go->priv->move_to, d3go->priv->curr_position));
      g_object_unref(task);
    }

  } else if (!MOVE_current_position(
                 MOVE_OBJECT(d3go), g_task_get_cancellable(task),
                 D3GO_GO_POSITION_current_pos_async_callback, task)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination %d was not reached on %d - internal error"),
                    d3go->priv->move_to, d3go->priv->curr_position));
    g_object_unref(task);
  }
}

static void WAITE_timeout_callback(GObject *source_object, GAsyncResult *res,
                                   gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  if (!MOVE_final_position(MOVE_OBJECT(d3go), g_task_get_cancellable(task),
                           D3GO_GO_SENSOR_final_position_async_callback,
                           task)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - internal error")));
    g_object_unref(task);
  }
}

void D3GO_GO_POSITION_current_pos_async_callback(GObject *source_object,
                                                 GAsyncResult *res,
                                                 gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  GError *error = NULL;
  if (!MOVE_current_position_finish(MOVE_OBJECT(d3go),
                                    &d3go->priv->curr_position, res, &error)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - %s"),
                    error != NULL ? error->message : "unknown"));
    if (error)
      g_error_free(error);
    g_object_unref(task);
  } else if (d3go->priv->curr_position == d3go->priv->move_to ||
             d3go->priv->curr_position == d3go->priv->move_to - 1 ||
             d3go->priv->curr_position == d3go->priv->move_to + 1) {
    if (!MOVE_get_stall_guard(MOVE_OBJECT(d3go), g_task_get_cancellable(task),
                              D3GO_GO_SENSOR_stall_guard_flag_finish_callback,
                              task)) {
      g_task_return_error(
          task, g_error_new(
                    ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - internal error")));
      g_object_unref(task);
    }
  } else if (MOVE_is_timeout(MOVE_OBJECT(d3go))) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached need %d pos %d - timeout"),
                    d3go->priv->move_to, d3go->priv->curr_position));
    g_object_unref(task);
  } else {
    lar_timer_default_run(g_task_get_cancellable(task), WAITE_timeout_callback,
                          0.04, task);
  }
}

void D3GO_GO_POSITION_async_callback(GObject *source_object, GAsyncResult *res,
                                     gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  gboolean result;
  GError *error = NULL;
  if (!MOVE_go_pos_finish(MOVE_OBJECT(d3go), &result, res, &error)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - %s"),
                    error != NULL ? error->message : "unknown"));
    if (error)
      g_error_free(error);
    g_object_unref(task);
  } else if (!MOVE_current_position(
                 MOVE_OBJECT(d3go), g_task_get_cancellable(task),
                 D3GO_GO_POSITION_current_pos_async_callback, task)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - internal error")));
    g_object_unref(task);
  }
}
static void START_timeout_callback(GObject *source_object, GAsyncResult *res,
                                   gpointer user_data) {
  GTask *task = G_TASK(user_data);
  if (g_task_return_error_if_cancelled(task)) {
    g_object_unref(task);
    return;
  }
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  GError *error = NULL;
  gboolean out_done = FALSE;
  MOVE_message(MOVE_OBJECT(d3go), _("%s run stepper 2 to position %d"),
               MOVE_node_name(MOVE_OBJECT(d3go)), d3go->priv->move_to);
  if (!MOVE_parameter_sync(MOVE_OBJECT(d3go), d3go->priv->parameter, &out_done,
                           g_task_get_cancellable(task), &error)) {
    g_task_return_error(
        task,
        g_error_new(ERROR_QUARK,
                    axis_object_get_error_number(MOVE_axis(MOVE_OBJECT(d3go))),
                    _("destination was not reached - %s"),
                    error != NULL ? error->message : "unknown"));
    if (error)
      g_error_free(error);
    g_object_unref(task);
  } else if (!MOVE_stepper_go_pos(MOVE_OBJECT(d3go), d3go->priv->move_to,
                                  g_task_get_cancellable(task),
                                  D3GO_GO_POSITION_async_callback, task)) {
    g_task_return_error(task, NULL);
    g_object_unref(task);
  }
}

static void D3GO_cancelled(GCancellable *cancel, GTask *task) {
  D3GO *d3go = D3GO_OBJECT(g_task_get_source_object(task));
  if (d3go->priv->curr_position > 0 &&
      d3go->priv->curr_position != d3go->priv->move_to) {
    MOVE_stepper_go_pos(MOVE_OBJECT(d3go), d3go->priv->curr_position, NULL,
                        NULL, NULL);
  }
}
MOVE *d3go_new(AxisObject *axis, guint part, guint to_pos, guint move_parameter,
               guint repeat) {
  if (repeat > 10)
    repeat = 10;
  MOVE *move = MOVE_OBJECT(
      g_object_new(D3GO_TYPE, "axis-object", axis, "node-object",
                   axis_node_object(axis), "part", part, "position", to_pos,
                   "parameter", move_parameter, "repeat", repeat, NULL));
  return move;
}

static void d3go_run(D3GO *d3go, GCancellable *cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
  GTask *task = g_task_new(d3go, cancellable, callback, user_data);
  d3go->priv->cancellable = g_object_ref(cancellable);
  d3go->priv->handler_id = g_cancellable_connect(
      g_task_get_cancellable(task), G_CALLBACK(D3GO_cancelled), task, NULL);
  MOVE_timer_start(MOVE_OBJECT(d3go));
  lar_timer_default_run(g_task_get_cancellable(task), START_timeout_callback,
                        0.020, task);
  // g_object_unref(task);
}
static void d3go_intern_run(MktTaskObject *move, GCancellable *cancellable,
                            GAsyncReadyCallback callback, gpointer user_data) {
  d3go_run(D3GO_OBJECT(move), cancellable, callback, user_data);
}

gboolean d3go_intern_finish(MktTaskObject *move, GTask *subTask,
                            GError **error) {
  gboolean result = g_task_propagate_boolean(subTask, error);
  D3GO *D3GO_object = D3GO_OBJECT(move);

  if (*error) {
    MOVE_object_error(MOVE_OBJECT(move), "move to pos %d error %s",
                      D3GO_object->priv->move_to, (*error)->message);
  } else {
    MOVE_object_done(MOVE_OBJECT(move), "move to pos %d done",
                     D3GO_object->priv->move_to);
  }
  return result;
}
static void D3GO_init_axis_interface(MoveAxisInterface *iface) {}

G_DEFINE_TYPE_WITH_CODE(D3GO, D3GO, MOVE_TYPE,
                        G_IMPLEMENT_INTERFACE(MOVE_TYPE_AXIS,
                                              D3GO_init_axis_interface))

// -------------------------------------------- Help functions
// --------------------------------------------------------

static void D3GO_constructed(GObject *object) {
  if (G_OBJECT_CLASS(D3GO_parent_class)->constructed)
    G_OBJECT_CLASS(D3GO_parent_class)->constructed(object);
}

static void D3GO_init(D3GO *D3GO_object) {
  D3GOPrivate *priv = D3GO_PRIVATE(D3GO_object);
  D3GO_object->priv = priv;
  D3GO_object->priv->repeat = 0;
  D3GO_object->priv->curr_position = 0;
  D3GO_object->priv->handler_id = 0;
  D3GO_object->priv->final_repeat = 0;
  D3GO_object->priv->final_pos = FALSE;
  D3GO_object->priv->cancellable = NULL;
  D3GO_object->priv->error = E1700;
}

static void D3GO_finalize(GObject *object) {
  D3GO *D3GO_object = D3GO_OBJECT(object);
  if (D3GO_object->priv->cancellable) {
    g_cancellable_disconnect(D3GO_object->priv->cancellable,
                             D3GO_object->priv->handler_id);
    g_object_unref(D3GO_object->priv->cancellable);
  }
  G_OBJECT_CLASS(D3GO_parent_class)->finalize(object);
}

static void D3GO_set_property(GObject *object, guint prop_id,
                              const GValue *value, GParamSpec *pspec) {
  g_return_if_fail(D3GO_IS(object));
  D3GO *D3GO_object = D3GO_OBJECT(object);
  switch (prop_id) {
  case PROP_MOVE_POS:
    D3GO_object->priv->move_to = g_value_get_uint(value);
    break;
  case PROP_PARAMETER:
    D3GO_object->priv->parameter = g_value_get_uint(value);
    break;
  case PROP_REPEAT:
    D3GO_object->priv->max_repeat = g_value_get_uint(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void D3GO_get_property(GObject *object, guint prop_id, GValue *value,
                              GParamSpec *pspec) {
  g_return_if_fail(D3GO_IS(object));
  // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.D3GOInterface",value,pspec))
  // return;
  D3GO *d3go = D3GO_OBJECT(object);
  switch (prop_id) {
  case PROP_MOVE_POS:
    g_value_set_uint(value, d3go->priv->move_to);
    break;
  case PROP_PARAMETER:
    g_value_set_uint(value, d3go->priv->parameter);
    break;
  case PROP_REPEAT:
    g_value_set_uint(value, d3go->priv->max_repeat);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void D3GO_class_init(D3GOClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(D3GOPrivate));
  object_class->finalize = D3GO_finalize;
  object_class->set_property = D3GO_set_property;
  object_class->get_property = D3GO_get_property;
  object_class->constructed = D3GO_constructed;
  MktTaskObjectClass *mclass = MKT_TASK_OBJECT_CLASS(klass);
  mclass->run = d3go_intern_run;
  mclass->finish = d3go_intern_finish;

  /*	klass->check_D3GOX        = NULL;
  klass->raw_value           = NULL;*/
  g_object_class_install_property(
      object_class, PROP_MOVE_POS,
      g_param_spec_uint(
          "position", "Go to position", "Go to positions", 0, G_MAXUINT16, 0,
          G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
      object_class, PROP_PARAMETER,
      g_param_spec_uint("parameter", "go speed parameter",
                        "Go to positions speed parameret", 0, G_MAXUINT16, 3,
                        G_PARAM_WRITABLE | G_PARAM_READABLE |
                            G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
      object_class, PROP_REPEAT,
      g_param_spec_uint("repeat", "search position again if fail",
                        "search position again if fail", 0, 10, 0,
                        G_PARAM_WRITABLE | G_PARAM_READABLE |
                            G_PARAM_CONSTRUCT_ONLY));
}

guint d3go_get_move_pos(D3GO *d3go) {
  g_return_val_if_fail(d3go != NULL, 0);
  g_return_val_if_fail(D3GO_IS(d3go), 0);
  return d3go->priv->move_to;
}

guint d3go_get_parameter(D3GO *d3go) {
  g_return_val_if_fail(d3go != NULL, 0);
  g_return_val_if_fail(D3GO_IS(d3go), 0);
  return d3go->priv->parameter;
}

/** @} */
