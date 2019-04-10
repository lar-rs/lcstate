/* allwinnerfuncs - Allwinner A20 CAN hardware depending part of can4linux drivers
*
* can4linux -- LINUX CAN device driver source
* 
* This file is subject to the terms and conditions of the GNU General Public
* License.  See the file "COPYING" in the main directory of this archive
* for more details.
*
* 
* Copyright (c) 2014-2015 Heinz-Juergen Oertel (hj.oertel@t-online.de)
*------------------------------------------------------------------
*
*/

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include "linux/delay.h"
#include "allwinner.h"
#include <linux/sched.h>

static void can_set_normal_mode(int minor);
static void can_set_reset_mode(int minor);

/* timing values 
 in this case we need for all 10 possible timings two bytes each.
 This might be different on some controllers */
u8 can_timing[10][2]={
	{CAN_TIM0_10K,  CAN_TIM1_10K},
	{CAN_TIM0_20K,  CAN_TIM1_20K},
	{CAN_TIM0_50K,  CAN_TIM1_50K},
	{CAN_TIM0_100K, CAN_TIM1_100K},
	{CAN_TIM0_125K, CAN_TIM1_125K},
	{CAN_TIM0_250K, CAN_TIM1_250K},
	{CAN_TIM0_500K, CAN_TIM1_500K},
	{CAN_TIM0_800K, CAN_TIM1_800K},
	{CAN_TIM0_1000K,CAN_TIM1_1000K}};



/* Board reset
   means the following procedure:
  set Reset Request
  wait for Rest request is changing - used to see if board is available
  initialize board (with values from /proc/sys/Can)
    set output control register
    set timings
    set acceptance mask
*/

static void can_set_normal_mode(int minor)
{
int i;
uint32_t mode = CAN_INL(minor, msel);

        for (i = 0; i < 100; i++) {
                /* check reset bit */
                if ((mode & CAN_MSEL_RST_SEL) == 0) {
                        return;
                }
                /* set chip to normal mode */
                CAN_RESETL(minor, msel, CAN_MSEL_RST_SEL);
                udelay(10);
                mode = CAN_INL(minor, msel);
        }
	pr_err("setting normal mode CAN[%d] failed\n", minor); 
}

static void can_set_reset_mode(int minor)
{
int i;
uint32_t mode = CAN_INL(minor, msel);

	/* setting reset mode can take some time */
	for (i = 0; i < 100; i++) {
                /* check reset bit */
                if (mode & CAN_MSEL_RST_SEL) {
                        return;
                }
                CAN_SETL(minor, msel, CAN_MSEL_RST_SEL);
		udelay(10);
                mode = CAN_INL(minor, msel);
        }
	pr_err("setting reset mode CAN[%d] failed\n", minor); 
}


#ifdef DEBUG
/* when ever we need while debugging some controller status information */
void can_showstat(int minor)
{
	if (proc_dbgmask && (proc_dbgmask & DBG_DATA)) {
		/* log some register contents */
		pr_info("msel      0x%08x\n", CAN_INL(minor, msel));
		pr_info("cmd       0x%08x\n", CAN_INL(minor, cmd));
		pr_info("sta       0x%08x\n", CAN_INL(minor, sta));
		pr_info("interrupt 0x%08x\n", CAN_INL(minor, interrupt));
		pr_info("inten     0x%08x\n", CAN_INL(minor, inten));
		pr_info("bustime   0x%08x\n", CAN_INL(minor, bustime));
		pr_info("tewl      0x%08x\n", CAN_INL(minor, tewl));
		pr_info("errc      0x%08x\n", CAN_INL(minor, errc));
//		pr_info("acccode   0x%08x\n", CAN_INL(minor, acccode));
//		pr_info("accmask   0x%08x\n", CAN_INL(minor, trbuf1));

	}
	return;
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
msg_fifo_t *Fifo;
unsigned long flags;
int rx_fifo = ((struct _instance_data *)(file->private_data))->rx_index;
u32 tmp;

    stat->type = CAN_TYPE_ALLWINNER_CAN;

    stat->baud = proc_baud[minor];
    tmp = CAN_INL(minor, sta);	/* Status register */
    stat->status     =  tmp & 0xFF;
    stat->error_code =  (tmp & CAN_STA_ERR_CODE_MASK) >> CAN_STA_ERR_CODE_POS;

    stat->error_warning_limit = CAN_INL(minor, tewl) & CAN_TEWL_MASK;

    tmp = CAN_INL(minor, errc);	/* read error counter */
    stat->rx_errors = (tmp & CAN_ERRC_RX_CNT_MASK) >> CAN_ERRC_RX_CNT_POS;
    stat->tx_errors = (tmp & CAN_ERRC_TX_CNT_MASK) >> CAN_ERRC_TX_CNT_POS;

    /* Disable CAN (All !!) Interrupts */
    /* !!!!!!!!!!!!!!!!!!!!! */
    /* save_flags(flags); cli(); */
    local_irq_save(flags);

    Fifo = &rx_buf[minor][rx_fifo];
    stat->rx_buffer_size = MAX_BUFSIZE;	/**< size of rx buffer  */
    /* number of messages */
    stat->rx_buffer_used =
    	(Fifo->head < Fifo->tail)
    	? (MAX_BUFSIZE - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    Fifo = &tx_buf[minor];
    stat->tx_buffer_size = MAX_BUFSIZE;	/* size of tx buffer  */
    /* number of messages */
    stat->tx_buffer_used = 
    	(Fifo->head < Fifo->tail)
    	? (MAX_BUFSIZE - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    /* Enable CAN Interrupts */
    /* !!!!!!!!!!!!!!!!!!!!! */
    /* restore_flags(flags); */
    local_irq_restore(flags);
    return 0;
}

int can_chip_reset(int minor)
{

    DBGIN();
	can_set_reset_mode(minor);

	can_set_timing(minor, proc_baud[minor]);
	can_set_mask(minor, proc_acccode[minor], proc_accmask[minor]);

    /* can_dump(minor); */
    DBGOUT();
    return 0;
}


/*
 * Configures bit timing registers directly.
 * Chip must be in configuration mode.
 */
int can_set_btr(int minor, int btr0, int btr1)
{
    (void)minor;
    (void)btr0;
    (void)btr1;

    DBGIN();
    pr_err("direkt setting of bit timing register not yet supported\n");

    DBGOUT();
    return -1;
}


/*
 * Configures bit timing.
 * Chip must be in configuration mode.
 */
int can_set_timing(int minor, int baud)
{
int i = 5;
int custom=0;
int isopen;

    DBGIN();

    isopen = atomic_read(&can_isopen[minor]);
    if ((isopen > 1) && (proc_baud[minor] != baud)) {
	DBGPRINT(DBG_DATA, ("isopen = %d", isopen));
	DBGPRINT(DBG_DATA, ("refused baud[%d]=%d already set to %d",
					minor, baud, proc_baud[minor]));
	return -1;
    }

    DBGPRINT(DBG_DATA, ("baud[%d]=%d", minor, baud));
    switch(baud) {
	case   10: i = 0; break;
	case   20: i = 1; break;
	case   50: i = 2; break;
	case  100: i = 3; break;
	case  125: i = 4; break;
	case  250: i = 5; break;
	case  500: i = 6; break;
	case  800: i = 7; break;
	case 1000: i = 8; break;
	default  : 
		custom=1;
		break;
    }

    /* hardware depending code follows here */

    if (custom) {
	/* set direct register values */
        /* CANout(minor, cantim0, (u8) (baud >> 8) & 0xff); */
        /* CANout(minor, cantim1, (u8) (baud & 0xff )); */
    } else {
	/* use table values, i is index */
	CAN_OUTL(minor, bustime, 
		(can_timing[i][0] + (can_timing[i][1] << 16)));
    }

    DBGOUT();
    return 0;
}

/*
   Reset error Counter information in /proc
   Clear pending Interrupts
   Set Interrupt sources
   Activate CAN

*/
int can_start_chip(int minor)
{
uint32_t irqmask;

	DBGIN();
	proc_rxerr[minor] = proc_txerr[minor] = 0L;



	/* clear interrupts, value not used */
	(void)CAN_INL(minor, interrupt);
	

	/* default interrupt mask setting */
	irqmask = 0
		| CAN_ERROR_PASSIVE_INT_ENABLE
		| CAN_OVERRUN_INT_ENABLE
		| CAN_ERROR_WARN_INT_ENABLE
		| CAN_TRANSMIT_INT_ENABLE
		| CAN_RECEIVE_INT_ENABLE;

	/* extended interrupt mask setting */
	if (errint == 1) {
		irqmask += (
		0
		| CAN_ARBITR_LOST_INT_ENABLE
		| CAN_BUS_ERR_INT_ENABLE);
	}

	// pr_info("Set irqmask register inten to 0x%0x\n", irqmask);
	CAN_SETL(minor, inten, irqmask);
	
	/* Switch to normal mode */
	can_set_normal_mode(minor);
	DBGPRINT(DBG_DATA, ("start msel = 0x%x", CAN_INL(minor, msel)));

	can_showstat(minor);

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
	
	CAN_SETL(minor, inten, 0);
	can_set_reset_mode(minor);


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

 must be done after CMD_STOP (can_stopchip)
 and before CMD_START (can_start_chip)
*/
int can_set_listenonlymode(int minor,
	int arg)	/* true - set Listen Only, false - reset */
{
	DBGIN();
	if (arg) {
		/* set listen only mode */
		CAN_SETL(minor, msel, CAN_MSEL_LST_ONLY_SEL);
	;
	} else {
		/* set active mode */
		CAN_RESETL(minor, msel, CAN_MSEL_LST_ONLY_SEL);
		;
	}

	DBGOUT();
	return 0;
}

/* set Acceptance Code and Mask Registers */
int can_set_mask (int minor, unsigned int code, unsigned int mask)
{
    DBGIN();
    /* set register values */
    CAN_OUTL(minor, acccode, code); 
    CAN_OUTL(minor, accmask, mask); 
    /* put values back in global variables for sysctl */
    proc_acccode[minor] = code;
    proc_accmask[minor] = mask;
    DBGOUT();
    return 0;
}


/*
 * Fill the CAN controller register for the CAN transmit object or FIFO
 */
int can_fill_tx_frame(int minor, canmsg_t *tx)
{
int i = 0;
int ext;
u8 tx2reg;

    ext = (tx->flags & MSG_EXT);	/* read message format */

    if (tx->flags & MSG_CANFD)
	    tx2reg = len2dlc(tx->length) & 0x0f;
    else
	    tx2reg = tx->length & 0x0f; /* limit length in DLC to 4 bit */

    if (tx->flags & MSG_RTR)
	    tx2reg |= CAN_BUF0_RTR_MASK;

    /* fill in message id, message data, .... */

	if (ext) {
		DBGPRINT(DBG_DATA, ("---> send ext message"));
		CAN_OUTL(minor, trbuf0, CAN_BUF0_EFF_MASK + tx2reg);
#if 1
/* the order of the bits in trbuf1 to trbuf4 does not the ones in the SJA100
 * nor in the Allwinner A20 documentation */

		CAN_OUTL(minor, trbuf1, (u8)(tx->id >> 21));
		CAN_OUTL(minor, trbuf2, (u8)(tx->id >> 13));
		CAN_OUTL(minor, trbuf3, (u8)(tx->id >> 5));
		CAN_OUTL(minor, trbuf4, (u8)(tx->id << 3) & 0xff);
#else
		CAN_OUTL(minor, trbuf1, (tx->id >> 3));
		CAN_OUTL(minor, trbuf2, ((tx->id & 0x07) << 5) | ((tx->id >> 24) & 0x1f));
		CAN_OUTL(minor, trbuf3, (tx->id >> 16));
		CAN_OUTL(minor, trbuf4, (tx->id >> 8));
#endif
	} else {
		DBGPRINT(DBG_DATA, ("---> send std message"));
		CAN_OUTL(minor, trbuf0, 0 + tx2reg);

		CAN_OUTL(minor, trbuf1, (u8)((tx->id) >> 3));
		CAN_OUTL(minor, trbuf2, (u8)(tx->id << 5) & 0xe0);
	}

	tx2reg = tx->length & 0x0f;         /* limit length in DLC to 4 bit */
	if (tx2reg > 8)
		tx2reg = 8; /* limit CAN message length */

	{ /* copy data */
	u32 *dptr;
		if (ext) {
			dptr = &((canregs_t *)can_iobase[minor])->trbuf5;
		} else {
			dptr = &((canregs_t *)can_iobase[minor])->trbuf3;
		}


		for (i = 0; i < tx2reg; i++) {
			writel(tx->data[i], dptr);
			dptr++;
		}
	}
	return i;
}

/* 
Single CAN frames or the very first Message are copied into the CAN controller
using this function. After that an transmission request is set in the
CAN controllers command register.
After a successful transmission, an interrupt will be generated,
which will be handled in the CAN ISR CAN_Interrupt()
*/
int can_send_message(int minor, canmsg_t *tx)
{
u32 stat;
int ret;

    DBGIN();

    /* wait for transmission complete, read canstat */
    can_showstat(minor);

    while (!((stat = CAN_INL(minor, sta)) & CAN_STA_TX_RDY))
		cond_resched();

    
    DBGPRINT(DBG_DATA,
		("CAN[%d]: tx.flags=%d tx.id=0x%x tx.length=%d stat=0x%x",
		minor, tx->flags, tx->id, tx->length, stat));

    ret = can_fill_tx_frame(minor, tx);

    /* issue transmission request to the CAN controller */
    CAN_OUTL(minor, cmd, CAN_CMD_TRANS_REQ);

    can_showstat(minor);

    /* 
     * Save last message that was sent.
     * Since can4linux 3.5 multiple processes can access
     * one CAN interface. On a CAN interrupt this message is copied into 
     * the receive queue of each process that opened this same CAN interface.
     */
    memcpy(
	(void *)&last_tx_object[minor],
	(void *)tx,
	sizeof(canmsg_t));

    DBGOUT();
    return ret;
}



/*
 * The plain interrupt
 *
 */

irqreturn_t can_interrupt(int irq, void *dev_id)
{
int minor;
int i;
int rx_fifo;
struct timeval  timestamp;
unsigned long flags;
int ext;			/* flag for extended message format */
uint32_t irqsrc, dummy;
msg_fifo_t   *RxFifo; 
msg_fifo_t   *TxFifo;
#if CAN_USE_FILTER
msg_filter_t *RxPass;
unsigned int id;
#endif 
#if 1
int first;
#endif 
unsigned int ecc;		/* error counter */


	(void)dummy;
	(void)ext;
	(void)flags;

#if CONFIG_TIME_MEASURE
    set_measure_pin();
#endif

    first  = 0;
    irqsrc = 0;


    minor = *(int *)dev_id;
    // pr_info("CAN - ISR ; minor = %d\n", *(int *)dev_id);

    RxFifo = &rx_buf[minor][0]; 
    TxFifo = &tx_buf[minor];
#if CAN_USE_FILTER
    RxPass = &Rx_Filter[minor];
#endif 

    /* read status if CAN has an interrupt pending */
    irqsrc = CAN_INL(minor, interrupt);

    if(irqsrc == 0) {
         /* first call to ISR, it's not for me ! */
#if CONFIG_TIME_MEASURE
	reset_measure_pin();
#endif
	return IRQ_RETVAL(IRQ_NONE);
    }

    /* Whatever interrupt we have, update the tx error counter
     * and rx error counter information in /proc/sys/dev/Can
     */
    dummy = CAN_INL(minor, errc);	/* read error counter */
    proc_txerrcounter[minor] = (dummy & CAN_ERRC_TX_CNT_MASK);
    proc_rxerrcounter[minor] = (dummy & CAN_ERRC_RX_CNT_MASK) >> CAN_ERRC_RX_CNT_POS;

    do {
    /* loop as long as the CAN controller shows interrupts */
    /* can_dump(minor); */
#if defined(DEBUG)
    /* how often do we loop through the ISR ? */
    /* if(first) pr_info("n = %d\n", first); */
    /* we can have a /proc/sys/dev/Can/irqloop
       to store the max counter value
       for debugging purposes to see how heavy the isr is used
       */
	first++;
	if (first > 10) return IRQ_RETVAL(IRQ_HANDLED);
#endif

	get_timestamp(minor, &timestamp);

	for(rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
	    RxFifo = &rx_buf[minor][rx_fifo];

	    RxFifo->data[RxFifo->head].timestamp = timestamp;

	    /* preset flags */
	    (RxFifo->data[RxFifo->head]).flags =
			    (RxFifo->status & BUF_OVERRUN ? MSG_BOVR : 0);
	}


	/*========== receive interrupt */
	if( irqsrc & CAN_RECEIVE_INT ) {
		int length;

	// pr_info(" CAN RX %d\n", minor);
		dummy  = CAN_INL(minor, frameinfo);

		/* ---------- fill frame data ----------------------- */
		/* handle all subscribed rx fifos */

		for (rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
			/* for every rx fifo */

			if (can_waitflag[minor][rx_fifo] == 1) {
				/* this FIFO is in use */
				/* prepare buffer to be used */
				RxFifo = &rx_buf[minor][rx_fifo];

			if (dummy & CAN_BUF0_RTR_MASK)
				(RxFifo->data[RxFifo->head]).flags |= MSG_RTR;

			ext = (dummy & CAN_BUF0_EFF_MASK);
			if (ext) {

#if 0
  /* the following statements were used to get the Id bit allocation
   * used in the trbuf[1-4] registers for the extended Id format */
pr_info(" 1 : %02x\n", (u8)CAN_INL(minor, trbuf1));
pr_info(" 2 : %02x\n", (u8)CAN_INL(minor, trbuf2));
pr_info(" 3 : %02x\n", (u8)CAN_INL(minor, trbuf3));
pr_info(" 4 : %02x\n", (u8)CAN_INL(minor, trbuf4));
#endif
				(RxFifo->data[RxFifo->head]).flags |= MSG_EXT;
				(RxFifo->data[RxFifo->head]).id =
					(
#if 1

/* the order of the bits in trbuf1 to trbuf4 does not the ones in the SJA100
 * nor in the Allwinner A20 documentation */
				  (  ((u8)(CAN_INL(minor, trbuf1))) << 21)
				| (  ((u8)(CAN_INL(minor, trbuf2))) << 13)
				| (  ((u8)(CAN_INL(minor, trbuf3))) << 5)
				| (  (((u8)(CAN_INL(minor, trbuf4))) >> 3) & 0x1f)
#else
				  ( ((u8)(CAN_INL(minor, trbuf1))) << 3)	/* 7:0 -> 10:3  */
				| ( ((u8)(CAN_INL(minor, trbuf2)) & 0xE0) >> 5)	/* 7:5 ->    2:0  */

				| ( ((u8)(CAN_INL(minor, trbuf4))) << 8)	/* 7:3 ->   15:11 */
				| ( ((u8)(CAN_INL(minor, trbuf3))) << 16)	/* 7:0 ->   23:16 */
				| ( ((u8)(CAN_INL(minor, trbuf2)) & 0x1f) << 24 )/* 4:0 ->   28:24 */ 

#endif
				) & CAN_EFF_MASK;

			} else {
				(RxFifo->data[RxFifo->head]).id =
				(
				  ( ((u8)(CAN_INL(minor, trbuf1))) << 3)
				| ( ((u8)(CAN_INL(minor, trbuf2))) >> 5)
				) & CAN_SFF_MASK;
			}
			/* get message length as received in the frame */
			length = dummy  & 0x0f;			/* strip length code */
			(RxFifo->data[RxFifo->head]).length = length;

			/* limit count to 8 bytes for number of data */
			if (length > 8)
				length = 8;

			{
			u32 *dptr;

				if (ext) {
					dptr = &((canregs_t *)can_iobase[minor])->trbuf5;
				} else {
					dptr = &((canregs_t *)can_iobase[minor])->trbuf3;
				}
				for (i = 0; i < length; i++) {
					(RxFifo->data[RxFifo->head]).data[i] =
						(u8)readl(dptr);
					dptr++;
				}
			}
			/* mark just written entry as OK and full */
			RxFifo->status = BUF_OK;
			/* Handle buffer wrap-around */
			++(RxFifo->head);
			RxFifo->head %= MAX_BUFSIZE;
			if (RxFifo->head == RxFifo->tail) {
				pr_err("CAN[%d][%d] RX: FIFO overrun\n",
						minor, rx_fifo);
				RxFifo->status = BUF_OVERRUN;
			}

			/*---------- kick the select() call  -*/
			/* This function will wake up all processes
			that are waiting on this event queue,
			that are in interruptible sleep
			*/
			/* pr_info(" should wake [%d][%d]\n",
				minor, rx_fifo); */
			wake_up_interruptible(&can_wait[minor][rx_fifo]);
			}
		}
		/* ---------- / fill frame data -------------------------------- */

		CAN_OUTL(minor, interrupt, CAN_RECEIVE_INT);
		/* release the CAN controller now */
		CAN_OUTL(minor, cmd, CAN_CMD_REL_RX_BUF);
		if (CAN_INL(minor, sta) & CAN_STA_DATA_OR) {
			pr_err("CAN[%d] Rx: Overrun Status\n", minor);
			CAN_OUTL(minor, cmd,  CAN_CMD_CLR_OR_FLAG);
		}


	}

	/*========== transmit interrupt */
	if( irqsrc & CAN_TRANSMIT_INT ) {
		/* CAN frame successfully sent */
		// pr_info("Transmit Interrupt\n");
		/* reset interrupt request */
		CAN_OUTL(minor, interrupt, CAN_TRANSMIT_INT);

			/* use time stamp sampled with last INT */
			last_tx_object[minor].timestamp = timestamp;

			/* depending on the number of open processes
			* the TX data has to be copied in different
			* rx fifos
			*/
			for (rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
				/* for every rx fifo */
				if (can_waitflag[minor][rx_fifo] == 1) {
					/* this FIFO is in use */
					/* pr_info("self copy to [%d][%d]\n", minor, rx_fifo); */

					/*
					* Don't copy the message in the receive queue
					* of the process that sent the message unless
					* this process requested selfreception.
					*/
					if ((last_tx_object[minor].cob == rx_fifo)
							&& (selfreception[minor][rx_fifo] == 0)) {
						/*
						pr_info("CAN[%d][%d] Don't copy message in my queue\n",
						minor, rx_fifo);
						*/
						continue;
					}

					/* prepare buffer to be used */
					RxFifo = &rx_buf[minor][rx_fifo];

					/*
					prinfo("ISR[%d] dlc= %d flags= 0x%03x\n",
						minor, last_tx_object[minor].length,
						last_tx_object[minor].flags);
					*/

					/* copying into the receive queue is like receiving it directly
					from CAN.
					Take care here of the data bytes length and DLC code CAN FD
					in classic CAN dlc == length
					with CAN FD that is different.
					*/
					memcpy(
					(void *)&RxFifo->data[RxFifo->head],
					(void *)&last_tx_object[minor],
					sizeof(canmsg_t));
					/* correct .length fill to next fitting CAN FD frame length
					RxFifo->data[RxFifo->head].length =
					dlc2len(RxFifo->data[RxFifo->head].length);
					*/

					/* Mark message as 'self sent/received' */
					RxFifo->data[RxFifo->head].flags |= MSG_SELF;

					/* increment write index */
					RxFifo->status = BUF_OK;
					++(RxFifo->head);
					RxFifo->head %= MAX_BUFSIZE;

					if (RxFifo->head == RxFifo->tail) {
					    	pr_err("CAN[%d][%d] RX: FIFO overrun\n",
								minor, rx_fifo);
						RxFifo->status = BUF_OVERRUN;
					}
					/*---------- kick the select() call  -*/
					/* This function will wake up all processes
					that are waiting on this event queue,
					that are in interruptible sleep
					*/
					wake_up_interruptible(&can_wait[minor][rx_fifo]);
				} /* this FIFO is in use */
			} /* end for loop filling all rx-fifos */



	    		/* check for tx fifo emty */
			if (TxFifo->free[TxFifo->tail] == BUF_EMPTY) {
				/* TX FIFO empty, nothing more to sent */
				/* pr_info("TXE\n"); */
				TxFifo->status = BUF_EMPTY;
				TxFifo->active = 0;
				/* This function will wake up all processes
				that are waiting on this event queue,
				that are in interruptible sleep
				*/
				wake_up_interruptible(&canout_wait[minor]);
				goto tx_done;
			}

			local_irq_save(flags);
			/* pr_info("CAN[%d][%d] enter\n", minor, rx_fifo); */

			/* The TX message FIFO contains other CAN frames to be sent
			* The next frame in the FIFO is copied into the last_tx_object
			* and directly into the hardware of the CAN controller
			*/
			memcpy(
				(void *)&last_tx_object[minor],
				(void *)&TxFifo->data[TxFifo->tail],
				sizeof(canmsg_t));

			can_fill_tx_frame(minor, &(TxFifo->data[TxFifo->tail]));



			CAN_OUTL(minor, cmd, CAN_CMD_TRANS_REQ);


			TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
			++(TxFifo->tail);
			TxFifo->tail %= MAX_BUFSIZE;

			/* leave critical section */
			/* pr_info("CAN[%d][%d] leave\n", minor, rx_fifo); */
			local_irq_restore(flags);


	}
tx_done:

	/*========== arbitration lost */
	if( irqsrc &  CAN_ARBITR_LOST_INT) {
	    /* pr_info("Arbitration Lost Interrupt\n"); */
	    proc_arbitrationlost[minor]++; 
	    /* RST 22.10.2015, jService */
	    CAN_OUTL(minor, interrupt, CAN_ARBITR_LOST_INT);
	}

	/*========== error status */
	if( irqsrc & (
	      CAN_ERROR_WARN_INT 
	    | CAN_ERROR_PASSIVE_INT
	    | CAN_BUS_ERR_INT
		)) {

		unsigned int status;
		unsigned int flags = 0;

		CAN_OUTL(minor, interrupt,
		(CAN_ERROR_WARN_INT | CAN_ERROR_PASSIVE_INT | CAN_BUS_ERR_INT));
		// pr_info("error Interrupt\n");
		ecc = (CAN_INL(minor, sta) & 0xFF0000) >> 16;

		(ecc & CAN_STA_ERR_DIR) ? proc_rxerr[minor]++ : proc_txerr[minor]++;

		/* insert error */
		status = CAN_INL(minor, sta);
		if (status & CAN_STA_BUS_STA) {
			flags |= MSG_BUSOFF;
			proc_txerr[minor]++;
			(RxFifo->data[RxFifo->head]).flags |= MSG_BUSOFF;
			/* pr_info(" MSG_BUSOF\n"); */
		}
		if (status & CAN_STA_ERR_STA) {
			flags |= MSG_PASSIVE;
			proc_txerr[minor]++;
			(RxFifo->data[RxFifo->head]).flags |= MSG_PASSIVE;
			/* pr_info(" MSG_PASSIVE\n"); */
		}

		for (rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
			/* for every rx fifo */
			if (can_waitflag[minor][rx_fifo] == 1) {
				/* this FIFO is in use */
				RxFifo = &rx_buf[minor][rx_fifo]; /* prepare buffer to be used */
				(RxFifo->data[RxFifo->head]).flags += flags;
				(RxFifo->data[RxFifo->head]).id = CANDRIVERERROR;
				/* two byte:
				1st byte multiplexor
				2nd byte data
				*/
				(RxFifo->data[RxFifo->head]).length = 2;
				(RxFifo->data[RxFifo->head]).data[0] = ecc;
				(RxFifo->data[RxFifo->head]).data[1] = ecc & 0x1f;
				RxFifo->status = BUF_OK;

				/* handle fifo wrap around */
				++(RxFifo->head);
				RxFifo->head %= MAX_BUFSIZE;
				if (RxFifo->head == RxFifo->tail) {
					pr_info("CAN[%d][%d] RX: FIFO overrun\n", minor, rx_fifo);
					RxFifo->status = BUF_OVERRUN;
				}
				/* tell someone that there is a new error message */
				wake_up_interruptible(&can_wait[minor][rx_fifo]);
			}
		}
	}
	/*========== CAN data overrun interrupt */
	if( irqsrc & CAN_OVERRUN_INT) {
		pr_err("CAN[%d]: controller RX overrun!\n", minor);
		/* R.STUERMER jService 01.10.2015 */
		CAN_OUTL(minor, interrupt, CAN_OVERRUN_INT);
	}

	irqsrc = CAN_INL(minor, interrupt);
	/* pr_info("    end of ISR, irqsrc = %x\n", irqsrc); */
	ecc = (CAN_INL(minor, sta) & 0xFF0000) >> 16;

    } while( irqsrc != 0 );

    DBGPRINT(DBG_DATA, (" => leave IRQ[%d]", minor));

    /*
     * this function is board, not CAN controller specific 
     * and not needed on the BananaPi
     *
     * board_clear_interrupts(minor);
     *
     */

#if CONFIG_TIME_MEASURE
    reset_measure_pin();
#endif

    return IRQ_RETVAL(IRQ_HANDLED);
}
