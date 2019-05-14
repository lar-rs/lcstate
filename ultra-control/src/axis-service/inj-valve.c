/*
 * @ingroup InjValve
 * @{
 * @file  mkt-task_timer.c	TaskTimer object
 * @brief This is TaskTimer object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "inj-valve.h"

#include "../../config.h"
#include "axis-application-object.h"
#include "axis-object.h"
#include "d3go-object.h"
#include "d3sensor-object.h"
#include "move-object.h"

#include <glib/gi18n-lib.h>

struct _InjValvePrivate {
    gboolean        open;
    NodesObject *   object;
    NodesDigital16 *digital16;
};

enum {
    TASK_TIMER_PROP0,
    TASK_VALVE_IS_OPEN,
};

G_DEFINE_TYPE_WITH_PRIVATE(InjValve, inj_valve, MKT_TYPE_TASK_OBJECT);

static void inj_valve_init(InjValve *inj_valve) {
    inj_valve->priv         = inj_valve_get_instance_private(inj_valve);
    inj_valve->priv->open   = FALSE;
    inj_valve->priv->object = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
    if (inj_valve->priv->object) {
        inj_valve->priv->digital16 = nodes_object_get_digital16(inj_valve->priv->object);
    }

    /* TODO: Add initialization code here */
}

static void inj_valve_finalize(GObject *object) {
    InjValve *inj_valve = INJ_VALVE(object);
    if (inj_valve->priv->object) g_object_unref(inj_valve->priv->object);
    if (inj_valve->priv->digital16) g_object_unref(inj_valve->priv->digital16);
    G_OBJECT_CLASS(inj_valve_parent_class)->finalize(object);
}

static void inj_valve_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    InjValve *task_timer = INJ_VALVE(object);
    switch (prop_id) {
    case TASK_VALVE_IS_OPEN:
        task_timer->priv->open = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void inj_valve_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    InjValve *task_timer = INJ_VALVE(object);
    switch (prop_id) {
    case TASK_VALVE_IS_OPEN:
        g_value_set_boolean(value, task_timer->priv->open);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void inj_valve_finish_intern(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    GError * error = NULL;
    gboolean result;
    if (!nodes_digital16_call_set_digital_out_finish(NODES_DIGITAL16(source_object), &result, res, &error)) {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }
    g_task_return_boolean(task, TRUE);
    g_object_unref(task);
}

void inj_valve_run(MktTaskObject *task, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    InjValve *injvalve = INJ_VALVE(task);
    GTask *   t        = g_task_new(task, cancellable, callback, user_data);
    if (injvalve->priv->digital16 == NULL) {
        g_task_return_error(t, g_error_new(mkt_task_error_quark(), E1700, "Node /com/lar/nodes/Digital1 not found"));
        return;
    }
    nodes_digital16_call_set_digital_out(injvalve->priv->digital16, 3, injvalve->priv->open, g_task_get_cancellable(t), inj_valve_finish_intern, t);
}

gboolean inj_valve_finish(MktTaskObject *self, GTask *task, GError **error) { return g_task_propagate_boolean(G_TASK(task), error); }

static void inj_valve_class_init(InjValveClass *klass) {
    GObjectClass *      object_class = G_OBJECT_CLASS(klass);
    MktTaskObjectClass *task_class   = MKT_TASK_OBJECT_CLASS(klass);
    // object_class->dispose           = mkt_atom_dispose;
    object_class->finalize     = inj_valve_finalize;
    object_class->set_property = inj_valve_set_property;
    object_class->get_property = inj_valve_get_property;
    task_class->run            = inj_valve_run;
    task_class->finish         = inj_valve_finish;

    GParamSpec *pspec;
    pspec = g_param_spec_boolean("is-open", "is open prop", "is open", FALSE, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property(object_class, TASK_VALVE_IS_OPEN, pspec);
}

MktTaskObject *inj_valve_new(gboolean is_open) {
    MktTaskObject *task = MKT_TASK_OBJECT(g_object_new(INJ_TYPE_VALVE, "is-open", is_open, NULL));
    return task;
}

MktTaskObject *injection_ptp_air(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Z_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Z_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    AchsenAchse *    achse     = achsen_object_get_achse(ACHSEN_OBJECT(axis));
    AchsenInjection *injection = achsen_object_get_injection(ACHSEN_OBJECT(axis));
    if(achse== NULL||injection == NULL ){
      g_scanner_error(scanner, "Axis object %s wrong format line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Z_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
          scanner->text);
      return NULL;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    while (g_scanner_cur_token(scanner) != G_TOKEN_EOF) {
        if (g_scanner_cur_token(scanner) == ';') {
            break;
        } else {
            switch (g_scanner_cur_token(scanner)) {
            case G_TOKEN_LEFT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_RIGHT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            default:
                g_scanner_error(
                    scanner, "wrong format  - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
                return FALSE;
            }
        }
        if (scanner->parse_errors != 0) break;
    }
    if (scanner->parse_errors == 0) {
        guint          position = achsen_achse_get_hold(achse) + (achsen_injection_get_furnace_air(injection) * 2.5);
        MktTaskObject *move     = MKT_TASK_OBJECT(d3go_new(axis, 2, position, 1, 0));
        mkt_task_object_set_name(move, "%s:Move(%d,1,0)", g_dbus_object_get_object_path(G_DBUS_OBJECT(axis)), position);
        return move;
    }
#pragma GCC diagnostic pop

    return NULL;
}

/** @} */
