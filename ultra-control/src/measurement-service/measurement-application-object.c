/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "measurement-application-object.h"

#include <market-time.h>
#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include <gio/gio.h>
#include "ultraconfig.h"
#include "ultra-stream-object.h"
#include "ultra-channel-object.h"
#include "ultra-integration-object.h"
#include "ultra-control-process.h"
#include "temperatur-observer.h"

#include "../../config.h"
#include <glib/gi18n-lib.h>

enum
{
  PROP_0,
};

struct _MeasurementApplicationObjectPrivate
{
  GCancellable *cancelable;
  guint plug_tag;
};

G_DEFINE_TYPE_WITH_PRIVATE(MeasurementApplicationObject,
                           measurement_application_object,
                           TERA_TYPE_SERVICE_OBJECT);

static void measurement_application_object_init(
    MeasurementApplicationObject *measurement_application_object)
{
  MeasurementApplicationObjectPrivate *priv =
      measurement_application_object_get_instance_private(
          measurement_application_object);
  priv->cancelable = g_cancellable_new();
  measurement_application_object->priv = priv;
  mkt_model_select_async(MKT_TYPE_CHANNEL_MODEL, NULL, NULL,
                         "UPDATE %s SET param_activated=0",
                         g_type_name(MKT_TYPE_CHANNEL_MODEL));
}

static void measurement_application_object_finalize(GObject *object)
{
  // MeasurementApplicationObject *measurement_application =
  // MEASUREMENT_APPLICATION_OBJECT(object);
  G_OBJECT_CLASS(measurement_application_object_parent_class)->finalize(object);
}

static void measurement_application_object_set_property(GObject *object,
                                                        guint prop_id,
                                                        const GValue *value,
                                                        GParamSpec *pspec)
{

  g_return_if_fail(MEASUREMENT_IS_APPLICATION_OBJECT(object));
  // MeasurementApplicationObject *data =
  // MEASUREMENT_APPLICATION_OBJECT(object);
  switch (prop_id)
  {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void measurement_application_object_get_property(GObject *object,
                                                        guint prop_id,
                                                        GValue *value,
                                                        GParamSpec *pspec)
{
  g_return_if_fail(MEASUREMENT_IS_APPLICATION_OBJECT(object));
  // MeasurementApplicationObject *data =
  // MEASUREMENT_APPLICATION_OBJECT(object);
  switch (prop_id)
  {

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
measurement_applictation_object_chech_hw(TeraServiceObject *service)
{
  LarpcDevice *device = mkt_pc_manager_client_get_device();
  if(device == NULL) {
      mkt_errors_come(E1700);
  }
  guint streams_lizense = 1;
  larpc_device_call_check_stream_license_sync(device, &streams_lizense, NULL, NULL);
  // Check Digital node 2
  if (streams_lizense > 2)
  {

    NodesObject *digital_node2 = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(), "/com/lar/nodes/Digital2"));
    if (digital_node2 == NULL)
    {
      mkt_log_error_message_sync("Control service:You need Digital Node 2 to start with %d Streams", streams_lizense);
      mkt_errors_come(E1700);
    }
  }
}

static void
measurement_application_object_activated(TeraServiceObject *service)
{
  // MeasurementApplicationObject *measurement_application =
  // MEASUREMENT_APPLICATION_OBJECT(service);
  ultra_control_new();
  ultra_integration_server_run(tera_service_dbus_connection());
  measurement_applictation_object_chech_hw(service);
  ultra_stream_server_run(tera_service_dbus_connection());
  ConfigureStreams(ultra_stream_server_object_manager());
  ultra_control_init_stirrers();
  temperatur_observer_sensor();
  service_simple_set_done(tera_service_get_simple(), TRUE);
  service_simple_emit_initialized(tera_service_get_simple(), TRUE);
  ConfigureLogger();
  // ultra_test_data();
}

static void measurement_application_object_class_init( MeasurementApplicationObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  TeraServiceObjectClass *app_class = TERA_SERVICE_OBJECT_CLASS(klass);
  object_class->finalize = measurement_application_object_finalize;
  object_class->set_property = measurement_application_object_set_property;
  object_class->get_property = measurement_application_object_get_property;
  app_class->activated = measurement_application_object_activated;
}
