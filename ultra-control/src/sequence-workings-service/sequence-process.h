/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup UltimateLibrary
 * @defgroup UltimateProcessObject
 * @ingroup  UltimateProcessObject
 * @{
 * @file  ultimate-process-object.h	PROCESS object header
 * @brief This is PROCESS object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef __ULTIMATE_PROCESS_H_
#define __ULTIMATE_PROCESS_H_

#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>


G_BEGIN_DECLS

void                             sequence_workers_process_change_status                   ( SequenceWorkersProcess *process  ,const gchar *format,...)G_GNUC_PRINTF (2, 3);
void                             sequence_workers_process_change_status_error             ( SequenceWorkersProcess *process  ,const gchar *format,...)G_GNUC_PRINTF (2, 3);

G_END_DECLS

#endif /* __ULTIMATE_PROCESS_H_ */
/** @} */
