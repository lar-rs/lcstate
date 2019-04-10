/*
 * can_close - can4linux CAN driver module
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 * Copyright (c) 2011 port GmbH Halle/Saale
 * (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * derived from the the LDDK can4linux version
 *     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 */

/**
* \file close.c
* \author Heinz-Jürgen Oertel
*
*/

#include <linux/pci.h>
#include "defs.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/***************************************************************************/
/**
*
* \brief int close(int fd);
* close a file descriptor
* \param fd The descriptor to close.
*
* \b close closes a file descriptor, so that it no longer
*  refers to any device and may be reused.
* \returns
* close returns zero on success, or -1 if an error occurred.
* \par ERRORS
*
* the following errors can occur
*
* \arg \c BADF \b fd isn't a valid open file descriptor
*
*/

int can_close(struct inode *inode, struct file *file)
{
	unsigned int minor = iminor(inode);
	int rx_fifo = ((struct _instance_data *)(file->private_data))->rx_index;

	DBGIN();

#ifdef CAN_USE_FILTER
	can_filter_cleanup(minor);
#endif
	/* kfree(NULL) is safe, check is probably not required */
	kfree(file->private_data);

	can_waitflag[minor][rx_fifo] = 0;
	selfreception[minor][rx_fifo] = 0;

	atomic_dec(&can_isopen[minor]);	/* flag device as free */
	format_proc_device_open_count();

	if (virtual != 0) {
		DBGOUT();
		return 0;
	}
	if (atomic_read(&can_isopen[minor]) > 0) {
		DBGPRINT(DBG_BRANCH, ("leaving close() without shut down"));
		DBGOUT();
		return 0;
	}

	/*
	 * all processes released the driver
	 * now shut down the CAN controller
	 */
	DBGPRINT(DBG_BRANCH, ("stop chip and release resources"));
	can_stopchip(minor);

#if !defined(PCM3680) && !defined(CPC_104) && !defined(CPC_PCM_104)
	/* call this before freeing any memory or io area.
	 * this can contain registers needed by can_freeirq()
	 */
	/* printk(KERN_INFO "    Releasing IRQ %d\n", IRQ[minor]); */
	can_freeirq(minor, IRQ[minor]);

	/* should the resources be released in a manufacturer specific file?
	 * is it always depending on the hardware?
	 */

#if defined(SSV_MCP2515) || defined(PCM9890) || defined(AuR_MCP2515) \
	|| defined(MCP2515SPI)
	;

#elif defined(ATCANMINI_PELICAN) || defined(GENERIC_I82527)
	release_region(proc_base[minor], can_range[minor]);

#elif defined(CAN_PORT_IO) && !defined(KVASER_PCICAN)

	/* printk("pci_release_region()\n"); */
	/*  code for CC_CANPCI */
	pci_release_region(can_pcidev[minor], 1);	/* LED control */
	pci_release_region(can_pcidev[minor], 2);	/* CAN-I/O */
#else
# if defined(CAN_INDEXED_PORT_IO)
	release_region(proc_base[minor], 2);
# else
#  ifndef CAN4LINUX_PCI
	/* This part is called for:
	   Zedboard, ....
	 */
	/* release I/O memory mapping -> release virtual memory */
	/* pr_info("iounmap %p\n", can_iobase[minor]); */
	iounmap(can_iobase[minor]);

	/* Release the memory region */
	/* pr_info("release mem %x\n", proc_base[minor]); */
	release_mem_region(proc_base[minor], can_range[minor]);

#  endif
# endif
#endif

#else /* !defined(TARGETS with can_release() in target.c */
	can_release(minor);
#endif /* !defined(TARGETS with can_release() in target.c */

	DBGOUT();
	return -EBADF;
}
