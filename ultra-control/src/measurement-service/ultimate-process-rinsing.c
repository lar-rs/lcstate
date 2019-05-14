/*
 * @ingroup UltimateProcessRinsing
 * @{
 * @file  ultimate-process_rinsing.c	ProcessObject object
 * @brief This is ProcessObject object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include "ultimate-process-rinsing.h"

#include <mktbus.h>
#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>



struct _UltimateProcessRinsingPrivate
{
	GCancellable   *cancelable;
	gboolean        need_rinsing;
	gboolean        need_rinsing;
	VesselsObject  *drain;
};


enum {
	RINSING_PROP0,
	RINSING_NEED_RINSING,
	RINSING_NEED_RINSE,
	RINSING_DRAIN_VESSEL,

};


#define ULTIMATE_PROCESS_RINSING_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), ULTIMATE_TYPE_PROCESS_RINSING, UltimateProcessRinsingPrivate))


G_DEFINE_TYPE (UltimateProcessRinsing, ultimate_process_rinsing, MKT_TYPE_PROCESS_OBJECT )



static void
ultimate_process_rinsing_done ( UltimateProcessRinsing *process_rinsing , gboolean done  )
{
	mkt_process_object_done(MKT_PROCESS_OBJECT(process_rinsing));

}


static void
rinsing_process_rinse_done ( MktProcessObject *object , MktTask *prepare )
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING(object);
	ultimate_process_rinsing_done(process_rinsing);
}




static void
ultimate_process_rinsing_run_prepare_real ( MktProcessObject *process )
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING(process);
	MktTask *task = mkt_process_get_task(MKT_PROCESS_OBJECT(process_rinsing),"Rinse");
	mkt_process_task_run_task(MKT_PROCESS_OBJECT(process_rinsing),task,rinsing_process_rinse_done);
}

static gboolean
ultimate_process_rinsing_start (  MktProcessObject *process )
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING(process);
	if(MKT_PROCESS_OBJECT_CLASS(ultimate_process_rinsing_parent_class)->start )
		MKT_PROCESS_OBJECT_CLASS(ultimate_process_rinsing_parent_class)->start  (process);
	g_cancellable_cancel(process_rinsing->priv->cancelable);
	return TRUE;
}


static void
ultimate_process_rinsing_stop (  MktProcessObject *process )
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING(process);
	g_cancellable_cancel(process_rinsing->priv->cancelable);
}


static void
ultimate_process_rinsing_init (UltimateProcessRinsing *ultimate_process_rinsing)
{
	ultimate_process_rinsing->priv                          =  ULTIMATE_PROCESS_RINSING_PRIVATE(ultimate_process_rinsing);
	ultimate_process_rinsing->priv->cancelable              = g_cancellable_new();
}

static void
ultimate_process_rinsing_finalize (GObject *object)
{
	G_OBJECT_CLASS (ultimate_process_rinsing_parent_class)->finalize (object);
}


static void
ultimate_process_rinsing_constructed (GObject *object)
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING(object);
	if(G_OBJECT_CLASS (ultimate_process_rinsing_parent_class)->constructed )
		G_OBJECT_CLASS (ultimate_process_rinsing_parent_class)->constructed (object);
}

static void
ultimate_process_rinsing_set_property(  GObject        *object,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING( object );
	switch(prop_id)
	{
	case RINSING_NEED_RINSING:
		process_rinsing->priv->need_rinsing = g_value_get_boolean( value );
		break;
	case RINSING_NEED_RINSE:
		process_rinsing->priv->need_rinsing = g_value_get_boolean( value );
		break;
	case RINSING_DRAIN_VESSEL:
			if(process_rinsing->priv->drain )g_object_unref(process_rinsing->priv->drain);
			process_rinsing->priv->drain = g_value_dup_object(value);
			break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
ultimate_process_rinsing_get_property(GObject        *object,
		guint           prop_id,
		GValue   *value,
		GParamSpec     *pspec)
{
	UltimateProcessRinsing *process_rinsing = ULTIMATE_PROCESS_RINSING( object );
	switch(prop_id)
	{
	case RINSING_NEED_RINSING:
		g_value_set_boolean(value,process_rinsing->priv->need_rinsing);
		break;
	case RINSING_NEED_RINSE:
		g_value_set_boolean(value,process_rinsing->priv->need_rinsing);
		break;
	case RINSING_DRAIN_VESSEL:
		g_value_set_object(value,process_rinsing->priv->drain);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
ultimate_process_rinsing_class_init (UltimateProcessRinsingClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (UltimateProcessRinsingClass));
	MktProcessObjectClass *pclass        = MKT_PROCESS_OBJECT_CLASS(klass);
	object_class->finalize               = ultimate_process_rinsing_finalize;
	object_class->set_property           = ultimate_process_rinsing_set_property;
	object_class->get_property           = ultimate_process_rinsing_get_property;
	object_class->constructed            = ultimate_process_rinsing_constructed;
	pclass->start                        = ultimate_process_rinsing_start;
	pclass->stop                         = ultimate_process_rinsing_stop;
	pclass->run                          = ultimate_process_rinsing_run_prepare_real;



	g_object_class_install_property (object_class,RINSING_NEED_RINSING,
			g_param_spec_uint ("replicate",
					"Injection port rinsing replicate",
					"Injection port rinsing replicate",
					1,20,1,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

	g_object_class_install_property (object_class,RINSING_NEED_RINSE,
			g_param_spec_boolean ("need-rinsing",
					"Need rinsing",
					"Need rinsing",
					FALSE,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

	g_object_class_install_property (object_class,RINSING_DRAIN_VESSEL,
			g_param_spec_object ("drain-vessel",
					"Drain vessel",
					"Drain vessel",
					VESSELS_TYPE_OBJECT,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
}



/** @} */
