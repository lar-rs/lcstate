/*
 * can_open - can4linux CAN driver module
 *
 * can4linux -- LINUX CAN device driver source
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 * Copyright (c) 2001 port GmbH Halle/Saale
 * Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013 Heinz-Jürgen Oertel (oe@port.de)
 *------------------------------------------------------------------
 */


/**
* \file open.c
* \author Heinz-Jürgen Oertel
*
*/

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


/* header of standard C - libraries */

/* header of common types */

/* shared common header */

/* header of project specific types */

/* project headers */
#include "defs.h"

/* local header */

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/
static  DEFINE_SPINLOCK(waitflag_lock);

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/
/* device minor already opened */
/* For can_isopen[minor] we use an atomic variable !!  page 85 Quade */
atomic_t can_isopen[MAX_CHANNELS];

/* local defined variables
---------------------------------------------------------------------------*/


/***************************************************************************/
/**
*
* \brief int open(const char *pathname, int flags);
* opens the CAN device for following operations
* \param pathname device pathname, usual /dev/can?
* \param flags is one of \c O_RDONLY, \c O_WRONLY or \c O_RDWR which request
*       opening  the  file  read-only,  write-only  or read/write,
*       respectively.
*
*
* The open call is used to "open" the device.
* Doing a first initialization according the to values in the /proc/sys/Can
* file system.
* Additional an ISR function is assigned to the IRQ.
*
* The CLK OUT pin is configured for creating the same frequency
* like the chips input frequency fclk (XTAL).
*
* If Vendor Option \a VendOpt is set to 's' the driver performs
* an hardware reset before initializing the chip.
*
* If compiled with CAN_MAX_OPEN > 1, open() can be called more than once
* by different processes.
*
* \returns
* open return the new file descriptor,
* or -1 if an error occurred (in which case, errno is set appropriately).
*
* \par ERRORS
* the following errors can occur
* \arg \c ENXIO  the file is a device special file
* and no corresponding device exists.
* \arg \c EINVAL illegal \b minor device number
* \arg \c EINVAL wrong IO-model format in /proc/sys/Can/IOmodel
* \arg \c EBUSY  IRQ for hardware is not available
* \arg \c EBUSY  I/O region for hardware is not available

*/

int can_open(struct inode *inode, struct file *file)
{
int retval;
struct _instance_data *iptr;		/* pointer to private data */
unsigned int minor = iminor(inode);

	DBGIN();
	retval = 0;

	if (minor >= MAX_CHANNELS) {
		pr_err("CAN: Illegal minor number %d\n", minor);
		DBGOUT();
		return -EINVAL;
	}
	/* Base address only makes sense in traditional parallel buses
	 * would should we put in here for SPI ?
	 */
	if (proc_base[minor] == 0x00 && virtual == 0) {
		/* No device available */
		pr_err("CAN[%d]: no device available\n", minor);
		DBGOUT();
		return -ENXIO;
	}

	if (CAN_MAX_OPEN == atomic_read(&can_isopen[minor])) {
		/* number of allowed open processes exceeded */
		pr_err("CAN[%d]: max number %d of open() calls reached\n",
			minor, CAN_MAX_OPEN);
		DBGOUT();
		return -ENXIO;
	}


	/* do things that have do be done the first time open() is called */
	if (0 == atomic_read(&can_isopen[minor])) {
		/* first time called, initialize hardware and global data */

		if (virtual == 0) {
			/* the following does all the board specific things
			   also memory remapping if necessary
			   and the CAN ISR is assigned */
			retval = can_vendor_init(minor);
			if (retval < 0) {
			    	pr_err("Error in can_vendor_init()\n");
				DBGOUT();
				return retval;
			}
	    }
	    /* Access macros based in proc_base[] should work now */
	    /* can_show_stat(minor); */

	    /* TX FIFO needs only to be initialized the first time
	     * CAN is opened */
	    can_tx_fifo_init(minor);
	}

	/* Starting here the IRQ is already requested.
	 * If something goes wrong now in the open call
	 * it must be freed.
	 */

	/* per open() call */
	iptr = (struct _instance_data *)
		kmalloc(sizeof(struct _instance_data), GFP_KERNEL);
	if (iptr == NULL) {
		pr_err("not enough kernel memory\n");
		retval = -ENOMEM;
		goto release_irq_and_leave;
	}
	iptr->su = FALSE;

	{
	/* look for free queue , RX_FIFO
	 * if two or more processes open at the same time
	 * we can have a race condition.
	 * Therefore this code is secured by a spin lock
	 */
	int i;
		spin_lock(&waitflag_lock);
		for (i = 0; i < CAN_MAX_OPEN; i++) {
			if (can_waitflag[minor][i] == 0)
				break;
			}
			spin_unlock(&waitflag_lock);

		/* iptr->rx_index     = atomic_read(&can_isopen[minor]); */
			iptr->rx_index     = i;
	}

	selfreception[minor][iptr->rx_index] = 0;
	file->private_data = (void *)iptr;

	/* initialize and mark used rx buffer in flags field */
	can_rx_fifo_init(minor, iptr->rx_index);

#if defined(CAN_USE_FILTER)
	can_filter_init(minor, iptr->rx_index);
#endif

	/* Last step
	 * if first time CAN is opened, reset the CAN controller
	 * and start it
	 * and mark it in the private data to be the process
	 * allowed to call some functions like Stop/start
	 * set change baud rate etc
	 *
	 */
	if (0 == atomic_read(&can_isopen[minor])) {
		/* mark this process as main process for changing
		   some parameters */
	    iptr->su = TRUE;

	    if (virtual == 0) {
		if (can_chip_reset(minor) < 0) {
			retval = -EINVAL;
			goto free_and_leave;
		}
		can_start_chip(minor); /* enables interrupts */
	    }
#if defined(X_DEBUG) /* not useful for every CAN controller ! */
	    can_show_stat(minor);
	    can_register_dump(minor);
	    {int i;
		for (i = 16; i < 20; i++)
			can_object_dump(minor, i);
	    }
#endif
	}

	/* flag device in use */
	atomic_inc(&can_isopen[minor]);
	format_proc_device_open_count();
	DBGOUT();
	return 0;

free_and_leave:
	kfree(file->private_data);
release_irq_and_leave:
	can_freeirq(minor, IRQ[minor]);
	DBGOUT();
	return retval;
}
