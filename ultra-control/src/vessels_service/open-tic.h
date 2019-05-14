

/**
 * @defgroup Ultra
 * @defgroup UltraFurnaceObject
 * @ingroup  UltraFurnaceObject
 * @{
 * @file  ultra-furnace-object.h	Furnace object header
 * @brief        Furnace object header file.
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author        A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _OPEN_FURNACE_H_
#define _OPEN_FURNACE_H_

#include "ultra-ticport-object.h"
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

void open_tic_operation(UltraTicportObject *furnace, GCancellable *cancellable, GDBusMethodInvocation *invocation);

#endif /* _OPEN_FURNACE_H_ */

/** @} */
