/*
 * can_write - can4linux CAN driver module
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
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013-2015 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 *------------------------------------------------------------------
 */

/**
* \file write.c
* \author Heinz-Jürgen Oertel
*
*/
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include <linux/sched.h>

/***************************************************************************/
/**

\brief size_t write(int fd, const char *buf, size_t count);
write CAN messages to the network
\param fd The descriptor to write to.
\param buf The data buffer to write (array of CAN canmsg_t).
\param count The number of CAN frames to write.

write  writes  up to \a count CAN messages to the CAN controller
referenced by the file descriptor fd from the buffer
starting at buf.

\par Errors

the following errors can occur

\li \c EBADF  fd is not a valid file descriptor or is not open for writing.

\li \c EINVAL fd is attached to an object which is unsuitable for writing.

\li \c EFAULT buf is outside your accessible address space.

\li \c EINTR  The call was interrupted by a signal before any data was written.

\returns
On success, the number of CAN messages written are returned
(zero indicates nothing was written).
On error, -1 is returned, and errno is set appropriately.

\internal
*/

ssize_t can_write(struct file *file, const char __user *buffer, size_t count,
		  loff_t *loff)
{
	unsigned int minor = iminor(file_inode(file));
	msg_fifo_t *tx_fifo = &tx_buf[minor];
	canmsg_t __user *addr;
	canmsg_t tx;
	unsigned long flags = 0; /* still needed for local_irq_save() ? */
	size_t written = 0;
	int retval = 0;
	int blocking;
	unsigned long _cnt;
	int rxfifoindex;

	DBGIN();
	/* spin_lock_irqsave(&write_splock[minor], flags ); */
#ifdef DEBUG_COUNTER
	Cnt1[minor] = Cnt1[minor] + 1;
#endif /* DEBUG_COUNTER */

	/* detect write mode */
	blocking = !(file->f_flags & O_NONBLOCK);

	DBGPRINT(DBG_DATA,
		 (" -- write %d msg, blocking=%d", (int)count, blocking));
	/* printk("w[%d/%d]", minor, tx_fifo->active); */
	addr = (canmsg_t __user *) buffer;
	DBGPRINT(DBG_DATA, (" flags %04X, len %d", addr->flags, addr->length));
#if 0
	pr_info("ID 0x%lx\n", addr->id);
	{

		int i;
		pr_info("buffer %p, addr %p, addrp %p\n", &buffer, &addr, addr);
		for (i = 0; i < 32; i++)
			pr_info("%2x ", buffer[i]);
		pr_info("\n");
	}
#endif

	if (!access_ok(VERIFY_READ, buffer, count * sizeof(canmsg_t))) {
		retval = -EINVAL;
		goto can_write_exit;
	}

	/* enter critical section */
	local_irq_save(flags);

	while (written < count) {

		if (virtual != 0) {
			/* virtual CAN write,
			 * put the frame in all RX queues only */
			int nrx_fifo;
			msg_fifo_t *rx_fifo;
			int16_t myindex =
			    (int16_t) ((struct _instance_data *)
				     (file->private_data))->rx_index;
			struct timeval timestamp;

			DBGPRINT(DBG_DATA,
			(" -- write msg %u, virtual, blocking=%d, size=%d",
				  (unsigned int)written, blocking,
				  (int)sizeof(canmsg_t)));

			/* depending on the number of open processes
			 * the TX data has to be copied in different
			 * RX FIFOs
			 */

			/* get one message from the userspace buffer */
			/* FIXME: with CANFD,
			 * does it make sense to copy only the number of data
			 * bytes specified in the length field of canmsg_t ?
			 * Instead of sizeof(canmsg_t) it is something like
			 * sizeof(canmsg_t) - CAN_MSG_LENGTH  + length
			 */
			_cnt = copy_from_user(
				(canmsg_t *) &tx,
				(canmsg_t __user *) &addr[written],
				sizeof(canmsg_t));

			/* we are taking this as receive time stamp */
			get_timestamp(minor, &timestamp);

#if defined CANFD
			if (tx.flags & MSG_CANFD) {
				/* correct the data length code for CAN FD
				 * and extended length
				 * The read command expects a dlc */
				tx.length = len2dlc(tx.length);
			}
#endif
			for (nrx_fifo = 0; nrx_fifo < CAN_MAX_OPEN; nrx_fifo++) {
				/* for every rx fifo */
				if (can_waitflag[minor][nrx_fifo] == 1) {
					/* this FIFO is in use */
					/*
					 * Don't copy the message
					 * in the receive queue
					 * of the process that sent the message
					 * unless
					 * this process requested selfreception.
					 */
					if ((myindex == nrx_fifo) &&
					    (selfreception[minor][nrx_fifo]
					     == 0))
						continue;
					/* prepare buffer to be used */
					rx_fifo = &rx_buf[minor][nrx_fifo];

					rx_fifo->data[rx_fifo->head].flags = 0;
					memcpy((void *)
					       &rx_fifo->data[rx_fifo->head],
					       (void *)&tx, sizeof(canmsg_t));
					/* Now copy the time stamp
					 * to the RX FIFO */
					rx_fifo->data[rx_fifo->head].
					    timestamp.tv_sec = timestamp.tv_sec;
					rx_fifo->data[rx_fifo->head].
					    timestamp.tv_usec =
					    timestamp.tv_usec;
#if defined CANFD
					/* in virtual mode,
					 * set the BRS bit to be evaluated
					 * by the receiver */
					if (proc_speedfactor[minor] > 1)
						rx_fifo->data[rx_fifo->
						      head].flags |= MSG_RBRS;
#endif
					/* Set software overflow flag */
					if ((rx_fifo->status & BUF_OVERRUN) !=
					    0) {
						rx_fifo->data[rx_fifo->
						      head].flags |= MSG_BOVR;
					}

					/* Mark message as 'self sent/received'
					 */
					if ((myindex == nrx_fifo) &&
					    (selfreception[minor][nrx_fifo]
					     != 0)) {
						rx_fifo->data[rx_fifo->
						      head].flags |= MSG_SELF;
					}
					/* increment write index */
					rx_fifo->status = BUF_OK;
					++(rx_fifo->head);
					rx_fifo->head %= MAX_BUFSIZE;

					if (rx_fifo->head == rx_fifo->tail) {
						pr_err
						("CAN[%d][%d] RX: FIFO overrun\n",
						     minor, nrx_fifo);
						rx_fifo->status = BUF_OVERRUN;
					}
		/*---------- kick the select() call  -*/
					wake_up_interruptible(
						&can_wait[minor][nrx_fifo]);
				}
				/* this FIFO is in use */
			}

		} else {
			/* we have a real hardware to handle */

			/* Do we really need to protect something here ????
			 * e.g. in this case the tx_fifo->free[tx_fifo->head]
			 * value
			 * If YES, we have to use spinlocks for synchronization
			 */

/* - new Blocking code -- */

			if (blocking) {
				if (wait_event_interruptible(
					canout_wait[minor],
					tx_fifo->free[tx_fifo->head]
					!= BUF_FULL)) {
					retval = -ERESTARTSYS;
					goto can_write_exit;
				}
			} else {
				/* there are data to write to the network */
				if (tx_fifo->free[tx_fifo->head] == BUF_FULL) {
					/* but there is already one message
					 * at this place */
					;
					/* write buffer full
					 * in non-blocking mode,
					 * leave write() */
					goto can_write_exit;
				}
			}

/* ---- */

			/*
			 * To know which process sent the message
			 * we need an index.  This is used in the TX IRQ
			 * to decide in which receive queue
			 * this message has to be copied (selfreception)
			 */
			rxfifoindex =
			    ((struct _instance_data *)(file->private_data))
			    ->rx_index;
			put_user(rxfifoindex, &addr[written].cob);
			/* addr[written].cob = rxfifoindex; */

			if (tx_fifo->active) {
				/* more than one data and actual data in queue,
				 * add this message to the TX queue
				 */
				/* copy one message to FIFO */
				_cnt = copy_from_user(
				(canmsg_t *) &(tx_fifo->data[tx_fifo->head]),
				(canmsg_t __user *) &addr[written],
						sizeof(canmsg_t));
				/* now this entry is FULL */
				tx_fifo->free[tx_fifo->head] = BUF_FULL;
				++tx_fifo->head;
				(tx_fifo->head) %= MAX_BUFSIZE;

			} else {
				/* copy message into local canmsg_t structure */
				_cnt = copy_from_user(
				(canmsg_t *) &tx,
				(canmsg_t __user *) &addr[written],
						sizeof(canmsg_t));
				/* f - fast -- use interrupts */
				if (count >= 1) {
					/* !!! CHIP abh. !!! */
					tx_fifo->active = 1;
				}
				/* write CAN msg data to the chip and enable
				 * the tx interrupt */
				/* Send, no wait */
				can_send_message(minor, &tx);
			}	/* tx_fifo->active */
		}
		DBGPRINT(DBG_DATA, ("    # %d: tail %d: head %d\n",
				    tx_fifo->head - tx_fifo->tail,
				    tx_fifo->tail, tx_fifo->head));
		written++;
		retval = written;
	}

can_write_exit:

	local_irq_restore(flags);

	/* spin_unlock_irqrestore(&write_splock[minor], flags); */
	DBGOUT();
	return retval;
}
