/*
 * @ingroup MktStirrerObject
 * @{
 * @file  mkt-stirrer-object.c	Stirrer object
 * @brief This is Stirrer control object description.
 *
 *
 *  Copyright (C) LAR  2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include <mktlib.h>
#include "mkt-can-manager-client.h"
#include "mkt-stirrer-object.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
	PROP_STIRRER_OBJECT_ADDRESS,
	PROP_STIRRER_OBJECT_ST_RUN,
};


struct _MktStirrerObjectPrivate
{
	gchar           *address;
	NodesObject     *node;
	MktParamuint32  *current_mod1;
	MktParamuint32  *current_mod2;
	MktParamuint32  *delay_mod1;
	MktParamuint32  *delay_mod2;
};

G_DEFINE_TYPE_WITH_PRIVATE (MktStirrerObject, mkt_stirrer_object, STIRRERS_TYPE_OBJECT_SKELETON);

/*
#define MKT_STIRRER_ERROR (mkt_stirrer_error_quark ())
GQuark mkt_stirrer_error_quark (void);

typedef enum
{
  MKT_STIRRER_HARDWARE_ERROR_FAILED,
  FOO_BAR_N_ERRORS
} FooBarError;

// foo-bar-error.c:

static const GDBusErrorEntry mkt_stirrer_error_entries[] =
{
  {MKT_STIRRER_HARDWARE_ERROR_FAILED, "com.lar.stirrers.Stirrer1.HardwareError"},
};

// Ensure that every error code has an associated D-Bus error name
G_STATIC_ASSERT (G_N_ELEMENTS (mkt_stirrer_error_entries) == FOO_BAR_N_ERRORS);

GQuark
mkt_stirrer_error_quark (void)
{
  static volatile gsize quark_volatile = 0;
  if(quark_volatile == 0)
	  g_dbus_error_register_error_domain ("mkt-stirrer-error-quark",
                                      &quark_volatile,
									  mkt_stirrer_error_entries,
                                      G_N_ELEMENTS (mkt_stirrer_error_entries));
  return (GQuark) quark_volatile;
}
*/



static void
stirrer_set_delay (MktStirrerObject *stirrer_object, guint delay )
{

	NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3(stirrer_object->priv->node);
	if(doppelmotor)
	{
		nodes_doppelmotor3_call_set_stepper1_delay(doppelmotor,delay,NULL,NULL,NULL);
		stirrers_simple_set_delay(stirrers_object_get_simple(STIRRERS_OBJECT(stirrer_object)),delay);
	}
}

static void
stirrer_set_current (MktStirrerObject *stirrer_object, guint current )
{
	guint count = stirrers_simple_get_count(stirrers_object_get_simple(STIRRERS_OBJECT(stirrer_object)));
	NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3(stirrer_object->priv->node);
	if(doppelmotor)
	{
		count =count>0?count:1;
		nodes_doppelmotor3_call_set_stepper1_current(doppelmotor,current*count,NULL,NULL,NULL);
		stirrers_simple_set_current(stirrers_object_get_simple(STIRRERS_OBJECT(stirrer_object)),current*count);
	}
}

static void
mkt_stirrer_init_NodesObject     ( MktStirrerObject *mkt_stirrer_object )
{
	g_return_if_fail(mkt_can_manager_client_nodes()!=NULL);
	mkt_stirrer_object->priv->node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(),mkt_stirrer_object->priv->address));
}

static gboolean
mkt_stirrer_object_initialize_real ( MktStirrerObject *mkt_stirrer_object )
{
	if(mkt_stirrer_object->priv->node)
	{
		StirrersSimple *stirrer = stirrers_object_get_simple(STIRRERS_OBJECT(mkt_stirrer_object));
		stirrers_simple_set_is_on(stirrer,FALSE);
		NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3(mkt_stirrer_object->priv->node);
		if(doppelmotor)
		{
			nodes_doppelmotor3_call_set_stepper1_mode(doppelmotor,1,NULL,NULL,NULL);
		}
		else
		{
			g_critical("Control node %s not found",mkt_stirrer_object->priv->address);
		}
		stirrer_set_delay(mkt_stirrer_object,stirrers_simple_get_delay_mod1(stirrer));
		stirrer_set_current(mkt_stirrer_object,stirrers_simple_get_current_mod1(stirrer));
		stirrers_simple_set_is_on(stirrer,TRUE);
		return TRUE;
	}
	return FALSE;
}

static gboolean
mkt_stirrer_initialize ( MktStirrerObject *mkt_stirrer_object )
{
	if(MKT_STIRRER_OBJECT_GET_CLASS(mkt_stirrer_object)->initialize)
		return MKT_STIRRER_OBJECT_GET_CLASS(mkt_stirrer_object)->initialize(mkt_stirrer_object);
	return FALSE;
}



static void
mkt_stirrer_object_init (MktStirrerObject *mkt_stirrer_object)
{
	MktStirrerObjectPrivate *priv      = mkt_stirrer_object_get_instance_private(mkt_stirrer_object);
	priv->address                      = NULL;
	mkt_stirrer_object->priv           = priv;
}


static void
mkt_stirrer_object_finalize (GObject *object)
{
	MktStirrerObject *stirrer = MKT_STIRRER_OBJECT(object);
	if(stirrer->priv->current_mod1)  g_object_unref(stirrer->priv->current_mod1);
	if(stirrer->priv->current_mod2)  g_object_unref(stirrer->priv->current_mod2);
	if(stirrer->priv->delay_mod1)    g_object_unref(stirrer->priv->delay_mod1);
	if(stirrer->priv->delay_mod2)    g_object_unref(stirrer->priv->delay_mod2);
	G_OBJECT_CLASS (mkt_stirrer_object_parent_class)->finalize (object);
}


static gboolean
mkt_stirrer_object_reset_callback (StirrersSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
	MktStirrerObject *stirrer = MKT_STIRRER_OBJECT(user_data);
	g_dbus_method_invocation_return_value (invocation,g_variant_new ("(b)", mkt_stirrer_initialize(stirrer)));
		return TRUE;
}

static gboolean
mkt_stirrer_object_run_mod1_callback (StirrersSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
	MktStirrerObject *stirrer = MKT_STIRRER_OBJECT(user_data);
	stirrers_simple_set_is_on(interface,FALSE);
	stirrer_set_delay(stirrer,stirrers_simple_get_delay_mod1(interface));
	stirrer_set_current(stirrer,stirrers_simple_get_current_mod1(interface));
	stirrers_simple_set_is_on(interface,TRUE);
	g_dbus_method_invocation_return_value (invocation,g_variant_new ("(b)", TRUE));
	return TRUE;
}


static gboolean
mkt_stirrer_object_run_mod2_callback (StirrersSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
	MktStirrerObject *stirrer = MKT_STIRRER_OBJECT(user_data);
	stirrers_simple_set_is_on(interface,FALSE);
	stirrer_set_delay(stirrer,stirrers_simple_get_delay_mod1(interface));
	stirrer_set_current(stirrer,stirrers_simple_get_current_mod2(interface));
	stirrers_simple_set_is_on(interface,TRUE);
	g_dbus_method_invocation_return_value (invocation,g_variant_new ("(b)", TRUE));
	return TRUE;
}

static gboolean
mkt_stirrer_object_update_callback (StirrersSimple *interface, GDBusMethodInvocation *invocation, gpointer user_data)
{
	MktStirrerObject *stirrer = MKT_STIRRER_OBJECT(user_data);
	NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3(stirrer->priv->node);
	if(doppelmotor)
	{
		guint current = 0;
		guint delay = 0;
		nodes_doppelmotor3_call_get_stepper1_current_sync(doppelmotor,&current,NULL,NULL);
		nodes_doppelmotor3_call_get_stepper1_delay_sync(doppelmotor,&delay,NULL,NULL);
		stirrers_simple_set_current(interface,current);
		stirrers_simple_set_delay(interface,delay);
		return TRUE;
	}
	return FALSE;
}


static void
mkt_stirrer_object_is_on_callback (StirrersSimple *stirrer, GParamSpec *pspec,  MktStirrerObject *stirrer_object )
{
	NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3(stirrer_object->priv->node);
	if(doppelmotor)
	{
		nodes_doppelmotor3_call_set_stepper1_on(doppelmotor,stirrers_simple_get_is_on(stirrer),NULL,NULL,NULL);
	}
	else
	{
		g_critical("Control node %s name vanished",stirrer_object->priv->address);
	}
}


static void
mkt_stirrer_object_constructed ( GObject *object )
{
	MktStirrerObject *stirrer = MKT_STIRRER_OBJECT(object);
	StirrersSimple *p = stirrers_simple_skeleton_new();
	g_signal_connect (p,"handle-reset",G_CALLBACK (mkt_stirrer_object_reset_callback),stirrer);
	g_signal_connect (p,"handle-run-mod1",G_CALLBACK (mkt_stirrer_object_run_mod1_callback),stirrer);
	g_signal_connect (p,"handle-run-mod2",G_CALLBACK (mkt_stirrer_object_run_mod2_callback),stirrer);
	g_signal_connect (p,"handle-update",G_CALLBACK (mkt_stirrer_object_update_callback),stirrer);

	stirrers_object_skeleton_set_simple(STIRRERS_OBJECT_SKELETON(stirrer),p);
	g_object_unref(p);
	MktParamuint32 *param = NULL;
	stirrer->priv->current_mod1= mkt_paramuint32_get("Stirrer.parameters",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"current-mod1");
	if(stirrer->priv->current_mod1 == NULL)
	{
		stirrer->priv->current_mod1 = MKT_PARAMUINT32(mkt_model_new(MKT_TYPE_PARAMUINT32_MODEL,"param-object-id","Stirrer.parameters","param-object-path",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"param-name","current-mod1","param-activated",TRUE,"value",70,NULL));
	}
	stirrer->priv->current_mod2 = mkt_paramuint32_get("Stirrer.parameters",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"current-mod2");
	if(stirrer->priv->current_mod2 == NULL)
	{
		stirrer->priv->current_mod2 = MKT_PARAMUINT32(mkt_model_new(MKT_TYPE_PARAMUINT32_MODEL,"param-object-id","Stirrer.parameters","param-object-path",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"param-name","current-mod2","param-activated",TRUE,"value",200,NULL));
	}

	stirrer->priv->delay_mod1= mkt_paramuint32_get("Stirrer.parameters",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"delay-mod1");
	if(stirrer->priv->delay_mod1 == NULL)
	{
		stirrer->priv->delay_mod1 = MKT_PARAMUINT32(mkt_model_new(MKT_TYPE_PARAMUINT32_MODEL,"param-object-id","Stirrer.parameters","param-object-path",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"param-name","delay-mod1","param-activated",TRUE,"value",50,NULL));
	}
	stirrer->priv->delay_mod2 = mkt_paramuint32_get("Stirrer.parameters",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"delay-mod2");
	if(stirrer->priv->delay_mod2 == NULL)
	{
		stirrer->priv->delay_mod2 = MKT_PARAMUINT32(mkt_model_new(MKT_TYPE_PARAMUINT32_MODEL,"param-object-id","Stirrer.parameters","param-object-path",g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),"param-name","delay-mod2","param-activated",TRUE,"value",50,NULL));
	}

	g_object_bind_property(stirrer->priv->current_mod1,"value",p,"current-mod1",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(stirrer->priv->current_mod2,"value",p,"current-mod2",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(stirrer->priv->delay_mod1,"value",p,"delay-mod1",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_object_bind_property(stirrer->priv->delay_mod2,"value",p,"delay-mod2",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
	g_signal_connect(p,"notify::is-on",G_CALLBACK(mkt_stirrer_object_is_on_callback),stirrer);

	mkt_stirrer_init_NodesObject(stirrer);
	G_OBJECT_CLASS (mkt_stirrer_object_parent_class)->constructed (object);
}

static void
mkt_stirrer_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (MKT_IS_STIRRER_OBJECT (object));
	MktStirrerObject *control = MKT_STIRRER_OBJECT(object);
	switch (prop_id)
	{
	case PROP_STIRRER_OBJECT_ADDRESS:
		if(control->priv->address ) g_free(control->priv->address);
		control->priv->address = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
mkt_stirrer_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (MKT_IS_STIRRER_OBJECT (object));
	MktStirrerObject *control = MKT_STIRRER_OBJECT(object);
	switch (prop_id)
	{
	case PROP_STIRRER_OBJECT_ADDRESS:
		g_value_set_string(value,control->priv->address);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
mkt_stirrer_object_class_init (MktStirrerObjectClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//MktHdwControlObjectClass* parent_class = MKT_HDW_CONTROL_OBJECT_CLASS (klass);
	object_class->finalize       = mkt_stirrer_object_finalize;
	object_class->set_property   = mkt_stirrer_object_set_property;
	object_class->get_property   = mkt_stirrer_object_get_property;
	object_class->constructed    = mkt_stirrer_object_constructed;
	klass->initialize            = mkt_stirrer_object_initialize_real;
	//parent_class->initialize     = NULL;
	//parent_class->emergency_stop = NULL;
	g_object_class_install_property (object_class,PROP_STIRRER_OBJECT_ADDRESS,
			g_param_spec_string  ("stirrer-node-address",
					_("Stirrer node address"),
					_("Stirrer node address"),
					"/com/lar/nodes/Doppelmotor1",
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));
}

gboolean
mkt_stirrer_object_reset                        ( MktStirrerObject *stirrer_object )
{
	g_return_val_if_fail(stirrer_object!=NULL,FALSE);
	g_return_val_if_fail(MKT_IS_STIRRER_OBJECT(stirrer_object),FALSE);
	return mkt_stirrer_initialize(stirrer_object);
}


/** @} */
