/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraconfig.h
 * Copyright (C) LAR 2017
 *
 */

#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_

#include <mktbus.h>
#include <mktlib.h>

gboolean ConfigureIsTestMode();
void ConfigureSetTestMode();
void ConfigureSetNormalMode();

gboolean ConfigureConvertParameterV52toV53();
Ultradevice *ConfigureDevice();
gboolean ConfigureBindRedis();
gboolean ConfigureAxis(TeraClientObject *client);
gboolean ConfigureAnalogs(TeraClientObject *client);
gboolean ConfigureSecurity(TeraClientObject *client);
gboolean ConfigureRelays(TeraClientObject *client);
gboolean ConfigureHumidity(TeraClientObject *client);
gboolean ConfigurePressure(TeraClientObject *client);
gboolean ConfigureVessels(TeraClientObject *client);
gboolean ConfigureAirflow(TeraClientObject *client);
gboolean ConfigureSensors(TeraClientObject *client);
gboolean ConfigureSequence(TeraClientObject *client);
gboolean ConfigureIntegrations(GDBusObjectManager *manager);    
gboolean ConfigureStreams(GDBusObjectManager *manager);
gboolean ConfigureTemperatur(MonitoringTemperatur *temp_observer);
gboolean ConfigurePC(TeraClientObject *client);
guint ConfigureCheckLicense();
void ConfigureLogger();
void ultra_test_data();

#endif /* _CONFIGURE_H_ */
