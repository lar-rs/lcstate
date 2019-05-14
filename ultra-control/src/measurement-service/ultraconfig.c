/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * axis.c
 * Copyright (C) LAR 2017
 *
 */

#include "ultraconfig.h"
#include <stdio.h>
#include <string.h>
#include <ultimate-library.h>
#include "../../config.h"
#include "ultra-integration-object.h"
#include "ultra-stream-object.h"

#include "temperatur-observer.h"

static gboolean isTestMode = FALSE;

void ConfigureSetTestMode() { isTestMode = TRUE && TEST_MODE; }

void ConfigureSetNormalMode() { isTestMode = FALSE; }

gboolean ConfigureIsTestMode() { return isTestMode; }

Ultradevice *ConfigureDevice()
{
    static Ultradevice *device = NULL;
    if (device == NULL)
    {
        device = ULTRADEVICE(g_object_new(TYPE_ULTRADEVICE, NULL));
    }
    return device;
}

static void parameter_change_protocol(GObject *object, GParamSpec *pspec,
                                      gpointer userData)
{
    GValue *value = mkt_value_new(pspec->value_type);
    g_object_get_property(object, pspec->name, value);
    gchar *desk1 = (gchar *)userData;
    mkt_log_message(MKT_LOG_STATE_PARAMETER_CHANGE, "%s %s changed to (%s)",
                    desk1 ? desk1 : "", pspec->_nick,
                    mkt_value_stringify_static(value));
    mkt_value_free(value);
}

static void parameter_object_connect(GObject *param_object, const gchar *desc)
{
    g_signal_connect(param_object, "notify",
                     G_CALLBACK(parameter_change_protocol), (gpointer)desc);
}

static void device_parameter_change_protocol(Ultradevice *device)
{
    parameter_object_connect(G_OBJECT(device), "");
    parameter_object_connect(G_OBJECT(UltradeviceGetFurnace(device)), "Furnace");
    parameter_object_connect(G_OBJECT(UltradeviceGetTicPort(device)), "TIC-Port");
    parameter_object_connect(G_OBJECT(UltradeviceGetV1(device)), "V1");
    parameter_object_connect(G_OBJECT(UltradeviceGetV2(device)), "V2");
    parameter_object_connect(G_OBJECT(UltradeviceGetV3(device)), "V3");
    parameter_object_connect(G_OBJECT(UltradeviceGetV4(device)), "V4");
    parameter_object_connect(G_OBJECT(UltradeviceGetV5(device)), "V5");
    parameter_object_connect(G_OBJECT(UltradeviceGetV6(device)), "V6");
    parameter_object_connect(G_OBJECT(UltradeviceGetXAxis(device)), "X-Axis");
    parameter_object_connect(G_OBJECT(UltradeviceGetYAxis(device)), "Y-Axis");
    parameter_object_connect(G_OBJECT(UltradeviceGetInjection(device)),
                             "Injection");
    parameter_object_connect(G_OBJECT(UltradeviceGetDilution(device)),
                             "Dilution");
    parameter_object_connect(G_OBJECT(UltradeviceGetAirflow(device)), "Airflow");
    parameter_object_connect(G_OBJECT(UltradeviceGetHumidity(device)),
                             "Humidity");
    parameter_object_connect(G_OBJECT(UltradeviceGetPressure(device)),
                             "Pressure");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay1(device)), "Relay #1");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay2(device)), "Relay #2");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay3(device)), "Relay #3");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay4(device)), "Relay #4");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay5(device)), "Relay #5");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay6(device)), "Relay #6");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay7(device)), "Relay #7");
    parameter_object_connect(G_OBJECT(UltradeviceGetRelay8(device)), "Relay #8");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir1TC1(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR1 TC 1");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir1TC2(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR1 TC 2");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir1TC3(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR1 TC 3");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir1TIC1(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR1 TIC 1");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir1TIC2(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR1 TIC 2");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir1TIC3(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR1 TIC 3");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir2TC1(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR2 TC 1");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir2TC2(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR2 TC 2");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir2TC3(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR2 TC 3");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir2TIC1(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR2 TiC 1");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir2TIC2(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR2 TiC 2");
    parameter_object_connect(G_OBJECT(m_LarIntegrationsGetNDir2TIC3(
                                 UltradeviceGetIntegrations(device))),
                             "NDIR2 TiC 3");
    parameter_object_connect(
        G_OBJECT(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(device))),
        "TNb TC");
    parameter_object_connect(
        G_OBJECT(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(device))),
        "TNb TIC");
    parameter_object_connect(
        G_OBJECT(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(device))),
        "CODo TC");
    
    parameter_object_connect(G_OBJECT(UltradeviceGetNDir1(device)), "Sensor NDIR1");
    parameter_object_connect(G_OBJECT(UltradeviceGetNDir2(device)), "Sensor NDIR2");
    parameter_object_connect(G_OBJECT(UltradeviceGetTNb(device)), "Sensor Zirox");
    parameter_object_connect(G_OBJECT(UltradeviceGetCODo(device)), "Sensor CO2");
    parameter_object_connect(G_OBJECT(UltradeviceGetTemperatur(device)), "Temperatur");
}

gboolean ConfigureBindRedis()
{
    Ultradevice *dev = ConfigureDevice();
    // Stream  #1 connect all parameters .
    NewRedisObject("device", dev, "device");
    NewRedisObject("device:stream:1", UltradeviceGetStream1(dev), "#1");
    // Statistics
    NewRedisObject("device:stream:1:statistics:online",
                   StatisticsGetOnline(
                       StreamGetStatistics(STREAM(UltradeviceGetStream1(dev)))),
                   "#1 online");
    NewRedisObject("device:stream:1:statistics:single",
                   StatisticsGetSingle(
                       StreamGetStatistics(STREAM(UltradeviceGetStream1(dev)))),
                   "#1 sigle");
    NewRedisObject("device:stream:1:statistics:calibration",
                   StatisticsGetCalibration(
                       StreamGetStatistics(STREAM(UltradeviceGetStream1(dev)))),
                   "#1 calibration");
    NewRedisObject("device:stream:1:statistics:check",
                   StatisticsGetCheck(
                       StreamGetStatistics(STREAM(UltradeviceGetStream1(dev)))),
                   "#1 check");
    // Measurement intervals
    NewRedisObject("device:stream:1:intervals:online",
                   IntervalsGetOnline(
                       StreamGetIntervals(STREAM(UltradeviceGetStream1(dev)))),
                   "#1 online");
    NewRedisObject("device:stream:1:intervals:calibration",
                   IntervalsGetCalibration(
                       StreamGetIntervals(STREAM(UltradeviceGetStream1(dev)))),
                   "#1 calibration");
    NewRedisObject(
        "device:stream:1:intervals:check",
        IntervalsGetCheck(StreamGetIntervals(STREAM(UltradeviceGetStream1(dev)))),
        "#1 check");
    // Online measuremenr Amount
    NewRedisObject("device:stream:1:amount",
                   StreamGetAmount(STREAM(UltradeviceGetStream1(dev))),
                   "#1 amount");
    // Channels
    NewRedisObject("device:stream:1:toc",
                   UltraStreamGetTocChannel(UltradeviceGetStream1(dev)),
                   "#1 TOC");
    NewRedisObject(
        "device:stream:1:toc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream1(dev))),
        "#1 TOC limit");
    NewRedisObject("device:stream:1:toc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream1(dev))),
                   "#1 TOC check limit");

    NewRedisObject("device:stream:1:tc",
                   UltraStreamGetTCChannel(UltradeviceGetStream1(dev)), "#1 TC");
    NewRedisObject(
        "device:stream:1:tc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream1(dev))),
        "#1 TC limit");
    NewRedisObject("device:stream:1:tc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream1(dev))),
                   "#1 TC check limit");

    NewRedisObject("device:stream:1:tic",
                   UltraStreamGetTicChannel(UltradeviceGetStream1(dev)),
                   "#1 TIC");
    NewRedisObject(
        "device:stream:1:tic:limit",
        ChannelGetLimit(UltraStreamGetTicChannel(UltradeviceGetStream1(dev))),
        "#1 TIC limit");
    NewRedisObject("device:stream:1:tic:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTicChannel(UltradeviceGetStream1(dev))),
                   "#1 TIC check limit");

    NewRedisObject("device:stream:1:tnb",
                   UltraStreamGetTnbChannel(UltradeviceGetStream1(dev)),
                   "#1 TNb");
    NewRedisObject(
        "device:stream:1:tnb:limit",
        ChannelGetLimit(UltraStreamGetTnbChannel(UltradeviceGetStream1(dev))),
        "#1 TNb limit");
    NewRedisObject("device:stream:1:tnb:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTnbChannel(UltradeviceGetStream1(dev))),
                   "#1 TNb check limit");

    NewRedisObject("device:stream:1:codo",
                   UltraStreamGetCODoChannel(UltradeviceGetStream1(dev)),
                   "#1 CODo");
    NewRedisObject(
        "device:stream:1:codo:limit",
        ChannelGetLimit(UltraStreamGetCODoChannel(UltradeviceGetStream1(dev))),
        "#1 CODo limit");
    NewRedisObject("device:stream:1:codo:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetCODoChannel(UltradeviceGetStream1(dev))),
                   "#1 CODo check limit");
    // Positions parameters
    NewRedisObject("device:stream:1:position",
                   UltraStreamGetVesselsPos(UltradeviceGetStream1(dev)),
                   "#1 CODo");

    // Stream #2 connect all parameters.
    NewRedisObject("device:stream:2", UltradeviceGetStream2(dev), "#2");
    // Statistics
    NewRedisObject("device:stream:2:statistics:online",
                   StatisticsGetOnline(
                       StreamGetStatistics(STREAM(UltradeviceGetStream2(dev)))),
                   "#2 online");
    NewRedisObject("device:stream:2:statistics:single",
                   StatisticsGetSingle(
                       StreamGetStatistics(STREAM(UltradeviceGetStream2(dev)))),
                   "#2 sigle");
    NewRedisObject("device:stream:2:statistics:calibration",
                   StatisticsGetCalibration(
                       StreamGetStatistics(STREAM(UltradeviceGetStream2(dev)))),
                   "#2 calibration");
    NewRedisObject("device:stream:2:statistics:check",
                   StatisticsGetCheck(
                       StreamGetStatistics(STREAM(UltradeviceGetStream2(dev)))),
                   "#2 check");
    // Measurement intervals
    NewRedisObject("device:stream:2:intervals:online",
                   IntervalsGetOnline(
                       StreamGetIntervals(STREAM(UltradeviceGetStream2(dev)))),
                   "#2 online");
    NewRedisObject("device:stream:2:intervals:calibration",
                   IntervalsGetCalibration(
                       StreamGetIntervals(STREAM(UltradeviceGetStream2(dev)))),
                   "#2 calibration");
    NewRedisObject(
        "device:stream:2:intervals:check",
        IntervalsGetCheck(StreamGetIntervals(STREAM(UltradeviceGetStream2(dev)))),
        "#2 check");
    // Online measuremenr Amount
    NewRedisObject("device:stream:2:amount",
                   StreamGetAmount(STREAM(UltradeviceGetStream2(dev))),
                   "#2 amount");
    // Channels
    NewRedisObject("device:stream:2:toc",
                   UltraStreamGetTocChannel(UltradeviceGetStream2(dev)),
                   "#2 TOC");
    NewRedisObject(
        "device:stream:2:toc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream2(dev))),
        "#2 TOC limit");
    NewRedisObject("device:stream:2:toc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream2(dev))),
                   "#2 TOC check limit");

    NewRedisObject("device:stream:2:tc",
                   UltraStreamGetTCChannel(UltradeviceGetStream2(dev)), "#2 TC");
    NewRedisObject(
        "device:stream:2:tc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream2(dev))),
        "#2 TC limit");
    NewRedisObject("device:stream:2:tc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream2(dev))),
                   "#2 TC check limit");

    NewRedisObject("device:stream:2:tic",
                   UltraStreamGetTicChannel(UltradeviceGetStream2(dev)),
                   "#2 TIC");
    NewRedisObject(
        "device:stream:2:tic:limit",
        ChannelGetLimit(UltraStreamGetTicChannel(UltradeviceGetStream2(dev))),
        "#2 TIC limit");
    NewRedisObject("device:stream:2:tic:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTicChannel(UltradeviceGetStream2(dev))),
                   "#2 TIC check limit");

    NewRedisObject("device:stream:2:tnb",
                   UltraStreamGetTnbChannel(UltradeviceGetStream2(dev)),
                   "#2 TNb");
    NewRedisObject(
        "device:stream:2:tnb:limit",
        ChannelGetLimit(UltraStreamGetTnbChannel(UltradeviceGetStream2(dev))),
        "#2 TNb limit");
    NewRedisObject("device:stream:2:tnb:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTnbChannel(UltradeviceGetStream2(dev))),
                   "#2 TNb check limit");

    NewRedisObject("device:stream:2:codo",
                   UltraStreamGetCODoChannel(UltradeviceGetStream2(dev)),
                   "#2 CODo");
    NewRedisObject(
        "device:stream:2:codo:limit",
        ChannelGetLimit(UltraStreamGetCODoChannel(UltradeviceGetStream2(dev))),
        "#2 CODo limit");
    NewRedisObject("device:stream:2:codo:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetCODoChannel(UltradeviceGetStream2(dev))),
                   "#2 CODo check limit");
    // Positions parameters
    NewRedisObject("device:stream:2:position",
                   UltraStreamGetVesselsPos(UltradeviceGetStream2(dev)),
                   "#2 CODo");

    NewRedisObject("device:stream:3", UltradeviceGetStream3(dev), "#3");
    // Statistics
    NewRedisObject("device:stream:3:statistics:online",
                   StatisticsGetOnline(
                       StreamGetStatistics(STREAM(UltradeviceGetStream3(dev)))),
                   "#3 online");
    NewRedisObject("device:stream:3:statistics:single",
                   StatisticsGetSingle(
                       StreamGetStatistics(STREAM(UltradeviceGetStream3(dev)))),
                   "#3 sigle");
    NewRedisObject("device:stream:3:statistics:calibration",
                   StatisticsGetCalibration(
                       StreamGetStatistics(STREAM(UltradeviceGetStream3(dev)))),
                   "#3 calibration");
    NewRedisObject("device:stream:3:statistics:check",
                   StatisticsGetCheck(
                       StreamGetStatistics(STREAM(UltradeviceGetStream3(dev)))),
                   "#3 check");
    // Measurement intervals
    NewRedisObject("device:stream:3:intervals:online",
                   IntervalsGetOnline(
                       StreamGetIntervals(STREAM(UltradeviceGetStream3(dev)))),
                   "#3 online");
    NewRedisObject("device:stream:3:intervals:calibration",
                   IntervalsGetCalibration(
                       StreamGetIntervals(STREAM(UltradeviceGetStream3(dev)))),
                   "#3 calibration");
    NewRedisObject(
        "device:stream:3:intervals:check",
        IntervalsGetCheck(StreamGetIntervals(STREAM(UltradeviceGetStream3(dev)))),
        "#3 check");
    // Online measuremenr Amount
    NewRedisObject("device:stream:3:amount",
                   StreamGetAmount(STREAM(UltradeviceGetStream3(dev))),
                   "#3 amount");
    // Channels
    NewRedisObject("device:stream:3:toc",
                   UltraStreamGetTocChannel(UltradeviceGetStream3(dev)),
                   "#3 TOC");
    NewRedisObject(
        "device:stream:3:toc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream3(dev))),
        "#3 TOC limit");
    NewRedisObject("device:stream:3:toc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream3(dev))),
                   "#3 TOC check limit");

    NewRedisObject("device:stream:3:tc",
                   UltraStreamGetTCChannel(UltradeviceGetStream3(dev)), "#3 TC");
    NewRedisObject(
        "device:stream:3:tc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream3(dev))),
        "#3 TC limit");
    NewRedisObject("device:stream:3:tc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream3(dev))),
                   "#3 TC check limit");

    NewRedisObject("device:stream:3:tic",
                   UltraStreamGetTicChannel(UltradeviceGetStream3(dev)),
                   "#3 TIC");
    NewRedisObject(
        "device:stream:3:tic:limit",
        ChannelGetLimit(UltraStreamGetTicChannel(UltradeviceGetStream3(dev))),
        "#3 TIC limit");
    NewRedisObject("device:stream:3:tic:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTicChannel(UltradeviceGetStream3(dev))),
                   "#3 TIC check limit");

    NewRedisObject("device:stream:3:tnb",
                   UltraStreamGetTnbChannel(UltradeviceGetStream3(dev)),
                   "#3 TNb");
    NewRedisObject(
        "device:stream:3:tnb:limit",
        ChannelGetLimit(UltraStreamGetTnbChannel(UltradeviceGetStream3(dev))),
        "#3 TNb limit");
    NewRedisObject("device:stream:3:tnb:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTnbChannel(UltradeviceGetStream3(dev))),
                   "#3 TNb check limit");

    NewRedisObject("device:stream:3:codo",
                   UltraStreamGetCODoChannel(UltradeviceGetStream3(dev)),
                   "#3 CODo");
    NewRedisObject(
        "device:stream:3:codo:limit",
        ChannelGetLimit(UltraStreamGetCODoChannel(UltradeviceGetStream3(dev))),
        "#3 CODo limit");
    NewRedisObject("device:stream:3:codo:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetCODoChannel(UltradeviceGetStream3(dev))),
                   "#3 CODo check limit");
    // Positions parameters
    NewRedisObject("device:stream:3:position",
                   UltraStreamGetVesselsPos(UltradeviceGetStream3(dev)),
                   "#3 CODo");

    NewRedisObject("device:stream:4", UltradeviceGetStream4(dev), "#4");
    // Statistics
    NewRedisObject("device:stream:4:statistics:online",
                   StatisticsGetOnline(
                       StreamGetStatistics(STREAM(UltradeviceGetStream4(dev)))),
                   "#4 online");
    NewRedisObject("device:stream:4:statistics:single",
                   StatisticsGetSingle(
                       StreamGetStatistics(STREAM(UltradeviceGetStream4(dev)))),
                   "#4 sigle");
    NewRedisObject("device:stream:4:statistics:calibration",
                   StatisticsGetCalibration(
                       StreamGetStatistics(STREAM(UltradeviceGetStream4(dev)))),
                   "#4 calibration");
    NewRedisObject("device:stream:4:statistics:check",
                   StatisticsGetCheck(
                       StreamGetStatistics(STREAM(UltradeviceGetStream4(dev)))),
                   "#4 check");
    // Measurement intervals
    NewRedisObject("device:stream:4:intervals:online",
                   IntervalsGetOnline(
                       StreamGetIntervals(STREAM(UltradeviceGetStream4(dev)))),
                   "#4 online");
    NewRedisObject("device:stream:4:intervals:calibration",
                   IntervalsGetCalibration(
                       StreamGetIntervals(STREAM(UltradeviceGetStream4(dev)))),
                   "#4 calibration");
    NewRedisObject(
        "device:stream:4:intervals:check",
        IntervalsGetCheck(StreamGetIntervals(STREAM(UltradeviceGetStream4(dev)))),
        "#4 check");
    // Online measuremenr Amount
    NewRedisObject("device:stream:4:amount",
                   StreamGetAmount(STREAM(UltradeviceGetStream4(dev))),
                   "#4 amount");
    // Channels
    NewRedisObject("device:stream:4:toc",
                   UltraStreamGetTocChannel(UltradeviceGetStream4(dev)),
                   "#4 TOC");
    NewRedisObject(
        "device:stream:4:toc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream4(dev))),
        "#4 TOC limit");
    NewRedisObject("device:stream:4:toc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream4(dev))),
                   "#4 TOC check limit");

    NewRedisObject("device:stream:4:tc",
                   UltraStreamGetTCChannel(UltradeviceGetStream4(dev)), "#4 TC");
    NewRedisObject(
        "device:stream:4:tc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream4(dev))),
        "#4 TC limit");
    NewRedisObject("device:stream:4:tc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream4(dev))),
                   "#4 TC check limit");

    NewRedisObject("device:stream:4:tic",
                   UltraStreamGetTicChannel(UltradeviceGetStream4(dev)),
                   "#4 TIC");
    NewRedisObject(
        "device:stream:4:tic:limit",
        ChannelGetLimit(UltraStreamGetTicChannel(UltradeviceGetStream4(dev))),
        "#4 TIC limit");
    NewRedisObject("device:stream:4:tic:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTicChannel(UltradeviceGetStream4(dev))),
                   "#4 TIC check limit");

    NewRedisObject("device:stream:4:tnb",
                   UltraStreamGetTnbChannel(UltradeviceGetStream4(dev)),
                   "#4 TNb");
    NewRedisObject(
        "device:stream:4:tnb:limit",
        ChannelGetLimit(UltraStreamGetTnbChannel(UltradeviceGetStream4(dev))),
        "#4 TNb limit");
    NewRedisObject("device:stream:4:tnb:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTnbChannel(UltradeviceGetStream4(dev))),
                   "#4 TNb check limit");

    NewRedisObject("device:stream:4:codo",
                   UltraStreamGetCODoChannel(UltradeviceGetStream4(dev)),
                   "#4 CODo");
    NewRedisObject(
        "device:stream:4:codo:limit",
        ChannelGetLimit(UltraStreamGetCODoChannel(UltradeviceGetStream4(dev))),
        "#4 CODo limit");
    NewRedisObject("device:stream:4:codo:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetCODoChannel(UltradeviceGetStream4(dev))),
                   "#4 CODo check limit");
    // Positions parameters
    NewRedisObject("device:stream:4:position",
                   UltraStreamGetVesselsPos(UltradeviceGetStream4(dev)),
                   "#4 CODo");

    NewRedisObject("device:stream:5", UltradeviceGetStream5(dev), "#5");
    // Statistics
    NewRedisObject("device:stream:5:statistics:online",
                   StatisticsGetOnline(
                       StreamGetStatistics(STREAM(UltradeviceGetStream5(dev)))),
                   "#5 online");
    NewRedisObject("device:stream:5:statistics:single",
                   StatisticsGetSingle(
                       StreamGetStatistics(STREAM(UltradeviceGetStream5(dev)))),
                   "#5 sigle");
    NewRedisObject("device:stream:5:statistics:calibration",
                   StatisticsGetCalibration(
                       StreamGetStatistics(STREAM(UltradeviceGetStream5(dev)))),
                   "#5 calibration");
    NewRedisObject("device:stream:5:statistics:check",
                   StatisticsGetCheck(
                       StreamGetStatistics(STREAM(UltradeviceGetStream5(dev)))),
                   "#5 check");
    // Measurement intervals
    NewRedisObject("device:stream:5:intervals:online",
                   IntervalsGetOnline(
                       StreamGetIntervals(STREAM(UltradeviceGetStream5(dev)))),
                   "#5 online");
    NewRedisObject("device:stream:5:intervals:calibration",
                   IntervalsGetCalibration(
                       StreamGetIntervals(STREAM(UltradeviceGetStream5(dev)))),
                   "#5 calibration");
    NewRedisObject(
        "device:stream:5:intervals:check",
        IntervalsGetCheck(StreamGetIntervals(STREAM(UltradeviceGetStream5(dev)))),
        "#5 check");
    // Online measuremenr Amount
    NewRedisObject("device:stream:5:amount",
                   StreamGetAmount(STREAM(UltradeviceGetStream5(dev))),
                   "#5 amount");
    // Channels
    NewRedisObject("device:stream:5:toc",
                   UltraStreamGetTocChannel(UltradeviceGetStream5(dev)),
                   "#5 TOC");
    NewRedisObject(
        "device:stream:5:toc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream5(dev))),
        "#5 TOC limit");
    NewRedisObject("device:stream:5:toc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream5(dev))),
                   "#5 TOC check limit");

    NewRedisObject("device:stream:5:tc",
                   UltraStreamGetTCChannel(UltradeviceGetStream5(dev)), "#5 TC");
    NewRedisObject(
        "device:stream:5:tc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream5(dev))),
        "#5 TC limit");
    NewRedisObject("device:stream:5:tc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream5(dev))),
                   "#5 TC check limit");

    NewRedisObject("device:stream:5:tic",
                   UltraStreamGetTicChannel(UltradeviceGetStream5(dev)),
                   "#5 TIC");
    NewRedisObject(
        "device:stream:5:tic:limit",
        ChannelGetLimit(UltraStreamGetTicChannel(UltradeviceGetStream5(dev))),
        "#5 TIC limit");
    NewRedisObject("device:stream:5:tic:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTicChannel(UltradeviceGetStream5(dev))),
                   "#5 TIC check limit");

    NewRedisObject("device:stream:5:tnb",
                   UltraStreamGetTnbChannel(UltradeviceGetStream5(dev)),
                   "#5 TNb");
    NewRedisObject(
        "device:stream:5:tnb:limit",
        ChannelGetLimit(UltraStreamGetTnbChannel(UltradeviceGetStream5(dev))),
        "#5 TNb limit");
    NewRedisObject("device:stream:5:tnb:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTnbChannel(UltradeviceGetStream5(dev))),
                   "#5 TNb check limit");

    NewRedisObject("device:stream:5:codo",
                   UltraStreamGetCODoChannel(UltradeviceGetStream5(dev)),
                   "#5 CODo");
    NewRedisObject(
        "device:stream:5:codo:limit",
        ChannelGetLimit(UltraStreamGetCODoChannel(UltradeviceGetStream5(dev))),
        "#5 CODo limit");
    NewRedisObject("device:stream:5:codo:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetCODoChannel(UltradeviceGetStream5(dev))),
                   "#5 CODo check limit");
    // Positions parameters
    NewRedisObject("device:stream:5:position",
                   UltraStreamGetVesselsPos(UltradeviceGetStream5(dev)),
                   "#5 CODo");

    NewRedisObject("device:stream:6", UltradeviceGetStream6(dev), "#6");
    // Statistics
    NewRedisObject("device:stream:6:statistics:online",
                   StatisticsGetOnline(
                       StreamGetStatistics(STREAM(UltradeviceGetStream6(dev)))),
                   "#6 online");
    NewRedisObject("device:stream:6:statistics:single",
                   StatisticsGetSingle(
                       StreamGetStatistics(STREAM(UltradeviceGetStream6(dev)))),
                   "#6 sigle");
    NewRedisObject("device:stream:6:statistics:calibration",
                   StatisticsGetCalibration(
                       StreamGetStatistics(STREAM(UltradeviceGetStream6(dev)))),
                   "#6 calibration");
    NewRedisObject("device:stream:6:statistics:check",
                   StatisticsGetCheck(
                       StreamGetStatistics(STREAM(UltradeviceGetStream6(dev)))),
                   "#6 check");
    // Measurement intervals
    NewRedisObject("device:stream:6:intervals:online",
                   IntervalsGetOnline(
                       StreamGetIntervals(STREAM(UltradeviceGetStream6(dev)))),
                   "#6 online");
    NewRedisObject("device:stream:6:intervals:calibration",
                   IntervalsGetCalibration(
                       StreamGetIntervals(STREAM(UltradeviceGetStream6(dev)))),
                   "#6 calibration");
    NewRedisObject(
        "device:stream:6:intervals:check",
        IntervalsGetCheck(StreamGetIntervals(STREAM(UltradeviceGetStream6(dev)))),
        "#6 check");
    // Online measuremenr Amount
    NewRedisObject("device:stream:6:amount",
                   StreamGetAmount(STREAM(UltradeviceGetStream6(dev))),
                   "#6 amount");
    // Channels
    NewRedisObject("device:stream:6:toc",
                   UltraStreamGetTocChannel(UltradeviceGetStream6(dev)),
                   "#6 TOC");
    NewRedisObject(
        "device:stream:6:toc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream6(dev))),
        "#6 TOC limit");
    NewRedisObject("device:stream:6:toc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream6(dev))),
                   "#6 TOC check limit");

    NewRedisObject("device:stream:6:tc",
                   UltraStreamGetTCChannel(UltradeviceGetStream6(dev)), "#6 TC");
    NewRedisObject(
        "device:stream:6:tc:limit",
        ChannelGetLimit(UltraStreamGetTocChannel(UltradeviceGetStream6(dev))),
        "#6 TC limit");
    NewRedisObject("device:stream:6:tc:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTocChannel(UltradeviceGetStream6(dev))),
                   "#6 TC check limit");

    NewRedisObject("device:stream:6:tic",
                   UltraStreamGetTicChannel(UltradeviceGetStream6(dev)),
                   "#6 TIC");
    NewRedisObject(
        "device:stream:6:tic:limit",
        ChannelGetLimit(UltraStreamGetTicChannel(UltradeviceGetStream6(dev))),
        "#6 TIC limit");
    NewRedisObject("device:stream:6:tic:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTicChannel(UltradeviceGetStream6(dev))),
                   "#6 TIC check limit");

    NewRedisObject("device:stream:6:tnb",
                   UltraStreamGetTnbChannel(UltradeviceGetStream6(dev)),
                   "#6 TNb");
    NewRedisObject(
        "device:stream:6:tnb:limit",
        ChannelGetLimit(UltraStreamGetTnbChannel(UltradeviceGetStream6(dev))),
        "#6 TNb limit");
    NewRedisObject("device:stream:6:tnb:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetTnbChannel(UltradeviceGetStream6(dev))),
                   "#6 TNb check limit");

    NewRedisObject("device:stream:6:codo",
                   UltraStreamGetCODoChannel(UltradeviceGetStream6(dev)),
                   "#6 CODo");
    NewRedisObject(
        "device:stream:6:codo:limit",
        ChannelGetLimit(UltraStreamGetCODoChannel(UltradeviceGetStream6(dev))),
        "#6 CODo limit");
    NewRedisObject("device:stream:6:codo:checkLimit",
                   ChannelGetCheckLimit(
                       UltraStreamGetCODoChannel(UltradeviceGetStream6(dev))),
                   "#6 CODo check limit");
    // Positions parameters
    NewRedisObject("device:stream:6:position",
                   UltraStreamGetVesselsPos(UltradeviceGetStream6(dev)),
                   "#6 CODo");

    NewRedisObject("device:furnace", UltradeviceGetFurnace(dev), "Furnace");
    NewRedisObject("device:ticport", UltradeviceGetTicPort(dev), "Tic-Port");
    NewRedisObject("device:vessel1", UltradeviceGetV1(dev), "V1");
    NewRedisObject("device:vessel2", UltradeviceGetV2(dev), "V2");
    NewRedisObject("device:vessel3", UltradeviceGetV3(dev), "V3");
    NewRedisObject("device:vessel4", UltradeviceGetV4(dev), "V4");
    NewRedisObject("device:vessel5", UltradeviceGetV5(dev), "V5");
    NewRedisObject("device:vessel6", UltradeviceGetV6(dev), "V6");
    NewRedisObject("device:x", UltradeviceGetXAxis(dev), "X-Axis");
    NewRedisObject("device:y", UltradeviceGetYAxis(dev), "Y-Axis");
    NewRedisObject("device:injection", UltradeviceGetInjection(dev),
                   "Injecion port");
    NewRedisObject("device:dilution", UltradeviceGetDilution(dev), "Dilution");
    NewRedisObject("device:airflow", UltradeviceGetAirflow(dev), "Airflow");
    NewRedisObject("device:humidity", UltradeviceGetHumidity(dev), "Humidity");
    NewRedisObject("device:pressure", UltradeviceGetPressure(dev), "Pressure");
    NewRedisObject("device:relay1", UltradeviceGetRelay1(dev), "Relay #1");
    NewRedisObject("device:relay2", UltradeviceGetRelay2(dev), "Relay #2");
    NewRedisObject("device:relay3", UltradeviceGetRelay3(dev), "Relay #3");
    NewRedisObject("device:relay4", UltradeviceGetRelay4(dev), "Relay #4");
    NewRedisObject("device:relay5", UltradeviceGetRelay5(dev), "Relay #5");
    NewRedisObject("device:relay6", UltradeviceGetRelay6(dev), "Relay #6");
    NewRedisObject("device:relay7", UltradeviceGetRelay7(dev), "Relay #7");
    NewRedisObject("device:relay8", UltradeviceGetRelay8(dev), "Relay #8");
    NewRedisObject("device:analog:0", UltradeviceGetAnalog0(dev), "Analog #0");
    GList *la = UltradeviceGetAnalogsOut(dev);
    GList *t = NULL;
    int n = 1;
    for (t = la; t != NULL; t = t->next, n++)
    {
        gchar *path = g_strdup_printf("device:analog:%d", n);
        gchar *disc = g_strdup_printf("Analog #%d", n);
        NewRedisObject(path, t->data, disc);
        g_free(path);
        g_free(disc);
    }

    // Device integrations
    NewRedisObject("device:integrations:ndir1Tc1",
                   m_LarIntegrationsGetNDir1TC1(UltradeviceGetIntegrations(dev)),
                   "NDir1 TC1 integration");
    NewRedisObject("device:integrations:ndir1Tc2",
                   m_LarIntegrationsGetNDir1TC2(UltradeviceGetIntegrations(dev)),
                   "NDir1 TC2 integration");
    NewRedisObject("device:integrations:ndir1Tc3",
                   m_LarIntegrationsGetNDir1TC3(UltradeviceGetIntegrations(dev)),
                   "NDir1 TC3 integration");
    NewRedisObject("device:integrations:ndir1Tic1",
                   m_LarIntegrationsGetNDir1TIC1(UltradeviceGetIntegrations(dev)),
                   "NDir1 TIC1 integration");
    NewRedisObject("device:integrations:ndir1Tic2",
                   m_LarIntegrationsGetNDir1TIC2(UltradeviceGetIntegrations(dev)),
                   "NDir1 TIC2 integration");
    NewRedisObject("device:integrations:ndir1Tic3",
                   m_LarIntegrationsGetNDir1TIC3(UltradeviceGetIntegrations(dev)),
                   "NDir1 TIC3 integration");
    NewRedisObject("device:integrations:ndir2Tc1",
                   m_LarIntegrationsGetNDir2TC1(UltradeviceGetIntegrations(dev)),
                   "NDir2 TC1 integration");
    NewRedisObject("device:integrations:ndir2Tc2",
                   m_LarIntegrationsGetNDir2TC2(UltradeviceGetIntegrations(dev)),
                   "NDir2 TC2 integration");
    NewRedisObject("device:integrations:ndir2Tc3",
                   m_LarIntegrationsGetNDir2TC3(UltradeviceGetIntegrations(dev)),
                   "NDir2 TC3 integration");
    NewRedisObject("device:integrations:ndir2Tic1",
                   m_LarIntegrationsGetNDir2TIC1(UltradeviceGetIntegrations(dev)),
                   "NDir2 TIC1 integration");
    NewRedisObject("device:integrations:ndir2Tic2",
                   m_LarIntegrationsGetNDir2TIC2(UltradeviceGetIntegrations(dev)),
                   "NDir2 TIC2 integration");
    NewRedisObject("device:integrations:ndir2Tic3",
                   m_LarIntegrationsGetNDir2TIC3(UltradeviceGetIntegrations(dev)),
                   "NDir2 TIC3 integration");
    NewRedisObject("device:integrations:tnbTc",
                   m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(dev)),
                   "TNb TC integration");
    NewRedisObject("device:integrations:tnbTic",
                   m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(dev)),
                   "TNb TIC integration");
    NewRedisObject("device:integrations:codo",
                   m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(dev)),
                   "CODo integration");
    NewRedisObject("device:ndir1",	UltradeviceGetNDir1(dev),"Sensor NDir1");
    NewRedisObject("device:ndir2",	UltradeviceGetNDir2(dev),"Sensor NDir2");
    NewRedisObject("device:tnb",	UltradeviceGetTNb(dev),"Sensor TNb");
    NewRedisObject("device:codo",	UltradeviceGetCODo(dev),"Sensor CODo");
    NewRedisObject("device:temperatur",	UltradeviceGetTemperatur(dev),"Temperatur");
    device_up_run_counter(DEVICE(dev));
    return TRUE;
}

gboolean ConfigureAxis(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    AchsenAchse *xAchse =
        X_AXIS() != NULL ? achsen_object_get_achse(X_AXIS()) : NULL;
    if (xAchse == NULL)
    {
        g_warning("X axis dbus interface failed");
        return FALSE;
    }
    AchsenAchse *yAchse =
        Y_AXIS() != NULL ? achsen_object_get_achse(Y_AXIS()) : NULL;
    if (yAchse == NULL)
    {
        g_warning("Y axis dbus interface failed");
        return FALSE;
    }
    AchsenInjection *injection =
        Z_AXIS() != NULL ? achsen_object_get_injection(ACHSEN_OBJECT(Z_AXIS()))
                         : NULL;
    AchsenAchse *injAchse =
        Z_AXIS() != NULL ? achsen_object_get_achse(Z_AXIS()) : NULL;
    if (injection == NULL || injAchse == NULL)
    {
        g_warning("Injection axis dbus interface failed");
        return FALSE;
    }
    if (device_get_run_counter(DEVICE(udev)) == 1)
    {
        g_object_set(UltradeviceGetXAxis(udev), "max", achsen_achse_get_max(xAchse),
                     "hold", achsen_achse_get_hold(xAchse), "current",
                     achsen_achse_get_current(xAchse), NULL);
        g_object_set(UltradeviceGetYAxis(udev), "max", achsen_achse_get_max(yAchse),
                     "hold", achsen_achse_get_hold(yAchse), "current",
                     achsen_achse_get_current(yAchse), NULL);
        g_object_set(
            UltradeviceGetInjection(udev), "max", achsen_achse_get_max(injAchse),
            "hold", achsen_achse_get_hold(injAchse), "current",
            achsen_achse_get_current(injAchse), "air",
            achsen_injection_get_air(injection), "rest",
            achsen_injection_get_rest(injection), "furnaceAir",
            achsen_injection_get_furnace_air(injection), "dilution",
            achsen_injection_get_dilution(injection), "rinsing",
            achsen_injection_get_rinsing(injection), "push",
            achsen_injection_get_injection_stepper_parameter(injection),
            "samplePull", achsen_injection_get_sample_stepper_parameter(injection),
            "rinsingPull",
            achsen_injection_get_rinsing_up_stepper_parameter(injection),
            "rinsingPush",
            achsen_injection_get_rinsing_down_stepper_parameter(injection), NULL);
    }

    g_object_bind_property(UltradeviceGetXAxis(udev), "max", xAchse, "max",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetXAxis(udev), "hold", xAchse, "hold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetXAxis(udev), "current", xAchse,
                           "current",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property(UltradeviceGetYAxis(udev), "max", yAchse, "max",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetYAxis(udev), "hold", yAchse, "hold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetYAxis(udev), "current", yAchse,
                           "current",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property(UltradeviceGetInjection(udev), "max", injAchse, "max",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "hold", injAchse,
                           "hold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "current", injAchse,
                           "current",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "air", injection, "air",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "rest", injection,
                           "rest",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "furnaceAir", injection,
                           "furnace-air",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "dilution", injection,
                           "dilution",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "rinsing", injection,
                           "rinsing",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "push", injection,
                           "injection-stepper-parameter",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "samplePull", injection,
                           "sample-stepper-parameter",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "rinsingPull",
                           injection, "rinsing-up-stepper-parameter",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetInjection(udev), "rinsingPush",
                           injection, "rinsing-down-stepper-parameter",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    return TRUE;
}

gboolean ConfigureAnalogs(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    gboolean initialize = device_get_run_counter(DEVICE(udev)) == 1;
    GDBusObjectManager *manager = tera_client_get_manager(client, "/analogs");
    if (manager)
    {
        GList *analogs = g_dbus_object_manager_get_objects(manager);
        GList *outs = UltradeviceGetAnalogsOut(udev);
        GList *l = NULL;
        for (l = analogs; l != NULL; l = l->next)
        {
            Analog *toBind = NULL;
            AnalogsOut *out = analogs_object_get_out(ANALOGS_OBJECT(l->data));
            if (out == NULL)
                continue;
            guint number = analogs_out_get_number(out);
            if (number == 0)
            {
                toBind = UltradeviceGetAnalog0(udev);
            }
            else
            {
                toBind = g_list_nth_data(outs, number - 1);
            }
            if (toBind != NULL)
            {
                if (initialize)
                {
                    // g_debug("initialize analog %d", number);
                    g_object_set(toBind, "min", analogs_out_get_scale_min(out), "max",
                                 analogs_out_get_scale_max(out), "lifezero",
                                 analogs_out_get_life_zero(out), "boolexpr",
                                 analogs_out_get_life_zero_string(out), NULL);
                }
                g_object_bind_property(toBind, "min", out, "scale-min",
                                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
                g_object_bind_property(toBind, "max", out, "scale-max",
                                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
                g_object_bind_property(toBind, "lifezero", out, "life-zero",
                                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
                g_object_bind_property(toBind, "boolexpr", out, "life-zero-string",
                                       G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
            }
        }
        if (analogs)
            g_list_free_full(analogs, g_object_unref);
        if (outs)
            g_list_free(outs);
    }
    return TRUE;
}

gboolean ConfigureSecurity(TeraClientObject *client)
{
    SecurityDevice *guard = TERA_GUARD();
    Ultradevice *udev = ConfigureDevice();
    device_check_serial(DEVICE(udev), security_device_get_device_name(guard));
    ConfigurePC(client);
    LarpcDevice *device = mkt_pc_manager_client_get_device();
    g_object_bind_property(udev, "serial", guard, "device-name",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(device, "streams", guard, "streams",
                           G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    return TRUE;
}

gboolean ConfigureRelays(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();

    if (device_get_run_counter(DEVICE(udev)) == 1)
    {
        g_object_set(UltradeviceGetRelay1(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_1())),
                     NULL);
        g_object_set(UltradeviceGetRelay2(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_2())),
                     NULL);
        g_object_set(UltradeviceGetRelay3(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_3())),
                     NULL);
        g_object_set(UltradeviceGetRelay4(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_4())),
                     NULL);
        g_object_set(UltradeviceGetRelay5(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_5())),
                     NULL);
        g_object_set(UltradeviceGetRelay6(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_6())),
                     NULL);
        g_object_set(UltradeviceGetRelay7(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_7())),
                     NULL);
        g_object_set(UltradeviceGetRelay8(udev), "boolexpr",
                     relays_simple_get_relay_string(
                         relays_object_get_simple(TERA_RELAY_8())),
                     NULL);
    }
    g_object_bind_property(UltradeviceGetRelay1(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_1()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay2(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_2()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay3(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_3()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay4(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_4()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay5(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_5()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay6(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_6()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay7(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_7()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetRelay8(udev), "boolexpr",
                           relays_object_get_simple(TERA_RELAY_8()),
                           "relay-string",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    return TRUE;
}

gboolean ConfigureHumidity(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    GDBusObject *object = tera_client_get_object(client, "/humidity/1");
    if (object)
    {
        MonitoringSensor *sensor =
            monitoring_object_get_sensor(MONITORING_OBJECT(object));
        if (device_get_run_counter(DEVICE(udev)) == 1)
        {
            g_object_set(UltradeviceGetHumidity(udev), "soll",
                         monitoring_sensor_get_sollwert(sensor), NULL);
        }
        g_object_bind_property(UltradeviceGetHumidity(udev), "soll", sensor,
                               "sollwert",
                               G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        if (sensor)
            g_object_unref(sensor);
        return TRUE;
    }
    return FALSE;
}
gboolean ConfigurePressure(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    GDBusObject *object = tera_client_get_object(client, "/pressure/1");
    if (object)
    {
        MonitoringSensor *sensor =
            monitoring_object_get_sensor(MONITORING_OBJECT(object));
        if (device_get_run_counter(DEVICE(udev)) == 1)
        {
            g_object_set(UltradeviceGetPressure(udev), "soll",
                         monitoring_sensor_get_sollwert(sensor), NULL);
        }
        g_object_bind_property(UltradeviceGetPressure(udev), "soll", sensor,
                               "sollwert",
                               G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        if (sensor)
            g_object_unref(sensor);
        return TRUE;
    }
    return FALSE;
}

gboolean ConfigureVessels(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    // Furnace
    VesselsSimple *fsimple = vessels_object_get_simple(ULTRA_FURNACE());
    if (fsimple == NULL)
    {
        g_warning("Furnace dbus object simple interface fail");
    }
    VesselsFurnace *furnace = vessels_object_get_furnace(ULTRA_FURNACE());
    // Ticport
    VesselsSimple *tsimple = vessels_object_get_simple(ULTRA_TICPORT());
    VesselsTicport *tic = vessels_object_get_ticport(ULTRA_TICPORT());
    // Vessel1
    VesselsSimple *vessel1 = vessels_object_get_simple(ULTRA_VESSEL1());
    VesselsSimple *vessel2 = vessels_object_get_simple(ULTRA_VESSEL2());
    VesselsSimple *vessel3 = vessels_object_get_simple(ULTRA_VESSEL3());
    VesselsSimple *vessel4 = vessels_object_get_simple(ULTRA_VESSEL4());
    VesselsSimple *vessel5 = vessels_object_get_simple(ULTRA_VESSEL5());
    VesselsSimple *vessel6 = vessels_object_get_simple(ULTRA_VESSEL6());
    VesselsDilution *dilution = vessels_object_get_dilution(ULTRA_VESSEL6());

    if (device_get_run_counter(DEVICE(udev)) == 1)
    {
        g_object_set(UltradeviceGetFurnace(udev), "xpos",
                     vessels_simple_get_pos_xachse(fsimple), "ypos",
                     vessels_simple_get_injection_pos(fsimple), "y1pos",
                     vessels_furnace_get_needle_pos(furnace), NULL);

        g_object_set(UltradeviceGetTicPort(udev), "xpos",
                     vessels_simple_get_pos_xachse(tsimple), "ypos",
                     vessels_simple_get_injection_pos(tsimple), "y1pos",
                     vessels_ticport_get_needle_pos(tic), "hasmotor",
                     vessels_ticport_get_is_motor(tic), NULL);

        g_object_set(UltradeviceGetV1(udev), "xpos",
                     vessels_simple_get_pos_xachse(vessel1), "ypos",
                     vessels_simple_get_injection_pos(vessel1), NULL);

        g_object_set(UltradeviceGetV2(udev), "xpos",
                     vessels_simple_get_pos_xachse(vessel2), "ypos",
                     vessels_simple_get_injection_pos(vessel2), NULL);
        g_object_set(UltradeviceGetV3(udev), "xpos",
                     vessels_simple_get_pos_xachse(vessel3), "ypos",
                     vessels_simple_get_injection_pos(vessel3), NULL);
        g_object_set(UltradeviceGetV4(udev), "xpos",
                     vessels_simple_get_pos_xachse(vessel4), "ypos",
                     vessels_simple_get_injection_pos(vessel4), NULL);
        g_object_set(UltradeviceGetV5(udev), "xpos",
                     vessels_simple_get_pos_xachse(vessel5), "ypos",
                     vessels_simple_get_injection_pos(vessel5), NULL);
        g_object_set(UltradeviceGetV6(udev), "xpos",
                     vessels_simple_get_pos_xachse(vessel6), "ypos",
                     vessels_simple_get_injection_pos(vessel6), "y1pos",
                     vessels_dilution_get_dilution_pos(dilution), NULL);
    }
    g_object_bind_property(UltradeviceGetFurnace(udev), "xpos", fsimple,
                           "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetFurnace(udev), "ypos", fsimple,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetFurnace(udev), "y1pos", furnace,
                           "needle-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property(UltradeviceGetTicPort(udev), "xpos", tsimple,
                           "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetTicPort(udev), "ypos", tsimple,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetTicPort(udev), "y1pos", tic,
                           "needle-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetTicPort(udev), "hasmotor", tic,
                           "is-motor",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    g_object_bind_property(UltradeviceGetV1(udev), "xpos", vessel1, "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV1(udev), "ypos", vessel1,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV2(udev), "xpos", vessel2, "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV2(udev), "ypos", vessel2,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV3(udev), "xpos", vessel3, "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV3(udev), "ypos", vessel3,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV4(udev), "xpos", vessel4, "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV4(udev), "ypos", vessel4,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV5(udev), "xpos", vessel5, "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV5(udev), "ypos", vessel5,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV6(udev), "xpos", vessel6, "pos-xachse",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV6(udev), "ypos", vessel6,
                           "injection-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetV6(udev), "y1pos", dilution,
                           "dilution-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    if (fsimple)
        g_object_unref(fsimple);
    if (furnace)
        g_object_unref(furnace);
    if (tsimple)
        g_object_unref(tsimple);
    if (tic)
        g_object_unref(tic);
    if (vessel1)
        g_object_unref(vessel1);
    if (vessel2)
        g_object_unref(vessel2);
    if (vessel3)
        g_object_unref(vessel3);
    if (vessel4)
        g_object_unref(vessel4);
    if (vessel5)
        g_object_unref(vessel5);
    if (vessel6)
        g_object_unref(vessel6);
    if (dilution)
        g_object_unref(dilution);

    return TRUE;
}

gboolean ConfigureAirflow(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    AirflowSensor *sensor = airflow_object_get_sensor(ULTRA_AIRFLOW());
    if (sensor == NULL)
    {
        g_warning("Airflow dbus object sensor interface fail");
    }
    if (device_get_run_counter(DEVICE(udev)) == 1)
    {
        g_object_set(UltradeviceGetAirflow(udev), "critical",
                     airflow_sensor_get_critical_error(sensor), "soll",
                     airflow_sensor_get_soll_value(sensor), "correlation",
                     airflow_sensor_get_correction(sensor), "deviation",
                     airflow_sensor_get_deviation(sensor), "adjustment",
                     airflow_sensor_get_adjustment_factor(sensor), "threshold",
                     airflow_sensor_get_injection_error_threshold(sensor), NULL);
    }
    g_object_bind_property(UltradeviceGetAirflow(udev), "critical", sensor,
                           "critical-error",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetAirflow(udev), "soll", sensor,
                           "soll-value",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetAirflow(udev), "correlation", sensor,
                           "correction",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetAirflow(udev), "deviation", sensor,
                           "deviation",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // Es wird immer auf 20.0 gesetzt und dadurch kommt die Meldung als ob ein
    // Parameter geandert wurde.
    g_object_bind_property(UltradeviceGetAirflow(udev), "adjustment", sensor,
                           "adjustment-factor",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetAirflow(udev), "threshold", sensor,
                           "injection-error-threshold",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_unref(sensor);
    return TRUE;
}

gboolean ConfigureSensors(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    /* Sensor object property .. 
    *  Sensor type g_object_class_install_property(object_class, PROP_MODEL, g_param_spec_uint("model", "model", "Sensor model number", 0, 11, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    *  State - es gibt gerade erst nur eine nutzliche State >0 wa bedeutet dass die Parameter mindestens ein mal geladen wurden.
    *     g_object_class_install_property(object_class, PROP_STATE, g_param_spec_uint("state", "state", "Sensor state", 0, 5, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    * 
    *  Range1 
    *    min : g_object_class_install_property(object_class, PROP_MIN1, g_param_spec_double("min-one", "Min", "min scale limit range one", -G_MAXDOUBLE, G_MAXDOUBLE, 0.0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    *    max : g_object_class_install_property(object_class, PROP_MAX1, g_param_spec_double("max-one", "max", "min scale limit range", -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    * 
    *  Range2 : nur NDir1,NDir2
    *    min : g_object_class_install_property(object_class, PROP_MIN2, g_param_spec_double("min-two", "Min", "min scale limit range one", -G_MAXDOUBLE, G_MAXDOUBLE, 0.0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    *    max : g_object_class_install_property(object_class, PROP_MAX2, g_param_spec_double("max-two", "max", "min scale limit range", -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    * 
    *  Range3 : nurt NDir1 , NDir2 
    *    min : g_object_class_install_property(object_class, PROP_MIN3, g_param_spec_double("min-three", "Min", "min scale limit range", -G_MAXDOUBLE, G_MAXDOUBLE, 0.0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    *    max : g_object_class_install_property(object_class, PROP_MAX3, g_param_spec_double("max-three", "max", "min scale limit range", -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    */
    Sensor *ndir1= UltradeviceGetNDir1(udev);
    if (!SensorGetState(ndir1)){
        //Hier erst die Parameter aus sensor componente lesen.
        g_object_set(ndir1,"state",1,NULL);
    }
//  ndir1 bind property G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE
    Sensor *ndir2= UltradeviceGetNDir2(udev);
    if (!SensorGetState(ndir2)){
        //Hier erst die Parameter aus sensor componente lesen.
        g_object_set(ndir2,"state",1,NULL);
    }
//  ndir2 bind property  G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE 
    Sensor *tnb= UltradeviceGetTNb(udev);
    if (!SensorGetState(tnb)){
        //Hier erst die Parameter aus sensor componente lesen.
        g_object_set(tnb,"state",1,NULL);
    }
//   tnb bind propery G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE 
    Sensor *codo= UltradeviceGetCODo(udev);
    if (!SensorGetState(codo)){

        g_object_set(codo,"state",1,NULL);
    }
//  codo bind property G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE 

    return TRUE;
} 

gboolean ConfigureSequence(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    SequenceWorkersDilution *dilution =
        sequence_object_get_workers_dilution(ULTRA_DILUTION_SEQUENCE_WORKER());

    if (device_get_run_counter(DEVICE(udev)) == 1)
    {
        g_object_set(UltradeviceGetDilution(udev), "volume",
                     sequence_workers_dilution_get_volume(dilution), "takePos",
                     sequence_workers_dilution_get_take_pos(dilution), "pushPos",
                     sequence_workers_dilution_get_push_pos(dilution), "repeat",
                     sequence_workers_dilution_get_repeat(dilution), NULL);
    }
    g_object_bind_property(UltradeviceGetDilution(udev), "volume", dilution,
                           "volume",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetDilution(udev), "takePos", dilution,
                           "take-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetDilution(udev), "pushPos", dilution,
                           "push-pos",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(UltradeviceGetDilution(udev), "repeat", dilution,
                           "repeat",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    return TRUE;
}

gboolean ConfigurePC(TeraClientObject *client)
{
    Ultradevice *udev = ConfigureDevice();
    LarpcDevice *device = mkt_pc_manager_client_get_device();
    g_object_bind_property(udev, "serial", device, "serial",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    return TRUE;
}

gboolean ConfigureIntegrations(GDBusObjectManager *manager)
{
    // Ultradevice *      udev        = ConfigureDevice();
    // gboolean           initial     = device_get_run_counter(DEVICE(udev)) == 1;
    // IntegrationObject *integration = ultra_integration_get_ndir1();
    // if (integration) {
    //     IntegrationTc *tc = integration_object_get_tc(integration);
    //     if (tc) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tc_get_start_threshold(tc),
    //             "stopThreshold",
    //                          integration_tc_get_stop_threshold(tc), "startMin",
    //                          integration_tc_get_start_min_time(tc), "stopMax",
    //                          integration_tc_get_stop_max_time(tc), "stopMin",
    //                          integration_tc_get_stop_min_time(tc), NULL);
    //         }
    //        g_object_bind_property(m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)),
    //        "startThreshold", tc, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //        G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tc, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)),
    //         "startMin", tc, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tc, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tc, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE); g_object_unref(tc);
    //     }
    //     IntegrationTic *tic = integration_object_get_tic(integration);
    //     if (tic) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tic_get_start_threshold(tic),
    //             "stopThreshold",
    //                          integration_tic_get_stop_threshold(tic),
    //                          "startMin",
    //                          integration_tic_get_start_min_time(tic),
    //                          "stopMax", integration_tic_get_stop_max_time(tic),
    //                          "stopMin", integration_tic_get_stop_min_time(tic),
    //                          NULL);
    //         }
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)),
    //         "startThreshold", tic, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tic, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)),
    //         "startMin", tic, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tic, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tic, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE); g_object_unref(tic);
    //     }
    // }

    // integration = ultra_integration_get_ndir2();
    // if (integration) {
    //     IntegrationTc *tc = integration_object_get_tc(integration);
    //     if (tc) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tc_get_start_threshold(tc),
    //             "stopThreshold",
    //                          integration_tc_get_stop_threshold(tc), "startMin",
    //                          integration_tc_get_start_min_time(tc), "stopMax",
    //                          integration_tc_get_stop_max_time(tc), "stopMin",
    //                          integration_tc_get_stop_min_time(tc), NULL);
    //         }
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)),
    //         "startThreshold", tc, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tc, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)),
    //         "startMin", tc, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tc, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tc, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);

    //         g_object_unref(tc);
    //     }
    //     IntegrationTic *tic = integration_object_get_tic(integration);
    //     if (tic) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tic_get_start_threshold(tic),
    //             "stopThreshold",
    //                          integration_tic_get_stop_threshold(tic),
    //                          "startMin",
    //                          integration_tic_get_start_min_time(tic),
    //                          "stopMax", integration_tic_get_stop_max_time(tic),
    //                          "stopMin", integration_tic_get_stop_min_time(tic),
    //                          NULL);
    //         }
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)),
    //         "startThreshold", tic, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tic, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)),
    //         "startMin", tic, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tic, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tic, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE); g_object_unref(tic);
    //     }
    // }
    // integration = ultra_integration_get_tnb();
    // if (integration) {
    //     IntegrationTc *tc = integration_object_get_tc(integration);
    //     if (tc) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tc_get_start_threshold(tc),
    //             "stopThreshold",
    //                          integration_tc_get_stop_threshold(tc), "startMin",
    //                          integration_tc_get_start_min_time(tc), "stopMax",
    //                          integration_tc_get_stop_max_time(tc), "stopMin",
    //                          integration_tc_get_stop_min_time(tc), NULL);
    //         }
    //         g_object_bind_property(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)),
    //         "startThreshold", tc, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tc, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)),
    //         "startMin", tc, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tc, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tc, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE); g_object_unref(tc);
    //     }
    //     IntegrationTic *tic = integration_object_get_tic(integration);
    //     if (tic) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tic_get_start_threshold(tic),
    //             "stopThreshold",
    //                          integration_tic_get_stop_threshold(tic),
    //                          "startMin",
    //                          integration_tic_get_start_min_time(tic),
    //                          "stopMax", integration_tic_get_stop_max_time(tic),
    //                          "stopMin", integration_tic_get_stop_min_time(tic),
    //                          NULL);
    //         }
    //         g_object_bind_property(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)),
    //         "startThreshold", tic, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tic, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)),
    //         "startMin", tic, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tic, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tic, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE); g_object_unref(tic);
    //     }
    // }
    // integration = ultra_integration_get_codo();
    // if (integration) {
    //     IntegrationTc *tc = integration_object_get_tc(integration);
    //     if (tc) {
    //         if (initial) {
    //             g_object_set(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)),
    //             "startThreshold", integration_tc_get_start_threshold(tc),
    //             "stopThreshold",
    //                          integration_tc_get_stop_threshold(tc), "startMin",
    //                          integration_tc_get_start_min_time(tc), "stopMax",
    //                          integration_tc_get_stop_max_time(tc), "stopMin",
    //                          integration_tc_get_stop_min_time(tc), NULL);
    //         }

    //         g_object_bind_property(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)),
    //         "startThreshold", tc, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)),
    //         "stopThreshold", tc, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)),
    //         "startMin", tc, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)),
    //         "stopMax", tc, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)),
    //         "stopMin", tc, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE); g_object_unref(tc);
    //     }
    // }
    // ultra_bus_sensor_set_tc_integration(UltraDBusSensorNDIR1(),m_LarIntegrationsGetNDir1TC(UltradeviceGetIntegrations(udev)));
    // ultra_bus_sensor_set_tic_integration(UltraDBusSensorNDIR1(),m_LarIntegrationsGetNDir1TIC(UltradeviceGetIntegrations(udev)));

    // tc = integration_object_get_tc(INTEGRATION_OBJECT(UltraDBusSensorNDIR2()));
    // tic =
    // integration_object_get_tic(INTEGRATION_OBJECT(UltraDBusSensorNDIR2()));

    // g_object_unref(tic);
    // ultra_bus_sensor_set_tc_integration(UltraDBusSensorNDIR2(),m_LarIntegrationsGetNDir2TC(UltradeviceGetIntegrations(udev)));
    // ultra_bus_sensor_set_tic_integration(UltraDBusSensorNDIR2(),m_LarIntegrationsGetNDir2TIC(UltradeviceGetIntegrations(udev)));

    // tc = integration_object_get_tc(INTEGRATION_OBJECT(UltraDBusSensorCODO()));
    // // IntegrationTic *tic =
    // integration_object_get_tic(INTEGRATION_OBJECT(UltraDBusSensorNDIR2()));

    // ultra_bus_sensor_set_tc_integration(UltraDBusSensorCODO(),m_LarIntegrationsGetCODo(UltradeviceGetIntegrations(udev)));
    // g_object_unref(tc);
    // // g_object_unref(tic);
    // tc = integration_object_get_tc(INTEGRATION_OBJECT(UltraDBusSensorTNB()));
    // tic = integration_object_get_tic(INTEGRATION_OBJECT(UltraDBusSensorTNB()));

    // // TIC
    // g_object_unref(tc);
    // g_object_unref(tic);
    // ultra_bus_sensor_set_tc_integration(UltraDBusSensorTNB(),m_LarIntegrationsGetTNbTC(UltradeviceGetIntegrations(udev)));
    // ultra_bus_sensor_set_tic_integration(UltraDBusSensorTNB(),m_LarIntegrationsGetTNbTIC(UltradeviceGetIntegrations(udev)));

    return TRUE;
}

// void bindChannelParameter(Channel *channel, UltraBusChannel *busChannel)
// {
//     ChannelsCalibration *cal =
//     channels_object_get_calibration(CHANNELS_OBJECT(busChannel));
//     ChannelsCheck *check =
//     channels_object_get_check(CHANNELS_OBJECT(busChannel)); ChannelsSimple
//     *simple = channels_object_get_simple(CHANNELS_OBJECT(busChannel));
//     g_object_bind_property(channel, "sensor", simple, "sensor",
//     G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "name", simple, "name",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "unit", simple, "unit",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "min", simple, "min",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "max", simple, "max",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "activated", simple, "activated",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "check", simple, "is-check",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "calibration", cal, "activated",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "analog", simple, "analog-out",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "factor", simple, "factor",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     g_object_bind_property(channel, "checkAnalog", check, "analog-out",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE); Limit *limit =
//     ChannelGetLimit(channel); Limit *chLimit = ChannelGetCheckLimit(channel);
//     if (limit)
//     {
//         g_object_bind_property(limit, "activated", simple, "limit-activated",
//         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//         g_object_bind_property(limit, "min", simple, "limit-min",
//         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//         g_object_bind_property(limit, "max", simple, "limit-max",
//         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     }
//     if (chLimit)
//     {
//         g_object_bind_property(chLimit, "activated", check,
//         "limit-activated", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//         g_object_bind_property(chLimit, "min", check, "limit-min",
//         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//         g_object_bind_property(chLimit, "max", check, "limit-max",
//         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     }
//     if (cal)
//         g_object_unref(cal);
//     if (check)
//         g_object_unref(check);
//     if (simple)
//         g_object_unref(simple);
// }


gboolean ConfigureTemperatur(MonitoringTemperatur *temp_observer){

    g_return_val_if_fail(temp_observer!=NULL,FALSE);
    Ultradevice *udev = ConfigureDevice();
    Temperatur *temp = UltradeviceGetTemperatur(udev);
    g_object_bind_property(temp, "check-furnace",temp_observer,
                           "check-furnace",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(temp, "check-housing",temp_observer,
                           "check-housing",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(temp, "furnace-soll",temp_observer,
                           "furnace-max",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    g_object_bind_property(temp, "housing-soll",temp_observer,
                           "housing-min",
                           G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    return TRUE;
}

gboolean ConfigureStreams(GDBusObjectManager *manager)
{
    g_return_val_if_fail(manager !=NULL,FALSE);
    g_return_val_if_fail(G_IS_DBUS_OBJECT_MANAGER(manager),FALSE);
    Ultradevice *udev = ConfigureDevice();
    GList *streams = g_dbus_object_manager_get_objects(manager);
    GList *si = NULL;
    for (si = streams; si != NULL; si = si->next)
    {
        StreamsSimple *simple = streams_object_get_simple(STREAMS_OBJECT(si->data));
        if (simple)
        {
            g_object_bind_property(
                UltradeviceGetStreamNumber(udev, streams_simple_get_number(simple)),
                "name", simple, "name",
                G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        }
        //     guint number = streams_simple_get_number(simple);

        //     UltraStream *stream = UltradeviceGetStreamNumber(udev, number);
        //     if (stream)
        //     {
        //         // StreamsOnline *online =
        //         streams_object_get_online(STREAMS_OBJECT(si->data));
        //         // if (online)
        //         // {
        //         //
        //         g_object_bind_property(StatisticsGetOnline(StreamGetStatistics(STREAM(stream))),
        //         "replicates", online, "replicates", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetOnline(StreamGetStatistics(STREAM(stream))),
        //         "outliers", online, "outliers", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetOnline(StreamGetStatistics(STREAM(stream))),
        //         "cv", online, "max-cv", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetOnline(StreamGetStatistics(STREAM(stream))),
        //         "jump", online, "jump", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //     g_object_bind_property(StreamGetAmount(STREAM(stream)),
        //         "counter", online, "amount-counter", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //     g_object_bind_property(StreamGetAmount(STREAM(stream)),
        //         "percentage", online, "amount-percentage",
        //         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(IntervalsGetOnline(StreamGetIntervals(STREAM(stream))),
        //         "interval", online, "interval", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //     g_object_bind_property(stream, "remote", online,
        //         "remote-control", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(IntervalsGetCheck(StreamGetIntervals(STREAM(stream))),
        //         "interval", online, "interval-check", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(IntervalsGetCheck(StreamGetIntervals(STREAM(stream))),
        //         "activated", online, "autocheck", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //     g_object_unref(online);
        //         // }
        //         // StreamsSingle *single =
        //         streams_object_get_single(STREAMS_OBJECT(si->data));
        //         // if (single)
        //         // {
        //         //
        //         g_object_bind_property(StatisticsGetSingle(StreamGetStatistics(STREAM(stream))),
        //         "replicates", single, "replicates", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetSingle(StreamGetStatistics(STREAM(stream))),
        //         "outliers", single, "outliers", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetSingle(StreamGetStatistics(STREAM(stream))),
        //         "cv", single, "max-cv", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //     g_object_unref(single);
        //         // }
        //         // StreamsCalibration *calibration =
        //         streams_object_get_calibration(STREAMS_OBJECT(si->data));
        //         // if (calibration)
        //         // {
        //         //
        //         g_object_bind_property(StatisticsGetCalibration(StreamGetStatistics(STREAM(stream))),
        //         "replicates", calibration, "replicates", G_BINDING_BIDIRECTIONAL
        //         | G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetCalibration(StreamGetStatistics(STREAM(stream))),
        //         "outliers", calibration, "outliers", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(StatisticsGetCalibration(StreamGetStatistics(STREAM(stream))),
        //         "cv", calibration, "max-cv", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(IntervalsGetCalibration(StreamGetIntervals(STREAM(stream))),
        //         "interval", calibration, "interval", G_BINDING_BIDIRECTIONAL |
        //         G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(IntervalsGetCalibration(StreamGetIntervals(STREAM(stream))),
        //         "activated", calibration, "autocalibration",
        //         G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //         //
        //         g_object_bind_property(stream,"autocalDeviation",calibration,"autocal-deviation",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
        //         // }
        //         StreamsUltra *ultra =
        //         streams_object_get_ultra(STREAMS_OBJECT(si->data)); if (ultra)
        //         {
        //             g_object_bind_property(stream, "volume", ultra,
        //             "sample-volume", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "injectionVolume", ultra, "injection-volume",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "injectionVolumeTic", ultra,
        //             "injection-volume-tic", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "fillingTime", ultra, "sample-filling-time",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "delay", ultra, "delay",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "delayTic", ultra,
        //             "delay-tic", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "preRinsing", ultra, "is-pre-rinsing",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "rinsingCount", ultra,
        //             "rinsing-count", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "afterRinsing", ultra, "is-after-rinsing",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "afterCount", ultra,
        //             "after-rinsing-count", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "codoInjection", ultra, "codo-injection",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "needStripping", ultra,
        //             "need-stripping", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "strippingTime", ultra, "stripping-time",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "dilutionType", ultra,
        //             "dilution-type", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "dilutionFactor", ultra, "dilution-factor",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "dilutionPumpTime", ultra,
        //             "dilution-pump-time", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "dilutionWaitTime", ultra, "dilution-wait-time",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "amountDeviation", ultra,
        //             "allowed-deviation", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE);

        //             g_object_bind_property(stream, "prinsingON", ultra,
        //             "process-rinsing", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "prinsingY1", ultra, "rinsing-pos-y1",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "prinsingY2", ultra,
        //             "rinsing-pos-y2", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "injVol", ultra, "rinsing-volume", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "waitAfter", ultra, "rinsing-wait-after",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(stream, "injRep", ultra,
        //             "rinsing-replicate", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(UltraStreamGetVesselsPos(stream),
        //             "online", ultra, "online-vessel", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(UltraStreamGetVesselsPos(stream),
        //             "drain", ultra, "drain-vessel", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(UltraStreamGetVesselsPos(stream),
        //             "calibration", ultra, "check-vessel", G_BINDING_BIDIRECTIONAL
        //             | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(UltraStreamGetVesselsPos(stream),
        //             "single", ultra, "calibration-vessel",
        //             G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
        //             g_object_bind_property(UltraStreamGetVesselsPos(stream),
        //             "check", ultra, "single-vessel", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE); g_object_bind_property(stream,
        //             "isDilution", ultra, "is-dilution", G_BINDING_BIDIRECTIONAL |
        //             G_BINDING_SYNC_CREATE);
        //         }

        //         // UltraBusChannel *channel =
        //         m_UltraBusStreamChannelTC(ULTRA_BUS_STREAM(si->data));
        //         // Channel *ch = UltraStreamGetTCChannel(stream);
        //         // if (channel != NULL && ch != NULL)
        //         // {
        //         //     bindChannelParameter(ch, channel);
        //         // }
        //         // channel =
        //         m_UltraBusStreamChannelTIC(ULTRA_BUS_STREAM(si->data));
        //         // ch = UltraStreamGetTicChannel(stream);
        //         // if (channel != NULL && ch != NULL)
        //         // {
        //         //     bindChannelParameter(ch, channel);
        //         // }
        //         // channel =
        //         m_UltraBusStreamChannelTOC(ULTRA_BUS_STREAM(si->data));
        //         // ch = UltraStreamGetTocChannel(stream);
        //         // if (channel != NULL && ch != NULL)
        //         // {
        //         //     bindChannelParameter(ch, channel);
        //         // }
        //         // channel =
        //         m_UltraBusStreamChannelCODo(ULTRA_BUS_STREAM(si->data));
        //         // ch = UltraStreamGetCODoChannel(stream);
        //         // if (channel != NULL && ch != NULL)
        //         // {
        //         //     bindChannelParameter(ch, channel);
        //         // }
        //         // channel =
        //         m_UltraBusStreamChannelTNb(ULTRA_BUS_STREAM(si->data));
        //         // ch = UltraStreamGetTnbChannel(stream);
        //         // if (channel != NULL && ch != NULL)
        //         // {
        //         //     bindChannelParameter(ch, channel);
        //         // }
        //     }
    }
    if (streams)
        g_list_free_full(streams, g_object_unref);

    return TRUE;
}

static void row_channel_plot_reload_plot_secect_callback(GSList *models,
                                                         gpointer user_data)
{
    g_return_if_fail(models != NULL);
}
void ultra_test_data()
{
    mkt_model_look(
        MKT_TYPE_MEASUREMENT_DATA, g_cancellable_new(),
        row_channel_plot_reload_plot_secect_callback, NULL,
        "select * from %s where measurement_channel = %" G_GUINT64_FORMAT
        " and measurement_changed > %f and measurement_changed < %f and "
        "measurement_identification = %d and measurement_type = 1 ORDER BY "
        "measurement_changed ASC;",
        g_type_name(MKT_TYPE_MEASUREMENT_DATA), (guint64)7, 0.0,
        market_db_time_now(), 202);
}

static gboolean checkLicense(const gchar *component, const gchar *device_id,
                             const gchar *mac_address)
{
    FILE *licfile;
    // ugly, but there is no way to parameterize this inside fscanf() anyway
    char read_component[32];
    char read_device_id[32];
    char read_hash[129];
    char seed[9];
    const gchar *home_dir = g_get_home_dir();
    // construct filename
    gchar *filepath =
        g_strconcat(home_dir, "/lic", component, "-", device_id, ".lic", NULL);
    if (!(licfile = fopen(filepath, "r")))
    {
        g_warning("unable to open license file: %s", filepath);
        g_free(filepath);
        return FALSE;
    }
    g_free(filepath);

    if ((4 != fscanf(licfile,
                     "LAR license #%8s for Analyser s/n %32s for %32s\n%128s",
                     seed, read_device_id, read_component, read_hash)))
    {
        fclose(licfile);
        g_warning("atom-pc: license: ERROR: wrong file format");
        return FALSE;
    }
    fclose(licfile);

    if (strcmp(read_component, component))
    {
        g_warning("atom-pc: license: ERROR: wrong component inside file");
        return FALSE;
    }

    if (strcmp(read_device_id, device_id))
    {
        g_warning("atom-pc: license: ERROR: wrong device id inside file");
        return FALSE;
    }

    // compute hash
    gchar *plaintext = g_strconcat(component, " ", device_id, " ", seed, " ",
                                   mac_address, "\n", NULL);
    gchar *hash = g_compute_checksum_for_string(G_CHECKSUM_SHA512, plaintext,
                                                strlen(plaintext));
    g_free(plaintext);

    if (strcmp(read_hash, hash))
    {
        g_free(hash);
        g_warning("atom-pc: license: ERROR: hashes don't match");
        return FALSE;
    }
    g_free(hash);

    return TRUE;
}

guint ConfigureCheckLicense()
{
    Ultradevice *dev = ConfigureDevice();
    const gchar max_streams = 6;
    if (ConfigureIsTestMode())
        return max_streams;
    const gchar *streams[] = {"DUMMY1st", "2st", "3st", "4st", "5st", "6st"};

    // actually go down from the maximum possible streamCount
    guint streamCount = max_streams;
    // check validity of current streamCount and if it is not found, decrement
    // streamCount as long as it is above 2
    LarpcDevice *device = mkt_pc_manager_client_get_device();
    while (!checkLicense(streams[streamCount - 1], device_get_serial(DEVICE(dev)),
                     larpc_device_get_mac(device)    ) &&
           streamCount-- > 2)
        ;
    return streamCount;
}
void ConfigureLogger()
{
    Ultradevice *dev = ConfigureDevice();
    device_parameter_change_protocol(dev);
}
// stream->priv->stream_model = mkt_model_select_one(MKT_TYPE_STREAM_MODEL,
// "select * from $tablename where param_object_path = '%s'",
// g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)));

//     if (stream->priv->stream_model == NULL) {
//         gchar *      pname         = g_strdup_printf("(#0%d)",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         const gchar *name          =
//         streams_tc_get_name(streams_object_get_aimple(STREAMS_OBJECT(stream)));
//         stream->priv->stream_model = mkt_model_new(MKT_TYPE_STREAM_MODEL,
//         "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "stream-name",
//         name, "param-name", pname, NULL); g_free(pname);
//     }
//     g_object_bind_property(stream->priv->stream_model, "stream-name",
//     streams_object_get_simple(STREAMS_OBJECT(stream)), "name",
//     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
//     streams_tc_set_id(streams_object_get_aimple(STREAMS_OBJECT(stream)),
//     mkt_model_ref_id(MKT_IMODEL(stream->priv->stream_model)));

//     stream->priv->online_category =
//         mkt_model_select_one(MKT_TYPE_CATEGORY_MODEL, "select * from
//         $tablename where param_object_path = '%s' and param_type = '%s'",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "online");
//     if (stream->priv->online_category == NULL) {
//       //REVIEW: check parameter name
//         gchar *pname                  = g_strdup_printf("(#0%d) online
//         interval",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         stream->priv->online_category =
//         mkt_model_new(MKT_TYPE_CATEGORY_MODEL, "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type",
//         "online", "category-interval",
//             720.0, "category-online", TRUE, "param-name", pname, NULL);
//         g_free(pname);
//     }

//     stream->priv->cal_category = mkt_model_select_one(
//         MKT_TYPE_CATEGORY_MODEL, "select * from $tablename where
//         param_object_path = '%s' and param_type = '%s'",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "calibration");
//     if (stream->priv->cal_category == NULL) {
//       //REVIEW: check parameter name
//         gchar *pname               = g_strdup_printf("(#0%d) auto calibration
//         interval",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         stream->priv->cal_category = mkt_model_new(MKT_TYPE_CATEGORY_MODEL,
//         "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type",
//         "calibration", "category-interval",
//             43200.0, "category-online", FALSE, "param-name", pname, NULL);
//         g_free(pname);
//     }

//     stream->priv->check_category =
//         mkt_model_select_one(MKT_TYPE_CATEGORY_MODEL, "select * from
//         $tablename where param_object_path = '%s' and param_type = '%s'",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "check");
//     if (stream->priv->check_category == NULL) {
//       //REVIEW: check parameter name
//         gchar *pname                 = g_strdup_printf("(#0%d) check
//         interval",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         stream->priv->check_category = mkt_model_new(MKT_TYPE_CATEGORY_MODEL,
//         "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type",
//         "check", "category-interval",
//             0.0, "category-online", FALSE, "param-name", pname, NULL);
//         g_free(pname);
//     }

//     stream->priv->online_statistic =
//         mkt_model_select_one(MKT_TYPE_STATISTIC_MODEL, "select * from
//         $tablename where param_object_path = '%s' and param_type = '%s'",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "online");
//     if (stream->priv->online_statistic == NULL) {
//       //REVIEW: check parameter name
//         gchar *pname = g_strdup_printf("(#0%d) online multiple
//         determination",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         stream->priv->online_statistic =
//             mkt_model_new(MKT_TYPE_STATISTIC_MODEL, "param-object-path",
//             g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)),
//             "param-type", "online", "param-name", pname, NULL);
//         g_free(pname);
//     }

//     stream->priv->single_statistic =
//         mkt_model_select_one(MKT_TYPE_STATISTIC_MODEL, "select * from
//         $tablename where param_object_path = '%s' and param_type = '%s'",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "single");
//     if (stream->priv->single_statistic == NULL) {
//       //REVIEW: check parameter "Stream %d single multiple determination"

//         gchar *pname = g_strdup_printf("(#0%d) single multiple
//         determination",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         stream->priv->single_statistic =
//             mkt_model_new(MKT_TYPE_STATISTIC_MODEL, "param-object-path",
//             g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)),
//             "param-type", "single", "param-name", pname, NULL);
//         g_free(pname);
//     }

//     stream->priv->cal_statistic = mkt_model_select_one(
//         MKT_TYPE_STATISTIC_MODEL, "select * from $tablename where
//         param_object_path = '%s' and param_type = '%s'",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "calibration");
//     if (stream->priv->cal_statistic == NULL) {
//       //REVIEW: check parameter "Stream %d calibration multiple
//       determination"
//         gchar *pname                = g_strdup_printf("(#0%d) calibration
//         multiple determination",
//         streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//         stream->priv->cal_statistic = mkt_model_new(MKT_TYPE_STATISTIC_MODEL,
//         "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type",
//         "calibration", "param-name",
//             pname, "statistic-replicates", 5, "statistic-outliers", 2,
//             "statistic-max-cv", 2.0, NULL);
//         g_free(pname);
//     }
// // if (stream->priv->measparam != NULL)
// g_object_unref(stream->priv->measparam); stream->priv->measparam =
// mkt_model_select_one(
//     ULTIMATE_TYPE_MESSPARM_OBJECT, "select * from $tablename where
//     param_object_path = '%s' and param_type = '%s'",
//     g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "main");
// if (stream->priv->measparam == NULL) {
//     gchar *name = g_strdup_printf("(#0%d) measurement",
//     streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));
//     stream->priv->measparam =
//         mkt_model_new(ULTIMATE_TYPE_MESSPARM_OBJECT, "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type",
//         "main", "param-name", name, NULL);
//     g_free(name);
// }

// g_object_bind_property(stream->priv->measparam, "ultimate-sample-volume",
// ultra, "sample-volume", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam,
// "ultimate-sample-filling-time", ultra, "sample-filling-time",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-injection-volume",
// ultra, "injection-volume", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam,
// "ultimate-injection-volume-tic", ultra, "injection-volume-tic",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-delay", ultra,
// "delay", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-delay-tic", ultra,
// "delay-tic", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-is-pre-rinsing",
// ultra, "is-pre-rinsing", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-rinsing-count",
// ultra, "rinsing-count", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-is-after-rinsing",
// ultra, "is-after-rinsing", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam,
// "ultimate-after-rinsing-count", ultra, "after-rinsing-count",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-codo-injection",
// ultra, "codo-injection", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-need-stripping",
// ultra, "need-stripping", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-stripping-time",
// ultra, "stripping-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-dilution-type",
// ultra, "dilution-type", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-dilution-factor",
// ultra, "dilution-factor", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam,
// "ultimate-dilution-pump-time", ultra, "dilution-pump-time",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam,
// "ultimate-dilution-wait-time", ultra, "dilution-wait-time",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-allowed-deviation",
// ultra, "allowed-deviation", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "ultimate-autocal-deviation",
// ultra, "autocal-deviation", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

// g_object_bind_property(stream->priv->measparam, "process-rinsing", ultra,
// "process-rinsing", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "prinsing-y1-pos", ultra,
// "rinsing-pos-y1", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "prinsing-injection-volume",
// ultra, "rinsing-volume", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam,
// "prinsing-injection-replicate", ultra, "rinsing-replicate",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "prinsing-wait-time", ultra,
// "rinsing-wait-inj", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "prinsing-y2-pos", ultra,
// "rinsing-pos-y2", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->measparam, "prinsing-wait-after", ultra,
// "rinsing-wait-after", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

// stream->priv->is_dilution = mkt_paramboolean_get(tera_service_id(),
// g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "is-dilution"); if
// (stream->priv->is_dilution == NULL) {
//     stream->priv->is_dilution =
//     MKT_PARAMBOOLEAN(mkt_model_new(MKT_TYPE_PARAMBOOLEAN_MODEL,
//     "param-object-id", tera_service_id(), "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-name",
//         "is-dilution", "param-activated", TRUE, "value", FALSE, NULL));
// }
// g_object_bind_property(stream->priv->is_dilution, "value", ultra,
// "is-dilution", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

// stream->priv->on_replicate = mkt_paramboolean_get(tera_service_id(),
// g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "on-replicate"); if
// (stream->priv->on_replicate == NULL) {
//     stream->priv->on_replicate =
//     MKT_PARAMBOOLEAN(mkt_model_new(MKT_TYPE_PARAMBOOLEAN_MODEL,
//     "param-object-id", tera_service_id(), "param-object-path",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-name",
//         "on-replicate", "param-activated", TRUE, "value", FALSE, NULL));
// }
// if (stream->priv->posparam != NULL) g_object_unref(stream->priv->posparam);
// stream->priv->posparam = mkt_model_select_one(
//     ULTIMATE_TYPE_POSPARM_OBJECT, "select * from $tablename where
//     param_object_path = '%s' and param_type = '%s'",
//     g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "main");

// if (stream->priv->posparam == NULL) {
//     gchar *pname = g_strdup_printf("(#0%d) positions",
//     streams_tc_get_number(streams_object_get_aimple(STREAMS_OBJECT(stream))));

//     stream->priv->posparam = mkt_model_new(ULTIMATE_TYPE_POSPARM_OBJECT,
//     "param-object-path",
//     g_dbus_object_get_object_path(G_DBUS_OBJECT(stream)), "param-type",
//     "main", "param-name", pname,
//         "calibration-vessel",
//         g_dbus_object_get_object_path(G_DBUS_OBJECT(ULTRA_VESSEL1())), NULL);
//     g_free(pname);
// }
// g_object_bind_property(stream->priv->posparam, "online-vessel", positions,
// "online-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->posparam, "single-vessel", positions,
// "single-vessel", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(stream->priv->posparam, "calibration-vessel",
// positions, "calibration-vessel", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(stream->priv->posparam,
// "check-vessel", positions, "check-vessel", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(stream->priv->posparam,
// "drain-vessel", positions, "drain-vessel", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(stream->priv->posparam,
// "dilution-vessel", positions, "dilution-vessel", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE);

// ---------------> CHANNEL

// if (channel->priv->channel) g_object_unref(channel->priv->channel);
// channel->priv->channel =
// MKT_CHANNEL(mkt_model_select_one(MKT_TYPE_CHANNEL_MODEL, "select * from
// $tablename where param_object_path='%s' and channel_type='main'",
// object_path)); if (channel->priv->channel == NULL) {
//     guint64 stream_id      =
//     streams_tc_get_id(streams_object_get_aimple(STREAMS_OBJECT(channel->priv->stream)));
//     gchar * name           = g_strdup_printf("Channel %d measurement",
//     channels_tc_get_number(ULTRA_CHANNEL_SIMPLE(channel))); gchar *
//     analog_path    = g_strdup_printf("/analogs/%d", channel->priv->number);
//     gchar * channel_name   = g_strdup_printf("CH%d", channel->priv->number);
//     channel->priv->channel =
//     MKT_CHANNEL(mkt_model_new(MKT_TYPE_CHANNEL_MODEL, "param-object-path",
//     object_path, "param-name", name, "channel-stream", stream_id,
//     "channel-type", "main",
//         "channel-analog-out", analog_path, "channel-max", 100.0,
//         "channel-name", channel_name, NULL));
//     mkt_model_delete_async(MKT_TYPE_SENSOR_DATA, NULL, NULL, NULL, "delete
//     from MktSensorData where data_creator=%" G_GUINT64_FORMAT,
//     mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
//     mkt_model_delete_async(
//         MKT_TYPE_MEASUREMENT_DATA, NULL, NULL, NULL, "delete from
//         MktMeasurementData where measurement_channel=%" G_GUINT64_FORMAT,
//         mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
//     g_free(name);
//     g_free(analog_path);
//     g_free(channel_name);
// }
// mkt_param_activate(MKT_PARAM(channel->priv->channel));

// //
// g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"stream",channel->priv->channel,"channel-stream",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
// //
// g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"sensor",channel->priv->channel,"channel-sensor",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

// g_object_bind_property(channel->priv->channel, "channel-result",
// ULTRA_CHANNEL_SIMPLE(channel), "result", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-changed", ULTRA_CHANNEL_SIMPLE(channel), "last-changed",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(channel->priv->channel, "channel-sensor",
// ULTRA_CHANNEL_SIMPLE(channel), "sensor", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-min", ULTRA_CHANNEL_SIMPLE(channel), "min", G_BINDING_BIDIRECTIONAL
// | G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-max", ULTRA_CHANNEL_SIMPLE(channel), "max", G_BINDING_BIDIRECTIONAL
// | G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-factor", ULTRA_CHANNEL_SIMPLE(channel), "factor",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(channel->priv->channel, "channel-activated",
// ULTRA_CHANNEL_SIMPLE(channel), "activated", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-check", ULTRA_CHANNEL_SIMPLE(channel), "is-check",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(channel->priv->channel, "channel-measurement",
// ULTRA_CHANNEL_SIMPLE(channel), "measurement", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-trigger", ULTRA_CHANNEL_SIMPLE(channel), "trigger",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(channel->priv->channel, "channel-analog-out",
// ULTRA_CHANNEL_SIMPLE(channel), "analog-out", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-name", ULTRA_CHANNEL_SIMPLE(channel), "name",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(channel->priv->channel, "channel-unit",
// ULTRA_CHANNEL_SIMPLE(channel), "unit", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-raw", ULTRA_CHANNEL_MEASUREMENT(channel), "last-round",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
// g_object_bind_property(channel->priv->channel, "channel-cv",
// ULTRA_CHANNEL_MEASUREMENT(channel), "last-cv", G_BINDING_BIDIRECTIONAL |
// G_BINDING_SYNC_CREATE);

// g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel), "is-allow",
// channel->priv->channel, "channel-allow", G_BINDING_DEFAULT |
// G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
// "channel-activated-cal",
// channels_object_get_calibration(CHANNELS_OBJECT(channel)), "activated",
// G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

gboolean ConvertParameterV52toV53()
{
    // Ultradevice *device = ConfigureDevice();
    RedisBinding *notFirstRun =
        NewRedisKey("notFirstStartV53", "0", "Check is it not first run");
    if (RedisValueGetInt(notFirstRun, NULL) == 1)
    {
        RedisValueSet(notFirstRun, "1");
    }
    g_object_unref(notFirstRun);

    // --------------> ANALOGS

    // if (analog_object->priv->analog)
    // g_object_unref(analog_object->priv->analog); analog_object->priv->analog =
    //     MKT_ANALOG(mkt_model_select_one(MKT_TYPE_ANALOG_MODEL, "select * from
    //     $tablename where param_object_path = '%s'",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(object))));
    // if (analog_object->priv->analog == NULL) {
    //     gchar *name                 = g_strdup_printf(_("Analog out %d"),
    //     analog_object->priv->analog_number); analog_object->priv->analog =
    //     MKT_ANALOG(mkt_model_new(MKT_TYPE_ANALOG_MODEL, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(analog_object)),
    //     "param-name", name, NULL));

    //     if (analog_object->priv->analog_type == ANALOG_NODE_TYPE_ANALOGEXT)
    //     g_object_set(analog_object->priv->analog, "analog-max", 32767.0, NULL);
    // }

    // -------------->RELAY
    // if (relay_boolexpr->priv->relay)
    // g_object_unref(relay_boolexpr->priv->relay); relay_boolexpr->priv->relay =
    //     MKT_RELAY(mkt_model_select_one(MKT_TYPE_RELAY_MODEL, "select * from
    //     $tablename where param_object_path = '%s'",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(relay_boolexpr))));
    // if (relay_boolexpr->priv->relay == NULL) {
    //     gchar *name                 = g_strdup_printf("Relay %d",
    //     relay_boolexpr->priv->number); relay_boolexpr->priv->relay =
    //     MKT_RELAY(mkt_model_new(MKT_TYPE_RELAY_MODEL, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(relay_boolexpr)),
    //     "param-name", name, NULL)); g_free(name);
    //     if(relay_boolexpr->priv->number==
    //     1)g_object_set(relay_boolexpr->priv->relay,"relay-string","WARN",NULL);
    //     else if(relay_boolexpr->priv->number==
    //     2)g_object_set(relay_boolexpr->priv->relay,"relay-string","FAIL",NULL);
    // }

    // ----------------> HUMIDITY
    // priv->humidity_settings     = g_settings_new("com.lar.sensor.humidity");
    // g_settings_bind(humidity->priv->humidity_settings, "soll-value", sensor,
    // "sollwert", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(humidity->priv->humidity_settings, "activated", sensor,
    // "activated", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(humidity->priv->humidity_settings, "critical", sensor,
    // "critical", G_SETTINGS_BIND_DEFAULT);

    // ---------------> PRESSURE
    // priv->pressure_settings     = g_settings_new("com.lar.sensor.pressure");
    // g_settings_bind(pressure->priv->pressure_settings, "soll-value", sensor,
    // "sollwert", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(pressure->priv->pressure_settings, "deviation-value",
    // sensor, "deviation", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(pressure->priv->pressure_settings, "activated", sensor,
    // "activated", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(pressure->priv->pressure_settings, "critical", sensor,
    // "critical", G_SETTINGS_BIND_DEFAULT);

    // -------------->AXIS
    // OLD : mus noch konvertiert werden .

    // MktAxis *axis_data =
    //     MKT_AXIS(mkt_model_select_one(MKT_TYPE_AXIS_MODEL, "select * from
    //     $tablename where param_object_path = '%s' and param_object_id = '%s'",
    //     ULTRA_AXIS_X_PATH, tera_service_id()));
    // if (axis_data == NULL) {
    //     axis_data = MKT_AXIS(mkt_model_new(
    //         MKT_TYPE_AXIS_MODEL, "param-object-id", tera_service_id(),
    //         "param-object-path", ULTRA_AXIS_X_PATH, "axis-hold", 45,
    //         "axis-max", 2500, "axis-current", 1200, "axis-reverse", FALSE,
    //         NULL));
    // }

    // X&Y Axes
    // if (axis_object->priv->axis_data) {
    //     g_object_bind_property(axis_object->priv->axis_data, "max",
    //     achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), "max",
    //     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    //     g_object_bind_property(axis_object->priv->axis_data, "hold",
    //     achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), "hold",
    //     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    //     g_object_bind_property(axis_object->priv->axis_data, "current",
    //     achsen_object_get_achse(ACHSEN_OBJECT(axis_object)), "current",
    //     G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // }
    // Create Y Axis .
    //  axis_data =
    // (mkt_model_select_one(MKT_TYPE_AXIS_MODEL, "select * from $tablename where
    // param_object_path = '%s' and param_object_id = '%s'", ULTRA_AXIS_Y_PATH,
    //  tera_service_id())); if (axis_data == NULL) {
    //      axis_data = MKT_AXIS(mkt_model_new(
    //          MKT_TYPE_AXIS_MODEL, "param-object-id", tera_service_id(),
    //          "param-object-path", ULTRA_AXIS_Y_PATH, "axis-hold", 80,
    //          "axis-max", 2000, "axis-current", 1200, "axis-reverse", FALSE,
    //          NULL));
    //  }

    // Create Z Axis .
    // axis_data = MKT_AXIS(mkt_model_select_one(MKT_TYPE_AXIS_MODEL, "select *
    // from $tablename where param_object_path = '%s' and param_object_id = '%s'",
    // ULTRA_AXIS_Z_PATH, tera_service_id())); if (axis_data == NULL) {
    //     axis_data = MKT_AXIS(mkt_model_new(
    //         MKT_TYPE_AXIS_MODEL, "param-object-id", tera_service_id(),
    //         "param-object-path", ULTRA_AXIS_Z_PATH, "axis-hold", 80,
    //         "axis-max", 1800, "axis-current", 1200, "axis-reverse", FALSE,
    //         NULL));
    // }
    // if (axis_object->priv->injection_data)
    // g_object_unref(axis_object->priv->injection_data);
    // axis_object->priv->injection_data =
    //     MKT_INJECTION(mkt_model_select_one(MKT_TYPE_INJECTION_MODEL, "select *
    //     from $tablename where param_object_path = '%s'",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(axis_object))));
    // if (axis_object->priv->injection_data == NULL) {
    //     axis_object->priv->injection_data =
    //     MKT_INJECTION(mkt_model_new(MKT_TYPE_INJECTION_MODEL,
    //     "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(axis_object)), NULL));
    // }s

    // --------------> FURNACE

    // if (ultra_vessel_object->priv->vessel_data)
    // g_object_unref(ultra_vessel_object->priv->vessel_data);
    // ultra_vessel_object->priv->vessel_data =
    //     MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, "select * from
    //     $tablename where param_object_path = '%s'",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object))));
    // if (ultra_vessel_object->priv->vessel_data == NULL) {
    //     ultra_vessel_object->priv->vessel_data =
    //     MKT_VESSEL(mkt_model_new(MKT_TYPE_VESSEL_MODEL, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object)),
    //     "param-name",
    //         "Furnace", "vessel-x-pos", 120, "vessel-y-pos", 1700,
    //         "vessel-y1-pos", 720, NULL));
    //     // REVIEW: check parameter name "Furnace"
    // }

    // --------------> TIC-Port

    // if (ultra_vessel_object->priv->vessel_data)
    // g_object_unref(ultra_vessel_object->priv->vessel_data);
    // ultra_vessel_object->priv->vessel_data =
    //     MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, "select * from
    //     $tablename where param_object_path = '%s'",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object))));
    // if (ultra_vessel_object->priv->vessel_data == NULL) {
    //     ultra_vessel_object->priv->vessel_data =
    //     MKT_VESSEL(mkt_model_new(MKT_TYPE_VESSEL_MODEL, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object)),
    //     "param-name",
    //         "TIC-Port", "vessel-x-pos", 832, "vessel-y-pos", 1200,
    //         "vessel-y1-pos", 450, "vessel-has-motor", FALSE, NULL));
    //     // REVIEW: check parameter name "TIC-Port"
    // }

    // -----------------------> V1-V5

    // if(ultra_vessel_object->priv->vessel_data)g_object_unref(ultra_vessel_object->priv->vessel_data);
    // ultra_vessel_object->priv->vessel_data =
    // MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, 		                                                  "select * from
    // $tablename where param_object_path = '%s'",
    // 		                                                   g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object))));
    // if(ultra_vessel_object->priv->vessel_data == NULL )
    // {
    // 	//REVIEW: check PN "Vessel %d"
    // 	gchar *name = g_strdup_printf("Vessel
    // %d",vessels_tc_get_number(vessels_object_get_simple(ULTRA_TICPORT())));
    // 	ultra_vessel_object->priv->vessel_data =
    // MKT_VESSEL(mkt_model_new(MKT_TYPE_VESSEL_MODEL,
    // 												          "param-object-path",g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_vessel_object)),
    // 														  "param-name",name,
    // 														  "vessel-x-pos",ultra_vessel_object->priv->default_pos,
    // 														  "vessel-y-pos",1250,
    // 														  NULL));
    // 	g_free(name);
    // }

    // --------------------> V6

    // if (ultra_dilution_object->priv->vessel_data)
    // g_object_unref(ultra_dilution_object->priv->vessel_data);
    // ultra_dilution_object->priv->vessel_data =
    //     MKT_VESSEL(mkt_model_select_one(MKT_TYPE_VESSEL_MODEL, "select * from
    //     $tablename where param_object_path = '%s'",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_dilution_object))));
    // if (ultra_dilution_object->priv->vessel_data == NULL) {
    //     // REVIEW: check parameter name  "Vessel %d"
    //     gchar *name                              = g_strdup_printf("Vessel %d",
    //     vessels_tc_get_number(vessels_object_get_simple(ULTRA_VESSEL6())));
    //     ultra_dilution_object->priv->vessel_data =
    //     MKT_VESSEL(mkt_model_new(MKT_TYPE_VESSEL_MODEL, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(ultra_dilution_object)),
    //         "param-name", name, "vessel-x-pos",
    //         ultra_dilution_object->priv->default_pos, "vessel-y-pos", 1250,
    //         "vessel-y1-pos", 1420, NULL));
    //     g_free(name);
    // }

    // ------------------->Dilution

    // MktParamdouble *param = NULL;
    // param                 = mkt_paramdouble_get(ULTRA_SEQUENCE_WORKERS_NAME,
    // g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "volume"); if (param
    // == NULL) {
    //     param = MKT_PARAMDOUBLE(mkt_model_new(MKT_TYPE_PARAMDOUBLE_MODEL,
    //     "param-object-id", ULTRA_SEQUENCE_WORKERS_NAME, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),
    //         "param-name", "volume", "param-activated", TRUE, "value", 1700.0,
    //         NULL));
    // }
    // MktParamint32 *int32 = NULL;
    // int32                = mkt_paramint32_get(ULTRA_SEQUENCE_WORKERS_NAME,
    // g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "take-pos"); if
    // (int32 == NULL) {
    //     int32 = MKT_PARAMINT32(mkt_model_new(MKT_TYPE_PARAMINT32_MODEL,
    //     "param-object-id", ULTRA_SEQUENCE_WORKERS_NAME, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),
    //         "param-name", "take-pos", "param-activated", TRUE, "value", 1420,
    //         NULL));
    // }
    // g_object_bind_property(int32, "value", dilution, "take-pos",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    //  int32 = mkt_paramint32_get(ULTRA_SEQUENCE_WORKERS_NAME,
    //  g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "push-pos");
    // if (int32 == NULL) {
    //     int32 = MKT_PARAMINT32(mkt_model_new(MKT_TYPE_PARAMINT32_MODEL,
    //     "param-object-id", ULTRA_SEQUENCE_WORKERS_NAME, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),
    //         "param-name", "push-pos", "param-activated", TRUE, "value", 1020,
    //         NULL));
    // }
    // g_object_bind_property(int32, "value", dilution, "push-pos",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE); int32 =
    // mkt_paramint32_get(ULTRA_SEQUENCE_WORKERS_NAME,
    // g_dbus_object_get_object_path(G_DBUS_OBJECT(object)), "repeat"); if (int32
    // == NULL) {
    //     int32 = MKT_PARAMINT32(mkt_model_new(MKT_TYPE_PARAMINT32_MODEL,
    //     "param-object-id", ULTRA_SEQUENCE_WORKERS_NAME, "param-object-path",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(object)),
    //         "param-name", "repeat", "param-activated", TRUE, "value", 3,
    //         NULL));
    // }
    // g_object_bind_property(int32, "value", dilution, "repeat",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    // --------------------> STREAM

    // ---------------> CHANNEL

    // if (channel->priv->channel) g_object_unref(channel->priv->channel);
    // channel->priv->channel =
    // MKT_CHANNEL(mkt_model_select_one(MKT_TYPE_CHANNEL_MODEL, "select * from
    // $tablename where param_object_path='%s' and channel_type='main'",
    // object_path)); if (channel->priv->channel == NULL) {
    //     guint64 stream_id      =
    //     streams_tc_get_id(streams_object_get_aimple(STREAMS_OBJECT(channel->priv->stream)));
    //     gchar * name           = g_strdup_printf("Channel %d measurement",
    //     channels_tc_get_number(ULTRA_CHANNEL_SIMPLE(channel))); gchar *
    //     analog_path    = g_strdup_printf("/analogs/%d", channel->priv->number);
    //     gchar * channel_name   = g_strdup_printf("CH%d",
    //     channel->priv->number); channel->priv->channel =
    //     MKT_CHANNEL(mkt_model_new(MKT_TYPE_CHANNEL_MODEL, "param-object-path",
    //     object_path, "param-name", name, "channel-stream", stream_id,
    //     "channel-type", "main",
    //         "channel-analog-out", analog_path, "channel-max", 100.0,
    //         "channel-name", channel_name, NULL));
    //     mkt_model_delete_async(MKT_TYPE_SENSOR_DATA, NULL, NULL, NULL, "delete
    //     from MktSensorData where data_creator=%" G_GUINT64_FORMAT,
    //     mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
    //     mkt_model_delete_async(
    //         MKT_TYPE_MEASUREMENT_DATA, NULL, NULL, NULL, "delete from
    //         MktMeasurementData where measurement_channel=%" G_GUINT64_FORMAT,
    //         mkt_model_ref_id(MKT_IMODEL(channel->priv->channel)));
    //     g_free(name);
    //     g_free(analog_path);
    //     g_free(channel_name);
    // }
    // mkt_param_activate(MKT_PARAM(channel->priv->channel));

    // //
    // g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"stream",channel->priv->channel,"channel-stream",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    // //
    // g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel),"sensor",channel->priv->channel,"channel-sensor",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

    // g_object_bind_property(channel->priv->channel, "channel-result",
    // ULTRA_CHANNEL_SIMPLE(channel), "result", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-changed", ULTRA_CHANNEL_SIMPLE(channel), "last-changed",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-sensor",
    // ULTRA_CHANNEL_SIMPLE(channel), "sensor", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-min", ULTRA_CHANNEL_SIMPLE(channel), "min",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-max",
    // ULTRA_CHANNEL_SIMPLE(channel), "max", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-factor", ULTRA_CHANNEL_SIMPLE(channel), "factor",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-activated",
    // ULTRA_CHANNEL_SIMPLE(channel), "activated", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-check", ULTRA_CHANNEL_SIMPLE(channel), "is-check",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-measurement",
    // ULTRA_CHANNEL_SIMPLE(channel), "measurement", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-trigger", ULTRA_CHANNEL_SIMPLE(channel), "trigger",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-analog-out",
    // ULTRA_CHANNEL_SIMPLE(channel), "analog-out", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-name", ULTRA_CHANNEL_SIMPLE(channel), "name",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-unit",
    // ULTRA_CHANNEL_SIMPLE(channel), "unit", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-raw", ULTRA_CHANNEL_MEASUREMENT(channel), "last-round",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    // g_object_bind_property(channel->priv->channel, "channel-cv",
    // ULTRA_CHANNEL_MEASUREMENT(channel), "last-cv", G_BINDING_BIDIRECTIONAL |
    // G_BINDING_SYNC_CREATE);

    // g_object_bind_property(ULTRA_CHANNEL_SIMPLE(channel), "is-allow",
    // channel->priv->channel, "channel-allow", G_BINDING_DEFAULT |
    // G_BINDING_SYNC_CREATE); g_object_bind_property(channel->priv->channel,
    // "channel-activated-cal",
    // channels_object_get_calibration(CHANNELS_OBJECT(channel)), "activated",
    // G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    // -------------------> INTEGRATION

    // static void ultra_integration_connect_model(UltraIntegrationObject
    // *integration) {
    //     gchar *        patch = NULL;
    //     IntegrationTc *tc    =
    //     integration_object_get_tc(INTEGRATION_OBJECT(integration)); if
    //     (integration->priv->integration)
    //     g_object_unref(integration->priv->integration); patch =
    //     g_strdup_printf("%s.TC",
    //     g_dbus_object_get_object_path(G_DBUS_OBJECT(integration)));
    //     integration->priv->integration =
    //         ULTIMATE_INTEGRATION(mkt_model_select_one(ULTIMATE_TYPE_INTEGRATION_OBJECT,
    //         "select * from $tablename where param_object_path = '%s' and
    //         param_type = 'TC'", patch));
    //     if (integration->priv->integration == NULL) {
    //         // REVIEW: parameter name
    //         gchar *name                    = g_strdup_printf("Integration
    //         TC-%s",
    //         ultra_integration_signal_kind_full_name(integration->priv->sensor));
    //         integration->priv->integration =
    //         ULTIMATE_INTEGRATION(mkt_model_new(ULTIMATE_TYPE_INTEGRATION_OBJECT,
    //         "param-object-path", patch, "param-type", "TC", "param-name", name,
    //         "start-threshold",
    //             0.002, "stop-threshold", 0.003, "start-min-time", 1.0,
    //             "stop-min-time", 80.0, "stop-max-time", 120.0, NULL));
    //         g_free(name);
    //     }
    //     g_free(patch);
    //     g_object_bind_property(integration->priv->integration,
    //     "start-threshold", tc, "start-threshold", G_BINDING_BIDIRECTIONAL |
    //     G_BINDING_SYNC_CREATE);
    //     g_object_bind_property(integration->priv->integration,
    //     "stop-threshold", tc, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //     G_BINDING_SYNC_CREATE);
    //     g_object_bind_property(integration->priv->integration,
    //     "start-min-time", tc, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //     G_BINDING_SYNC_CREATE);
    //     g_object_bind_property(integration->priv->integration, "stop-min-time",
    //     tc, "stop-min-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    //     g_object_bind_property(integration->priv->integration, "stop-max-time",
    //     tc, "stop-max-time", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    //     IntegrationTic *tic =
    //     integration_object_get_tic(INTEGRATION_OBJECT(integration)); if (tic) {
    //         if (integration->priv->integrationTIC)
    //         g_object_unref(integration->priv->integrationTIC); patch =
    //         g_strdup_printf("%s.TIC",
    //         g_dbus_object_get_object_path(G_DBUS_OBJECT(integration)));
    //         integration->priv->integrationTIC =
    //             ULTIMATE_INTEGRATION(mkt_model_select_one(ULTIMATE_TYPE_INTEGRATION_OBJECT,
    //             "select * from $tablename where param_object_path = '%s' and
    //             param_type = 'TIC'", patch));
    //         if (integration->priv->integrationTIC == NULL) {
    //             // REVIEW: parameter name
    //             gchar *name = g_strdup_printf("Integration TIC-%s",
    //             ultra_integration_signal_kind_full_name(integration->priv->sensor));
    //             integration->priv->integrationTIC =
    //             ULTIMATE_INTEGRATION(mkt_model_new(ULTIMATE_TYPE_INTEGRATION_OBJECT,
    //             "param-object-path", patch, "param-type", "TIC", "param-name",
    //             name,
    //                 "start-threshold", 0.002, "stop-threshold", 0.003,
    //                 "start-min-time", 1.0, "stop-min-time", 80.0,
    //                 "stop-max-time", 120.0, NULL));
    //             g_free(name);
    //         }
    //         g_free(patch);
    //         g_object_bind_property(integration->priv->integrationTIC,
    //         "start-threshold", tic, "start-threshold", G_BINDING_BIDIRECTIONAL
    //         | G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(integration->priv->integrationTIC,
    //         "stop-threshold", tic, "stop-threshold", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(integration->priv->integrationTIC,
    //         "start-min-time", tic, "start-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(integration->priv->integrationTIC,
    //         "stop-min-time", tic, "stop-min-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //         g_object_bind_property(integration->priv->integrationTIC,
    //         "stop-max-time", tic, "stop-max-time", G_BINDING_BIDIRECTIONAL |
    //         G_BINDING_SYNC_CREATE);
    //     }
    // }

    // ------------------> Airflow

    // priv->airflow_settings = g_settings_new("com.lar.tera.airflow");
    // g_signal_connect(priv->airflow_settings, "changed",
    // G_CALLBACK(ultra_airflow_settings_changed), ultra_airflow_object);
    // g_settings_bind(airflow->priv->airflow_settings, "critical-error", sensor,
    // "critical-error", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(airflow->priv->airflow_settings, "soll-value", sensor,
    // "soll-value", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(airflow->priv->airflow_settings, "correction-value",
    // sensor, "correction", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(airflow->priv->airflow_settings, "deviation-value", sensor,
    // "deviation", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(airflow->priv->airflow_settings, "adjustment-factor",
    // sensor, "adjustment-factor", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(airflow->priv->airflow_settings, "inj-analyse-time",
    // sensor, "inj-analyse-timeout", G_SETTINGS_BIND_DEFAULT);
    // g_settings_bind(airflow->priv->airflow_settings,
    // "injection-error-threshold", sensor, "injection-error-threshold",
    // G_SETTINGS_BIND_DEFAULT); static void
    // ultra_airflow_settings_changed(GSettings *settings, gchar *key, gpointer
    // user_data) {
    //             GSettingsSchema *   schema;
    //             GSettingsSchemaKey *skey;
    //             g_object_get(settings, "settings-schema", &schema, NULL);
    //             skey = g_settings_schema_get_key(schema, key);
    //             if (0 == g_strcmp0("soll-value", key)) {
    //                     GVariant *value = g_settings_get_value(settings, key);
    //                     gdouble val   = g_variant_get_double(value);
    //                     g_settings_schema_key_unref(skey);
    //                     g_variant_unref(value);
    //             }
    //             g_settings_schema_unref(schema);
    //     }

    // -------------------------> DILUTION
    // RedisBindProperty(NewRedisHash("dilution","volume","1700","Dilution
    // volume"),dilution,"volume",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
    // RedisBindProperty(NewRedisHash("dilution","takePos","1420","Dilution take
    // position"),dilution,"take-pos",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
    // RedisBindProperty(NewRedisHash("dilution","pushPos","1020","Dilution push
    // position"),dilution,"push-pos",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
    // RedisBindProperty(NewRedisHash("dilution","repeat","3","Dilution
    // repeat"),dilution,"repeat",G_BINDING_BIDIRECTIONAL|G_BINDING_SYNC_CREATE);
    return TRUE;
}
