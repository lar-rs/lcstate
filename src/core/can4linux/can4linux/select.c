/* can_select
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
 * (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 *------------------------------------------------------------------
 */
#include "defs.h"

unsigned int can_select(struct file *file, struct poll_table_struct *wait)
{
unsigned int minor = iminor(file_inode(file));
int rx_fifoindex = ((struct _instance_data *)(file->private_data))->rx_index;
msg_fifo_t *rx_fifo = &rx_buf[minor][rx_fifoindex];
msg_fifo_t *tx_fifo = &tx_buf[minor];
unsigned int mask = 0;

	/* DBGIN("can_select"); */
	/* DBGPRINT(DBG_DATA,("minor = %d", minor)); */
#ifdef DEBUG
	/* can_show_stat(minor); */
#endif

	/* DBGPRINT(DBG_BRANCH,("POLL: fifo empty,poll waiting...\n")); */

	/* every event queue that could wake up the process
	 * and change the status of the poll operation
	 * can be added to the poll_table structure by
	 * calling the function poll_wait:
	 */
	/*     _select para, wait queue, _select para */
	poll_wait(file, &can_wait[minor][rx_fifoindex], wait);
	poll_wait(file, &canout_wait[minor], wait);

	/* DBGPRINT(DBG_BRANCH,("POLL: wait returned\n")); */
	if (rx_fifo->head != rx_fifo->tail) {
		/* fifo has some telegrams */
		/* Return a bit mask
		 * describing operations that could be immediately performed
		 * without blocking.
		 */
		/*
		 * POLLIN This bit must be set
		 *        if the device can be read without blocking.
		 * POLLRDNORM This bit must be set
		 * if "normal'' data is available for reading.
		 * A readable device returns (POLLIN | POLLRDNORM)
		 *
		 *
		 *
		 */
		mask |= POLLIN | POLLRDNORM;	/* readable */
	}
	if (tx_fifo->head == tx_fifo->tail) {
		/* fifo is empty */
		/* Return a bit mask
		 * describing operations that could be immediately performed
		 * without blocking.
		 */
		/*
		 * POLLOUT This bit must be set
		 *        if the device can be written without blocking.
		 * POLLWRNORM This bit must be set
		 */
		mask |= POLLOUT | POLLWRNORM;	/* writeable */
	}
	/* DBGOUT(); */
	return mask;
}
