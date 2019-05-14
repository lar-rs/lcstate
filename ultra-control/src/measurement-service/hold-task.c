/*
* @ingroup RinsingTask
 * @{
 * @file  rinsing-task.c	Task object
 * @brief This is Task object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include <ultimate-library.h>

#include "ultra-integration-object.h"
#include "ultra-stream-object.h"
#include "ultimate-channel.h"
#include "rinsing-task.h"
#include "ultra-control-process.h"


#include "../../config.h"
#include <glib/gi18n-lib.h>


struct _RinsingTaskPrivate
{
	VesselsObject     *drain_object;
	guint              replicate;
	guint              repeat;

	guint              rinsing_check_tag;
	guint              waite_run;

};

/* signals */

enum {
	RINSING_MOVE_FREE,
	LAST_SIGNAL
};



//static guint rinsing_action_signals[LAST_SIGNAL]  ;


enum {
	TASK_PROP0,
	RINSING_DRAIN_VESSEL,
	RINSING_REPLICATE
};


G_DEFINE_TYPE_WITH_PRIVATE (RinsingTask, rinsing_task, MKT_TYPE_TASK);

static void
ultra_rinsing_disconnect_handlers ( RinsingTask *rinsing_task )
{

}

static void
rinsing_task_done (  RinsingTask *task  )
{
	ultra_rinsing_disconnect_handlers(task);
	mkt_task_done(MKT_TASK(task),TRUE);
}

static void
rinsing_task_failed (  RinsingTask *task  )
{
	ultra_rinsing_disconnect_handlers(task);
	mkt_task_done(MKT_TASK(task),FALSE);
}


static void
Y_GO_HOLD_BACK_async_callback (GObject *source_object,
				     GAsyncResult *res,
				     gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_hold_finish(achsen_object_get_achse(Y_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Y go hold position failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);

	}
	else
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("task done"));
		rinsing_task_done(rinsing_task);
	}
}


static void Z_GO_HOLD_async_callback         ( GObject *source_object, GAsyncResult *res,gpointer user_data );
static void Z_GO_AFTER_HOLD_async_callback   ( GObject *source_object, GAsyncResult *res,gpointer user_data );
static void
Z_GO_RINSING_async_callback (GObject *source_object,
				     GAsyncResult *res,
				     gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_injection_call_go_rinsing_finish(achsen_object_get_injection(Z_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Injection go rinsing failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);

	}
	else
	{
		gboolean is_done = FALSE;
		GError *error = NULL;
		if(!valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()),&is_done,NULL,&error))
		{
			mkt_task_status(MKT_TASK(rinsing_task),_("Close injection valve fail - %s"),error?error->message:"unknown");
			if(error)g_error_free(error);
			rinsing_task_failed(rinsing_task);
			return;
		}
		mkt_task_status(MKT_TASK(rinsing_task),_("Injection go to hold"));
		achsen_achse_call_go_hold(achsen_object_get_achse(Z_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Z_GO_AFTER_HOLD_async_callback,rinsing_task);
	}
}

static void
Z_GO_AFTER_HOLD_async_callback (GObject *source_object,
		GAsyncResult *res,
		gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_hold_finish(achsen_object_get_achse(Z_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Z - go to hold position failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);
	}
	else
	{
		if(rinsing_task->priv->repeat <  rinsing_task->priv->replicate)
		{
			gboolean is_done = FALSE;
			GError *error = NULL;
			if(!valves_simple_call_open_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()),&is_done,NULL,&error))
			{
				mkt_task_status(MKT_TASK(rinsing_task),_("Open injection valve fail - %s"),error?error->message:"unknown");
				if(error)g_error_free(error);
				rinsing_task_failed(rinsing_task);
				return;
			}
			rinsing_task->priv->repeat++;
			mkt_task_status(MKT_TASK(rinsing_task),_("Injection go to rinsing"));
			achsen_injection_call_go_rinsing(achsen_object_get_injection(Z_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Z_GO_RINSING_async_callback,rinsing_task);
		}
		else
		{
			gboolean is_done = FALSE;
			GError *error = NULL;
			if(!valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()),&is_done,NULL,&error))
			{
				mkt_task_status(MKT_TASK(rinsing_task),_("Close injection valve fail - %s"),error?error->message:"unknown");
				if(error)g_error_free(error);
				rinsing_task_failed(rinsing_task);
				return;
			}
			mkt_task_status(MKT_TASK(rinsing_task),_("Y - go back to hold position"));
			achsen_achse_call_go_hold(achsen_object_get_achse(Y_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Y_GO_HOLD_BACK_async_callback,rinsing_task);
		}
	}
}

static void
Z_GO_SENSOR_async_callback (GObject *source_object,
		GAsyncResult *res,
		gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_hold_finish(achsen_object_get_achse(Z_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Z - go to hold position failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);
	}
	else
	{
		gboolean is_done = FALSE;
		GError *error = NULL;
		if(!valves_simple_call_open_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()),&is_done,NULL,&error))
		{
			mkt_task_status(MKT_TASK(rinsing_task),_("Close injection valve fail - %s"),error?error->message:"unknown");
			if(error)g_error_free(error);
			rinsing_task_failed(rinsing_task);
			return ;
		}
		mkt_task_status(MKT_TASK(rinsing_task),_("Injection go to hold"));
		achsen_achse_call_go_hold(achsen_object_get_achse(Z_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Z_GO_AFTER_HOLD_async_callback,rinsing_task);
	}
}

void
Z_GO_HOLD_async_callback (GObject *source_object,
		GAsyncResult *res,
		gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_hold_finish(achsen_object_get_achse(Z_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Z - go to hold position failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);
	}
	else
	{

		mkt_task_status(MKT_TASK(rinsing_task),_("Injection go to sensor"));
		achsen_achse_call_go_sensor(achsen_object_get_achse(Z_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Z_GO_SENSOR_async_callback,rinsing_task);
	}
}



static void
Y_INJECTION_POS_async_callback (GObject *source_object,
				     GAsyncResult *res,
				     gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;
	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_to_position_finish(achsen_object_get_achse(Y_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Y - go to needle position failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);
	}
	else
	{
		gboolean is_done = FALSE;
		GError *error = NULL;
		if(!valves_simple_call_close_sync(valves_object_get_simple(ULTRA_VALVE_INJECTION()),&is_done,NULL,&error))
		{
			mkt_task_status(MKT_TASK(rinsing_task),_("Close injection valve fail - %s"),error?error->message:"unknown");
			rinsing_task_failed(rinsing_task);
			return;
		}
		mkt_task_status(MKT_TASK(rinsing_task),_("Injection go to hold"));
		achsen_achse_call_go_hold(achsen_object_get_achse(Z_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Z_GO_HOLD_async_callback,rinsing_task);
	}
}



static void
X_VESSEL_POS_async_callback (GObject *source_object,
				     GAsyncResult *res,
				     gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_to_position_finish(achsen_object_get_achse(X_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("X - go to drain failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);
	}
	else
	{
		guint injection_pos = vessels_simple_get_injection_pos(vessels_object_get_simple(VESSELS_OBJECT(rinsing_task->priv->drain_object)));
		mkt_task_status(MKT_TASK(rinsing_task),_("Y - go to injection position %d"),injection_pos);
		achsen_achse_call_go_to_position(achsen_object_get_achse(Y_AXIS()),injection_pos,mkt_task_cancellable(MKT_TASK(rinsing_task)),Y_INJECTION_POS_async_callback,rinsing_task);
	}
}


static void
Y_GO_HOLD_async_callback (GObject *source_object,
				     GAsyncResult *res,
				     gpointer user_data)
{
	RinsingTask *rinsing_task    = RINSING_TASK(user_data);
	if(mkt_task_is_cancelled(MKT_TASK(rinsing_task)))
		return;

	gboolean result    = FALSE;
	GError  *error = NULL;
	if(!achsen_achse_call_go_hold_finish(achsen_object_get_achse(Y_AXIS()),&result,res,&error))
	{
		mkt_task_status(MKT_TASK(rinsing_task),_("Y go hold position failed - %s"),error?error->message:"unknown");
		if(error)g_error_free(error);
		rinsing_task_failed(rinsing_task);
	}
	else
	{
		guint vessels_pos = vessels_simple_get_pos_xachse(vessels_object_get_simple(VESSELS_OBJECT(rinsing_task->priv->drain_object)));
		mkt_task_status(MKT_TASK(rinsing_task),_("X - go to drain position %d"),vessels_pos);
		achsen_achse_call_go_to_position(achsen_object_get_achse(X_AXIS()),vessels_pos,mkt_task_cancellable(MKT_TASK(rinsing_task)),X_VESSEL_POS_async_callback,rinsing_task);
	}
}

static gboolean
rinsing_task_start( MktTask *task )
{

	RinsingTask *rinsing_task    = RINSING_TASK(task);
	g_cancellable_reset( mkt_task_cancellable(MKT_TASK(rinsing_task)));
	rinsing_task->priv->repeat     = 0;
	mkt_task_status(MKT_TASK(rinsing_task),_("Y - go to hold position"));
	achsen_achse_call_go_hold(achsen_object_get_achse(Y_AXIS()),mkt_task_cancellable(MKT_TASK(rinsing_task)),Y_GO_HOLD_async_callback,rinsing_task);
	return TRUE;
}

static gboolean
rinsing_task_cancel (  MktTask *task  )
{
	RinsingTask *rinsing_task    = RINSING_TASK(task);

	/*if(sequence_workers_process_get_busy(sequence_object_get_workers_process(ULTRA_INJECTION_SEQUENCE_WORKER())))
		sequence_workers_process_call_stop(sequence_object_get_workers_process(ULTRA_INJECTION_SEQUENCE_WORKER()),NULL,NULL,NULL);
	if(sequence_workers_process_get_busy(sequence_object_get_workers_process(ULTRA_SAMPLING_SEQUENCE_WORKER())))
		sequence_workers_process_call_stop(sequence_object_get_workers_process(ULTRA_SAMPLING_SEQUENCE_WORKER()),NULL,NULL,NULL);
	if(sequence_workers_process_get_busy(sequence_object_get_workers_process(ULTRA_RINSING_SEQUENCE_WORKER())))
		sequence_workers_process_call_stop(sequence_object_get_workers_process(ULTRA_SAMPLING_SEQUENCE_WORKER()),NULL,NULL,NULL);*/
	ultra_rinsing_disconnect_handlers(rinsing_task);
	return TRUE;
}


static void
rinsing_task_init (RinsingTask *rinsing_task)
{
	rinsing_task->priv = rinsing_task_get_instance_private(rinsing_task);
	tera_pumps_manager_client_new();
	ultra_vessels_manager_client_new();

	rinsing_task->priv->rinsing_check_tag       =  0;
	rinsing_task->priv->replicate               =  1;
	rinsing_task->priv->drain_object            =  g_object_ref(ULTRA_VESSEL3());
	/* TODO: Add initialization code here */
}

static void
rinsing_task_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	RinsingTask *task = RINSING_TASK(object);
	if(task->priv->waite_run ) g_source_remove(task->priv->waite_run);
	if(task->priv->rinsing_check_tag ) g_source_remove(task->priv->rinsing_check_tag);
	g_object_unref(task->priv->drain_object);
	G_OBJECT_CLASS (rinsing_task_parent_class)->finalize (object);
}


static void
rinsing_task_set_property(  GObject        *object,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
	RinsingTask *task = RINSING_TASK( object );
	switch(prop_id)
	{
	case RINSING_DRAIN_VESSEL:
		if(task->priv->drain_object )g_object_unref(task->priv->drain_object);
		task->priv->drain_object = g_value_dup_object(value);
		break;
	case RINSING_REPLICATE:
		task->priv->replicate = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
rinsing_task_get_property(GObject        *object,
		guint           prop_id,
		GValue   *value,
		GParamSpec     *pspec)
{
	RinsingTask *task = RINSING_TASK( object );
	switch(prop_id)
	{
	case RINSING_DRAIN_VESSEL:
		g_value_set_object(value,task->priv->drain_object);
		break;
	case RINSING_REPLICATE:
		g_value_set_uint(value,task->priv->replicate);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}



static void
rinsing_task_class_init (RinsingTaskClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//object_class->dispose           = rinsing_atom_dispose;
	object_class->finalize          = rinsing_task_finalize;
	object_class->set_property      = rinsing_task_set_property;
	object_class->get_property      = rinsing_task_get_property;
	MKT_TASK_CLASS(klass)->run_task = rinsing_task_start;
	MKT_TASK_CLASS(klass)->cancel_task = rinsing_task_cancel;

	g_object_class_install_property (object_class,RINSING_REPLICATE,
			g_param_spec_uint ("rinsing-replicate",
					"Rinsing injection",
					"Rinsing injection",
					1,30,20,
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT  ));
	g_object_class_install_property (object_class,RINSING_DRAIN_VESSEL,
			g_param_spec_object ("rinsing-drain",
					"Rinsing vessel drain object",
					"Rinsing vessel drain object",
					VESSELS_TYPE_OBJECT,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

}




/** @} */
