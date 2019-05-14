/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "temperatur-observer.h"
#include "ultraconfig.h"
#include <gio/gio.h>
#include <mktlib.h>

enum
{
	PROP_0,
};

#include "../../config.h"
#include <glib/gi18n-lib.h>

#define TEMPERATUR_TYPE_OBJECT (temperatur_object_get_type())
#define TEMPERATUR_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TEMPERATUR_TYPE_OBJECT, TemperaturObject))
#define TEMPERATUR_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TEMPERATUR_TYPE_OBJECT, TemperaturObjectClass))
#define TEMPERATUR_IS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TEMPERATUR_TYPE_OBJECT))
#define TEMPERATUR_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TEMPERATUR_TYPE_OBJECT))
#define TEMPERATUR_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), TEMPERATUR_TYPE_OBJECT, TemperaturObjectClass))

typedef struct _TemperaturObjectClass TemperaturObjectClass;
typedef struct _TemperaturObject TemperaturObject;

typedef struct _TemperaturObjectPrivate TemperaturObjectPrivate;
struct _TemperaturObjectClass
{
	GObjectClass parent_class;
};

struct _TemperaturObject
{
	GObject parent_instance;
	TemperaturObjectPrivate *priv;
};

GType temperatur_object_get_type(void) G_GNUC_CONST;

struct _TemperaturObjectPrivate
{
	GCancellable *check;
	NodesObject *nodes_object;
	NodesAnalog1 *analog_node1;
	guint tag;
	gboolean fan;
	gboolean defect_fan;

	gboolean brocked;
	gboolean too_hot;
	gboolean too_cold;

	MonitoringTemperatur *temp;
};

#define TEMPERATUR_OBJECT_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TEMPERATUR_TYPE_OBJECT, TemperaturObjectPrivate))

static gboolean is_activated(TemperaturObject *temperatur)
{
	return monitoring_temperatur_get_check_furnace(temperatur->priv->temp) || monitoring_temperatur_get_check_housing(temperatur->priv->temp);
}

static NodesAnalog1 *
temperatur_get_NodesObject(TemperaturObject *temperatur)
{
	g_return_val_if_fail(mkt_can_manager_client_nodes() != NULL, NULL);
	temperatur->priv->nodes_object = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Analog1"));
	if (temperatur->priv->nodes_object == NULL)
	{
		return NULL;
	}
	return nodes_object_get_analog1(temperatur->priv->nodes_object);
}
static NodesDigital16 *
get_digital16()
{
	g_return_val_if_fail(mkt_can_manager_client_nodes() != NULL, NULL);
	NodesObject *node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital1"));
	if (node != NULL)
	{
		NodesDigital16 *digital = nodes_object_get_digital16(node);

		g_object_unref(node);
		return digital;
	}
	return NULL;
}
static void fan_state(TemperaturObject *temperatur)
{
	NodesDigital16 *digital = get_digital16();
	if (digital == NULL)
	{
		if (!temperatur->priv->defect_fan)
			mkt_log_error_message_sync("control: temperatur monitoring digital fan control node not found");
		temperatur->priv->defect_fan = TRUE;
		return;
	}
	GError *error = NULL;
	gboolean result = FALSE;
	nodes_digital16_call_set_digital_out_sync(digital, 12, !temperatur->priv->fan, &result, NULL, &error);
	if (error)
	{
		g_dbus_error_strip_remote_error(error);
		if (!temperatur->priv->defect_fan)
			mkt_log_error_message_sync("control: temperatur monitoring error - %s", error->message);
		temperatur->priv->defect_fan = TRUE;
		g_error_free(error);
	}
	g_object_unref(digital);
}

static void ofenraum_ok(TemperaturObject *temperatur)
{
	if (!temperatur->priv->too_hot)
		return;
	temperatur->priv->too_hot = FALSE;
	mkt_errors_clean(E1872);
}

// Innenraum - Temperaturuberwachung meldet zu calt
static void ofenraum_too_hot(TemperaturObject *temperatur)
{
	if (temperatur->priv->too_hot)
		return;
	temperatur->priv->too_hot = TRUE;
	mkt_errors_come(E1872);
}

static void innenraum_ok(TemperaturObject *temperatur)
{
	if (temperatur->priv->too_cold || !temperatur->priv->fan)
	{
		temperatur->priv->too_cold = FALSE;
		temperatur->priv->fan = TRUE;
		fan_state(temperatur);
		mkt_errors_clean(E1871);
	}
}
// Innenraum - Temperaturuberwachung meldet zu calt
static void innenraum_too_cold(TemperaturObject *temperatur)
{
	if (temperatur->priv->too_cold)
		return;

	temperatur->priv->too_cold = TRUE;
	mkt_errors_come(E1871);
	temperatur->priv->fan = FALSE;
	fan_state(temperatur);
}

// Temperature.
static void brocked_error(TemperaturObject *temperatur)
{
	if (is_activated(temperatur))
	{
		if (!temperatur->priv->brocked)
		{
			temperatur->priv->brocked = TRUE;
			mkt_errors_come(E1870);
		}
	}
}

G_DEFINE_TYPE(TemperaturObject, temperatur_object, G_TYPE_OBJECT)

static void
temperatur_set_innenraum_temperatur(TemperaturObject *temperatur, gdouble temp_c)
{
	monitoring_temperatur_set_housing_current(temperatur->priv->temp, temp_c);
	if (!monitoring_temperatur_get_check_housing(temperatur->priv->temp))
	{
		innenraum_ok(temperatur);
	}
	else
	{
		gdouble min = monitoring_temperatur_get_housing_min(temperatur->priv->temp);
		if (temp_c < min)
		{
			innenraum_too_cold(temperatur);
		}
		else if (temperatur->priv->too_cold)
		{
			if (temp_c > min + 5.0)
			{
				innenraum_ok(temperatur);
			}
		}
	}
}

static void
temperatur_set_ofenraum_temperatur(TemperaturObject *temperatur, gdouble temp_c)
{
	monitoring_temperatur_set_furnace_current(temperatur->priv->temp, temp_c);
	gdouble max = monitoring_temperatur_get_furnace_max(temperatur->priv->temp);
	if (!monitoring_temperatur_get_check_furnace(temperatur->priv->temp))
	{
		ofenraum_ok(temperatur);
	}
	else
	{
		if (temp_c > max)
			ofenraum_too_hot(temperatur);
		else
			ofenraum_ok(temperatur);
	}
}

// -------------------------------------------- Help functions --------------------------------------------------------

static void
temperatur_innenraum_async_callback(GObject *source_object,
									GAsyncResult *res,
									gpointer user_data)
{
	TemperaturObject *temperatur = TEMPERATUR_OBJECT(user_data);
	GError *error = NULL;
	gdouble out_value = 0.0;
	if (!nodes_analog1_call_get_temp3_finish(temperatur->priv->analog_node1, &out_value, res, &error))
	{
		if (error->code != G_IO_ERROR_CANCELLED)
			g_critical("Get temperatur3 value  error - %s", error ? error->message : "unknown");
		if (error)
			g_error_free(error);
	}
	else
	{
		temperatur_set_innenraum_temperatur(temperatur, out_value / 10.0);
	}
}

static void
temperatur_ofenraum_async_callback(GObject *source_object,
								   GAsyncResult *res,
								   gpointer user_data)
{
	TemperaturObject *temperatur = TEMPERATUR_OBJECT(user_data);
	GError *error = NULL;
	gdouble out_value = 0.0;
	if (!nodes_analog1_call_get_temp2_finish(temperatur->priv->analog_node1, &out_value, res, &error))
	{
		if (error->code != G_IO_ERROR_CANCELLED)
			g_critical("Get temperatur2 value  error - %s", error ? error->message : "unknown");
		if (error)
			g_error_free(error);
	}
	else
	{
		temperatur_set_ofenraum_temperatur(temperatur, out_value / 10.0);
	}
}

gboolean
temperatur_object_waite_read(gpointer user_data)
{
	TemperaturObject *temperatur = TEMPERATUR_OBJECT(user_data);
	if (is_activated(temperatur))
	{
		if (temperatur->priv->check)
		{
			g_cancellable_cancel(temperatur->priv->check);
			g_object_unref(temperatur->priv->check);
		}
		temperatur->priv->check = g_cancellable_new();
		nodes_analog1_call_get_temp2(temperatur->priv->analog_node1, temperatur->priv->check, temperatur_ofenraum_async_callback, temperatur);
		nodes_analog1_call_get_temp3(temperatur->priv->analog_node1, temperatur->priv->check, temperatur_innenraum_async_callback, temperatur);
	}
	else
	{
		ofenraum_ok(temperatur);
		innenraum_ok(temperatur);
		if (temperatur->priv->brocked)
			mkt_errors_clean(E1870);
	}
	return TRUE;
}

static void
temperatur_object_waite_read_destroy(gpointer user_data)
{
	TemperaturObject *temperatur = TEMPERATUR_OBJECT(user_data);
	temperatur->priv->tag = 0;
}

static void
temperatur_object_constructed(GObject *object)
{
	TemperaturObject *temperatur = TEMPERATUR_OBJECT(object);
	temperatur->priv->temp = monitoring_temperatur_skeleton_new();
	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(temperatur->priv->temp), tera_service_dbus_connection(), TERA_TEMPERATUR_OBSERVER_PATH, NULL);
	ConfigureTemperatur(g_object_ref(temperatur->priv->temp));
	temperatur->priv->analog_node1 = temperatur_get_NodesObject(temperatur);
	if (temperatur->priv->analog_node1 == NULL)
	{
		brocked_error(temperatur);
	}
	else
	{
		g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(temperatur->priv->analog_node1), 500);
		temperatur->priv->tag = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, 2, temperatur_object_waite_read, temperatur, temperatur_object_waite_read_destroy);
	}
	NodesDigital16 *digital = get_digital16();
	if(digital){

		g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(digital), 500);
		g_object_unref(digital);
	}
	temperatur->priv->fan = TRUE;
	fan_state(temperatur);

	if (G_OBJECT_CLASS(temperatur_object_parent_class)->constructed)
		G_OBJECT_CLASS(temperatur_object_parent_class)->constructed(object);
}

static void
temperatur_object_init(TemperaturObject *temperatur_object)
{
	TemperaturObjectPrivate *priv = TEMPERATUR_OBJECT_PRIVATE(temperatur_object);
	temperatur_object->priv = priv;
	priv->nodes_object = NULL;
	priv->analog_node1 = NULL;
	priv->check = NULL;
	priv->too_cold = FALSE;
	priv->too_hot = FALSE;
	mkt_errors_clean(E1870);
	mkt_errors_clean(E1871);
	mkt_errors_clean(E1872);

	//Settings property connection ...
	/* TODO: Add initialization code here */
}

static void
temperatur_object_finalize(GObject *object)
{
	TemperaturObject *temperatur = TEMPERATUR_OBJECT(object);
	if (temperatur->priv->check)
	{
		g_cancellable_cancel(temperatur->priv->check);
		g_object_unref(temperatur->priv->check);
		temperatur->priv->check = NULL;
	}
	if (temperatur->priv->tag)
		g_source_remove(temperatur->priv->tag);

	G_OBJECT_CLASS(temperatur_object_parent_class)->finalize(object);
}

static void
temperatur_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail(TEMPERATUR_IS_OBJECT(object));
	//TemperaturObject *temperatur = TEMPERATUR_OBJECT(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
temperatur_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail(TEMPERATUR_IS_OBJECT(object));
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
temperatur_object_class_init(TemperaturObjectClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(TemperaturObjectPrivate));
	object_class->finalize = temperatur_object_finalize;
	object_class->set_property = temperatur_object_set_property;
	object_class->get_property = temperatur_object_get_property;
	object_class->constructed = temperatur_object_constructed;
}

static TemperaturObject *temperatur_observer = NULL;

MonitoringTemperatur *temperatur_observer_sensor()
{
	if (temperatur_observer == NULL)
	{
		temperatur_observer = TEMPERATUR_OBJECT(g_object_new(TEMPERATUR_TYPE_OBJECT, NULL));
	}
	return temperatur_observer->priv->temp;
}
