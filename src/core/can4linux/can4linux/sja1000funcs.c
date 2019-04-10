/* can_sja1000funcs
*
* can4linux -- LINUX CAN device driver source
*
* This file is subject to the terms and conditions of the GNU General Public
* License.  See the file "COPYING" in the main directory of this archive
* for more details.
*
*
* Copyright (c) 2002-2010 oe @ port GmbH Halle/Saale
* (c) 2012-2016 Heinz-Juergen Oertel (hj.oertel@t-online.de)
*------------------------------------------------------------------
*
*--------------------------------------------------------------------------
*
*
* modification history
* --------------------
* since the project is on SourceForge using svn, not longer maintained
*
*
*/


/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include "linux/delay.h"
#include "sja1000.h"
#include <linux/sched.h>

/* int	CAN_Open = 0; */

/* timing values */
u8 CanTiming[10][2] = {
	{CAN_TIM0_10K,   CAN_TIM1_10K},
	{CAN_TIM0_20K,   CAN_TIM1_20K},
	{CAN_TIM0_50K,   CAN_TIM1_50K},
	{CAN_TIM0_100K,  CAN_TIM1_100K},
	{CAN_TIM0_125K,  CAN_TIM1_125K},
	{CAN_TIM0_250K,  CAN_TIM1_250K},
	{CAN_TIM0_500K,  CAN_TIM1_500K},
	{CAN_TIM0_800K,  CAN_TIM1_800K},
	{CAN_TIM0_1000K, CAN_TIM1_1000K} };


#if defined(CAN_INDEXED_PORT_IO) || defined(CAN_INDEXED_MEM_IO)
canregs_t *regbase;
#endif

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
void can_showstat(int board)
{
	if (proc_dbgmask && (proc_dbgmask & DBG_DATA)) {
		pr_info(" MODE 0x%x,", CAN_IN(board, canmode));
		pr_info(" STAT 0x%x,", CAN_IN(board, canstat));
		pr_info(" IRQE 0x%x,", CAN_IN(board, canirq_enable));
		pr_info(" INT 0x%x\n", CAN_IN(board, canirq));
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
msg_fifo_t *Fifo;
unsigned long flags;
int rx_fifo = ((struct _instance_data *)(file->private_data))->rx_index;


	stat->type = CAN_TYPE_SJA1000;

	stat->baud = proc_baud[minor];
	stat->status = CAN_IN(minor, canstat);
	stat->error_warning_limit = CAN_IN(minor, errorwarninglimit);
	stat->rx_errors  = CAN_IN(minor, rxerror);
	stat->tx_errors  = CAN_IN(minor, txerror);
	/* reading should reset this register */
	stat->error_code = CAN_IN(minor, errorcode);

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
u8 status;

	DBGIN();
	DBGPRINT(DBG_DATA, (" INT 0x%x", CAN_IN(minor, canirq)));

	CAN_OUT(minor, canmode, CAN_RESET_REQUEST);



	/* for(i = 0; i < 100; i++) SLOW_DOWN_IO; */
	udelay(10);

	status = CAN_IN(minor, canstat);

	DBGPRINT(DBG_DATA, ("status=0x%x mode=0x%x", status,
	    CAN_IN(minor, canmode)));
#if 0
	if (!(CAN_IN(minor, canmode) & CAN_RESET_REQUEST)) {
		pr_err("ERROR: no board present!\n");
		/* MOD_DEC_USE_COUNT; */
#if defined(PCM3680) || defined(CPC104) || defined(CPC_PCM_104)
		CAN_Release(minor);
#endif
		DBGOUT();
		return -1;
	}
#endif

	DBGPRINT(DBG_DATA,
		("[%d] CAN_mode 0x%x", minor, CAN_IN(minor, canmode)));
	/* select mode: Basic or PeliCAN */
	CAN_OUT(minor, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
	CAN_OUT(minor, canmode, CAN_RESET_REQUEST + CAN_MODE_DEF);

	/* Board specific output control */
	if (proc_outc[minor] == 0)
		proc_outc[minor] = CAN_OUTC_VAL;

	CAN_OUT(minor, canoutc, proc_outc[minor]);

	can_set_timing(minor, proc_baud[minor]);
	can_set_mask(minor, proc_acccode[minor], proc_accmask[minor]);
	DBGPRINT(DBG_DATA,
		("[%d] CAN_mode 0x%x", minor, CAN_IN(minor, canmode)));

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
	DBGIN();
	DBGPRINT(DBG_DATA, ("[%d] btr0=%d, btr1=%d", minor, btr0, btr1));

	/* select mode: Basic or PeliCAN */
	CAN_OUT(minor, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
	CAN_OUT(minor, cantim0, (u8) (btr0 & 0xff));
	CAN_OUT(minor, cantim1, (u8) (btr1 & 0xff));
	DBGPRINT(DBG_DATA,
		("tim0=0x%x tim1=0x%x",
		 CAN_IN(minor, cantim0), CAN_IN(minor, cantim1)));
	DBGOUT();
	return 0;
}


/*
 * Configures bit timing.
 * Chip must be in configuration mode.
 */
int can_set_timing(int minor, int baud)
{
int i = 5;
int custom;
int isopen;

	DBGIN();

	isopen = atomic_read(&can_isopen[minor]);
	if ((isopen > 1) && (proc_baud[minor] != baud)) {
		DBGPRINT(DBG_DATA, ("isopen = %d", isopen));
		DBGPRINT(DBG_DATA, ("refused baud[%d]=%d already set to %d",
						minor, baud, proc_baud[minor]));
	return -1;
	}

	custom = 0;
	DBGPRINT(DBG_DATA, ("baud[%d]=%d", minor, baud));
	switch (baud) {
	case   10:
		i = 0;
		break;
	case   20:
		i = 1;
		break;
	case   50:
		i = 2;
		break;
	case  100:
		i = 3;
		break;
	case  125:
		i = 4;
		break;
	case  250:
		i = 5;
		break;
	case  500:
		i = 6;
		break;
	case  800:
		i = 7;
		break;
	case 1000:
		i = 8;
		break;
	default:
		custom = 1;
		break;
	}
	/* select mode: Basic or PeliCAN */
	CAN_OUT(minor, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
	if (custom) {
		CAN_OUT(minor, cantim0, (u8) (baud >> 8) & 0xff);
		CAN_OUT(minor, cantim1, (u8) (baud & 0xff));
	} else {
		CAN_OUT(minor, cantim0, (u8) CanTiming[i][0]);
		CAN_OUT(minor, cantim1, (u8) CanTiming[i][1]);
	}
	DBGPRINT(DBG_DATA, ("tim0=0x%x tim1=0x%x",
		CAN_IN(minor, cantim0), CAN_IN(minor, cantim1)));

	DBGOUT();
	return 0;
}


int can_start_chip(int minor)
{
int  irqmask;

	DBGIN();
	DBGPRINT(DBG_DATA,
		("[%d] CAN_mode 0x%x", minor, CAN_IN(minor, canmode)));

	proc_rxerr[minor] = proc_txerr[minor] = 0L;
	/*
	CAN_OUT( minor,cancmd, (CAN_RELEASE_RECEIVE_BUFFER
			  | CAN_CLEAR_OVERRUN_STATUS) );
	*/

	udelay(10);
	/* clear interrupts, value not used */
	(void)CAN_IN(minor, canirq);



	/* possible interrupt sources at the sja1000
	IER.7 BEIE Bus Error Interrupt	      CAN_BUS_ERR_INT_ENABLE
	IER.6 ALIE Arbitration Lost Interrupt CAN_ARBITR_LOST_INT_ENABLE
	IER.5 EPIE Error Passive Interrupt    CAN_ERROR_PASSIVE_INT_ENABLE
	IER.4 WUIE Wake-Up Interrupt	      CAN_WAKEUP_INT_ENABLE
	IER.3 DOIE Data Overrun Interrupt     CAN_OVERRUN_INT_ENABLE
	IER.2 EIE Error Warning Interrupt     CAN_ERROR_WARN_INT_ENABLE
	IER.1 TIE Transmit Interrupt	      CAN_TRANSMIT_INT_ENABLE
	IER.0 RIE Receive Interrupt Enable    CAN_RECEIVE_INT_ENABLE
	*/
	/* Interrupts on Rx, TX, any Status change and data overrun
	*
	* Two special diagnosis interrupts
	*   CAN_BUS_ERR_INT_ENABLE
	*   CAN_ARBITR_LOST_INT_ENABLE
	* are only enabled, if the flag is set when loading the driver
	* /sbin/insmod *.ko errint=1
	*/

	/* default interrupt mask setting */
	irqmask = 0
		+ CAN_OVERRUN_INT_ENABLE
		+ CAN_ERROR_WARN_INT_ENABLE
		+ CAN_TRANSMIT_INT_ENABLE
		+ CAN_RECEIVE_INT_ENABLE;

	/* extended interrupt mask setting */
	if (errint == 1) {
		irqmask += (
		  CAN_BUS_ERR_INT_ENABLE
		+ CAN_ARBITR_LOST_INT_ENABLE);
	}

	CAN_SET(minor, canirq_enable, irqmask);

	CAN_RESET(minor, canmode, CAN_RESET_REQUEST);
	DBGPRINT(DBG_DATA, ("start mode=0x%x", CAN_IN(minor, canmode)));

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
	CAN_SET(minor, canmode, CAN_RESET_REQUEST);
	DBGOUT();
	return 0;
}

/* set value of the output control register */
int can_set_mode(int minor, int arg)
{
	DBGIN();
	DBGPRINT(DBG_DATA, ("[%d] outc=0x%x", minor, arg));
	CAN_OUT(minor, canoutc, arg);
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
	if (arg)
		CAN_SET(minor, canmode, CAN_LISTEN_ONLY_MODE);
	else
		CAN_RESET(minor, canmode, CAN_LISTEN_ONLY_MODE);

	DBGOUT();
	return 0;
}

int can_set_mask(int minor, unsigned int code, unsigned int mask)
{
#ifdef CPC_PCI
# define R_OFF 4 /* offset 4 for the EMS CPC-PCI card */
#else
# define R_OFF 1
#endif

	DBGIN();
	DBGPRINT(DBG_DATA, ("[%d] acc=0x%x mask=0x%x",  minor, code, mask));
	CAN_OUT(minor, frameinfo,
			(unsigned char)((unsigned int)(code >> 24) & 0xff));
	CAN_OUT(minor, frame.extframe.canid1,
			(unsigned char)((unsigned int)(code >> 16) & 0xff));
	CAN_OUT(minor, frame.extframe.canid2,
			(unsigned char)((unsigned int)(code >>  8) & 0xff));
	CAN_OUT(minor, frame.extframe.canid3,
			(unsigned char)((unsigned int)(code >>  0) & 0xff));

	CAN_OUT(minor, frame.extframe.canid4,
			(unsigned char)((unsigned int)(mask >> 24) & 0xff));
	CAN_OUT(minor, frame.extframe.canxdata[0 * R_OFF],
			(unsigned char)((unsigned int)(mask >> 16) & 0xff));
	CAN_OUT(minor, frame.extframe.canxdata[1 * R_OFF],
			(unsigned char)((unsigned int)(mask >>  8) & 0xff));
	CAN_OUT(minor, frame.extframe.canxdata[2 * R_OFF],
			(unsigned char)((unsigned int)(mask >>  0) & 0xff));

	/* put values back in global variables for sysctl */
	proc_acccode[minor] = code;
	proc_accmask[minor] = mask;
	DBGOUT();
	return 0;
}


/*
Single CAN frames or the very first Message are copied into the CAN controller
using this function. After that an transmission request is set in the
CAN controllers command register.
After a successful transmission, an interrupt will be generated,
which will be handled in the CAN ISR CAN_Interrupt()

Take care of The MSG_CANFD Flag and treat messages and dlc/length code
different.  The SJA doesn't know anything about longer date then 8 byte,
but copy all bytes according length to the last_tx_object.
*/
int can_send_message(int minor, canmsg_t *tx)
{
int i;
int ext;			/* message format to send */
u8 tx2reg, stat;

	DBGIN();
	i = 0;

	while (!((stat = CAN_IN(minor, canstat)) & CAN_TRANSMIT_BUFFER_ACCESS))
		cond_resched();


	DBGPRINT(DBG_DATA,
		("CAN[%d]: tx.flags=%d tx.id=0x%x tx.length=%d stat=0x%x",
		minor, tx->flags, tx->id, tx->length, stat));

	ext = (tx->flags & MSG_EXT);	/* read message format */

	/* fill the frame info and identifier fields */

	/* with canfd we get the length information
	 * which can be larger than 4 bit, take care
	*/
	if (tx->flags & MSG_CANFD)
		tx2reg = len2dlc(tx->length) & 0x0f;
	else
		tx2reg = tx->length & 0x0f; /* limit length in DLC to 4 bit */

	if (tx->flags & MSG_RTR)
		tx2reg |= CAN_RTR;

	if (ext) {
		DBGPRINT(DBG_DATA, ("---> send ext message"));
		CAN_OUT(minor, frameinfo, CAN_EFF + tx2reg);
		CAN_OUT(minor, frame.extframe.canid1, (u8)(tx->id >> 21));
		CAN_OUT(minor, frame.extframe.canid2, (u8)(tx->id >> 13));
		CAN_OUT(minor, frame.extframe.canid3, (u8)(tx->id >> 5));
		CAN_OUT(minor, frame.extframe.canid4, (u8)(tx->id << 3) & 0xff);
	} else {
		DBGPRINT(DBG_DATA, ("---> send std message"));
		CAN_OUT(minor, frameinfo, CAN_SFF + tx2reg);
		CAN_OUT(minor, frame.stdframe.canid1, (u8)((tx->id) >> 3));
		CAN_OUT(minor, frame.stdframe.canid2, (u8)(tx->id << 5) & 0xe0);
	}

	/* SJA1000 is classic CAN with dlc instead number of bytes.
	dlc is stored correct in the frameinfo register.
	Now restrict the byte count to classic CAN max 8 data bytes.
	But tx->length goes unchanged into last_tx_object.
	*/
	tx2reg = tx->length & 0x0f;         /* limit length in DLC to 4 bit */
	if (tx2reg > 8)
		tx2reg = 8; /* limit CAN message length to 8 */

	/* - fill data ---------------------------------------------------- */
	if (ext) {
		for (i = 0; i < tx2reg; i++) {
			CAN_OUT(minor, frame.extframe.canxdata[R_OFF * i],
					tx->data[i]);
		}
	} else {
		for (i = 0; i < tx2reg; i++) {
			CAN_OUT(minor, frame.stdframe.candata[R_OFF * i],
					tx->data[i]);
		}
	}
	/* - /end --------------------------------------------------------- */
	CAN_OUT(minor, cancmd, CAN_TRANSMISSION_REQUEST);

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
	return i;
}


/*
 * The plain SJA1000 interrupt
 *
 *				RX ISR           TX ISR
 *                              8/0 byte
 *                               _____            _   ___
 *                         _____|     |____   ___| |_|   |__
 *---------------------------------------------------------------------------
 * 1) CPUPentium 75 - 200
 *  75 MHz, 149.91 bogomips
 *  AT-CAN-MINI                 42/27µs            50 µs
 *  CPC-PCI		        35/26µs
 * ---------------------------------------------------------------------------
 * 2) AMD Athlon(tm) Processor 1M
 *    2011.95 bogomips
 *  AT-CAN-MINI  std            24/12µs            ?? µs
 *               ext id           /14µs
 *
 *
 * 1) 1Byte = (42-27)/8 = 1.87 µs
 * 2) 1Byte = (24-12)/8 = 1.5  µs
 *
 *
 *
 * RX Int with to Receive channels:
 * 1)                _____   ___
 *             _____|     |_|   |__
 *                   30    5  20  µs
 *   first ISR normal length,
 *   time between the two ISR -- short
 *   sec. ISR shorter than first, why? it's the same message
 */

irqreturn_t can_interrupt(int irq, void *dev_id)
{
int minor;
int i;
int rx_fifo;
struct timeval  timestamp;
unsigned long flags;
int ext;			/* flag for extended message format */
int irqsrc, dummy;
msg_fifo_t   *RxFifo;
msg_fifo_t   *TxFifo;
#if CAN_USE_FILTER
msg_filter_t *RxPass;
unsigned int id;
#endif
#if 1
int first;
#endif
unsigned int ecc;

#if CONFIG_TIME_MEASURE
	/* outb(0xff, 0x378);   */
	/* set port to high */
	set_measure_pin();
#endif

	minor = *(int *)dev_id;
	/* pr_info("CAN - ISR ; minor = %d\n", *(int *)dev_id); */

	i     = 0;
	first = 0;
	ecc   = 0;


	RxFifo = &rx_buf[minor][0];
	TxFifo = &tx_buf[minor];
#if CAN_USE_FILTER
	RxPass = &Rx_Filter[minor];
#endif

	irqsrc = CAN_IN(minor, canirq);
	if (irqsrc == 0) {
		/* first call to ISR, it's not for me ! */
#if CONFIG_TIME_MEASURE
		reset_measure_pin();
#endif
		return IRQ_RETVAL(IRQ_NONE);
	}
#if defined(CCPC104)
	pc104_irqack();
#endif

	/* Whatever interrupt we have, update the tx error counter
	* and rx error counter information in /proc/sys/dev/Can
	*
	* OR - should we do it in the inner loop ?
	*/

	proc_txerrcounter[minor] = CAN_IN(minor, txerror);
	proc_rxerrcounter[minor] = CAN_IN(minor, rxerror);

	do {
		/* loop as long as the CAN controller shows interrupts */
		DBGPRINT(DBG_DATA, (" => got IRQ[%d]: 0x%0x", minor, irqsrc));
		/* can_dump(minor); */
#if 1
		/* how often do we loop through the ISR ? */
		/* if (first)
			pr_info("n = %d\n", first); */
		first++;
		if (first > 10)
			return IRQ_RETVAL(IRQ_HANDLED);
#endif

		get_timestamp(minor, &timestamp);

		for (rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
			RxFifo = &rx_buf[minor][rx_fifo];

			RxFifo->data[RxFifo->head].timestamp = timestamp;

			/* preset flags */
			(RxFifo->data[RxFifo->head]).flags =
				(RxFifo->status & BUF_OVERRUN ? MSG_BOVR : 0);
		}


		/*========== receive interrupt */
		if (irqsrc & CAN_RECEIVE_INT) {
			int length;

			/* pr_info(" CAN RX %d\n", minor); */
			dummy  = CAN_IN(minor, frameinfo);

			/* ---------- fill frame data ----------------------- */
			/* handle all subscribed rx fifos */

		for (rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
			/* for every rx fifo */

			/*
			pr_info(" used fifos [%d][%d] = %d\n",
				minor, rx_fifo,
				can_waitflag[minor][rx_fifo]);
			*/

			if (can_waitflag[minor][rx_fifo] == 1) {
				/* this FIFO is in use */
				/* prepare buffer to be used */
				RxFifo = &rx_buf[minor][rx_fifo];
				/* pr_info("> filling buffer [%d][%d]\n",
					minor, rx_fifo);  */

			if (dummy & CAN_RTR)
				(RxFifo->data[RxFifo->head]).flags |= MSG_RTR;

			if (dummy & CAN_EFF)
				(RxFifo->data[RxFifo->head]).flags |= MSG_EXT;

			ext = (dummy & CAN_EFF);
			if (ext) {
				(RxFifo->data[RxFifo->head]).id =
				((unsigned int)(CAN_IN(minor, frame.extframe.canid1)) << 21)
				+ ((unsigned int)(CAN_IN(minor, frame.extframe.canid2)) << 13)
				+ ((unsigned int)(CAN_IN(minor, frame.extframe.canid3)) << 5)
				+ ((unsigned int)(CAN_IN(minor, frame.extframe.canid4)) >> 3);
			} else {
				(RxFifo->data[RxFifo->head]).id =
				(
				((unsigned int)(CAN_IN(minor, frame.stdframe.canid1)) << 3)
				+ ((unsigned int)(CAN_IN(minor, frame.stdframe.canid2)) >> 5)
				) & CAN_SFF_MASK;
			}
			/* get message length as received in the frame */
			length = dummy  & 0x0f;			/* strip length code */
			(RxFifo->data[RxFifo->head]).length = length;

			/* limit count to 8 bytes for number of data */
			if (length > 8)
				length = 8;

			for (i = 0; i < length; i++) {
				if (ext) {
					(RxFifo->data[RxFifo->head]).data[i] =
					CAN_IN(minor, frame.extframe.canxdata[R_OFF * i]);
				} else {
					(RxFifo->data[RxFifo->head]).data[i] =
					CAN_IN(minor, frame.stdframe.candata[R_OFF * i]);
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

		/* release the CAN controller now */
		CAN_OUT(minor, cancmd, CAN_RELEASE_RECEIVE_BUFFER);
		if (CAN_IN(minor, canstat) & CAN_DATA_OVERRUN) {
			pr_err("CAN[%d] Rx: Overrun Status\n", minor);
			CAN_OUT(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS);
		}

		}

		/*========== transmit interrupt */
		if (irqsrc & CAN_TRANSMIT_INT) {
			/* CAN frame successfully sent */
			u8 tx2reg;
			unsigned int id;

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

#ifdef VERBOSE
					pr_info(
						"CAN[%d][%d] Copy message from %d in queue id 0x%lx 0x%x\n",
						minor, rx_fifo,
						last_tx_object[minor].cob,
						last_tx_object[minor].id,
						last_tx_object[minor].data[0]);
#endif
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
					/* correct .length fill to next fitting CAN FD frame length */
					RxFifo->data[RxFifo->head].length =
					dlc2len(RxFifo->data[RxFifo->head].length);

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

			/* enter critical section */
			/* spin_lock_irqsave(&write_splock[minor], flags); */

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

			tx2reg = (TxFifo->data[TxFifo->tail]).length;
			if ((TxFifo->data[TxFifo->tail]).flags & MSG_RTR)
				tx2reg |= CAN_RTR;

			ext = (TxFifo->data[TxFifo->tail]).flags & MSG_EXT;
			id = (TxFifo->data[TxFifo->tail]).id;
			if (ext) {
				DBGPRINT(DBG_DATA, ("---> send ext message"));
				CAN_OUT(minor, frameinfo, CAN_EFF + tx2reg);
				CAN_OUT(minor, frame.extframe.canid1, (u8)(id >> 21));
				CAN_OUT(minor, frame.extframe.canid2, (u8)(id >> 13));
				CAN_OUT(minor, frame.extframe.canid3, (u8)(id >> 5));
				CAN_OUT(minor, frame.extframe.canid4, (u8)(id << 3) & 0xff);
			} else {
				DBGPRINT(DBG_DATA, ("---> send std message"));
				CAN_OUT(minor, frameinfo, CAN_SFF + tx2reg);
				CAN_OUT(minor, frame.stdframe.canid1, (u8)((id) >> 3));
				CAN_OUT(minor, frame.stdframe.canid2, (u8)(id << 5) & 0xe0);
			}

			tx2reg &= 0x0f;		/* restore length only */
			if (tx2reg > 8)
				tx2reg = 8;	/* restrict to 8 */
			if (ext) {
				for (i = 0; i < tx2reg; i++) {
					CAN_OUT(minor, frame.extframe.canxdata[R_OFF * i],
					(TxFifo->data[TxFifo->tail]).data[i]);
				}
			} else {
				for (i = 0; i < tx2reg; i++) {
					CAN_OUT(minor, frame.stdframe.candata[R_OFF * i],
					(TxFifo->data[TxFifo->tail]).data[i]);
				}
			}

			CAN_OUT(minor, cancmd, CAN_TRANSMISSION_REQUEST);

			TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
			++(TxFifo->tail);
			TxFifo->tail %= MAX_BUFSIZE;

			/* leave critical section */
			/* pr_info("CAN[%d][%d] leave\n", minor, rx_fifo); */
			local_irq_restore(flags);
			/* spin_unlock_irqrestore(&write_splock[minor], flags); */

		}
tx_done:

		/*========== arbitration lost */
		if (irqsrc &  CAN_ARBITR_LOST_INT)
			proc_arbitrationlost[minor]++;

		/*========== error status */
		if (irqsrc & (
			CAN_ERROR_WARN_INT
			| CAN_ERROR_PASSIVE_INT
			| CAN_BUS_ERR_INT
			)) {

			/*
			This bit is set when the CAN controller detects
			an error on the CAN-bus.
			If a bus error occurs, the corresponding bus error interrupt
			is always forced, if enabled. At the same time, the current
			position of the bit stream processor is captured into the
			error code capture register. The content within this register
			is fixed until the users software has read out its content
			once. The capture mechanism is then activated again.

			The corresponding interrupt flag located in the interrupt
			register is cleared during the read access to the interrupt
			register. A new bus error interrupt is not possible until the
			capture register is read out once.

			See NXP application note AN97076, table 6 and 7
			*/
			unsigned int status;
			unsigned int flags = 0;

			ecc = CAN_IN(minor, errorcode);

#if 0
			pr_info("CAN[%d]: error interrupt! ecc 0x%0X\n", minor, ecc);
			pr_info("CAN error: %s, code 0x%x, seg 0x%x\n  ",
			(ecc & (1<<5)) ? "RX" : "TX",
			ecc >> 6,
			ecc & 0x1f);
			switch (ecc >> 6) {
			case 0:
				pr_info("bit error, ");
				break;
			case 1:
				pr_info("form error, ");
				break;
			case 2:
				pr_info("stuff error, ");
				break;
			case 3:
				pr_info("other error, ");
				break;
			}
			switch (ecc & CAN_ECC_SEGMENT_MASK) {
			case 3:
				pr_info("start of frame\n");
				break;
			case 2:
				pr_info("id.28 to id.21\n");
				break;
			case 6:
				pr_info("id.20 to id.18\n");
				break;
			case 4:
				pr_info("bit SRTR\n");
				break;
			case 5:
				pr_info("bit IDE\n");
				break;
			case 7:
				pr_info("id.17 to id.13\n");
				break;
			case 0x0f:
				pr_info("id.12 to id.5\n");
				break;
			case 0x0e:
				pr_info("id.4 to id.0\n");
				break;
			case 0x0c:
				pr_info("bit RTR\n");
				break;
			case 0x0d:
				pr_info("reserverd bit 1\n");
				break;
			case 9:
				pr_info("reserverd bit 0\n");
				break;
			case 0x0b:
				pr_info("data length code\n");
				break;
			case 0x0a:
				pr_info("data field\n");
				break;
			case 8:
				pr_info("crc sequence\n");
				break;
			case 0x18:
				pr_info("crc delimiter\n");
				break;
			case 0x19:
				pr_info("ack slot\n");
				break;
			case 0x1b:
				pr_info("ack delimiter\n");
				break;
			case 0x1a:
				pr_info("end of frame\n");
				break;
			case 0x12:
				pr_info("intermission\n");
				break;
			case 0x11:
				pr_info("active error flag\n");
				break;
			case 0x16:
				pr_info("passiver error flag\n");
				break;
			case 0x13:
				pr_info("tolerate dominant bits\n");
				break;
			case 0x17:
				pr_info("error delimiter\n");
				break;
			case 0x1c:
				pr_info("overload flag\n");
				break;
			default:
				pr_info("??\n");
				break;
			}
#endif

			(ecc & CAN_ECC_DIRECTION_MASK) ? proc_rxerr[minor]++ : proc_txerr[minor]++;

			/* insert error */
			status = CAN_IN(minor, canstat);
			if (status & CAN_BUS_STATUS) {
				flags |= MSG_BUSOFF;
				proc_txerr[minor]++;
				(RxFifo->data[RxFifo->head]).flags |= MSG_BUSOFF;
				/* pr_info(" MSG_BUSOF\n"); */
			}
			if (status & CAN_ERROR_STATUS) {
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
		if (irqsrc & CAN_OVERRUN_INT) {
			int status;
			pr_err("CAN[%d]: controller overrun!\n", minor);
			proc_overrun[minor]++;

			/* insert error */
			status = CAN_IN(minor, canstat);
			proc_rxerr[minor]++;

			for (rx_fifo = 0; rx_fifo < CAN_MAX_OPEN; rx_fifo++) {
				/* for every rx fifo */
				if (can_waitflag[minor][rx_fifo] == 1) {
					/* this FIFO is in use */
					if (status & CAN_DATA_OVERRUN)
						(RxFifo->data[RxFifo->head]).flags += MSG_OVR;

					(RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
					(RxFifo->data[RxFifo->head]).length = 0;
					RxFifo->status = BUF_OK;
					++(RxFifo->head);
					RxFifo->head %= MAX_BUFSIZE;
					if (RxFifo->head == RxFifo->tail) {
						pr_err("CAN[%d][%d] RX: FIFO overrun\n",
								minor, rx_fifo);
						RxFifo->status = BUF_OVERRUN;
					}
					/* tell someone that there is a new error message */
					wake_up_interruptible(&can_wait[minor][rx_fifo]);
				}
			}

			CAN_OUT(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS);
		}
		irqsrc = CAN_IN(minor, canirq);
		ecc = CAN_IN(minor, errorcode);

	} while (irqsrc != 0);

	DBGPRINT(DBG_DATA, (" => leave IRQ[%d]", minor));

	/*
	EMS macht es anders.
	Tritt ein Interrupt auf, werden alle möglichen
	CAN controller abgerappelt
	und erst zum Schluss board_clear_interrupts() aufgerufen
	*/
	board_clear_interrupts(minor);


#if CONFIG_TIME_MEASURE
	/* outb(0x00, 0x378);   */
	reset_measure_pin();
#endif

	return IRQ_RETVAL(IRQ_HANDLED);
}
