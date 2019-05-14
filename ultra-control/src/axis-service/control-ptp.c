/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup XYSystem
 * @{
 * @file  ControlPtp-object.c
 * @brief This is ControlPtp model object description.
 *
 *  Copyright (C) LAR  2016
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "axis-application-object.h"
#include "axis-object.h"
#include "inj-valve.h"

#include "control-ptp.h"
#include "d3go-object.h"
#include "d3sensor-object.h"
#include "move-object.h"
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>

enum {
    PROP_0,
    PROP_COMMANDS,
    PROP_AXIS,
};

#include "../../config.h"
#include <glib/gi18n-lib.h>

/* signals */
// Test:'X_axe:GO(2300,3,1);Y_axe:GO(1200,3,1);Y_axe:HOLD(3,1);X_axe:HOLD(3,1);Y_axe:GO(1200,3,1);Y_axe:HOLD(3,1);X_axe:GO(2300,3,1);Y_axe:GO(1200,3,1);Y_axe:HOLD(3,1);X_axe:SENSOR();'
// static guint ControlPtp_signals[LAST_SIGNAL];

struct _ControlPtpPrivate {
    gchar *             commands;
    GScanner *          scanner;
    GList *             operations;
    GDBusObjectManager *axis_manager;
    GError *            scanner_error;
    // GSetting   *settings;
};

#define CONTROL_PTP_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CONTROL_TYPE_PTP, ControlPtpPrivate))

G_DEFINE_TYPE(ControlPtp, control_ptp, G_TYPE_OBJECT);

typedef MktTaskObject *(*GetOperation)(GScanner *scanner);

// static GQuark ptp_error_quark(void) {
//     static GQuark error;
//     if (!error) error = g_quark_from_static_string("ptp-move-error");
//     return error;
// }

void scanner_error_message_report(GScanner *scanner, gchar *message, gboolean error) {
    ControlPtp *ptp = CONTROL_PTP(scanner->user_data);
    if (ptp->priv->scanner_error) g_error_free(ptp->priv->scanner_error);
    ptp->priv->scanner_error = g_error_new(mkt_error_quark(), E1710, "scann operations error - %s", message);
}

static MktTaskObject *axis_move_to_position(AxisObject *axis, GScanner *scanner) {
    guint part    = axis_object_get_part(axis);
    guint counter = 0;
    guint pos     = 0;
    guint par     = 0;
    guint repeat  = 0;
    while (g_scanner_cur_token(scanner) != G_TOKEN_EOF) {
        if (g_scanner_cur_token(scanner) == ';') {
            break;
        } else {
            switch (g_scanner_cur_token(scanner)) {
            case G_TOKEN_INT:
                switch (counter) {
                case 0:
                    pos = g_scanner_cur_value(scanner).v_int;
                    break;
                case 1:
                    par = g_scanner_cur_value(scanner).v_int;
                    break;
                case 2:
                    repeat = g_scanner_cur_value(scanner).v_int;
                    break;
                default:
                    g_scanner_error(scanner, "Move to position too few arguments to function call error at line:%d pos:%d token:%d '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner),
                        g_scanner_cur_token(scanner), scanner->text);
                    break;
                }
                counter++;
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_LEFT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_RIGHT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_COMMA:
                g_scanner_get_next_token(scanner);
                break;
            default:
                g_scanner_error(
                    scanner, "Move wrong format  - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
                return FALSE;
            }
        }
        if (scanner->parse_errors != 0) break;
    }
    if (scanner->parse_errors == 0 && counter == 3) {
        MktTaskObject *move = MKT_TASK_OBJECT(d3go_new(axis, part, pos, par, repeat));
        mkt_task_object_set_name(move,"%s:Move(%d,%d,%d)",axis_object_get_name(axis), pos, par, repeat);
        return move;
    } else {
        g_scanner_error(scanner, "Move to position too few arguments to function call error at line:%d pos:%d token:%d '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner),
            g_scanner_cur_token(scanner), scanner->text);
    }

    return NULL;
}

static MktTaskObject *axis_move_to_sensor(AxisObject *axis, GScanner *scanner) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    guint              part = axis_object_get_part(axis);

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
                    scanner, "Sensor wrong format  - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
                return FALSE;
            }
        }
        if (scanner->parse_errors != 0) break;
    }
    if (scanner->parse_errors == 0) {
        MktTaskObject *move = MKT_TASK_OBJECT(d3sensor_new(axis, part));
        mkt_task_object_set_name(move,"%s:Sensor()",axis_object_get_name(axis));

        return move;
    }
#pragma GCC diagnostic pop

    return NULL;
}

MktTaskObject *axis_move_to_hold(AxisObject *axis, GScanner *scanner) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    AchsenAchse *      achse   = achsen_object_get_achse(ACHSEN_OBJECT(axis));
    guint              part    = axis_object_get_part(axis);
    guint              counter = 1;
    guint              pos     = 0;
    guint              par     = 0;
    guint              repeat  = 0;
    if (achse) {
        pos = achsen_achse_get_hold(achse);
    }
    while (g_scanner_cur_token(scanner) != G_TOKEN_EOF) {
        if (g_scanner_cur_token(scanner) == ';') {
            break;
        } else {
            switch (g_scanner_cur_token(scanner)) {
            case G_TOKEN_INT:
                switch (counter) {
                case 1:
                    par = g_scanner_cur_value(scanner).v_int;
                    break;
                case 2:
                    repeat = g_scanner_cur_value(scanner).v_int;
                    break;
                default:
                    g_scanner_error(scanner, "Move to hold too few arguments to function call error at line:%d pos:%d token:%d '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner),
                        g_scanner_cur_token(scanner), scanner->text);
                    break;
                }
                counter++;
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_LEFT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_RIGHT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_COMMA:
                g_scanner_get_next_token(scanner);
                break;
            default:
                g_scanner_error(
                    scanner, "Hold wrong format  - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
                return FALSE;
            }
        }
        if (scanner->parse_errors != 0) break;
    }
    if (scanner->parse_errors == 0 && counter == 3) {
        MktTaskObject *move = MKT_TASK_OBJECT(d3go_new(axis, part, pos, par, repeat));
        mkt_task_object_set_name(move,"%s:Hold(%d,%d,%d)",axis_object_get_name(axis), pos, par, repeat);

        return move;
    } else {
        g_scanner_error(scanner, "Move to hold too few arguments to function call error at line:%d pos:%d token:%d '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner),
            g_scanner_cur_token(scanner), scanner->text);
    }
#pragma GCC diagnostic pop

    return NULL;
}

static MktTaskObject *MoveX(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_X_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_X_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_position(axis, scanner);
}

static MktTaskObject *HoldX(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_X_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_X_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_hold(axis, scanner);
}
static MktTaskObject *SensorX(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_X_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_X_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_sensor(axis, scanner);
}

static MktTaskObject *MoveY(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Y_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Y_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_position(axis, scanner);
}

static MktTaskObject *HoldY(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Y_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Y_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_hold(axis, scanner);
}
static MktTaskObject *SensorY(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Y_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Y_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_sensor(axis, scanner);
}
static MktTaskObject *MoveInj(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Z_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Z_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_position(axis, scanner);
}

static MktTaskObject *HoldInj(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Z_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Z_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_hold(axis, scanner);
}

static MktTaskObject *SensorInj(GScanner *scanner) {
    AxisObject *axis = AXIS_OBJECT(g_dbus_object_manager_get_object(axis_application_get_object_manajer(), ULTRA_AXIS_Z_PATH));
    if (axis == NULL || !ACHSEN_IS_OBJECT(axis)) {
        g_scanner_error(scanner, "Axis object %s not found line:%d pos:%d token:%d '%s'", ULTRA_AXIS_Z_PATH, g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner),
            scanner->text);
        return NULL;
    }
    return axis_move_to_sensor(axis, scanner);
}



static MktTaskObject *inj_valve_create(GScanner *scanner, gboolean is_open) {
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
                    scanner, "iV wrong format  - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
                return FALSE;
            }
        }
        if (scanner->parse_errors != 0) break;
    }
    if (scanner->parse_errors == 0) {
        MktTaskObject *vopen = inj_valve_new(is_open);
        mkt_task_object_set_name(vopen,"%s",is_open?"V3Open":"V3Close");

        return vopen;
    }
#pragma GCC diagnostic pop

    return NULL;
}

static MktTaskObject *V3Open(GScanner *scanner) {
    return inj_valve_create(scanner, TRUE);
}
static MktTaskObject *V3Close(GScanner *scanner) {
    return inj_valve_create(scanner, FALSE);
}

static MktTaskObject *Wait(GScanner *scanner) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    gdouble            wait    = 0.0;
    guint              counter = 0;
    while (g_scanner_cur_token(scanner) != G_TOKEN_EOF) {
        if (g_scanner_cur_token(scanner) == ';') {
            break;
        } else {
            switch (g_scanner_cur_token(scanner)) {
            case G_TOKEN_FLOAT:
                switch (counter) {
                case 0:
                    wait = g_scanner_cur_value(scanner).v_float;
                    break;
                default:
                    g_scanner_error(scanner, "WAIT too few arguments to function call error at line:%d pos:%d token:%d '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner),
                        g_scanner_cur_token(scanner), scanner->text);
                    break;
                }
                counter++;
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_LEFT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            case G_TOKEN_RIGHT_PAREN:
                g_scanner_get_next_token(scanner);
                break;
            default:
                g_scanner_error(
                    scanner, "Wait wrong format  - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
                return FALSE;
            }
        }
        if (scanner->parse_errors != 0) break;
    }
    if (scanner->parse_errors == 0) {
        MktTaskObject *timer = MKT_TASK_OBJECT(g_object_new(MKT_TYPE_TASK_TIMER, "seconds", wait, NULL));
        mkt_task_object_set_name(timer,"Waite(%f)",wait);

        return timer;
    }
#pragma GCC diagnostic pop

    return NULL;
}

static MktTaskObject *load_operation(ControlPtp *object, GScanner *scanner) {
    GetOperation operation = (GetOperation)g_scanner_scope_lookup_symbol(scanner, 1, g_scanner_cur_value(scanner).v_identifier);
    if (operation == NULL) {
        g_scanner_error(scanner, "operation function '%s' have a wrong format - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_value(scanner).v_identifier, g_scanner_cur_line(scanner),
            g_scanner_cur_position(scanner), g_scanner_cur_token(scanner), scanner->text);
        return NULL;
    }
    g_scanner_get_next_token(scanner);
    MktTaskObject *subtask = operation(scanner);
    return subtask;
}

static void control_ptp_load_script(ControlPtp *object) {

    if (object->priv->axis_manager == NULL) {
        g_scanner_error(object->priv->scanner, "axis manager not found");
        return;
    }
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "MoveX", MoveX);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "HoldX", HoldX);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "SensorX", SensorX);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "MoveY", MoveY);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "HoldY", HoldY);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "SensorY", SensorY);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "MoveInj", MoveInj);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "HoldInj", HoldInj);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "SensorInj", SensorInj);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "AirInj", injection_ptp_air);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "V3O", V3Open);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "V3C", V3Close);
    g_scanner_scope_add_symbol(object->priv->scanner, 1, "Wait", Wait);

    // Test:'SensorY();SensorX();SensorInj();MoveX(2300,1,0);MoveY(1200,1,2);MoveInj(1600,2,0);V3O();HoldInj(2,1);V3C();SensorInj();HoldInj(1,1);MoveInj(1000,2,0);HoldY(1,1);SensorY();HoldY(1,1);HoldX(3,2);SensorX();HoldX(1,0)'


    g_scanner_input_text(object->priv->scanner, object->priv->commands, g_utf8_strlen(object->priv->commands, 4096));
    g_scanner_get_next_token(object->priv->scanner);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    while (g_scanner_cur_token(object->priv->scanner) != G_TOKEN_EOF) {
        MktTaskObject *subtask = NULL;
        switch (g_scanner_cur_token(object->priv->scanner)) {
        case G_TOKEN_IDENTIFIER:
            subtask = load_operation(object, object->priv->scanner);
            if (subtask != NULL) {
                object->priv->operations = g_list_append(object->priv->operations, subtask);
            }
            break;
        case ';':
          break;
        default:
            g_scanner_error(object->priv->scanner, "unknown symbol - error at line:%d pos:%d token:%d - '%s'", g_scanner_cur_line(object->priv->scanner), g_scanner_cur_position(object->priv->scanner),
                g_scanner_cur_token(object->priv->scanner), object->priv->scanner->text);
            break;
        }
        g_scanner_get_next_token(object->priv->scanner);
        if (object->priv->scanner->parse_errors > 0) break;
    }
#pragma GCC diagnostic pop
}

static void control_ptp_init(ControlPtp *object) {
    ControlPtpPrivate *priv                              = CONTROL_PTP_PRIVATE(object);
    object->priv                                         = priv;
    object->priv->commands                               = NULL;
    object->priv->scanner                                = g_scanner_new(NULL);
    object->priv->scanner->config->cset_skip_characters  = " 	\t\n";
    object->priv->scanner->config->scan_symbols          = TRUE;
    object->priv->scanner->config->symbol_2_token        = TRUE;
    object->priv->scanner->config->scan_string_sq        = TRUE;
    object->priv->scanner->config->scan_identifier_1char = TRUE;
    object->priv->scanner->msg_handler                   = scanner_error_message_report;
    object->priv->scanner->user_data                     = object;
    object->priv->axis_manager                           = NULL;

    // Settings property connection ...
    /* TODO: Add initialization code here */
}

static void control_ptp_constructed(GObject *object) {
    ControlPtp *control = CONTROL_PTP(object);
    control_ptp_load_script(control);
    G_OBJECT_CLASS(control_ptp_parent_class)->constructed(object);
}

static void control_ptp_finalize(GObject *object) {
    ControlPtp *control = CONTROL_PTP(object);
    if (control->priv->scanner) g_scanner_destroy(control->priv->scanner);
    if (control->priv->commands) g_free(control->priv->commands);
    if (control->priv->axis_manager) g_object_unref(control->priv->axis_manager);
    if (control->priv->operations) g_list_free_full(control->priv->operations, g_object_unref);
    if (control->priv->scanner_error) g_error_free(control->priv->scanner_error);

    G_OBJECT_CLASS(control_ptp_parent_class)->finalize(object);
}

static void control_ptp_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(CONTROL_IS_PTP(object));
    ControlPtp *control = CONTROL_PTP(object);
    switch (prop_id) {
    case PROP_COMMANDS:
        if (control->priv->commands) g_free(control->priv->commands);
        control->priv->commands = g_value_dup_string(value);
        break;
    case PROP_AXIS:
        if (control->priv->axis_manager) g_free(control->priv->axis_manager);
        control->priv->axis_manager = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void control_ptp_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(CONTROL_IS_PTP(object));
    // if(mkt_dbus_get_remote_property(MKT_DBUS_OBJECT(object),"com.lar.ControlPtpInterface",value,pspec)) return;
    ControlPtp *control = CONTROL_PTP(object);
    switch (prop_id) {
    case PROP_COMMANDS:
        g_value_set_string(value, control->priv->commands);
        break;
    case PROP_AXIS:
        g_value_set_object(value, control->priv->axis_manager);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void control_ptp_class_init(ControlPtpClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    g_type_class_add_private(klass, sizeof(ControlPtpPrivate));
    object_class->finalize     = control_ptp_finalize;
    object_class->set_property = control_ptp_set_property;
    object_class->get_property = control_ptp_get_property;
    object_class->constructed  = control_ptp_constructed;

    g_object_class_install_property(
        object_class, PROP_COMMANDS, g_param_spec_string("commands", "ptp control commands script", "ptp control commands script", "", G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
    g_object_class_install_property(
        object_class, PROP_AXIS, g_param_spec_object("axis", "axis manager", "axis manager", G_TYPE_DBUS_OBJECT_MANAGER, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));

    /*	klass->check_ControlPtpX        = NULL;
    klass->raw_value           = NULL;*/
}
static void run_next_operation(GTask *task);

static void operation_done(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GTask *task = G_TASK(user_data);
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    GError *error = NULL;
    if (MKT_IS_TASK_OBJECT(source_object)) {
        MktTaskObject *subTask = MKT_TASK_OBJECT(source_object);
        if (!mkt_task_object_finish(subTask, res, &error)) {
            g_task_return_error(task, g_error_new(mkt_error_quark(), error?error->code:E1700, "operation %s fail - %s",mkt_task_object_get_id_name(subTask),error?error->message:"unknown"));
            g_object_unref(task);
            if(error)g_error_free(error);
            return;
        }
    } else {
        g_task_return_error(task, g_error_new(mkt_error_quark(),E1710, "Incorrect operation in the operation list"));
    }
    ControlPtp *ptp = CONTROL_PTP(g_task_get_source_object(task));
    if (ptp->priv->operations == NULL) {
        g_task_return_boolean(task, TRUE);
        g_object_unref(task);
        return;
    }
    run_next_operation(task);
}

void run_next_operation(GTask *task) {
    if (g_task_return_error_if_cancelled(task)) {
        g_object_unref(task);
        return;
    }
    ControlPtp *ptp = CONTROL_PTP(g_task_get_source_object(task));
    if (MKT_IS_TASK_OBJECT(ptp->priv->operations->data)) {
        MktTaskObject *subTask = MKT_TASK_OBJECT(ptp->priv->operations->data);
        ptp->priv->operations  = g_list_remove(ptp->priv->operations, subTask);
        mkt_task_object_run(subTask, g_task_get_cancellable(task), operation_done, task);
        g_object_unref(subTask);
    } else {
        g_task_return_error(task, g_error_new(mkt_error_quark(),E1710, "Unknown operation in the operation list"));
        g_object_unref(task);
        return;
    }
}

void cotrol_ptp_run(ControlPtp *control, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *task = g_task_new(control, cancellable, callback, user_data);
    if (control->priv->scanner_error) {
        g_task_return_error(task, control->priv->scanner_error);
        g_object_unref(task);
        return;
    }
    if (control->priv->operations == NULL) {
        g_task_return_error(task, g_error_new(mkt_error_quark(), E1710, "The operation list is empty"));
        g_object_unref(task);
        return;
    }
    run_next_operation(task);
}

/** @} */
