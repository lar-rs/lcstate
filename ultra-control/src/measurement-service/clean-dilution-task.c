/*
* @ingroup CleanDilutionTask
 * @{
 * @file  clean_dilution-task.c	Task object
 * @brief This is Task object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include <ultimate-library.h>

#include "clean-dilution-task.h"
#include "ultimate-channel.h"
#include "ultra-control-process.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

struct _CleanDilutionTaskPrivate {

    guint clean_dilution_check_tag;
    guint waite_run;
};

/* signals */

enum { CLEAN_DILUTION_MOVE_FREE, LAST_SIGNAL };

// static guint clean_dilution_action_signals[LAST_SIGNAL]  ;

enum {
    TASK_PROP0,
};

G_DEFINE_TYPE_WITH_PRIVATE(CleanDilutionTask, clean_dilution_task, MKT_TYPE_TASK);

static void clean_dilution_task_done(CleanDilutionTask *task) { mkt_task_done(MKT_TASK(task), TRUE); }

void clean_dilution_pump_DONE(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    gboolean           is_done             = FALSE;
    CleanDilutionTask *clean_dilution_task = CLEAN_DILUTION_TASK(user_data);

    if (lar_timer_default_finish(res, NULL)) {

        pumps_pump_call_stop_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, NULL, NULL);
        mkt_task_status(MKT_TASK(clean_dilution_task), _("task done"));
        clean_dilution_task_done(clean_dilution_task);
    } else
        mkt_task_done(MKT_TASK(clean_dilution_task), FALSE);
}

static gboolean clean_dilution_task_start(MktTask *task) {

    CleanDilutionTask *clean_dilution_task = CLEAN_DILUTION_TASK(task);
    gboolean           is_done             = FALSE;

    pumps_pump_call_start_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, mkt_task_cancellable(MKT_TASK(task)), NULL);
    lar_timer_default_run(mkt_task_cancellable(MKT_TASK(clean_dilution_task)), clean_dilution_pump_DONE, 3.0, clean_dilution_task);

    return TRUE;
}

static gboolean clean_dilution_task_cancel(MktTask *task) {
    gboolean is_done = FALSE;
    pumps_pump_call_stop_sync(pumps_object_get_pump(TERA_PUMP_6()), &is_done, NULL, NULL);
    return TRUE;
}

static void clean_dilution_task_init(CleanDilutionTask *clean_dilution_task) {
    clean_dilution_task->priv = clean_dilution_task_get_instance_private(clean_dilution_task);
    tera_pumps_manager_client_new();
    ultra_vessels_manager_client_new();
}

static void clean_dilution_task_finalize(GObject *object) {
    // CleanDilutionTask *task = CLEAN_DILUTION_TASK(object);
    G_OBJECT_CLASS(clean_dilution_task_parent_class)->finalize(object);
}

static void clean_dilution_task_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    // CleanDilutionTask *task = CLEAN_DILUTION_TASK( object );
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void clean_dilution_task_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    // CleanDilutionTask *task = CLEAN_DILUTION_TASK( object );
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void clean_dilution_task_class_init(CleanDilutionTaskClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // object_class->dispose           = clean_dilution_atom_dispose;
    object_class->finalize             = clean_dilution_task_finalize;
    object_class->set_property         = clean_dilution_task_set_property;
    object_class->get_property         = clean_dilution_task_get_property;
    MKT_TASK_CLASS(klass)->run_task    = clean_dilution_task_start;
    MKT_TASK_CLASS(klass)->cancel_task = clean_dilution_task_cancel;
}

/** @} */
