/*
 * @copyright  Copyright (C) LAR  2015
 * @author A.Smolkov <asmolkov@lar.com>
 */
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ultimate-library.h>
#include "channels-generated-code.h"

#include "mkt-process-object.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
    PROCESS_WILL_START,
    PROCESS_START,
    PROCESS_STOP,
    PROCESS_WILL_BREAK,
    PROCESS_WILL_NEXT,
    PROCESS_ONLINE,
    PROCESS_OFFLINE,
    PROCESS_LAST_SIGNAL
};

static guint mkt_process_object_signals[PROCESS_LAST_SIGNAL];

enum
{
    STATUS_RUN,
    STATUS_DONE,
    STATUS_ERROR,
    STATUS_CANCELLED
};

struct _MktProcessObjectPrivate
{
    ProcessSimple *simple;
    MktModel *cathegory;
    MktModel *statistic;
    MktProcess *process;
    GList *channels;
    guint identification;
    MktError *warning;
    MktError *critical;
    guint status;
    gboolean activated;
    gboolean remote_signal;
    guint remote_mess;
    MktProcessTaskDoneCallback task_done;
};

enum
{
    PROCESS_PROP0,
    PROCESS_DEFAULT_INTERVAL,
    PROCESS_DEFAULT_CATHEGORY,
    PROCESS_DEFAULT_STATISTIC,
    PROCESS_DEFAULT_IDENTIFICATION,
    PROCESS_DEFAULT_STATUS
};

typedef struct _MktTaskMachineTypes MktTaskMachineTypes;

G_DEFINE_TYPE_WITH_PRIVATE(MktProcessObject, mkt_process_object, PROCESS_TYPE_OBJECT_SKELETON);

gboolean mkt_process_calculate_measurement_statistic(MktProcessObject *process, ChannelsObject *channel)
{
    guint current_replicates;
    guint max_replicates;
    guint outlier;
    gdouble max_cv;
    gboolean THRESHOLD = FALSE;
    ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));

    current_replicates = process_simple_get_current_replicate(simple);
    max_replicates = process_simple_get_replicates(simple);
    outlier = process_simple_get_outliers(simple);
    max_cv = process_simple_get_max_cv(simple);

    ChannelsMeasurement *meas = channels_object_get_measurement(CHANNELS_OBJECT(channel));
    ChannelsSimple *channel_simple = channels_object_get_simple(CHANNELS_OBJECT(channel));

    if (meas == NULL)
    {
        channels_simple_set_measure_error(channel_simple, TRUE);
        return FALSE;
    }
    gboolean enoughReplicates = FALSE;

    GSList *data = mkt_model_select(MKT_TYPE_MEASUREMENT_DATA, "select * from $tablename where measurement_channel = %" G_GUINT64_FORMAT " and measurement_trigger = %u",
                                    channels_simple_get_link(channel_simple), channels_simple_get_measurement(channel_simple));
    if (data == NULL)
    {
        channels_simple_set_measure_error(channel_simple, TRUE);
        return FALSE;
    }
    guint curr_replicate = g_slist_length(data);
    gint curr_outlier = 0;
    channels_measurement_set_curr_replicate(channels_object_get_measurement(CHANNELS_OBJECT(channel)), current_replicates);
    GSList *l = NULL;
    for (l = data; l != NULL; l = l->next)
    {
        // mkt_model_print_stdout(MKT_MODEL(l->data));
        mktMPSet(MKT_MODEL(l->data), "measurement-outlier", FALSE);
    }
    // g_debug("Calculate channel %s statistic
    // ",g_dbus_object_get_object_path(G_DBUS_OBJECT(channel)));
    gdouble raw = 0.;
    // mkt_debug("New value %d = %f", mchannel->priv->curr_replicate, );
    gdouble standardDeviation = 0.;
    gdouble max_d_sqr = 0., d_sqr = 0.;
    gdouble sum_d_sqr = 0.;
    gdouble cv = 0.0;
    guint interrupt = 0;

    // g_debug("Replicate %d repCounter=%d outlier= %d enjoy replicates
    // %d",outlier,max_replicates, curr_replicate, enoughReplicates);
    while (!enoughReplicates)
    {
        curr_replicate = 0;
        curr_outlier = 0;
        interrupt++;
        // g_debug("START:Replicate %d",max_replicates);
        raw = 0.0;

        for (l = data; l != NULL; l = l->next)
        {
            if (!mkt_measurement_outlier(MKT_MEASUREMENT(l->data)))
            {
                raw += mkt_measurement_value(MKT_MEASUREMENT(l->data));
                curr_replicate++;
            }
            else
            {
                curr_outlier++;
            }
        }
        // g_debug("BERECHNET: Replicate %d repCounter=%d outlier= %d curr_outlier =
        // %d enjoy replicates %d",max_replicates, curr_replicate,outlier,
        // curr_outlier, enoughReplicates);
        raw /= curr_replicate;
        if (max_replicates == 1)
        {
            enoughReplicates = TRUE;
            // TEST:	g_debug("measurement statistics done max_replicates == 1;
            // enoughReplicates=TRUE");
            break;
        }
        if (1 == curr_replicate)
        {
            // TEST:	g_debug(" measurement statistics break waiting for 2nd
            // replicate\n");
            break;
        }
        sum_d_sqr = 0.;
        for (l = data; l != NULL; l = l->next)
            if (!mkt_measurement_outlier(MKT_MEASUREMENT(l->data)))
                sum_d_sqr += (mkt_measurement_value(MKT_MEASUREMENT(l->data)) - raw) * (mkt_measurement_value(MKT_MEASUREMENT(l->data)) - raw);

        standardDeviation = sqrt(sum_d_sqr / (curr_replicate - 1));
        // g_debug("standardDeviation = %f",standardDeviation);
        // g_debug("fabs( mchannel->priv->meanValue) = %f",fabs( raw));
        if (raw > 0.000000)
            cv = 100. * standardDeviation / fabs(raw);

        if (max_replicates > curr_replicate)
        {
            // TEST:	g_debug(" measurement statistics break with
            // max_replicates>curr_replicate");
            break;
        }
        if (max_replicates <= curr_replicate && (cv < max_cv || cv < 0.0099))
        {
            enoughReplicates = TRUE;
            // TEST:	g_debug(" measurement statistics break with
            // max_replicates<=curr_replicate && (cv<max_cv || cv<0.0099);
            // enoughReplicates=TRUE");
            break;
        }
        if (max_replicates == curr_replicate && (curr_outlier >= outlier))
        {
            enoughReplicates = TRUE;
            // g_debug("max_replicates == curr_replicate-curr_outlier");
            break;
        }

        max_d_sqr = G_MINDOUBLE;
        GSList *outlier_object = NULL;
        for (l = data; l != NULL; l = l->next)
        {
            // TEST:	g_printf(" %s%f\n",
            // mkt_measurement_outlier(MKT_MEASUREMENT(l->data))?"*":"",
            // mkt_cal_data_value(MKT_MEASUREMENT(l->data)));
            if (!mkt_measurement_outlier(MKT_MEASUREMENT(l->data)))
            {
                d_sqr = (mkt_measurement_value(MKT_MEASUREMENT(l->data)) - raw) * (mkt_measurement_value(MKT_MEASUREMENT(l->data)) - raw);
                // TRACE:mkt_trace ("val %f d_sqr=%f",channel->values[i].value,d_sqr);
                if (d_sqr > max_d_sqr || isnan(mkt_measurement_value(MKT_MEASUREMENT(l->data))) || isinf(mkt_measurement_value(MKT_MEASUREMENT(l->data))))
                {
                    outlier_object = l;
                    // TRACE:mkt_trace ("%d d_sqr=%f>%f",iv,d_sqr,max_d_sqr);
                    max_d_sqr = d_sqr;
                }
            }
        }
        /*if (outlier_object==NULL)
           {
                for(l=data;l!=NULL;l=l->next)
                {
                        g_print(" measurement =
           %s%f\n",mkt_measurement_outlier(MKT_MEASUREMENT(l->data))?"*":" ",
           mkt_measurement_value(MKT_MEASUREMENT(l->data)));
                }
           }*/
        if (outlier_object)
        {
            mktMPSet(outlier_object->data, "measurement-outlier", TRUE);
            // TEST:	printf(" measurement outlier %f at index %d\n",
            // mkt_measurement_value(MKT_MEASUREMENT(outlier->data)),mchannel->priv->curr_outlier);
            // g_debug("%d",mkt_measurement_outlier(MKT_MEASUREMENT(outlier->data)));
        }
        if (interrupt > (max_replicates + outlier) * 2)
        {
            enoughReplicates = TRUE;
            // TEST:	g_debug(" measurement statistics break witch interrupt more
            // than 100 repetitions");
            break;
        }
    }

    // g_debug("Calculate statistic CV=%f raw = %f",cv,raw);
    channels_measurement_set_last_cv(meas, cv);
    channels_measurement_set_last_round(meas, raw);
    if (process_simple_get_online_process(simple) && current_replicates == 1)
    {
        gdouble jumpratio;
        jumpratio = 1. + process_simple_get_jump(simple) / 100.;
        if (1. < jumpratio && channels_simple_get_is_measurement(channel_simple))
        {
            gdouble lastval = channels_simple_get_online_result(channel_simple);
            gdouble val = channels_simple_get_curr_value(channel_simple);
            if (lastval == 0.0)
                return FALSE;
            if (jumpratio > fabs(val / lastval) && jumpratio > fabs(lastval / val))
            {
                THRESHOLD = TRUE;
            }
        }
    }
    if (THRESHOLD || enoughReplicates)
    {
        channels_simple_set_result(channels_object_get_simple(CHANNELS_OBJECT(channel)), raw);
        channels_simple_set_statistic_done(channels_object_get_simple(CHANNELS_OBJECT(channel)), THRESHOLD || enoughReplicates);
    }

    g_slist_free_full(data, g_object_unref);
    return THRESHOLD || enoughReplicates;
}

static gboolean mkt_process_object_start_handle_callback(ProcessSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(user_data);
    gboolean result = mkt_process_object_start(process);
    g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", result));
    return TRUE;
}

static gboolean mkt_process_object_remove_handle_callback(ProcessSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(user_data);
    mkt_process_object_break(process);
    return TRUE;
}
static gboolean mkt_process_object_user_next_handle_callback(ProcessSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(user_data);
    mkt_process_object_user_next(process);
    return TRUE;
}

static void mkt_process_object_init(MktProcessObject *mkt_process_object)
{
    mkt_process_object->priv = mkt_process_object_get_instance_private(mkt_process_object);
    mkt_process_object->priv->remote_mess = 0;
}

static void mkt_process_object_change_measurement_interval(ProcessSimple *process, GParamSpec *pspec, MktProcessObject *process_object)
{
    //	g_print("CHANGED: Process %s reinit interval time
    //%f\n",process_simple_get_name(process),process_simple_get_interval(process));
    if (process_simple_get_start_time(process_object->priv->simple) < market_db_time_now())
        process_simple_set_start_time(process, market_db_time_now() + process_simple_get_interval(process));
}

/*TEST:
   static void
   change_cathegory_measurement_interval ( MktCategory *category , GParamSpec
 * pspec, MktProcessObject  *process_object)
   {
        //	g_print("CHANGED: Category %s interval
   %f\n",mkt_param_name(MKT_PARAM(category)),mkt_category_interval(category));

   }


   static void
   test_start_time ( ProcessSimple *process , GParamSpec *pspec, MktProcessObject
 * process_object)
   {
        //	g_print("test_start_time %s start time
   %f\n",process_simple_get_name(process),process_simple_get_start_time(process));

   }*/

// static void mkt_process_object_change_remote_signal(ProcessSimple *process, GParamSpec *pspec, MktProcessObject *process_object)
// {
//     if (process_simple_get_remote_control(process))
//     {
//         if (process_simple_get_remote_signal(process) && !process_object->priv->remote_signal)
//         {
//             process_object->priv->remote_signal = TRUE;
//             process_simple_set_start_time(process, market_db_time_now() + 1.0);
//         }
//         else if (!process_simple_get_remote_signal(process))
//         {
//             process_object->priv->remote_signal = FALSE;
//         }
//     }
// }

static void mkt_process_object_constructed(GObject *object)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(object);
    process->priv->simple = process_simple_skeleton_new();
    process->priv->activated = TRUE;
    // process->priv->remote_delay = 0.0;
    process_object_skeleton_set_simple(PROCESS_OBJECT_SKELETON(process), process->priv->simple);
    // g_object_unref(process->  priv->simple);
    if (process->priv->cathegory != NULL)
    {
        g_object_bind_property(process->priv->cathegory, "category-interval", process->priv->simple, "interval", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->cathegory, "remote-control", process->priv->simple, "remote-control", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->cathegory, "category-online", process->priv->simple, "online", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->cathegory, "category-runs", process->priv->simple, "was-runs", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        // (process->priv->cathegory,"notify::category-interval",G_CALLBACK
        // (change_cathegory_measurement_interval),object);
    }

    if (process->priv->statistic != NULL)
    {
        g_object_bind_property(process->priv->statistic, "statistic-replicates", process->priv->simple, "replicates", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->statistic, "statistic-outliers", process->priv->simple, "outliers", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->statistic, "statistic-max-cv", process->priv->simple, "max-cv", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->statistic, "statistic-jump", process->priv->simple, "jump", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->statistic, "statistic-amount-counter", process->priv->simple, "amount-counter", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        g_object_bind_property(process->priv->statistic, "statistic-amount-percentage", process->priv->simple, "amount-percentage", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    }
    process_simple_set_stream(process->priv->simple, "nil");

    g_signal_connect(process->priv->simple, "handle-start", G_CALLBACK(mkt_process_object_start_handle_callback), object);
    g_signal_connect(process->priv->simple, "handle-remove", G_CALLBACK(mkt_process_object_remove_handle_callback), object);
    g_signal_connect(process->priv->simple, "handle-user-next", G_CALLBACK(mkt_process_object_user_next_handle_callback), object);
    g_signal_connect(process->priv->simple, "notify::interval", G_CALLBACK(mkt_process_object_change_measurement_interval), object);
    // g_signal_connect(process->priv->simple, "notify::remote-signal", G_CALLBACK(mkt_process_object_change_remote_signal), object);
    // TEST:g_signal_connect
    // (process->priv->simple,"notify::start-time",G_CALLBACK
    // (test_start_time),object);
    process_simple_set_identification(process->priv->simple, process->priv->identification);
    process_simple_set_interruptible(process->priv->simple, TRUE);

    process->priv->process = MKT_PROCESS(mkt_model_select_one(MKT_TYPE_PROCESS_MODEL,
                                                              "select * from %s where process_path = "
                                                              "'%s' ORDER BY ref_id DESC LIMIT 1",
                                                              g_type_name(MKT_TYPE_PROCESS_MODEL), g_dbus_object_get_object_path(G_DBUS_OBJECT(process))));
    if (process->priv->process)
    {
        process_simple_set_last_process(process->priv->simple, mkt_model_ref_id(MKT_IMODEL(process->priv->process)));
    }
    gchar *pname = g_strdup_printf("p%d", process_simple_get_identification(process->priv->simple));
    gchar *path = g_build_path("/", g_get_home_dir(), pname, NULL);
    g_remove(path);
    g_free(path);
    g_free(pname);

    if (G_OBJECT_CLASS(mkt_process_object_parent_class)->constructed)
        G_OBJECT_CLASS(mkt_process_object_parent_class)->constructed(object);
}

static void mkt_process_object_finalize(GObject *object)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(object);
    g_source_remove_by_user_data(process);
    G_OBJECT_CLASS(mkt_process_object_parent_class)->finalize(object);
}

static void mkt_process_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(object);

    switch (prop_id)
    {
    case PROCESS_DEFAULT_CATHEGORY:
        process->priv->cathegory = g_value_dup_object(value);
        break;
    case PROCESS_DEFAULT_STATISTIC:
        process->priv->statistic = g_value_dup_object(value);
        break;

    case PROCESS_DEFAULT_IDENTIFICATION:
        process->priv->identification = g_value_get_uint(value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void mkt_process_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(object);
    switch (prop_id)
    {
    case PROCESS_DEFAULT_CATHEGORY:
        g_value_set_object(value, process->priv->cathegory);
        break;
    case PROCESS_DEFAULT_STATISTIC:
        g_value_set_object(value, process->priv->statistic);
        break;
    case PROCESS_DEFAULT_IDENTIFICATION:
        g_value_set_uint(value, process->priv->identification);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void mkt_process_object_class_init(MktProcessObjectClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = mkt_process_object_set_property;
    object_class->get_property = mkt_process_object_get_property;
    object_class->finalize = mkt_process_object_finalize;
    object_class->constructed = mkt_process_object_constructed;
    klass->start = NULL;
    klass->run = NULL;
    klass->finish = NULL;
    klass->online = NULL;
    klass->offline = NULL;
    klass->interval_trigger = NULL;

    g_object_class_install_property(
        object_class, PROCESS_DEFAULT_CATHEGORY,
        g_param_spec_object("default-category", _("Category model object"), _("Category model object"), MKT_TYPE_CATEGORY_MODEL, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(
        object_class, PROCESS_DEFAULT_STATISTIC,
        g_param_spec_object("default-statistic", _("Statistic model object"), _("Statistic model object"), MKT_TYPE_STATISTIC_MODEL, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property(
        object_class, PROCESS_DEFAULT_IDENTIFICATION,
        g_param_spec_uint("process-identification", _("Category model object"), _("Category model object"), 0, G_MAXUINT, 1, G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));

    mkt_process_object_signals[PROCESS_WILL_START] =
        g_signal_new("process-will-be-run", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);
    mkt_process_object_signals[PROCESS_START] = g_signal_new("process-run", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    mkt_process_object_signals[PROCESS_STOP] = g_signal_new("process-stop", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    mkt_process_object_signals[PROCESS_WILL_BREAK] =
        g_signal_new("process-will-be-break", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    mkt_process_object_signals[PROCESS_WILL_NEXT] =
        g_signal_new("process-will-be-next", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    mkt_process_object_signals[PROCESS_ONLINE] = g_signal_new("process-online", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    mkt_process_object_signals[PROCESS_OFFLINE] =
        g_signal_new("process-offline", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);
}

static void mkt_process_object_online_reinit_start_interval(MktProcessObject *process)
{
    if (process_simple_get_start_time(process->priv->simple) < market_db_time_now())
    {
        process_simple_set_start_time(process->priv->simple, market_db_time_now() + process_simple_get_interval(process->priv->simple));
    }
}

static gboolean mkt_process_object_online_callback(gpointer user_data)
{
    MktProcessObject *process = MKT_PROCESS_OBJECT(user_data);
    if (process_simple_get_is_online(process->priv->simple))
    {
        if (process_simple_get_remote_control(process->priv->simple))
        {
            if (process_simple_get_remote_signal(process->priv->simple))
            {
                // process->priv->remote_delay = market_db_time_now() + 20.0;
                if (process->priv->remote_mess == 0 || (process_simple_get_start_time(process->priv->simple) > 0.0 && process_simple_get_start_time(process->priv->simple) < market_db_time_now()))
                {
                    process->priv->remote_mess++;
                    process_simple_set_remote_signal(process->priv->simple, FALSE);
                    process_simple_set_start_time(process->priv->simple, market_db_time_now() + process_simple_get_interval(process->priv->simple));
                    mkt_process_object_start(process);
                    return FALSE;
                }
            }
        }
        else
        {
            if (MKT_PROCESS_OBJECT_GET_CLASS(process)->interval_trigger)
            {
                gdouble to_sec = process_simple_get_start_time(process->priv->simple) - market_db_time_now();
                if (to_sec < 0.0)
                    to_sec = 0.0;
                MKT_PROCESS_OBJECT_GET_CLASS(process)->interval_trigger(process, to_sec);
            }
            if (process_simple_get_start_time(process->priv->simple) > 0.0 && process_simple_get_start_time(process->priv->simple) < market_db_time_now())
            {
                process_simple_set_start_time(process->priv->simple, market_db_time_now() + process_simple_get_interval(process->priv->simple));
                mkt_process_object_start(process);
                return FALSE;
            }
        }
    }
    return TRUE;
}
static void mkt_process_notify_task_status(MktTaskObject *task, GParamSpec *pspec, MktProcessObject *process)
{
    mkt_process_object_status(MKT_PROCESS_OBJECT(process), _("Task:%s (%s)"), mkt_task_object_get_id_name(task), mkt_task_object_get_status(task));
}

void mkt_process_bind_task_status(MktProcessObject *process, MktTaskObject *task) { g_signal_connect(task, "notify::status", G_CALLBACK(mkt_process_notify_task_status), process); }

/*
   static gboolean
   process_task_stop_timeout ( gpointer user_data )
   {
        g_debug("process_task_stop_timeout");
        MktProcessObject *process = MKT_PROCESS_OBJECT( user_data );
        process->priv->run_task = 0;
        MktTask *done_task = process->priv->runned;
        process->priv->runned = NULL;
        if(!mkt_task_is_critical(done_task))
        {
                g_debug("process_task_stop_timeout 1");
                if(process->priv->task_done)
                {
                        g_debug("process_task_stop_timeout 2");
                        process->priv->task_done( process,done_task);
                }
                else
                {
                        g_debug("process_task_stop_timeout 3");
                        mkt_process_object_critical(process,"task %s no callback
   function %s",
                                        mkt_task_get_id_name(done_task),mkt_task_get_status(done_task));
                }
        }
        else
        {
                g_debug("process_task_stop_timeout 4");
                mkt_process_object_critical(process,"funktion %s kritische
   Fehler %s",mkt_task_get_id_name(done_task),mkt_task_get_status(done_task));
        }
        return FALSE;
   }*/

gboolean mkt_process_object_online(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
    if (!mkt_process_object_is_activate(process))
        return FALSE;
    if (!process_simple_get_online_process(process->priv->simple) || !process_simple_get_online(process->priv->simple))
        return FALSE;
    if (!process_simple_get_is_online(process->priv->simple))
        g_signal_emit_by_name(process, "process-online");
    g_source_remove_by_user_data(process);
    process_simple_set_is_online(process->priv->simple, TRUE);
    mkt_process_object_online_reinit_start_interval(process);
    mkt_process_object_activate(process);
    g_timeout_add_seconds(1, mkt_process_object_online_callback, process);
    return TRUE;
}

void mkt_process_object_offline(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_PROCESS_OBJECT(process));
    process_simple_set_is_online(process->priv->simple, FALSE);
    g_source_remove_by_user_data(process);
    process->priv->remote_mess = 0;
    if (process_simple_get_online_process(process->priv->simple))
        g_signal_emit_by_name(process, "process-offline");
    process_simple_set_start_time(process->priv->simple, 0.0);
}

gboolean mkt_process_object_start(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
    process_simple_set_wait(process->priv->simple, TRUE);
    g_signal_emit_by_name(process, "process-will-be-run");
    if (MKT_PROCESS_OBJECT_GET_CLASS(process)->start)
        MKT_PROCESS_OBJECT_GET_CLASS(process)->start(process);
    return TRUE;
}

void mkt_process_object_user_next(MktProcessObject *process)
{
    if (process_simple_get_wait_user_next(process->priv->simple))
    {
        g_signal_emit_by_name(process, "process-will-be-next");
        process_simple_set_wait_user_next(process->priv->simple, FALSE);
    }
}

static void process_halt(MktProcessObject *process)
{
    process_simple_set_busy(process->priv->simple, FALSE);
    process_simple_set_wait_user_next(process->priv->simple, FALSE);
    process_simple_set_wait(process->priv->simple, FALSE);
    process_simple_set_run(process->priv->simple, FALSE);
    g_signal_emit_by_name(process, "process-stop");
    if (process->priv->process)
    {
        mkt_process_update_stop(process->priv->process);
        if (process->priv->process)
            g_object_unref(process->priv->process);
        process->priv->process = NULL;
    }
}

void mkt_process_object_stop(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    process_halt(process);
}

void mkt_process_object_break(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    process_simple_set_wait(process->priv->simple, FALSE);
    g_signal_emit_by_name(process, "process-will-be-break");
}

void mkt_process_object_critical(MktProcessObject *process, const gchar *format, ...)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    va_list args;
    gchar *new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    process_simple_set_status(process->priv->simple, new_status);
    mkt_log_error_message_sync("process:%s critical - %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(process)), new_status);
    g_free(new_status);
    process_simple_set_critical(process->priv->simple, TRUE);
    mkt_process_object_status(MKT_PROCESS_OBJECT(process), _("Process critical error"));
}

gboolean mkt_process_object_finish(MktProcessObject *process, GAsyncResult *res, GError **error)
{
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(res != NULL, FALSE);
    g_return_val_if_fail(G_IS_TASK(res), FALSE);
    gboolean ret = FALSE;
    if (!process_simple_get_remote_signal(process->priv->simple))
    {
        process->priv->remote_mess = 0;
    }
    if (MKT_PROCESS_OBJECT_GET_CLASS(process)->finish)
    {
        ret = MKT_PROCESS_OBJECT_GET_CLASS(process)->finish(process, G_TASK(res), error);
    }
    else
    {
        ret = g_task_propagate_boolean(G_TASK(res), error);
    }
    if (!ret && (*error) && (*error)->code != G_IO_ERROR_CANCELLED)
    {
        mkt_log_error_message_sync("process:%s critical - %s", g_dbus_object_get_object_path(G_DBUS_OBJECT(process)), (*error) ? (*error)->message : "unknown error");
        process_simple_set_status(process->priv->simple, (*error) ? (*error)->message : "unknown error");
        process->priv->status = STATUS_ERROR;
    }
    else
    {
        mkt_process_object_status(MKT_PROCESS_OBJECT(process), _("Process done"));
        process->priv->status = STATUS_DONE;
    }
    mkt_process_object_stop(process);
    return ret;
}

gboolean mkt_process_object_run(MktProcessObject *process, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
    process_simple_set_busy(process->priv->simple, TRUE);
    process_simple_set_critical(process->priv->simple, FALSE);
    process_simple_set_warnings(process->priv->simple, 0);
    process_simple_set_wait_user_next(process->priv->simple, FALSE);
    process_simple_set_current_replicate(process->priv->simple, 1);
    process_simple_set_solution(process->priv->simple, 1);
    process_simple_set_was_runs(process->priv->simple, (process_simple_get_was_runs(process->priv->simple) + 1));
    // if(process_simple_get_is_online(process->priv->simple))process_simple_set_start_time(process->priv->simple,market_db_time_now());

    // g_print("........................................SET INDENTIFICATION
    // %d\n",process->priv->identification);
    if (process_simple_get_is_measurement(process->priv->simple) && process->priv->identification >= 100)
    {
        if (process->priv->process)
            g_object_unref(process->priv->process);
        process->priv->process = NULL;
        process->priv->process =
            MKT_PROCESS(mkt_model_new(MKT_TYPE_PROCESS_MODEL, "process-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(process)), "process-identification", process->priv->identification,
                                      "process-stream", process_simple_get_stream_id(process->priv->simple), "process-description", process_simple_get_description(process->priv->simple),
                                      "process-name", process_simple_get_name(process->priv->simple), "process-kind", process_simple_get_kind_type(process->priv->simple), NULL));
        mkt_process_update_start(process->priv->process);
        mkt_process_run_analyse(process->priv->process);
        if (process->priv->process)
            process_simple_set_last_process(process->priv->simple, mkt_model_ref_id(MKT_IMODEL(process->priv->process)));
    }
    process_simple_set_run(process->priv->simple, TRUE);
    process_simple_set_wait(process->priv->simple, FALSE);
    process->priv->status = STATUS_RUN;
    if (process->priv->identification > 300 && process->priv->identification < 400)
    {
        mkt_log_message(MKT_LOG_STATE_MEASUREMENT, "Process %s%s (#%d) running", process_simple_get_is_online(process->priv->simple) ? "auto " : "",
                        process_simple_get_description(process->priv->simple), (guint)process_simple_get_stream_id(process->priv->simple));
    }
    else if (process->priv->identification > 100)
    {
        mkt_log_message(MKT_LOG_STATE_MEASUREMENT, "Process %s (#%d) running", process_simple_get_description(process->priv->simple), (guint)process_simple_get_stream_id(process->priv->simple));
    }
    process_simple_set_runs(process->priv->simple, (process_simple_get_runs(process->priv->simple) + 1));
    if (MKT_PROCESS_OBJECT_GET_CLASS(process)->run)
    {
        MKT_PROCESS_OBJECT_GET_CLASS(process)->run(process, cancellable, callback, user_data);
        return TRUE;
    }
    g_warning("Process %s have not run function", g_dbus_object_get_object_path(G_DBUS_OBJECT(process)));
    return FALSE;
}

void mkt_process_object_done(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
}

void mkt_process_object_status(MktProcessObject *process, const gchar *format, ...)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    va_list args;
    gchar *new_status;
    va_start(args, format);
    new_status = g_strdup_vprintf(format, args);
    va_end(args);
    process_simple_set_status(process->priv->simple, new_status);
    g_free(new_status);
}

void mkt_process_object_message(MktProcessObject *process, const gchar *format, ...)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    va_list args;
    gchar *message;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);
    g_message("%s|%s|%s", market_db_get_date_lar_format(market_db_time_now()), process_simple_get_full_name(process->priv->simple), message);
    g_free(message);
}

void mkt_process_object_trace(gpointer process, const gchar *format, ...)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    MktProcessObject *p = MKT_PROCESS_OBJECT(process);
    va_list args;
    gchar *message;
    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);

    gchar *pname = g_strdup_printf("p%d", process_simple_get_identification(p->priv->simple));
    gchar *path = g_build_path("/", g_get_home_dir(), pname, NULL);
    FILE *f = fopen(path, "a+");
    if (f != NULL)
    {
        fprintf(f, "%s:%s %s %s %s %d - %s\n", market_db_get_date_lar_format(market_db_time_now()), process_simple_get_is_online(p->priv->simple) ? "online" : "offline",
                process_simple_get_busy(p->priv->simple) ? "busy" : "free", process_simple_get_run(p->priv->simple) ? "run" : "-", process_simple_get_wait(p->priv->simple) ? "wait" : "-",
                process_simple_get_current_replicate(p->priv->simple), message);
        fflush(f);
        fclose(f);
    }
    g_free(pname);
    g_free(path);
    g_free(message);
}

void mkt_process_object_update_start_time(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS(process));
    if (process->priv->process)
        mkt_process_update_start(process->priv->process);
}

void mkt_process_object_update_stop_time(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    if (process->priv->process)
        mkt_process_update_stop(process->priv->process);
}

MktProcess *mkt_process_object_get_original(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, NULL);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), NULL);
    return process->priv->process;
}

guint mkt_process_object_get_state(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, 0);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), 0);
    return process_simple_get_number(process->priv->simple);
}

guint64 mkt_process_object_ref_id(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, 0);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), 0);
    return mkt_model_ref_id(MKT_IMODEL(process->priv->process));
}

void mkt_process_object_save(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    g_return_if_fail(process->priv->process != NULL);
    MktProcess *save_process = MKT_PROCESS(mkt_model_new(MKT_TYPE_PROCESS_MODEL, "process-path", g_dbus_object_get_object_path(G_DBUS_OBJECT(process)), NULL));
    mkt_model_copy_param(MKT_MODEL(process->priv->process), MKT_MODEL(save_process));
    g_object_unref(save_process);
}

GList *mkt_process_object_channels(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, NULL);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), NULL);
    return process->priv->channels;
}

void mkt_process_object_add_channel(MktProcessObject *process, ChannelsObject *channel)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    if (process->priv->channels == NULL || NULL == g_list_find(process->priv->channels, channel))
    {
        process->priv->channels = g_list_append(process->priv->channels, g_object_ref(channel));
    }
}

guint mkt_process_object_get_status(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, STATUS_ERROR);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), STATUS_ERROR);
    return process->priv->status;
}

gboolean mkt_process_object_is_activate(MktProcessObject *process)
{
    g_return_val_if_fail(process != NULL, FALSE);
    g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
    return process->priv->activated;
}
void mkt_process_object_activate(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    process->priv->activated = TRUE;
}
void mkt_process_object_deactivate(MktProcessObject *process)
{
    g_return_if_fail(process != NULL);
    g_return_if_fail(MKT_IS_PROCESS_OBJECT(process));
    mkt_process_object_stop(process);
    mkt_process_object_offline(process);
}
// gboolean mkt_process_object_remote_delay(MktProcessObject *process)
// {
//     g_return_val_if_fail(process != NULL, FALSE);
//     g_return_val_if_fail(MKT_IS_PROCESS_OBJECT(process), FALSE);
//     return market_db_time_now() > process->priv->remote_delay;
// }
