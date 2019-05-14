/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraerrors.h
 * Copyright (C) LAR 2017
 *

 */

#ifndef _ULTRA_ERRORS_H_
#define _ULTRA_ERRORS_H_

#include <glib.h>




/**
 * EUltraErrors:
 * @ULTRA_ERROR_EMPTY_STREAM: Stream is not found
 *
 * Enum with the available views.
 */

typedef enum /*< flags,prefix=Lar >*/
{
    UERROR_EMPTY_STREAM ,   /*<nick=Stream is not defined>*/
    UERROR_NO_STREAM_LIC,   /*<nick=Stream license not fount>*/
    UERROR_NO_CHANNELS ,    /*<nick=there are no activated channels>*/
    UERROR_NO_SAMPLE_PUMP , /*<nick=Sample pump not found>*/
    UERROR_WORKER_NOT_FOUND , /*<nick=Worker not found>*/
    UERROR_ANALYZE_TIMEOUT ,  /*<nick=Analyze timeout>*/
    UERROR_NO_SENSOR ,        /*<nick=Integration sensor not found>*/
    UERROR_BELOW_RANGE ,      /*<nick=Min range below >*/
    UERROR_EXEEDED_RANGE,      /*<nick=Max range exeeded>*/
    UERROR_HWCT_MISSING,     /*<nick=Hardware content missing>*/
    UERROR_ANL1_MISSING,     /*<nick=Analog 1 missing>*/
    UERROR_DMN1_MISSING,     /*<nick=Doppel Motornode 1 missing>*/
    UERROR_DMN2_MISSING,     /*<nick=Doppel Motornode 2 missing>*/
    UERROR_DIG1_MISSING,     /*<nick=Digital 1 missing>*/
    UERROR_DIG2_MISSING,     /*<nick=Digital 2 missing>*/
    UERROR_DIG3_MISSING,     /*<nick=Digital 3 missing>*/
    UERROR_AERN_MISSING,     /*<nick=Analogerweiterung 1 missing>*/
} UErrors;




GQuark UltraErrorsQuark(void);


#endif /* _ULTRA_ERRORS_H_ */
