/* genericfuncs - generic hardware depending part of can4linux drivers
*
* can4linux -- LINUX CAN device driver source
* 
* This file is subject to the terms and conditions of the GNU General Public
* License.  See the file "COPYING" in the main directory of this archive
* for more details.
*
* 
* Copyright (c) 2012-2013 Heinz-Juergen Oertel (hj.oertel@t-online.de)
*------------------------------------------------------------------
*
*/

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include "linux/delay.h"
#include "dcan.h"
#include <linux/sched.h>


/* int	CAN_Open = 0; */

/* timing values 
 in this case we need for all 10 possible timings two bytes each.
 This might be different on some controllers */
u8 CanTiming[10][2]={
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


#ifdef DEBUG
/* when ever we need while debugging some controller status information */
void can_showstat(int board)
{
    if (proc_dbgmask && (proc_dbgmask & DBG_DATA)) {
	/* log some register contents */
	/*
	pr_info(" STAT 0x%x,", CAN_IN(board, status_register));
	*/
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

#if 1
    (void)inode;
    (void)file;
    (void)stat;
    (void)flags;

    (void)minor;
    (void)Fifo;
    (void)rx_fifo;
#else
    stat->type = CAN_TYPE_SJA1000;

    stat->baud = proc_baud[minor];
    stat->status = CANin(minor, canstat);
    stat->error_warning_limit = CANin(minor, errorwarninglimit);
    stat->rx_errors = CANin(minor, rxerror);
    stat->tx_errors = CANin(minor, txerror);
    stat->error_code= CANin(minor, errorcode); /* should reset this register */

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
#endif
    return 0;
}

int can_chip_reset(int minor)
{

    (void)minor;


    DBGIN();


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
        /* CANout(minor,cantim0, (u8) CanTiming[i][0]); */
        /* CANout(minor,cantim1, (u8) CanTiming[i][1]); */
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
    (void)minor;

    DBGIN();
    proc_rxerr[minor] = proc_txerr[minor] = 0L;

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

 must be done after CMD_START (CAN_StopChip)
 and before CMD_START (CAN_StartChip)
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
int can_set_mask (int minor, unsigned int code, unsigned int mask)
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
using this function. After that an transmission request is set in the
CAN controllers command register.
After a succesful transmission, an interrupt will be generated,
which will be handled in the CAN ISR CAN_Interrupt()
*/
int can_send_message(int minor, canmsg_t *tx)
{
int i = 0;

    (void)minor;
    (void)tx;

    DBGIN();

    /* wait for transmission complete, read canstat 
    while ( ! ((stat=CANin(minor, canstat)) & CAN_TRANSMIT_BUFFER_ACCESS)) {

	    #if LINUX_VERSION_CODE >= 131587
	    # if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	    cond_resched();
	    # else
	    if( current->need_resched ) schedule();
	    # endif
	    #else
	    if( need_resched ) schedule();
	    #endif
    }
    */


    /* fill in message id, message data, .... */



    /* issue transmission request to the CAN controller */



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
 * The plain interrupt
 *
 */

irqreturn_t can_interrupt(int irq, void *dev_id)
{
int minor;
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
unsigned int ecc;		/* error counter */


	(void)ecc;
	(void)dummy;
	(void)ext;
	(void)flags;

#if CONFIG_TIME_MEASURE
    set_measure_pin();
#endif

    first  = 0;
    irqsrc = 0;


    minor = *(int *)dev_id;
    /* pr_info("CAN - ISR ; minor = %d\n", *(int *)dev_id); */

    RxFifo = &rx_buf[minor][0]; 
    TxFifo = &tx_buf[minor];
#if CAN_USE_FILTER
    RxPass = &Rx_Filter[minor];
#endif 

    /* read status if CAN has an interrupt pending */
    /*
    irqsrc = CAN_IN(minor, canirq);
    */


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

    /*
    proc_txerrcounter[minor] = CAN_IN(minor, txerror);
    proc_rxerrcounter[minor] = CAN_IN(minor, rxerror);
    */

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

#if 0
	/*========== receive interrupt */
	if( irqsrc & CAN_RECEIVE_INT ) {

	}

	/*========== transmit interrupt */
	if( irqsrc & CAN_TRANSMIT_INT ) {

	    /* check for tx fifo emty */
	    goto tx_done;

	}
tx_done:

	/*========== arbitration lost */
	if( irqsrc &  CAN_ARBITR_LOST_INT) {
	    proc_arbitrationlost[minor]++; 
	}

	/*========== error status */
	if( irqsrc & (
	      CAN_ERROR_WARN_INT 
	    | CAN_ERROR_PASSIVE_INT
	    | CAN_BUS_ERR_INT
		)) {

	}
	/*========== CAN data overrun interrupt */
	if( irqsrc & CAN_OVERRUN_INT ) {

	}
#endif

    } while( irqsrc != 0 );

    DBGPRINT(DBG_DATA, (" => leave IRQ[%d]", minor));

    /*
     * this function is a baord, not CAN controller specific */
    //board_clear_interrupts(minor);

#if CONFIG_TIME_MEASURE
    reset_measure_pin();
#endif

    return IRQ_RETVAL(IRQ_HANDLED);
}
