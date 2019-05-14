/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultra-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
ultra-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ultra-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ULTRASTREAM_OBJECT_H_
#define _ULTRASTREAM_OBJECT_H_

#include <mktbus.h>
#include <mktlib.h>
#include "mkt-process-object.h"

G_BEGIN_DECLS


enum {
    ULTRA_PROCESS_ONLINE = 200,
    ULTRA_PROCESS_CHECK = 400, 
};

#define ULTRA_TYPE_STREAM_OBJECT (ultra_stream_object_get_type())
#define ULTRA_STREAM_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_STREAM_OBJECT, UltraStreamObject))
#define ULTRA_STREAM_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_STREAM_OBJECT, UltraStreamObjectClass))
#define ULTRA_IS_STREAM_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_STREAM_OBJECT))
#define ULTRA_IS_STREAM_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_STREAM_OBJECT))
#define ULTRA_STREAM_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_STREAM_OBJECT, UltraStreamObjectClass))

typedef struct _UltraStreamObjectClass   UltraStreamObjectClass;
typedef struct _UltraStreamObject        UltraStreamObject;
typedef struct _UltraStreamObjectPrivate UltraStreamObjectPrivate;

enum {
    STATE_UNKNOWN,
    STATE_SINGLE,
    STATE_ONLINE,
    STATE_CALIBRATION,
    STATE_CHECK,
};

struct _UltraStreamObjectClass {
    StreamsObjectSkeletonClass parent_class;
};

struct _UltraStreamObject {
    StreamsObjectSkeleton     parent_instance;
    UltraStreamObjectPrivate *priv;
};

GType ultra_stream_object_get_type(void) G_GNUC_CONST;

void ultra_stream_server_run(GDBusConnection *connection);
GDBusObjectManager *ultra_stream_server_object_manager(void );
MktProcessObject *ultra_stream_calibration_process(UltraStreamObject *stream);

GList *ultra_stream_channels(UltraStreamObject *stream);

void ultra_stream_change_status(UltraStreamObject *stream, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

void ultra_stream_set_state(UltraStreamObject *stream, guint state);
PumpsObject *ultra_stream_get_pump(UltraStreamObject *stream);
void ultra_stream_set_default_pump(UltraStreamObject *stream);
void ultra_stream_set_pump(UltraStreamObject *stream, PumpsObject *pump);

VesselsObject *ultra_stream_get_drain(UltraStreamObject *stream);
VesselsObject *ultra_stream_get_sample(UltraStreamObject *stream);
GList *ultra_stream_process_channels(UltraStreamObject *stream);
gboolean ultra_stream_get_sampling(UltraStreamObject *stream);
void ultra_stream_no_sampling(UltraStreamObject *stream);
void ultra_stream_on_sampling(UltraStreamObject *stream);

ChannelsObject* UltraDBusStreamChannelTC(UltraStreamObject *stream);
ChannelsObject* UltraDBusStreamChannelTIC(UltraStreamObject *stream);
ChannelsObject* UltraDBusStreamChannelTOC(UltraStreamObject *stream);
ChannelsObject* UltraDBusStreamChannelTNb(UltraStreamObject *stream);
ChannelsObject* UltraDBusStreamChannelCODo(UltraStreamObject *stream);
ProcessObject*  UltraDBusStreamGetOnlineProcess(UltraStreamObject *stream);
ProcessObject*  UltraDBusStreamGetSingleProcess(UltraStreamObject *stream);
ProcessObject*  UltraDBusStreamGetCalibrationProcess(UltraStreamObject *stream);
ProcessObject*  UltraDBusStreamGetCheckProcess(UltraStreamObject *stream);

void ultra_stream_object_online(UltraStreamObject *stream );
void ultra_stream_object_offline(UltraStreamObject *stream );
void ultra_stream_object_set_remote_address(UltraStreamObject *stream,guint din);
guint ultra_stream_object_get_remote_address(UltraStreamObject *stream);
guint ultra_stream_object_get_number(UltraStreamObject *stream);

void ultra_stream_check_models(UltraStreamObject *stream);

G_END_DECLS

#endif /* _ULTRASTREAM_OBJECT_H_ */
