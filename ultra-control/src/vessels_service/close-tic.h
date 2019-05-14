

/**
 * @defgroup Ultra
 * @defgroup UltraTicportObject
 * @ingroup  UltraTicportObject
 * @{
 * @file  ultra-furnace-object.h	Furnace object header
 * @brief        Furnace object header file.
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author        A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _CLOSE_FURNACE_H_
#define _CLOSE_FURNACE_H_

#include "ultra-ticport-object.h"
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

void close_ticport_operation(UltraTicportObject *ticport, GCancellable *cancellable, GDBusMethodInvocation *invocation);

#endif /* _CLOSE_FURNACE_H_ */

/** @} */
