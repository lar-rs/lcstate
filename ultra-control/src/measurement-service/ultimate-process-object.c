/*
 * @ingroup UltimateProcessObject
 * @{
 * @file  ultimate-process_object.c	ProcessObject object
 * @brief This is ProcessObject object description.
 *
 *
 * @copyright  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */


#include "ultimate-process-object.h"

#include <ultimate-library.h>

#include "../../config.h"
#include <glib/gi18n-lib.h>

#include "prepare-task.h"
#include "analyze-task.h"
#include "ultra-stream-object.h"
#include "ultra-integration-object.h"
#include "ultra-channel-object.h"
#include "ultra-control-process.h"


struct _UltimateProcessObjectPrivate
{
	StreamsObject       *stream;
};


enum {
	ULTIMATE_PROP0,
	ULTIMATE_STREAM,
	};


#define ULTIMATE_PROCESS_OBJECT_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), ULTIMATE_TYPE_PROCESS_OBJECT, UltimateProcessObjectPrivate))



StreamsObject*
ultimate_process_stream (UltimateProcessObject *process_object)
{
	g_return_val_if_fail(process_object!=NULL,NULL);
	g_return_val_if_fail(ULTIMATE_IS_PROCESS_OBJECT(process_object),NULL);
	return process_object->priv->stream;
}

G_DEFINE_TYPE (UltimateProcessObject, ultimate_process_object, MKT_TYPE_PROCESS_OBJECT)



static void
ultimate_process_object_init (UltimateProcessObject *ultimate_process_object)
{
	ultimate_process_object->priv                          =  ULTIMATE_PROCESS_OBJECT_PRIVATE(ultimate_process_object);
	ultimate_process_object->priv->stream                  =  NULL;
}

static void
ultimate_process_object_finalize (GObject *object)
{
	G_OBJECT_CLASS (ultimate_process_object_parent_class)->finalize (object);
}


static void
ultimate_process_object_stream_name_changed ( GObject *object , GParamSpec *pspec , UltimateProcessObject *process_object )
{
	gchar *full_name = g_strdup_printf("%s (%s)",process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process_object))),streams_simple_get_name(streams_object_get_simple(process_object->priv->stream)));
	process_simple_set_full_name(process_object_get_simple(PROCESS_OBJECT(process_object)),full_name);
	g_free(full_name);
}

static void
ultimate_process_object_constructed (GObject *object)
{
	/* TODO: Add deinitalization code here */
	UltimateProcessObject *process_object = ULTIMATE_PROCESS_OBJECT(object);

	if(G_OBJECT_CLASS (ultimate_process_object_parent_class)->constructed )
		G_OBJECT_CLASS (ultimate_process_object_parent_class)->constructed (object);

	if(process_object->priv->stream == NULL)
	{
		// g_error("ultimate process stream was not defined");
		g_assert_nonnull(process_object->priv->stream);
	}

	process_simple_set_stream(process_object_get_simple(PROCESS_OBJECT(process_object)),g_dbus_object_get_object_path(G_DBUS_OBJECT(process_object->priv->stream)));
	gchar *full_name = g_strdup_printf("%s (%s)",process_simple_get_name(process_object_get_simple(PROCESS_OBJECT(process_object))),streams_simple_get_name(streams_object_get_simple(process_object->priv->stream)));
	process_simple_set_full_name(process_object_get_simple(PROCESS_OBJECT(process_object)),full_name);
	g_free(full_name);
	g_signal_connect(streams_object_get_simple(process_object->priv->stream),"notify::name",G_CALLBACK(ultimate_process_object_stream_name_changed),process_object);
	g_signal_connect(process_object_get_simple(PROCESS_OBJECT(process_object)),"notify::name",G_CALLBACK(ultimate_process_object_stream_name_changed),process_object);

	//
}

static void
ultimate_process_object_set_property(  GObject        *object,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
	UltimateProcessObject *process_object = ULTIMATE_PROCESS_OBJECT( object );
	switch(prop_id)
	{
	case ULTIMATE_STREAM:
		if(process_object->priv->stream)g_object_unref(process_object->priv->stream);
		process_object->priv->stream = g_value_dup_object(value);
		break;


	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
ultimate_process_object_get_property(GObject        *object,
		guint           prop_id,
		GValue   *value,
		GParamSpec     *pspec)
{
	UltimateProcessObject *process_object = ULTIMATE_PROCESS_OBJECT( object );
	switch(prop_id)
	{
	case ULTIMATE_STREAM:
		g_value_set_object(value,process_object->priv->stream);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

/* signals */

static void
ultimate_process_object_class_init (UltimateProcessObjectClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (UltimateProcessObjectClass));
	//MktProcessObjectClass *pclass        = MKT_PROCESS_OBJECT_CLASS(klass);
	//object_class->dispose              = ultimate_atom_dispose;
	object_class->finalize               = ultimate_process_object_finalize;
	object_class->set_property           = ultimate_process_object_set_property;
	object_class->get_property           = ultimate_process_object_get_property;
	object_class->constructed            = ultimate_process_object_constructed;

	g_object_class_install_property (object_class,ULTIMATE_STREAM,
				g_param_spec_object ("process-stream",
						"Analyze stream object",
						"Analyze stream object",
						STREAMS_TYPE_OBJECT,
						G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));

}



/** @} */
