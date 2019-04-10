/*
 * can_read - can4linux CAN driver module
 *
 * can4linux -- LINUX CAN device driver source
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Copyright (c) 2001 port GmbH Halle/Saale
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013-15 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 *------------------------------------------------------------------
 */

/**
* \file read.c
* \author Heinz-Jürgen Oertel
*
* Module Description
* see Doxygen Doc for all possibilities
*
*
*
*/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include "defs.h"
#include <linux/wait.h>
#include <linux/sched.h>


/***************************************************************************/
/**
*
* \brief ssize_t read(int fd, void *buf, size_t count);
* the read system call
* \param fd The descriptor to read from.
* \param buf The destination data buffer (array of CAN canmsg_t).
* \param count The number of CAN frames to read.
*
* read() attempts to read up to \a count CAN messages
* (\b not \b bytes! ) from file descriptor fd
* into the buffer starting at buf.
* buf must be large enough to hold count times the size of
* one CAN message structure \b canmsg_t.
*
* \code
int got;
canmsg_t rx[80];			// receive buffer for read()

    got = read(can_fd, rx , 80 );
    if( got > 0) {
      ...
    } else {
	// read returned with error
	fprintf(stderr, "- Received got = %d\n", got);
	fflush(stderr);
    }

* \endcode
*
* \par ERRORS
*
* the following errors can occur
*
* \arg \c EINVAL \b buf points not to an large enough area,
*
* \returns
* On success, the number of CAN messages read is returned
* (zero indicates end of file).
* It is not an  error if this number is
* smaller than the number of messages requested;
* this may happen for example
* because fewer messages are actually available right now,
* or because read() was interrupted by a signal.
* On error, -1 is returned, and errno is set  appropriately.
*
* \internal
*/

ssize_t can_read(struct file *file, char __user *buffer,
	size_t count, loff_t *loff)
{
	size_t written = 0;
	unsigned long _cnt;	/* return value of copy_*_user */
	unsigned int minor = iminor(file_inode(file));
	int blocking;
	int rx_fifoindex =
	    ((struct _instance_data *)(file->private_data))->rx_index;

	/* msg_fifo_t *rx_fifo = &rx_buf[minor][0]; */
	msg_fifo_t *rx_fifo = &rx_buf[minor][rx_fifoindex];
	canmsg_t __user *addr;

	DBGIN();

	/* pr_info(" : reading in fifo[%d][%d]\n", minor, rx_fifoindex); */

	addr = (canmsg_t __user *) buffer;
	blocking = !(file->f_flags & O_NONBLOCK);

	if (!access_ok
	    (VERIFY_WRITE, (void __user *)buffer, count * sizeof(canmsg_t))) {
		DBGOUT();
		return -EINVAL;
	}
	/* while( written < count && rx_fifo->status == BUF_OK )  */
	while (written < count) {

		/* Look if there are currently messages in the rx queue */
		if (rx_fifo->tail == rx_fifo->head) {
			rx_fifo->status = BUF_EMPTY;

			if (blocking) {
				/* printk("empty and blocking, %d = %d\n", */
				/* rx_fifo->tail , rx_fifo->head ); */
				if (wait_event_interruptible
				    (can_wait[minor][rx_fifoindex],
				     rx_fifo->tail != rx_fifo->head))
					return -ERESTARTSYS;
			} else
				break;
		}

		/* convert the received dlc into a byte number in case of CAN FD
		 * this is better done here instead of the ISR
		 * */
		// pr_info("read() Length in the rx fifo %d\n", (rx_fifo->data[rx_fifo->tail].length));
		if (rx_fifo->data[rx_fifo->tail].flags  & MSG_CANFD)
			rx_fifo->data[rx_fifo->tail].length =
				dlc2len(rx_fifo->data[rx_fifo->tail].length);
		// pr_info("read() returned length %d\n", (rx_fifo->data[rx_fifo->tail].length));

		/* copy one message to the user process */
		_cnt = copy_to_user((void __user *)&(addr[written]),
			    (canmsg_t *) &(rx_fifo->data[rx_fifo->tail]),
			    sizeof(canmsg_t));
		written++;
		++(rx_fifo->tail);
		(rx_fifo->tail) %= MAX_BUFSIZE;
	}
 	DBGOUT();
	return written;
}
