/* xcanpsfuncs - CAN controller Xilinx xcanps part of can4linux drivers
*
* can4linux -- LINUX CAN device driver source
*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 * Copyright (c) 2013 Heinz-Juergen Oertel (oe@port.de)
 *------------------------------------------------------------------
*/
/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include "linux/delay.h"
#include "xcanps.h"
#include <linux/sched.h>


/* int	CAN_Open = 0; */

/* timing values
 in this case we need for all 10 possible timings two bytes each.
 This might be different on some controllers */
static u8 can_timing[10][2] = {
	{CAN_BRPR_10K,   CAN_BTR_10K},
	{CAN_BRPR_20K,   CAN_BTR_20K},
	{CAN_BRPR_50K,   CAN_BTR_50K},
	{CAN_BRPR_100K,  CAN_BTR_100K},
	{CAN_BRPR_125K,  CAN_BTR_125K},
	{CAN_BRPR_250K,  CAN_BTR_250K},
	{CAN_BRPR_500K,  CAN_BTR_500K},
	{CAN_BRPR_800K,  CAN_BTR_800K},
	{CAN_BRPR_1000K, CAN_BTR_1000K} };



/* Board reset
   means the following procedure:
  set Reset Request
  wait for Rest request is changing - used to see if board is available
  initialize board (with values from /proc/sys/Can)
    set output control register
    set timings
    set acceptance mask
*/


#ifdef DEBUG
/* when ever we need while debugging some controller status information */
int can_show_stat(int minor)
{
	/* Show only the first n register valuaes */
	can_register_dump(minor, 7);
	return 0;
}

void can_register_dump(int minor, int n)
{
int i;
char *register_names[] = {
	"srr", "msr", "brpr", "btr", "ecr", "esr", "sr", "isr",
	"ier", "icr", "tcr", "wir", "txfifo_id",
	"txfifo_dlc", "txfifo_data1", "txfifo_data2", "txhpb_id",
	"txhpb_dlc", "txhpb_data1",
	"txhpb_data2", "rxfifo_id", "rxfifo_dlc", "rxfifo_data1",
	"rxfifo_data2", "afr", "afmr1",
	"afir1", "afmr2", "afir2", "afmr3", "afir3", "afmr4", "afir4" };

	if (n > 33)
		n = 33;
	pr_info("-----------\n");
	for (i = 0; i < n; i++) {
		pr_info("%p %13s: 0x%08x\n",
			can_iobase[0] + 4*i,
			register_names[i],
			(__raw_readl((void __iomem *)can_iobase[0]+4*i)));
	}
}
#endif

/* can_getstat - read back as many status information as possible
*
* Because the CAN protocol itself describes different kind of information
* already, and the driver has some generic information
* (e.g about it's buffers)
* we can define a more or less hardware independent format.
*
*
* exception:
* ERROR WARNING LIMIT REGISTER (EWLR)
* The SJA1000 defines a EWLR, reaching this Error Warning Level
* an Error Warning interrupt can be generated.
* The default value (after hardware reset) is 96. In reset
* mode this register appears to the CPU as a read/write
* memory. In operating mode it is read only.
* Note, that a content change of the EWLR is only possible,
* if the reset mode was entered previously. An error status
* change (see status register; Table 14) and an error
* warning interrupt forced by the new register content will not
* occur until the reset mode is cancelled again.
*/

int can_getstat(
	struct inode *inode,
	struct file *file,
	can_statuspar_t *stat
	)
{
unsigned int minor = iminor(inode);
msg_fifo_t *fifo;
unsigned long flags;
unsigned long ecr;
int nrx_fifo = ((struct _instance_data *)(file->private_data))->rx_index;

	stat->type = CAN_TYPE_XCANPS;

	stat->baud = proc_baud[minor];
	stat->status = CAN_INL(minor, sr);
	stat->error_warning_limit = 96;

	ecr = CAN_INL(minor, ecr);
	stat->rx_errors = (ecr & XCANPS_ECR_REC_MASK) >> XCANPS_ECR_REC_SHIFT;
	stat->tx_errors = ecr & XCANPS_ECR_TEC_MASK;

	stat->error_code = CAN_INL(minor, esr);

	/* Disable CAN (All !!) Interrupts */
	/* !!!!!!!!!!!!!!!!!!!!! */
	/* save_flags(flags); cli(); */
	local_irq_save(flags);

	fifo = &rx_buf[minor][nrx_fifo];
	stat->rx_buffer_size = MAX_BUFSIZE;	/**< size of rx buffer  */
	/* number of messages */
	stat->rx_buffer_used =
	    (fifo->head < fifo->tail)
	    ? (MAX_BUFSIZE - fifo->tail + fifo->head)
	    : (fifo->head - fifo->tail);
	fifo = &tx_buf[minor];
	stat->tx_buffer_size = MAX_BUFSIZE;	/* size of tx buffer  */
	/* number of messages */
	stat->tx_buffer_used =
	    (fifo->head < fifo->tail)
	    ? (MAX_BUFSIZE - fifo->tail + fifo->head)
	    : (fifo->head - fifo->tail);
	/* Enable CAN Interrupts */
	/* !!!!!!!!!!!!!!!!!!!!! */
	/* restore_flags(flags); */
	local_irq_restore(flags);
	return 0;
}

int can_chip_reset(int minor)
{
	DBGIN();
	/* Reset CAN controller */
	/* Writing a 1 to the SRST bit in the SRR register.
	 * The controller enters Configuration mode
	 * immediately following the software reset.
	 */
	CAN_OUTL(minor, srr, XCANPS_SRR_SRST_MASK);

	DBGOUT();
	return 0;
}


/*
 * Configures bit timing registers directly. Chip must be in bus off state.
 */
int can_set_btr(int minor, int btr0, int btr1)
{
	(void)minor;
	(void)btr0;
	(void)btr1;

	DBGIN();

	DBGOUT();
	return 0;
}


/*
 * Configures bit timing. Chip must be in configuration mode.
 */
int can_set_timing(int minor, int baud)
{
int i	   = 4;	/* default index into table == 125kbit/s */
int custom;
int isopen;
int retval;

	DBGIN();
	retval = 0;
	custom = 0;

	isopen = atomic_read(&can_isopen[minor]);
	if ((isopen > 1) && (proc_baud[minor] != baud)) {
		DBGPRINT(DBG_DATA, ("isopen = %d", isopen));
		DBGPRINT(DBG_DATA, ("refused baud[%d]=%d already set to %d",
						minor, baud, proc_baud[minor]));
		return -1;
	}

	DBGPRINT(DBG_DATA, ("baud[%d]=%d", minor, baud));
	switch (baud) {
	case   10:
	    i = 0; break;
	case   20:
	    i = 1; break;
	case   50:
	    i = 2; break;
	case  100:
	    i = 3; break;
	case  125:
	    i = 4; break;
	case  250:
	    i = 5; break;
	case  500:
	    i = 6; break;
	case  800:
	    i = 7; break;
	case 1000:
	    i = 8; break;
	default:
		custom = 1;
		break;
	}

	/* hardware depending code follows here */

	if (custom) {
		/* set direct register values */
		CAN_OUTL(minor, brpr, (u8) (baud >> 8) & 0xff);
		CAN_OUTL(minor, btr,  (u8) (baud & 0xff));
	} else {
		/* use table values, i is index */
		/* If value of BTR is 0xff - bit rate is not supported */
		if (can_timing[i][1] == 0xff) {
			retval = -1;
		} else {
			CAN_OUTL(minor, brpr, (u8) can_timing[i][0]);
			CAN_OUTL(minor, btr,  (u8) can_timing[i][1]);
		}
	}

	DBGOUT();
	return retval;
}


/*
   Reset error Counter information in /proc
   Clear pending Interrupts
   Set Interrupt sources
   Activate CAN

*/
int can_start_chip(int minor)
{
	(void)minor;

	DBGIN();
	proc_rxerr[minor] = proc_txerr[minor] = 0L;

	/* set normal operating mode */
	/* Normal mode, reset MSR: LBACK, SNOOP, SLEEP
	 * that is msr content is == 0
	 * For test purposes it might useful to set the CAN
	 * in loop-back mode:
	 * CAN_OUTL(minor, ms, XCANPS_MSR_LBACK_MASK);
	 *
	 */
	CAN_OUTL(minor, msr, 0);

	can_set_timing(minor, proc_baud[minor]);
	can_set_mask(minor, proc_acccode[minor], proc_accmask[minor]);
	/* set the Chip ENable bit */
	CAN_OUTL(minor, srr, XCANPS_SRR_CEN_MASK);
	/* can_register_dump(minor, 7); */

	/* now clear pending interrupts */


	/* And set used and supported interrupt sources
	 * setting XCANPS_IXR_TXFEMP_MASK makes no sense now.
	 * the interrupt comes always when TXFIFO is empty.
	 * Much to often hoe can4linux uses TX.
	 */
	CAN_OUTL(minor, ier,
		XCANPS_IXR_TXOK_MASK
	      | XCANPS_IXR_RXOK_MASK
	      | XCANPS_IXR_RXOFLW_MASK
	      | XCANPS_IXR_BSOFF_MASK
	      | XCANPS_IXR_ERROR_MASK
	    );

	DBGOUT();
	return 0;
}


/*
* If the driver is used by more than one application,
* one should take care that this functionality (like some others)
* can not be called by any application.
* Stopping the shared CAN will stop it for all other processes as well.
*
* can4linux blocks this function (and others)  in ioctl.c
*/
int can_stopchip(int minor)
{
	DBGIN();

	DBGOUT();
	return 0;
}

/* set value of the output control register */
int can_set_mode(int minor, int arg)
{
	(void)minor;
	(void)arg;

	DBGIN();

	DBGOUT();
	return 0;
}

/*
Listen-Only Mode
In listen-only mode, the CAN module is able to receive messages
without giving an acknowledgment.
Since the module does not influence the CAN bus in this mode
the host device is capable of functioning like a monitor
or for automatic bit-rate detection.

 must be done after CMD_START (can_stopchip)
 and before CMD_START (can_start_chip)
*/
int can_set_listenonlymode(int minor,
	int arg)	/* true - set Listen Only, false - reset */
{
	(void)minor;
	(void)arg;

	DBGIN();
	if (arg) {
		/* set listen only mode */
		;
	} else {
		/* set active mode */
		;
	}

	DBGOUT();
	return 0;
}

/* set Acceptance Code and Mask Registers */
int can_set_mask(int minor, unsigned int code, unsigned int mask)
{
	(void)minor;
	(void)code;
	(void)mask;

	DBGIN();
	/* set register values */

	/* put values back in global variables for sysctl */
	proc_acccode[minor] = code;
	proc_accmask[minor] = mask;
	DBGOUT();
	return 0;
}


/*
Single CAN frames or the very first Message are copied into the CAN controller
using this function.
After a succesful transmission, an interrupt will be generated,
which will be handled in the CAN ISR can_interrupt()
*/
int can_send_message(int minor, canmsg_t *tx)
{
int stat;
unsigned int id;

	DBGIN();
	id = 0;
	/* wait for transmission complete, read canstat  */
	while ((stat = CAN_INL(minor, sr) & XCANPS_SR_TXFLL_MASK))
		cond_resched();

	/* fill in message id, message data, .... */
	if (tx->flags & MSG_EXT) {
		/* Extended frame format */
		id = (tx->id & 0x3FFFF) << XCANPS_IDR_ID2_SHIFT;
		id |= (tx->id & 0x1FFC0000) << XCANPS_IDR_ID1_X_SHIFT;
		id |= 1 << XCANPS_IDR_IDE_SHIFT;
		if (tx->flags & MSG_RTR)
			id |= XCANPS_IDR_RTR_MASK;
	} else {
		/* Base frame format */
		id = tx->id << XCANPS_IDR_ID1_SHIFT;
		id &= XCANPS_IDR_ID1_MASK;
		if (tx->flags & MSG_RTR)
			id |= XCANPS_IDR_SRR_MASK;
	}
	CAN_OUTL(minor, txfifo_id, id);
	CAN_OUTL(minor, txfifo_dlc, tx->length << XCANPS_DLCR_DLC_SHIFT);

	CAN_OUTL(minor, txfifo_data1,
		  (tx->data[0] << 24)
		+ (tx->data[1] << 16)
		+ (tx->data[2] <<  8)
		+ (tx->data[3]));
	CAN_OUTL(minor, txfifo_data2,
		  (tx->data[4] << 24)
		+ (tx->data[5] << 16)
		+ (tx->data[6] <<  8)
		+ (tx->data[7]));

	/* issue transmission request to the CAN controller */
	/* The Xilinx xcan doesn't need a special request
	 * to send the frame.
	 * It is send, afetr the last of the four words is written
	 */

	/*
	 * Save last message that was sent.
	 * Since can4linux 3.5 multiple processes can access one CAN interface.
	 * On a CAN interrupt this message is copied
	 * into the receive queue of each process
	 * that opened this same CAN interface.
	 */
	memcpy(
		(void *)&last_tx_object[minor],
		(void *)tx,
		sizeof(canmsg_t));

	DBGOUT();
	return 0;
}


/*
 * The plain interrupt
 *
 */

irqreturn_t can_interrupt(int irq, void *dev_id)
{
int minor;
int nrx_fifo;
struct timeval  timestamp;
unsigned long flags;
int ext;			/* flag for extended message format */
unsigned int id;
int irqsrc;
msg_fifo_t   *rx_fifo;
msg_fifo_t   *tx_fifo;
#if defined(CAN_USE_FILTER)
msg_filter_t *rx_passed;
#endif
int first;

#if defined(CONFIG_TIME_MEASURE)
	set_measure_pin();
#endif


	minor = *(int *)dev_id;
	first = 0;

	rx_fifo = &rx_buf[minor][0];
	tx_fifo = &tx_buf[minor];
#if defined(CAN_USE_FILTER)
	rx_passed = &rx_filter[minor];
#endif

	/* read status if CAN has an interrupt pending */
	irqsrc = CAN_INL(minor, isr);

	/* not all bits in the Interrupt source masks
	 * are handled Interrupt conditions.
	 * Therefore mask them out
	 */
#if 0
	irqsrc &=  XCANPS_IXR_TXOK_MASK
		 | XCANPS_IXR_BSOFF_MASK
		 | XCANPS_IXR_RXOK_MASK
		 | XCANPS_IXR_RXOFLW_MASK
		 ;
	/* pr_info("CAN - ISR ; minor = %d, isr= 0x%08x\n",
		    *(int *)dev_id, irqsrc); */
#endif

	if (irqsrc == 0) {
		/* first call to ISR, it's not for me ! */
#if defined(CONFIG_TIME_MEASURE)
		reset_measure_pin();
#endif
		return IRQ_RETVAL(IRQ_NONE);
	}

	/* clear interrupts */
	CAN_OUTL(minor, icr, XCANPS_IXR_ALL);

	/* Whatever interrupt we have, update the tx error counter
	 * and rx error counter information in /proc/sys/dev/Can
	 */
	{
	int ecr;
		ecr = CAN_INL(minor, ecr);
		proc_txerrcounter[minor] = ecr & XCANPS_ECR_TEC_MASK;
		proc_rxerrcounter[minor]
		    = (ecr & XCANPS_ECR_REC_MASK) >> XCANPS_ECR_REC_SHIFT;
	}

	do {
		/* loop as long as the CAN controller shows interrupts */
		/* can_dump(minor); */
#if defined(DEBUG)
		/* how often do we loop through the ISR ? */
		/* if(first) printk("n = %d\n", first); */
		/* we can have a /proc/sys/dev/Can/irqloop
		   to store the max counter value
		   for debugging purposes to see how heavy the isr is used
		 */
		first++;
		if (first > 10)
			return IRQ_RETVAL(IRQ_HANDLED);
#endif

	get_timestamp(minor, &timestamp);

	for (nrx_fifo = 0; nrx_fifo < CAN_MAX_OPEN; nrx_fifo++) {
		rx_fifo = &rx_buf[minor][nrx_fifo];

		rx_fifo->data[rx_fifo->head].timestamp = timestamp;

		/* preset flags */
		(rx_fifo->data[rx_fifo->head]).flags =
			(rx_fifo->status & BUF_OVERRUN ? MSG_BOVR : 0);
	}


	/*========== receive interrupt */
	if (irqsrc & XCANPS_IXR_RXOK_MASK) {
		u32 dummy;
		u32 length;
		u32 data1, data2;

	    /* can_register_dump(minor, 33); */

	    /*
	     pr_info("isr: %08x\n", CAN_INL(minor, isr));

	     mdelay 2 ms for testing the RX FIFO Overflow
	     with sending fast at 125K
	     an overflow should happen after twice the number
	     of RX Fifo size frames send.
	     mdelay(2); */

	    /* read IDR */
	    /* dummy  = CAN_INL(minor, rxfifo_id); */
	    dummy = (__raw_readl((void __iomem *)can_iobase[minor] + 0x50));
	    /*  pr_info(" id %08x\n", dummy); */

	    /* get message length as received in the frame */
	    /* strip length code */
	    /*  length = CAN_INL(minor, rxfifo_dlc); */
	    length = (__raw_readl((void __iomem *)can_iobase[minor] + 0x54));
	    /*  pr_info("dlc %08x\n", length); */
	    length = length >> XCANPS_DLCR_DLC_SHIFT;

	    /* Read out frame data */
	    /*  data1 = CAN_INL(minor, rxfifo_data1); */
	    data1 = (__raw_readl((void __iomem *)can_iobase[minor] + 0x58));
	    /*  data2 = CAN_INL(minor, rxfifo_data2); */
	    data2 = (__raw_readl((void __iomem *)can_iobase[minor] + 0x5c));

	    /* ---------- fill frame data -------------------------------- */
	    /* handle all subscribed rx fifos */

	for (nrx_fifo = 0; nrx_fifo < CAN_MAX_OPEN; nrx_fifo++) {
		/* for every rx fifo */

		if (can_waitflag[minor][nrx_fifo] == 1) {
			/* this FIFO is in use */
			/* prepare buffer to be used */
			rx_fifo = &rx_buf[minor][nrx_fifo];
			/* pr_info("> filling buffer [%d][%d]\n",
				minor, nrx_fifo);  */
			if (dummy & XCANPS_IDR_IDE_MASK) {
				/* received extended Id frame */
				(rx_fifo->data[rx_fifo->head]).flags |= MSG_EXT;
				if (dummy & XCANPS_IDR_RTR_MASK)
					(rx_fifo->data[rx_fifo->head]).flags
					    |= MSG_RTR;
				(rx_fifo->data[rx_fifo->head]).id =
				((dummy & XCANPS_IDR_ID2_MASK) >> XCANPS_IDR_ID2_SHIFT)
				| ((dummy & XCANPS_IDR_ID1_MASK) >> 3);
			} else {
				/* received base Id frame */
				if (dummy & XCANPS_IDR_SRR_MASK)
					(rx_fifo->data[rx_fifo->head]).flags |= MSG_RTR;
				(rx_fifo->data[rx_fifo->head]).id =
				(dummy & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT;
			}

			/* get message length as received in the frame */
			/* strip length code */
			(rx_fifo->data[rx_fifo->head]).length = length;

			(rx_fifo->data[rx_fifo->head]).data[0] = data1 >> 24;
			(rx_fifo->data[rx_fifo->head]).data[1] = data1 >> 16;
			(rx_fifo->data[rx_fifo->head]).data[2] = data1 >> 8;
			(rx_fifo->data[rx_fifo->head]).data[3] = data1;

			(rx_fifo->data[rx_fifo->head]).data[4] = data2 >> 24;
			(rx_fifo->data[rx_fifo->head]).data[5] = data2 >> 16;
			(rx_fifo->data[rx_fifo->head]).data[6] = data2 >> 8;
			(rx_fifo->data[rx_fifo->head]).data[7] = data2;

			/* mark just written entry as OK and full */
			rx_fifo->status = BUF_OK;
			/* Handle buffer wrap-around */
			++(rx_fifo->head);
			rx_fifo->head %= MAX_BUFSIZE;
			if (unlikely(rx_fifo->head == rx_fifo->tail)) {
				pr_err("CAN[%d][%d] RX: FIFO overrun\n",
				minor, nrx_fifo);
				rx_fifo->status = BUF_OVERRUN;
			}

			/*---------- kick the select() call  -*/
			/* This function will wake up all processes
			   that are waiting on this event queue,
			   that are in interruptible sleep
			*/
			/* pr_info(" should wake [%d][%d]\n",
				minor, nrx_fifo); */
			wake_up_interruptible(&can_wait[minor][nrx_fifo]);

		}
	} /* for( rx fifos ...) */
	/* ---------- / fill frame data -------------------------------- */
	} /* end RX Interrupt */

	/*========== transmit interrupt */
	if (irqsrc & XCANPS_IXR_TXOK_MASK) {
		u32 dlc;
		/* CAN frame successfully sent */
		/* pr_info("==> TX Interrupt\n"); */
		/* use time stamp sampled with last INT */
		last_tx_object[minor].timestamp = timestamp;

		/* depending on the number of open processes
		 * the TX data has to be copied in different
		 * rx fifos
		 */
	    for (nrx_fifo = 0; nrx_fifo < CAN_MAX_OPEN; nrx_fifo++) {
		/* for every rx fifo */
		if (can_waitflag[minor][nrx_fifo] == 1) {
		    /* this FIFO is in use */
		    /* pr_info("self copy to [%d][%d]\n", * minor, nrx_fifo); */

		    /*
		     * Don't copy the message in the receive queue
		     * of the process that sent the message unless
		     * this process requested selfreception.
		     */
		    if ((last_tx_object[minor].cob == nrx_fifo) &&
			    (selfreception[minor][nrx_fifo] == 0))
				continue;


		    /* prepare buffer to be used */
		    rx_fifo = &rx_buf[minor][nrx_fifo];

		    memcpy(
			(void *)&rx_fifo->data[rx_fifo->head],
			(void *)&last_tx_object[minor],
			sizeof(canmsg_t));

		    /* Mark message as 'self sent/received' */
		    rx_fifo->data[rx_fifo->head].flags |= MSG_SELF;

		    /* increment write index */
		    rx_fifo->status = BUF_OK;
		    ++(rx_fifo->head);
		    rx_fifo->head %= MAX_BUFSIZE;

		    if (unlikely(rx_fifo->head == rx_fifo->tail)) {
			pr_err("CAN[%d][%d] RX: FIFO overrun\n",
				minor, nrx_fifo);
			rx_fifo->status = BUF_OVERRUN;
		    }
		    /*---------- kick the select() call  -*/
		    /* This function will wake up all processes
		       that are waiting on this event queue,
		       that are in interruptible sleep
		    */
		    wake_up_interruptible(&can_wait[minor][nrx_fifo]);
		} /* this FIFO is in use */
	    } /* end for loop filling all rx-fifos */


		if (tx_fifo->free[tx_fifo->tail] == BUF_EMPTY) {
			/* TX FIFO empty, nothing more to sent */
			/* pr_info("TXE\n"); */
			tx_fifo->status = BUF_EMPTY;
			tx_fifo->active = 0;
			/* This function will wake up all processes
			that are waiting on this event queue,
			that are in interruptible sleep
			*/
			wake_up_interruptible(&canout_wait[minor]);
			goto tx_done;
	    }

	    /* enter critical section */
	    /* spin_lock_irqsave(&write_splock[minor], flags); */

	    local_irq_save(flags);
	    /* pr_info("CAN[%d][%d] enter\n", minor, nrx_fifo); */

	    /* The TX message FIFO contains other CAN frames to be sent
	     * The next frame in the FIFO is copied into the last_tx_object
	     * and directly into the hardware of the CAN controller
	     */
	    memcpy(
		    (void *)&last_tx_object[minor],
		    (void *)&tx_fifo->data[tx_fifo->tail],
		    sizeof(canmsg_t));

	    ext = (tx_fifo->data[tx_fifo->tail]).flags & MSG_EXT;
	    id = (tx_fifo->data[tx_fifo->tail]).id;
	    if (ext) {
		u32 tmpid = id;
		DBGPRINT(DBG_DATA, ("---> send ext message"));
		/* Extended frame format */
		id = (id & 0x3FFFF) << XCANPS_IDR_ID2_SHIFT;
		id |= (tmpid & 0x1FFC0000) << XCANPS_IDR_ID1_X_SHIFT;
		id |= 1 << XCANPS_IDR_IDE_SHIFT;
		if ((tx_fifo->data[tx_fifo->tail]).flags & MSG_RTR)
			id |= XCANPS_IDR_RTR_MASK;
	    } else {
		DBGPRINT(DBG_DATA, ("---> send std message"));
		/* Base frame format */
		id = id << XCANPS_IDR_ID1_SHIFT;
		id &= XCANPS_IDR_ID1_MASK;
		if ((tx_fifo->data[tx_fifo->tail]).flags & MSG_RTR)
			id |= XCANPS_IDR_SRR_MASK;
	    }
	    CAN_OUTL(minor, txfifo_id, id);

	    dlc = (tx_fifo->data[tx_fifo->tail]).length;
	    dlc &= 0x0f;		/* restore length only */
	    CAN_OUTL(minor, txfifo_dlc, dlc << XCANPS_DLCR_DLC_SHIFT);

	    CAN_OUTL(minor, txfifo_data1,
		      ((tx_fifo->data[tx_fifo->tail]).data[0] << 24)
		    + ((tx_fifo->data[tx_fifo->tail]).data[1] << 16)
		    + ((tx_fifo->data[tx_fifo->tail]).data[2] <<  8)
		    + ((tx_fifo->data[tx_fifo->tail]).data[3]));
	    CAN_OUTL(minor, txfifo_data2,
		      ((tx_fifo->data[tx_fifo->tail]).data[4] << 24)
		    + ((tx_fifo->data[tx_fifo->tail]).data[5] << 16)
		    + ((tx_fifo->data[tx_fifo->tail]).data[6] <<  8)
		    + ((tx_fifo->data[tx_fifo->tail]).data[7]));
	    /* no transmission request needed,
	     * XCAN sends after all four TXBUF registers are filled
	     */


	    /* now this entry is EMPTY */
	    tx_fifo->free[tx_fifo->tail] = BUF_EMPTY;
	    ++(tx_fifo->tail);
	    tx_fifo->tail %= MAX_BUFSIZE;

	    /* leave critical section */
	    /* pr_info("CAN[%d][%d] leave\n", minor, nrx_fifo); */
	    local_irq_restore(flags);
	    /* spin_unlock_irqrestore(&write_splock[minor], flags); */


	}
tx_done:

	/*========== arbitration lost */
	if (irqsrc &  XCANPS_IXR_ARBLST_MASK)
		proc_arbitrationlost[minor]++;

	/*========== error status */
	if (irqsrc & (
		  XCANPS_IXR_BSOFF_MASK
		| XCANPS_IXR_ERROR_MASK
		    )) {
		int status;

		/* ESTAT  bits in SR Register always reflect CAN status
		 *
		 * XCANPS_SR_ESTAT_MASK
		 * XCANPS_SR_ESTAT_SHIFT
		 *
		 *
		 *
		 *
		 * */
		status = CAN_INL(minor, sr) >> XCANPS_SR_ESTAT_SHIFT;
		/*
		 *  00: Indicates Configuration Mode (CONFIG = 1).
		 *			Error State is undefined.
		 *  01: Indicates Error Active State.
		 *  11: Indicates Error Passive State.
		 *  10: Indicates Bus Off State.
		 *
		 */

		pr_info("CAN[%d], error Interrupt ESTAT %d\n",
			minor, status & 0x03);
		CAN_OUTL(minor, icr,
			  XCANPS_IXR_BSOFF_MASK
			| XCANPS_IXR_ERROR_MASK);

	}
	/*========== CAN data overrun interrupt */
	if (irqsrc & XCANPS_IXR_RXOFLW_MASK) {
		/* This bit indicates that a message has been lost.
		 * This condition occurs when a new message is beeing received
		 * and the receive FIFO is full.
		 * This bit can be cleared by writing to the ICR .
		 * This bit is also cleared when a 0 is written
		 * to the CEN bit in the SRR.
		 *
		 * It is very difficult to test this condition.
		 * One way of doing so: delay the isr CAN reception
		 * See above = receive interrupt ===
		 */
		pr_err("CAN[%d]: controller RX FIFO overrun!\n", minor);
		proc_overrun[minor]++;

		/* insert error */
		proc_rxerr[minor]++;
	for (nrx_fifo = 0; nrx_fifo < CAN_MAX_OPEN; nrx_fifo++) {
		/* for every rx fifo */
		if (can_waitflag[minor][nrx_fifo] == 1) {
			/* this FIFO is in use */
			(rx_fifo->data[rx_fifo->head]).flags |= MSG_OVR;
			(rx_fifo->data[rx_fifo->head]).id = 0xFFFFFFFF;
			(rx_fifo->data[rx_fifo->head]).length = 0;
			rx_fifo->status = BUF_OK;
			++(rx_fifo->head);
			rx_fifo->head %= MAX_BUFSIZE;
			if (unlikely(rx_fifo->head == rx_fifo->tail)) {
				pr_err("CAN[%d][%d] RX: FIFO overrun\n",
					minor, nrx_fifo);
				rx_fifo->status = BUF_OVERRUN;
			}
			/* tell someone that there is a new error message */
			wake_up_interruptible(&can_wait[minor][nrx_fifo]);
		}
	    }
	    /* Clear proc_overrun status */
	    CAN_OUTL(minor, icr, XCANPS_IXR_RXOFLW_MASK);
	}

	/* check again for pending interrupts */
	irqsrc = CAN_INL(minor, isr);
	/* pr_info("CAN - ISR ;  isr= 0x%08x\n",  irqsrc); */
#if 0
	irqsrc &=  XCANPS_IXR_TXOK_MASK
		 | XCANPS_IXR_BSOFF_MASK
		 | XCANPS_IXR_RXOK_MASK
		 | XCANPS_IXR_RXOFLW_MASK;
#endif
	} while (irqsrc != 0);

	DBGPRINT(DBG_DATA, (" => leave IRQ[%d]", minor));

/*    board_clear_interrupts(minor); */

#if defined(CONFIG_TIME_MEASURE)
	reset_measure_pin();
#endif

	return IRQ_RETVAL(IRQ_HANDLED);
}
