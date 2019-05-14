/*
 * @ingroup CheckProcess
 * @{
 * @file  check-process.c	Process object
 * @brief This is Process object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include "check-process.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "prepare-task.h"
#include "analyze-task.h"
#include "ultra-stream-object.h"
#include "ultra-integration-object.h"
#include "ultra-channel-object.h"


struct _CheckProcessPrivate
{
	GList               *values_TC;
	GList               *values_TIC;
	GList               *values_TOC;
	GList               *measurement_channels;

};


enum {
	CHECK_PROP0,

};


#define CHECK_PROCESS_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHECK_TYPE_PROCESS, CheckProcessPrivate))


G_DEFINE_TYPE(CheckProcess, check_process, ULTIMATE_TYPE_PROCESS_OBJECT)

static void
check_process_set_analyze ( CheckProcess  *process , GList *channels , gboolean analyze )
{
	GList *chl = NULL;
	for(chl = channels;chl!=NULL;chl=chl->next)
	{
		ChannelsMeasurement *measurement = channels_object_get_measurement(CHANNELS_OBJECT(chl->data));
		if(measurement)	channels_measurement_set_measure(measurement,analyze);
	}
}


static void
check_process_break(  CheckProcess *process  )
{
	check_process_set_analyze(process,process->priv->measurement_channels,FALSE);
}

static void
ultra_check_process_done (  CheckProcess *process  )
{
	check_process_break(process);
	mkt_process_object_done(MKT_PROCESS_OBJECT(process));
}


static void
check_start_measurement ( CheckProcess *process )
{
	GList   *chl                  = NULL;
	for(chl=process->priv->measurement_channels;chl!=NULL;chl=chl->next)
	{
		ultimate_channel_start_measurement(ULTIMATE_CHANNEL(chl->data));
	}
}

static void
check_next_measurement( CheckProcess  *process  )
{
	GList   *chl                  = NULL;
	for(chl=process->priv->measurement_channels;chl!=NULL;chl=chl->next)
	{
		ultimate_channel_next_measurement(ULTIMATE_CHANNEL(chl->data));
	}
}

static void
check_transmit_value( CheckProcess  *process  )
{
	GList   *chl                  = NULL;
	for(chl=process->priv->measurement_channels;chl!=NULL;chl=chl->next)
	{
		ultimate_channel_transmit_M_result(ULTIMATE_CHANNEL(chl->data));

	}
}

static void
check_transmit_integration_value ( CheckProcess  *process )
{
	ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
	GList *channels = NULL;
	for(channels = process->priv->measurement_channels;channels!=NULL;channels=channels->next)
	{
		if(channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channels->data))))
		{
			ultimate_channel_transmit_M_replicate(ULTIMATE_CHANNEL(channels->data),process_simple_get_current_replicate(simple));
		}
	}
	for(channels = process->priv->measurement_channels;channels!=NULL;channels=channels->next)
	{
		if(!channels_simple_get_is_measurement(channels_object_get_simple(CHANNELS_OBJECT(channels->data))))
		{
			ultimate_channel_transmit_M_replicate(ULTIMATE_CHANNEL(channels->data),process_simple_get_current_replicate(simple));
		}
	}
}

static void
check_recalculate_statistic_value( CheckProcess  *process  )
{
	ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(process));
	GList *channels = NULL;
	for(channels = process->priv->measurement_channels;channels!=NULL;channels=channels->next)
	{
		ultimate_channel_calculate_M_statistic(ULTIMATE_CHANNEL(channels->data),
						process_simple_get_current_replicate(simple),
						process_simple_get_replicates(simple),
						process_simple_get_outliers(simple),
						process_simple_get_max_cv(simple));
	}
}

static gboolean
check_runned_measurement_values ( CheckProcess  *check )
{
	gboolean runned_value = FALSE;
	GList   *chl                  = NULL;
	GList   *channels             = check->priv->measurement_channels;
	for(chl=channels;chl!=NULL;chl=chl->next)
	{
		ChannelsSimple *simple = NULL;
		simple      = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
		if((!channels_simple_get_statistic_done(simple))
			&&(!channels_simple_get_measure_error(simple)))
		{
			runned_value = TRUE;
		}
	}
	return runned_value;
}



static void    check_process_analyse_done ( MktProcessObject *object,  MktTask *analyze  );


static gboolean
check_process_start_tc_analyse ( CheckProcess  *check )
{
	if(check->priv->values_TC == NULL)return FALSE;
	MktTask *task = mkt_process_get_task(MKT_PROCESS_OBJECT(check),"TC-Analyze");
	check_process_set_analyze(check,check->priv->values_TC,TRUE);
	analyze_task_set_channels(ANALYZE_TASK(task),check->priv->values_TC);
	mkt_process_task_run_task(MKT_PROCESS_OBJECT(check),task,check_process_analyse_done);
	return TRUE;

}

static gboolean
check_process_start_tic_analyse ( CheckProcess  *check )
{
	if(check->priv->values_TIC == NULL)return FALSE;
	MktTask *task = mkt_process_get_task(MKT_PROCESS_OBJECT(check),"TIC-Analyze");
	check_process_set_analyze(check,check->priv->values_TIC,TRUE);
	analyze_task_set_channels(ANALYZE_TASK(task),check->priv->values_TIC);
	mkt_process_task_run_task(MKT_PROCESS_OBJECT(check),task,check_process_analyse_done);
	return TRUE;
}


static void
check_process_analyse_done (MktProcessObject *object,  MktTask *analyze )
{
	CheckProcess *check = CHECK_PROCESS(object);
	check_process_set_analyze(check,check->priv->measurement_channels,FALSE);
	if(!analyze_task_is_tic(ANALYZE_TASK(analyze)))
	{
		if(check_process_start_tic_analyse(check))
		{
			return;
		}
	}
	check_transmit_integration_value(check);
	ProcessSimple *simple = process_object_get_simple(PROCESS_OBJECT(check));
	if(process_simple_get_current_replicate(simple)<process_simple_get_replicates(simple))
	{
		process_simple_set_current_replicate(simple,(process_simple_get_current_replicate(simple)+1));
		if(!check_process_start_tc_analyse(check))
		{
			if(!check_process_start_tic_analyse(check))
			{
				mkt_process_object_critical(MKT_PROCESS_OBJECT(check),"Channels not found");
				return;
			}
			return;
		}
		else
			return;
	}
	check_recalculate_statistic_value(check);
	if(check_runned_measurement_values(check))
	{
		process_simple_set_current_replicate(simple,(process_simple_get_current_replicate(simple)+1));
		if(!check_process_start_tc_analyse(check))
		{
			if(!check_process_start_tic_analyse(check))
			{
				mkt_process_object_critical(MKT_PROCESS_OBJECT(check),"Channels not found");
				return;
			}
			else
				return;
		}
		else
			return;

	}
	else
	{
		check_transmit_value(check);
		ultra_check_process_done(check);
		return;
	}
}

static gboolean
check_process_run_analyse_real ( CheckProcess *check )
{
	check_start_measurement(check);
	check_next_measurement(check);
	streams_simple_set_processing_measurement(streams_object_get_simple(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(check))),TRUE);
	if(!check_process_start_tc_analyse(check))
	{
		if(!check_process_start_tic_analyse(check))
		{
			mkt_process_object_critical(MKT_PROCESS_OBJECT(check),"Channels not found");
			return FALSE;
		}
	}
	return TRUE;
}




static void
check_process_prepare_done ( MktProcessObject *object , MktTask *prepare )
{
	CheckProcess *check = CHECK_PROCESS(object);
	check_process_run_analyse_real(check);
}


static void
check_process_realize_channels ( CheckProcess *check  )
{
	if(check->priv->values_TC)            g_list_free(check->priv->values_TC);
	if(check->priv->values_TIC)           g_list_free(check->priv->values_TIC);
	if(check->priv->values_TOC)           g_list_free(check->priv->values_TOC);
	if(check->priv->measurement_channels) g_list_free(check->priv->measurement_channels);
	check->priv->values_TC  = NULL;
	check->priv->values_TIC = NULL;
	check->priv->values_TOC = NULL;
	check->priv->measurement_channels = NULL;
	GList   *chl                  = NULL;
	GList   *channels             = ultra_stream_channels(ULTRA_STREAM_OBJECT(ultimate_process_stream(ULTIMATE_PROCESS_OBJECT(check))));
	for(chl=channels;chl!=NULL;chl=chl->next)
	{

		ChannelsSimple      *simple = NULL;
		simple = channels_object_get_simple(CHANNELS_OBJECT(chl->data));
		if(channels_simple_get_is_activate(simple) )
		{
			check->priv->measurement_channels = g_list_append(check->priv->measurement_channels ,chl->data);
			channels_simple_set_measure_kind(simple,"V");
			if(channels_simple_get_is_measurement(simple) )
			{
				if(!channels_simple_get_tic(simple) )
					check->priv->values_TC  = g_list_append(check->priv->values_TC ,chl->data);
				else
					check->priv->values_TIC  = g_list_append(check->priv->values_TIC ,chl->data);
			}
			else if(channels_simple_get_is_calculated(simple))
				check->priv->values_TOC  = g_list_append(check->priv->values_TOC ,chl->data);
		}
	}
	//guint chM = (check->priv->values_TC!=NULL?g_list_length(check->priv->values_TC):0)+ (check->priv->values_TIC!=NULL?g_list_length(check->priv->values_TIC):0)+ (check->priv->values_TOC!=NULL?g_list_length(check->priv->values_TOC):0);
}

static gboolean
check_process_start ( MktProcessObject *process)
{
	CheckProcess *check = CHECK_PROCESS(process);
	check_process_realize_channels(check);
	return check->priv->values_TC!=NULL || check->priv->values_TIC!=NULL;
}



static void
check_process_stop ( MktProcessObject *process)
{
	CheckProcess *check = CHECK_PROCESS(process);
	check_process_break(check);
}

static void
check_process_run  ( MktProcessObject *process)
{
	CheckProcess *check = CHECK_PROCESS(process);
	check_process_realize_channels(check);
	MktTask *task = mkt_process_get_task(MKT_PROCESS_OBJECT(check),"Prepare");
	mkt_process_task_run_task(MKT_PROCESS_OBJECT(check),task,check_process_prepare_done);
}


static void
check_process_init (CheckProcess *check_process)
{
	//tera_pumps_manager_client_new();
	//ultra_vessels_manager_client_new();
	check_process->priv                          =  CHECK_PROCESS_PRIVATE(check_process);
	check_process->priv->measurement_channels    =  NULL;
	check_process->priv->values_TC               =  NULL;
	check_process->priv->values_TIC              =  NULL;


	/* TODO: Add initialization code here */
}

static void
check_process_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	CheckProcess *process = CHECK_PROCESS(object);
	if(process->priv->values_TC)                g_list_free    (process->priv->values_TC);
	if(process->priv->values_TIC)               g_list_free    (process->priv->values_TIC);
	if(process->priv->measurement_channels)     g_list_free    (process->priv->measurement_channels);
	G_OBJECT_CLASS (check_process_parent_class)->finalize (object);
}

static void
check_process_constructed (GObject *object)
{
	/* TODO: Add deinitalization code here */
	//
	if(G_OBJECT_CLASS (check_process_parent_class)->constructed )
		G_OBJECT_CLASS (check_process_parent_class)->constructed (object);
	//CheckProcess *process = CHECK_PROCESS(object);
	//
}

static void
check_process_set_property(  GObject        *object,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
	//CheckProcess *process = CHECK_PROCESS( object );
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
check_process_get_property(GObject        *object,
		guint           prop_id,
		GValue   *value,
		GParamSpec     *pspec)
{
	//CheckProcess *process = CHECK_PROCESS( object );
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

/* signals */

static void
check_process_class_init (CheckProcessClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	MktProcessObjectClass *pclass        = MKT_PROCESS_OBJECT_CLASS(klass);
	g_type_class_add_private (klass, sizeof (CheckProcessClass));
	//object_class->dispose              = check_atom_dispose;
	object_class->finalize               = check_process_finalize;
	object_class->set_property           = check_process_set_property;
	object_class->get_property           = check_process_get_property;
	object_class->constructed            = check_process_constructed;
	pclass->stop                         = check_process_stop;
	pclass->start                        = check_process_start;
	pclass->run                          = check_process_run;

/*	check_action_signals[PROCESS_RUN] =
			g_signal_new ("process-run",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_LAST ,
					0,
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0,
					G_TYPE_NONE);*/

}


/** @} */
