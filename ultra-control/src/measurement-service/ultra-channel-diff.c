/*
 * @ingroup UltraChannelDiff
 * @{
 * @file  ultra-channel-object.c	Pump object
 * @brief This is Pump control object description.
 *
 *
 *  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include "ultra-stream-object.h"
#include "ultra-channel-diff.h"

#include <math.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
	PROP_CHANNEL_DIFF_STREAM,
	PROP_CHANNEL_DIFF_NUMBER,
	PROP_CHANNEL_DIFF_TC,
	PROP_CHANNEL_DIFF_TIC,

};


struct _UltraChannelDiffPrivate
{
	guint                    type;
	UltraStreamObject       *stream;
	guint                    number;
	gchar                   *value_type;
	MktChannel              *channel;
	MktLimit                *limit;
	MktLimit                *limit_ch;

	ChannelsObject          *TC;
	ChannelsObject          *TIC;
	GArray                  *amount_array;
};





#define ULTRA_CHANNEL_DIFF_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), ULTRA_TYPE_CHANNEL_DIFF, UltraChannelDiffPrivate))


static ChannelsSimple*
ULTRA_CHANNEL_SIMPLE ( UltraChannelDiff *channel)
{
	ChannelsSimple *simple = channels_object_get_simple ( CHANNELS_OBJECT(channel));
	g_assert_nonnull(simple);
	return simple;
}

/// Ultra channel interface functions
static MktChannel*
ultra_channel_diff_get_channel_model ( UltimateChannel *channel)
{
	UltraChannelDiff *channel_object = ULTRA_CHANNEL_DIFF(channel);
	return channel_object->priv->channel;

}


static gboolean
ultra_channel_diff_start_measurement ( UltimateChannel *channel)
{
	//UltraChannelDiff *channel_object = ULTRA_CHANNEL_DIFF(channel);
	return TRUE;

}

static gboolean
ultra_channel_diff_transmit_M_replicate               ( UltimateChannel *channel, MktProcessObject *process )
{
	UltraChannelDiff *channel_object = ULTRA_CHANNEL_DIFF(channel);
	guint   measurement_count = 0;
	MktProcess *process_model  = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
	if(process_model == NULL )
	  {
			mkt_errors_report(E2010, _("Process %s origin model not found"), process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))));
	    return FALSE;
	  }
	ChannelsSimple      *simple      = channels_object_get_simple(CHANNELS_OBJECT(channel));
	if(channel_object->priv->TC != NULL || channel_object->priv->TIC != NULL )
	{
		gdouble tcv  = channels_simple_get_curr_value(channels_object_get_simple(channel_object->priv->TC));
		gdouble ticv = channels_simple_get_curr_value(channels_object_get_simple(channel_object->priv->TIC));

		if(tcv<0.0) tcv  = 0.0;
		if(ticv<0.0)ticv = 0.0;
		gdouble diff = tcv-ticv;

		measurement_count = channels_simple_get_measurement(simple);
		if(diff<0.0)diff =0.0;
		diff = diff * channels_simple_get_factor(simple);
		channels_simple_set_curr_value(simple,diff);
		// value = (raw - intercept) / slope
		gint replicate = process_simple_get_current_replicate(process_object_get_simple(PROCESS_OBJECT(process)));
		MktModel *model = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA,
				"measurement-identification",mkt_process_identification(process_model),
				"measurement-channel",mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)),
				"measurement-process",mkt_model_ref_id(MKT_IMODEL(process_model)),
				"measurement-stream",mkt_process_stream(process_model),
				"measurement-type",0,
				"measurement-changed",market_db_time_now(),
				"measurement-value",diff,
				"measurement-value-row",diff,
				"measurement-trigger",measurement_count,
				"measurement-signal",0,
				"measurement-replicate",replicate,
				"measurement-name",channels_simple_get_name(simple),
				"measurement-unit",channels_simple_get_unit(simple),NULL);
		g_object_unref(model);

	}
	else
	{
		channels_simple_set_measure_error(simple,TRUE);
	}
	return TRUE;
}

static gboolean
ultra_channel_diff_transmit_M_result( UltimateChannel *channel , MktProcessObject *process)
{
	UltraChannelDiff *channel_object = ULTRA_CHANNEL_DIFF(channel);

	MktProcess *process_model  = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
	if(process_model == NULL )
	  {
			mkt_errors_report(E2010, _("Process %s origin model not found"), process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))));
	    return FALSE;
	  }
	guint   measurement_count = 0;
	gdouble cv      = 0.0;
	gdouble result  = 0.0;
	ChannelsSimple      *simple      = channels_object_get_simple(CHANNELS_OBJECT(channel));
	if(channels_simple_get_statistic_done(simple)
			&& !channels_simple_get_transmit_done(simple))
	{
		measurement_count = channels_simple_get_measurement(simple);
		cv                = channels_measurement_get_last_cv(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
		result            = channels_measurement_get_last_round(channels_object_get_measurement(CHANNELS_OBJECT(channel)));

		MktModel *model   = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA,
				"measurement-identification",mkt_process_identification(process_model),
				"measurement-channel",mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)),
				"measurement-process",mkt_model_ref_id(MKT_IMODEL(process_model)),
				"measurement-stream",mkt_process_stream(process_model),
				"measurement-type",1,
				"measurement-changed",market_db_time_now(),
				"measurement-value",result,
				"measurement-cv",cv,
				"measurement-trigger",measurement_count,
				"measurement-signal",0,
				"measurement-replicate",0,
				"measurement-name",channels_simple_get_name(simple),
				"measurement-unit",channels_simple_get_unit(simple),NULL);
		g_object_unref(model);
		channels_simple_set_result(simple,result);
		channels_simple_set_last_changed(simple,market_db_time_now());
		channels_simple_set_measurement_values(simple,channels_simple_get_measurement_values(simple)+1);
		/*if(security_device_get_expiry_type(TERA_GUARD()) == ONLINE_EXPIRY)
					ultra_channel_transmit_to_analog_out(channel);*/
		guint triger = channels_simple_get_measurement_trigger(simple);
		channels_simple_set_measurement_trigger(simple,triger+1);
		channels_simple_set_transmit_done(simple,TRUE);
		return TRUE;
	}
	else
	{
		channels_simple_set_measure_error(simple,TRUE);
	}
	return FALSE;
}


static gboolean
ultra_channel_diff_reset_measurement         ( UltimateChannel *channel )
{
	ChannelsSimple *simple  = channels_object_get_simple(CHANNELS_OBJECT(channel));
	channels_simple_set_statistic_done(simple,FALSE);
	channels_simple_set_transmit_done(simple,FALSE);
	channels_simple_set_measure_error(simple,FALSE);
	ultimate_channel_change_status(channel,"reseted");
	return TRUE;
}

static void
ultra_channel_diff_calculation_error         ( UltimateChannel *channel )
{

	ultimate_channel_change_status(channel,_("Calculate error"));
}



static void
ultra_channel_diff_amount_init ( UltimateChannel *channel )
{
	UltraChannelDiff *channel_object = ULTRA_CHANNEL_DIFF(channel);
	if(channel_object->priv->amount_array)g_array_free(channel_object->priv->amount_array,TRUE);
	channel_object->priv->amount_array = g_array_new(TRUE,TRUE,sizeof(gdouble));

}

static void
ultra_channel_diff_amount_transmit ( UltimateChannel *channel , MktProcessObject *process )
{
	UltraChannelDiff *channel_object = ULTRA_CHANNEL_DIFF(channel);

	MktProcess *process_model  = mkt_process_object_get_original(MKT_PROCESS_OBJECT(process));
	if(process_model == NULL )
	  {
			mkt_errors_report(E2010, _("Process %s origin model not found"), process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process))));
	    return ;
	  }
	guint amount_counter     = process_simple_get_amount_counter(process_object_get_simple(PROCESS_OBJECT(process)));
	gdouble av_mv_percentage = process_simple_get_amount_percentage(process_object_get_simple(PROCESS_OBJECT(process)));
	gdouble measValue = channels_measurement_get_last_round(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
	gdouble cv        = channels_measurement_get_last_cv(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
	if(channel_object->priv->amount_array==NULL)
	{
		channel_object->priv->amount_array = g_array_new(TRUE,TRUE,sizeof(gdouble));
	}
	if(amount_counter > 0 && av_mv_percentage > 0.0001 )
	{

		gdouble currentAverage = measValue;

		if(channel_object->priv->amount_array->len >0 )
		{
			currentAverage = 0.0;
			guint i = 0;
			for (i = 0; i < channel_object->priv->amount_array->len; i++)
				currentAverage += g_array_index (channel_object->priv->amount_array, gdouble, i) ;

			currentAverage /=channel_object->priv->amount_array->len;
		}
		if (measValue > currentAverage * (1 + (av_mv_percentage/100.0))
				||measValue < currentAverage * (1 - (av_mv_percentage/100.0)))
		{
			if(channel_object->priv->amount_array)g_array_free(channel_object->priv->amount_array,TRUE);
			channel_object->priv->amount_array = g_array_new(TRUE,TRUE,sizeof(gdouble));
		}
		guint i = 0;
		currentAverage = 0.0;
		channel_object->priv->amount_array = g_array_append_val(channel_object->priv->amount_array,measValue);
		for (i = 0; i < channel_object->priv->amount_array->len; i++)
			currentAverage += g_array_index (channel_object->priv->amount_array, gdouble, i);

		currentAverage /=channel_object->priv->amount_array->len;
		if(i>=amount_counter)
			g_array_remove_index(channel_object->priv->amount_array,0);
		measValue = currentAverage;

	}
	guint   measurement_count = 0;
	guint   trigger = 0;
	ChannelsSimple      *simple      = channels_object_get_simple(CHANNELS_OBJECT(channel));
	if(channels_simple_get_statistic_done(simple)
			&& !channels_simple_get_transmit_done(simple))
	{
		measurement_count = channels_simple_get_measurement(simple);
		trigger           = channels_simple_get_trigger(simple);
		cv                = channels_measurement_get_last_cv(channels_object_get_measurement(CHANNELS_OBJECT(channel)));
		MktModel *model   = mkt_model_new(MKT_TYPE_MEASUREMENT_DATA,
				"measurement-identification",mkt_process_identification(process_model),
				"measurement-channel",mkt_model_ref_id(MKT_IMODEL(channel_object->priv->channel)),
				"measurement-stream",mkt_process_stream(process_model),
				"measurement-process",mkt_model_ref_id(MKT_IMODEL(process_model)),
				"measurement-type",1,
				"measurement-changed",market_db_time_now(),
				"measurement-value",measValue,
				"measurement-cv",cv,
				"measurement-trigger",measurement_count,
				"measurement-signal",trigger,
				"measurement-replicate",0,
				"measurement-name",channels_simple_get_name(simple),
				"measurement-unit",channels_simple_get_unit(simple),NULL);
		g_object_unref(model);
		channels_simple_set_result(simple,measValue);
		channels_simple_set_last_changed(simple,market_db_time_now());
		channels_simple_set_transmit_done(simple,TRUE);
		channels_simple_set_measurement_values(simple,channels_simple_get_measurement_values(simple)+1);
		guint triger = channels_simple_get_measurement_trigger(simple);
		channels_simple_set_measurement_trigger(simple,triger+1);
	}
}

static void
ultra_channel_diff_init_interface  ( UltimateChannelInterface *iface )
{
	iface->channel_model                = ultra_channel_diff_get_channel_model;
	iface->start_measurement            = ultra_channel_diff_start_measurement;
	iface->transmit_M_replicate         = ultra_channel_diff_transmit_M_replicate;
	iface->transmit_M_result            = ultra_channel_diff_transmit_M_result;
	iface->amount_init                  = ultra_channel_diff_amount_init;
	iface->amount_transmit              = ultra_channel_diff_amount_transmit;
	iface->reset_measurement            = ultra_channel_diff_reset_measurement;
	iface->calculate_error              = ultra_channel_diff_calculation_error;
}


G_DEFINE_TYPE_WITH_CODE (UltraChannelDiff, ultra_channel_diff, CHANNELS_TYPE_OBJECT_SKELETON,
		                                    G_IMPLEMENT_INTERFACE (ULTIMATE_TYPE_CHANNEL,
		                                    		ultra_channel_diff_init_interface) )





// Watch property calback function

static void
ultra_channel_load_main_channel ( UltraChannelDiff *channel  )
{
	const gchar *object_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(channel));

	if(channel->priv->channel)g_object_unref(channel->priv->channel);
	channel->priv->channel = MKT_CHANNEL(mkt_model_select_one(MKT_TYPE_CHANNEL_MODEL,
			"select * from %s where param_object_path='%s' and channel_type='main'",g_type_name(MKT_TYPE_CHANNEL_MODEL),
			object_path));
	if(channel->priv->channel == NULL)
	{
		guint64 stream_id = streams_simple_get_id(streams_object_get_simple(STREAMS_OBJECT(channel->priv->stream)));
		gchar *name = g_strdup_printf("Channel %d measurement",channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel)));
		gchar *analog_path = g_strdup_printf("/analogs/%d",channel->priv->number);
		channel->priv->channel = MKT_CHANNEL(mkt_model_new(MKT_TYPE_CHANNEL_MODEL,
				"param-object-path",object_path,
				"channel-stream",stream_id,
				"channel-type","main",
				"channel-analog-out",analog_path,
				"channel-max",100.0,
				"param-name",name,
				NULL));
		g_free(analog_path);
		g_free(name);
	}
	mkt_param_activate(MKT_PARAM(channel->priv->channel));
	channels_simple_set_link(channels_object_get_simple(CHANNELS_OBJECT(channel)),mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
	//g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"stream",channel->priv->channel,"channel-stream",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	//g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"sensor",channel->priv->channel,"channel-sensor",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-result",ULTRA_CHANNEL_SIMPLE(channel),"result",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-changed",ULTRA_CHANNEL_SIMPLE(channel),"last-changed",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-min",ULTRA_CHANNEL_SIMPLE(channel),"min",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-max",ULTRA_CHANNEL_SIMPLE(channel),"max",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-factor",ULTRA_CHANNEL_SIMPLE(channel),"factor",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-activated",ULTRA_CHANNEL_SIMPLE(channel),"activated",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-measurement",ULTRA_CHANNEL_SIMPLE(channel),"measurement",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-trigger",ULTRA_CHANNEL_SIMPLE(channel),"trigger",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-analog-out",ULTRA_CHANNEL_SIMPLE(channel),"analog-out",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-name",ULTRA_CHANNEL_SIMPLE(channel),"name",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->channel,"channel-unit",ULTRA_CHANNEL_SIMPLE(channel),"unit",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);

	g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"is-allow",channel->priv->channel,"channel-allow",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

	if(channel->priv->limit)g_object_unref(channel->priv->limit);
	channel->priv->limit = MKT_LIMIT(mkt_model_select_one(MKT_TYPE_LIMIT_MESSAGE,
			"select * from %s where param_object_path='%s'and param_type='Measurement'",g_type_name(MKT_TYPE_LIMIT_MESSAGE),
			object_path));
	guint limit_number = channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel));
	if(channel->priv->limit == NULL)
	{
		gchar *limit_name = g_strdup_printf("L%d_",limit_number);
		gchar *name = g_strdup_printf("Channel %d limit",channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel)));
		channel->priv->limit = MKT_LIMIT(mkt_model_new(MKT_TYPE_LIMIT_MESSAGE,
				"param-object-path",object_path,
				"param-type","Measurement",
				"limit-number",limit_number,
				"param-name",name,
				"limit-name",limit_name,
				NULL));
		g_free(name);
		g_free(limit_name);
	}
	g_object_set(channel->priv->limit,"limit-pending",FALSE,NULL);
	g_object_bind_property(channel->priv->limit,"limit-name",ULTRA_CHANNEL_SIMPLE(channel),"limit-name",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->limit,"limit-min",ULTRA_CHANNEL_SIMPLE(channel),"limit-min",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->limit,"limit-max",ULTRA_CHANNEL_SIMPLE(channel),"limit-max",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->limit,"limit-pending",ULTRA_CHANNEL_SIMPLE(channel),"limit-pending",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(channel->priv->limit,"limit-activated",ULTRA_CHANNEL_SIMPLE(channel),"limit-activated",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);


	ChannelsCheck *check = channels_object_get_check(CHANNELS_OBJECT(channel));
	if(check)
	{
		if(channel->priv->limit_ch)g_object_unref(channel->priv->limit_ch);

		channel->priv->limit_ch = MKT_LIMIT(mkt_model_select_one(MKT_TYPE_LIMIT_MESSAGE,
				"select * from %s where param_object_path='%s' and param_type='Check'",g_type_name(MKT_TYPE_LIMIT_MESSAGE),
				object_path));

		if(channel->priv->limit_ch == NULL)
		{
			gchar *limit_name = g_strdup_printf("LV%d_",limit_number);
			gchar *name = g_strdup_printf("Channel %d check limit",channels_simple_get_number(ULTRA_CHANNEL_SIMPLE(channel)));
			channel->priv->limit_ch = MKT_LIMIT(mkt_model_new(MKT_TYPE_LIMIT_MESSAGE,
					"param-object-path",object_path,
					"param-type","Check",
					"param-name",name,
					"limit-number",limit_number,
					"limit-name",limit_name,
					"limit-activated",TRUE,
					NULL));
			g_free(name);
			g_free(limit_name);
		}
		g_object_set(channel->priv->limit_ch,"limit-pending",FALSE,NULL);
		if(!mkt_limit_activated(channel->priv->limit_ch))
			g_object_set(channel->priv->limit_ch,"limit-activated",TRUE,NULL);
		g_object_bind_property(channel->priv->limit_ch,"limit-name",check,"limit-name",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
		g_object_bind_property(channel->priv->limit_ch,"limit-min",check,"limit-min",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
		g_object_bind_property(channel->priv->limit_ch,"limit-max",check,"limit-max",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
		g_object_bind_property(channel->priv->limit_ch,"limit-pending",check,"limit-pending",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
		g_object_bind_property(channel->priv->limit_ch,"limit-activated",check,"limit-activated",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
		g_object_bind_property(channel->priv->channel,"channel-check-analog-out",check,"analog-out",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);

	}

}


static void
ultra_channel_diff_init (UltraChannelDiff *ultra_channel_diff)
{

	UltraChannelDiffPrivate *priv      = ULTRA_CHANNEL_DIFF_PRIVATE(ultra_channel_diff);
	priv->channel                        = NULL;
	priv->stream                         = NULL;
	ultra_channel_diff->priv           = priv;
}


static void
ultra_channel_diff_finalize (GObject *object)
{
	UltraChannelDiff *channel = ULTRA_CHANNEL_DIFF(object);
	if(channel->priv->channel)       g_object_unref    ( channel->priv->channel );
	if(channel->priv->stream)        g_object_unref    ( channel->priv->stream);
	G_OBJECT_CLASS (ultra_channel_diff_parent_class)->finalize (object);
}


static void
ultra_channle_calculate_status ( UltraChannelDiff *channel )
{
	ChannelsSimple      *simple = NULL;
	simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
	gboolean activated     =  TRUE;
	if(channel->priv->TC == NULL || channel->priv->TIC == NULL )
	{
		channels_simple_set_is_allow(simple,FALSE);
		return;
	}
	// Check TC
	if(!( channels_simple_get_is_activate(channels_object_get_simple(channel->priv->TC)) ))
		activated = FALSE;
	if(!( channels_simple_get_is_activate(channels_object_get_simple(channel->priv->TIC))))
		activated = FALSE;
	if(streams_ultra_get_need_stripping(streams_object_get_ultra(STREAMS_OBJECT(channel->priv->stream))))
		activated = FALSE;
	channels_simple_set_is_allow(simple,activated);
}


static void
ultra_channel_calculate_internal_status ( UltraChannelDiff *channel )
{
	ChannelsSimple      *simple = NULL;
	simple = channels_object_get_simple(CHANNELS_OBJECT(channel));
	gboolean is_activate = channels_simple_get_is_allow(simple)&&channels_simple_get_activated(simple);
	channels_simple_set_is_activate(simple,is_activate);
}

static void
ultra_channel_change_channel_status ( GObject *object , GParamSpec *pspec , UltraChannelDiff *channel )
{
	ultra_channle_calculate_status(channel);
}

static void
ultra_channel_change_internal_channel_status ( GObject *object , GParamSpec *pspec , UltraChannelDiff *channel )
{
	ultra_channel_calculate_internal_status(channel);
}

static void
ultra_channel_change_stream_stripping_status ( GObject *object , GParamSpec *pspec , UltraChannelDiff *channel )
{
	ultra_channle_calculate_status(channel);
}

static void
ultra_channel_diff_constructed ( GObject *object )
{
	UltraChannelDiff *channel = ULTRA_CHANNEL_DIFF(object);
	ChannelsSimple *simple = channels_simple_skeleton_new();

	channels_object_skeleton_set_simple(CHANNELS_OBJECT_SKELETON(object),simple);
	channels_simple_set_stream(simple,g_dbus_object_get_object_path(G_DBUS_OBJECT(channel->priv->stream)));
	channels_simple_set_number(simple,channel->priv->number);
	channels_simple_set_is_calculated(simple,TRUE);
	channels_simple_set_is_measurement(simple,FALSE);
	channels_simple_set_is_calibration(simple,FALSE);
	g_object_unref(simple);

	ChannelsMeasurement *meas = channels_measurement_skeleton_new();
	channels_object_skeleton_set_measurement(CHANNELS_OBJECT_SKELETON(object),meas);
	g_object_unref(meas);

	g_signal_connect(channels_object_get_simple(CHANNELS_OBJECT(channel)),"notify::is-allow",G_CALLBACK(ultra_channel_change_internal_channel_status),channel);
	g_signal_connect(channels_object_get_simple(CHANNELS_OBJECT(channel)),"notify::activated",G_CALLBACK(ultra_channel_change_internal_channel_status),channel);

	g_signal_connect(channels_object_get_simple(channel->priv->TC),"notify::is-activate",G_CALLBACK(ultra_channel_change_channel_status),channel);

	g_signal_connect(channels_object_get_simple(channel->priv->TIC),"notify::is-activate",G_CALLBACK(ultra_channel_change_channel_status),channel);
	g_signal_connect(streams_object_get_ultra(STREAMS_OBJECT(channel->priv->stream)),"notify::need-stripping",G_CALLBACK(ultra_channel_change_stream_stripping_status),channel);

	ChannelsSingle *single = channels_single_skeleton_new();
	channels_object_skeleton_set_single(CHANNELS_OBJECT_SKELETON(object),single);
	g_object_unref(single);

	ChannelsCheck *check = channels_check_skeleton_new();
	channels_object_skeleton_set_check(CHANNELS_OBJECT_SKELETON(object),check);
	g_object_unref(check);

	ultra_channel_load_main_channel(channel);
	ultra_channle_calculate_status(channel);
	ultra_channel_calculate_internal_status(channel);
	MktMeasurement *measurement =MKT_MEASUREMENT(mkt_model_select_one(MKT_TYPE_MEASUREMENT_DATA,"select * from %s where measurement_channel=%"G_GUINT64_FORMAT" and  measurement_art = 'S' ORDER BY measurement_trigger DESC LIMIT 1",g_type_name(MKT_TYPE_MEASUREMENT_DATA),
			channels_simple_get_link(simple)));
	if(measurement){
		channels_single_set_last_measurement(single,mkt_measurement_trigger(measurement));
		g_object_unref(measurement);
	}
	// if(measurement==NULL){
		// g_warning("channel %s measurement empty",g_dbus_object_get_object_path(G_DBUS_OBJECT(channel->priv->stream)));
	// }
	//g_signal_connect (integration_object_get_simple(INTEGRATION_OBJECT(channel->priv->integration)),"integrated",G_CALLBACK (ultra_channel_diff_add_calibration_point_callback),object);
	//g_signal_connect(TERA_GUARD(),"stop-all",G_CALLBACK(ultra_channel_diff_system_stop_all_callback), channel);

	G_OBJECT_CLASS (ultra_channel_diff_parent_class)->constructed (object);
}

static void
ultra_channel_diff_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ULTRA_IS_CHANNEL_DIFF (object));
	UltraChannelDiff *channel = ULTRA_CHANNEL_DIFF(object);
	switch (prop_id)
	{
	case PROP_CHANNEL_DIFF_STREAM:
		if(channel->priv->stream)g_object_unref(channel->priv->stream);
		channel->priv->stream = g_value_dup_object(value);
		break;
	case PROP_CHANNEL_DIFF_NUMBER:
		channel->priv->number = g_value_get_uint(value);
		break;
	case PROP_CHANNEL_DIFF_TC:
		if(channel->priv->TC)g_object_unref(channel->priv->TC);
		channel->priv->TC = g_value_dup_object(value);
		break;
	case PROP_CHANNEL_DIFF_TIC:
		if(channel->priv->TIC)g_object_unref(channel->priv->TIC);
		channel->priv->TIC = g_value_dup_object(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
ultra_channel_diff_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ULTRA_IS_CHANNEL_DIFF (object));
	UltraChannelDiff *channel = ULTRA_CHANNEL_DIFF(object);
	switch (prop_id)
	{
	case PROP_CHANNEL_DIFF_STREAM:
		g_value_set_object(value,channel->priv->stream);
		break;

	case PROP_CHANNEL_DIFF_NUMBER:
		g_value_set_uint(value,channel->priv->number);
		break;
	case PROP_CHANNEL_DIFF_TC:
		g_value_set_object(value,channel->priv->TC);
		break;
	case PROP_CHANNEL_DIFF_TIC:
		g_value_set_object(value,channel->priv->TIC);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
ultra_channel_diff_class_init (UltraChannelDiffClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (UltraChannelDiffClass));
	object_class->finalize       = ultra_channel_diff_finalize;
	object_class->set_property   = ultra_channel_diff_set_property;
	object_class->get_property   = ultra_channel_diff_get_property;
	object_class->constructed    = ultra_channel_diff_constructed;
	//parent_class->initialize     = NULL;
	//parent_class->emergency_stop = NULL;
	g_object_class_install_property (object_class,PROP_CHANNEL_DIFF_STREAM,
			g_param_spec_object ("channel-stream",
					"Channel stream number",
					"Channel stream number",
					ULTRA_TYPE_STREAM_OBJECT,
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));
	g_object_class_install_property (object_class,PROP_CHANNEL_DIFF_NUMBER,
			g_param_spec_uint  ("channel-number",
					"Channel number",
					"Channel number",
					0,200,0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  | G_PARAM_CONSTRUCT_ONLY ));
	g_object_class_install_property (object_class,PROP_CHANNEL_DIFF_TC,
			g_param_spec_object ("channel-tc",
					"Channel diff tc",
					"Channel diff tc",
					CHANNELS_TYPE_OBJECT_SKELETON,
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));
	g_object_class_install_property (object_class,PROP_CHANNEL_DIFF_TIC,
			g_param_spec_object ("channel-tic",
					"Channel diff tic",
					"Channel diff tic",
					CHANNELS_TYPE_OBJECT_SKELETON,
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));
}





/** @} */
