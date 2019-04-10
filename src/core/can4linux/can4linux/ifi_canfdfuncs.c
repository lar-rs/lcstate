/* ifi_canfdfuncs - generic hardware depending part of can4linux drivers
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
#include "ifi_canfd.h"
#include <linux/delay.h>
#include <linux/sched.h>

/* show compile parameters, defined in ixxat_pci.c */
extern void ifi_canfd_parshow(int minor);

/* int	CAN_Open = 0; */

/* timing values 
 in this case we need for all 10 possible timings one u32 each.
 This might be different on some controllers
 Beginning with the CAN FD support, multiples of the arbitration bit rate are needed
 *1, *2, *3, *4 *8 
 PlugFest Test is done using - Arbitration Phase Bit Rate  0.500 Mbit/s
- Data Phase Bit Rates
	* 1.000 Mbit/s
	* 2.000 Mbit/s
	* 4.000 Mbit/s
	* 5.000 Mbit/s
	* 6.667 Mbit/s
	* 8.000 Mbit/s
	* 10.00 Mbit/s
 */
#define ATIMINGS 10
#define DTIMINGS  5
u32 can_timing[ATIMINGS][DTIMINGS] = {
	/* arbitration rate	*1	*2	*4	*8 	     */
    	{ CAN_TIME_10K,	   0, 0, 0, 0},		
	{ CAN_TIME_20K,	   0, 0, 0, 0},
	{ CAN_TIME_50K,	   CAN_FTIME_50K,   CAN_FTIME_100K,  CAN_FTIME_200K,   CAN_FTIME_400K},
	{ CAN_TIME_100K,   CAN_FTIME_100K,  CAN_FTIME_200K,  CAN_FTIME_400K,   CAN_FTIME_800K},
	{ CAN_TIME_125K,   CAN_FTIME_125K,  CAN_FTIME_250K,  CAN_FTIME_500K,   CAN_FTIME_1000K},
	{ CAN_TIME_250K,   CAN_FTIME_250K,  CAN_FTIME_500K,  CAN_FTIME_1000K,  CAN_FTIME_2000K},
	{ CAN_TIME_500K,   CAN_FTIME_500K,  CAN_FTIME_1000K, CAN_FTIME_2000K,  CAN_FTIME_4000K},
	{ CAN_TIME_800K,   0, 0, 0, 0},
	{ CAN_TIME_1000K,  CAN_FTIME_1000K, CAN_FTIME_2000K, CAN_FTIME_4000K,  CAN_FTIME_8000K}
	};

/* supported with the new IFI CAN FD:
Document changed February 20 2015, Core Version: 020F5B14
IFI Called it 7_9_8_8_bit
*/
u32 can_timing_iso[ATIMINGS][DTIMINGS] = {
	/* arbitration rate	*1	*2	*4	*8 	     */
    	{ CAN_TIMEI_10K,	   0, 0, 0, 0},		
	{ CAN_TIMEI_20K,	   0, 0, 0, 0},
	{ CAN_TIMEI_50K,    CAN_FTIMEI_50K,   CAN_FTIMEI_100K,  CAN_FTIMEI_200K,   CAN_FTIMEI_400K},
	{ CAN_TIMEI_100K,   CAN_FTIMEI_100K,  CAN_FTIMEI_200K,  CAN_FTIMEI_400K,   CAN_FTIMEI_800K},
	{ CAN_TIMEI_125K,   CAN_FTIMEI_125K,  CAN_FTIMEI_250K,  CAN_FTIMEI_500K,   CAN_FTIMEI_1000K},
	{ CAN_TIMEI_250K,   CAN_FTIMEI_250K,  CAN_FTIMEI_500K,  CAN_FTIMEI_1000K,  CAN_FTIMEI_2000K},
	{ CAN_TIMEI_500K,   CAN_FTIMEI_500K,  CAN_FTIMEI_1000K, CAN_FTIMEI_2000K,  CAN_FTIMEI_4000K},
	{ CAN_TIMEI_800K,   0, 0, 0, 0},
	{ CAN_TIMEI_1000K,  CAN_FTIMEI_1000K, CAN_FTIMEI_2000K, CAN_FTIMEI_4000K,  CAN_FTIMEI_8000K}
	};


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
u32 tmp;
    if (proc_dbgmask && (proc_dbgmask & DBG_DATA)) {
	/* log some register contents */
	tmp = CAN_INL(board, stcmd);
	pr_info(" STAT 0x%08x\n", tmp);
	if (tmp & IFI_CANFD_STCMD_ENA_MSK)
		printk(" CAN enabled, ");
	else
		printk(" CAN disabled, ");

	if (tmp & IFI_CANFD_STCMD_SEACTIV_MSK)
		printk(" CAN error active, ");
	if (tmp & IFI_CANFD_STCMD_SEPASS_MSK)
		printk(" CAN error passive, ");
	if (tmp & IFI_CANFD_STCMD_SEBOFF_MSK)
		printk(" CAN is Bus--Off, ");
	if (tmp & IFI_CANFD_STCMD_LOOP_MSK)
		printk(" CAN is in loop back mode, ");
	if (tmp & IFI_CANFD_STCMD_LEXT_MSK)
		printk(" CAN is in extern loop back mode, ");
	if (tmp & IFI_CANFD_STCMD_noCANFD_MSK)
		printk(" CAN is NOT in CAN FD, ");
	else { 
		if (tmp & IFI_CANFD_STCMD_ISO_ENA_MSK)
		    printk(" CAN is in ISO CAN FD mode, ");
		else
		    printk(" CAN is in non-ISO CAN FD mode, ");
	}
	    	


	tmp = CAN_INL(board, rxfifo);
	pr_info(" RX fifo 0x%08x\n", tmp);
	if (tmp & IFI_CANFD_RFIFO_EMPTY_MSK)
		printk(" empty, ");

	tmp = CAN_INL(board, txfifo);
	pr_info(" TX fifo 0x%08x\n", tmp);
	if (tmp & IFI_CANFD_TFIFO_EMPTY_MSK)
		printk(" empty, ");
	if (tmp & IFI_CANFD_TFIFO_FULL_MSK)
		printk(" full, ");
	if (tmp & IFI_CANFD_TFIFO_OVER_MSK)
		printk("overflow , ");
	if (tmp & IFI_CANFD_TFIFO_INVALID_MSK)
		printk(" invalid access, ");
	if (tmp & IFI_CANFD_TFIFO_RES_MSK)
		printk(" reset fifo, ");

	tmp = CAN_INL(board, irqmask);
	pr_info("IRQ Mask           0x%08X\n", tmp);
	tmp = CAN_INL(board, interrupt);
	pr_info("Interrupts pending 0x%08X\n", tmp);

    }
    return;
}
#endif

void can_register_dump(int minor)
{
int i;
	pr_info("Address %p\n", can_iobase[minor]);
	for (i = 0; i < 256; i += 4) { 
		if ((i % 16) == 0) printk("\n%04x: ", i);  
		printk(" 0x%08x", readl((void __iomem *)(can_iobase[minor] + i)));
	}
}

/**
 * write filter
 * ################################################################# */
int ifi_canfd_wr_filter(int minor, u32 filternr, u32 mask, u32 filter)
{
  int rc = -1; 

	if (filternr < 256) {
		writel(mask,
		(void __iomem *)(can_iobase[minor]+(0x800 + filternr * 8))); 
		writel(filter,
		(void __iomem *)(can_iobase[minor]+(0x800 + (filternr * 8) + 4))); 
		rc = 0;
	} else {
		rc = -1;
	}
  return rc;
}

void ifi_canfd_dump_filter(int minor, int number)
{
int i;
pr_info("Base %p, Filter base %p\n", can_iobase[minor],
					(can_iobase[minor]+(0x800)));
	for (i = 0; i < number; i++) {
		pr_info("Filter %3d(%p): mask 0x%08x, id 0x%08x\n", i,
		can_iobase[minor] + 0x800 + 8 * i,
		readl((void __iomem *)(can_iobase[minor]+(0x800 + 8 * i))),
		readl((void __iomem *)(can_iobase[minor]+(0x800 + (8 * i + 4))))
		); 
	}
}

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
u32 tmp;
int rx_fifo = ((struct _instance_data *)(file->private_data))->rx_index;

    stat->type = CAN_TYPE_IFI_CAN_FD;

    stat->baud = proc_baud[minor];
    stat->status = CAN_INL(minor, stcmd);
    stat->error_warning_limit = 96;  /* fixed in IFI CAN ?? */

    tmp = CAN_INL(minor, error);
    stat->rx_errors = (tmp >> 16) & 0xFF;
    stat->tx_errors = tmp & 0xFF;
    stat->error_code= 0; /* should reset this register */

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
int index = 5;		/* index into bit timing table */
int findex = 0;		/* index into fast bit timing table */
int custom=0;
int isopen;
/* got this from 
 * http://stackoverflow.com/questions/1052818/create-a-pointer-to-two-dimensional-array
 */
typedef u32 array_of_DTIMINGS_u32_t[DTIMINGS];
array_of_DTIMINGS_u32_t *timing;

	if(noniso == 1) {
	    timing = can_timing;
	    DBGPRINT(DBG_DATA, ("Using the IFI 4_12_6_6_bit timing\n"));
	}
	else {
	    timing = can_timing_iso;
	    DBGPRINT(DBG_DATA, ("Using the IFI 7_9_8_8_bit timing\n"));
	}

	DBGIN();

	isopen = atomic_read(&can_isopen[minor]);
	if ((isopen > 1) && (proc_baud[minor] != baud)) {
		DBGPRINT(DBG_DATA, ("isopen = %d", isopen));
		DBGPRINT(DBG_DATA, ("refused baud[%d]=%d already set to %d",
						minor, baud, proc_baud[minor]));
		return -1;
	}

	/* CAN timing  */

	DBGPRINT(DBG_DATA, ("baud[%d]=%d", minor, baud));
	switch(baud) {
		case   10:
			index = 0;
			break;
		case   20:
			index = 1;
			break;
		case   50:
			index = 2;
			break;
		case  100:
			index = 3;
			break;
		case  125:
			index = 4;
			break;
		case  250:
			index = 5;
			break;
		case  500:
			index = 6;
			break;
		case  800:
			index = 7;
			break;
		case 1000:
			index = 8;
			break;
		default  : 
			custom=1;
		break;
	}

	/* factor index
	 *   1	   1
	 *   2	   2
	 *   4	   3
	 *   8	   4 */
    findex = proc_speedfactor[minor];
    if ( findex == 4) findex = 3;
    if ( findex == 8) findex = 4;


    /* hardware depending code follows here */

	if (custom) {
		/* FIXME: set direct register values */
	    	pr_err("Custom bit timing setting not implemented yet\n");
	} else {
		/* use table values, index is index 
		 * first for arbitration bit rate */
		CAN_OUTL(minor, time, timing[index][0]);	
		/* and for the data phase */
		// pr_info(" minor %d bittiming index %d and %d\n",
		//		minor, index, findex);


		CAN_OUTL(minor, ftime, timing[index][findex]);
#if 0 /* display CAN timing info */
		pr_info(" %d:%d   --- time = 0x%08x, ftime 0x%08x\n",
			index, findex,
			timing[index][0],
			timing[index][findex]);
#endif
	}

	/*
	TransmitterDelay [11..0]
	: TXdelay in can_clock ticks, set to center of fast bit

	FIXME: calculate the clock tick time,
	proc_tdelay is in ns
	*/
	{
	unsigned tick = (1000000000ul / proc_clock); 
	// pr_info("tick = %d, val = %d\n", tick, proc_tdelay[minor] / tick);
	CAN_OUTL(minor, tdelay, (proc_tdelay[minor] / tick) | IFI_CANFD_TDELAY_ENA_MSK);
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

	/* reset command */
	CAN_OUTL(minor, stcmd, 0xDEADCAFD);   /* Hard reset */
	CAN_OUTL(minor, stcmd, 0x00000000);

	can_set_timing(minor, proc_baud[minor]);
	can_set_mask(minor, proc_acccode[minor], proc_accmask[minor]);
 

	// CAN_OUTL(minor, irqmask, 0);

	CAN_OUTL(minor, rxfifo, IFI_CANFD_RFIFO_RES_MSK);	// reset receive fifo
	CAN_OUTL(minor, rxfifo, 0x00000000);			// enable receive fifo
	CAN_OUTL(minor, txfifo, IFI_CANFD_TFIFO_RES_MSK);	// reset transmit fifo
	CAN_OUTL(minor, txfifo, 0x00000000);			// enable transmit fifo

	CAN_OUTL(minor, repeat, 0x00000000);	// default repeat transmission endless
	CAN_OUTL(minor, suspend, 0x00000000);			// default suspend 0

	CAN_OUTL(minor, irqmask, 
		0
    		| (IFI_CANFD_INT_M_ERR_MSK)
		| (IFI_CANFD_INT_M_OVER_MSK)
		| (IFI_CANFD_INT_M_TFIFO_MSK
			| IFI_CANFD_INT_TEMPTY_MSK)
		| (IFI_CANFD_INT_M_RFIFO_MSK
			| IFI_CANFD_INT_RNEMPTY_MSK)
		);
	CAN_OUTL(minor, interrupt, 0x7fffffff);		// clear all IRQs, don't set bit31 !!

	{
	/* enable CAN core
	 * If CAN FD is enabled, than classic CAN and CAN FD is processd 
 	 * setting the bit IFI_CANFD_STCMD_noCANFD_MSK to 1
	 * disables all CAN-FD protocol handling,
	 * IP-core workes in CAN-basic modes only.
	 * Setting bit IFI_CANFD_STCMD_ISO_ENA_MSK to 1
	 * enables ISO standard protocol handling,
	 *    0: use non-ISO, BOSCH specification 1.0 April 2012
	 */
	u32 cmd = IFI_CANFD_STCMD_ENA_MSK
		+ IFI_CANFD_STCMD_NM_MSK;
	if(!noniso)
	    cmd += IFI_CANFD_STCMD_ISO_ENA_MSK; 
		// += IFI_CANFD_STCMD_noCANFD_MSK);	// disable CAN FD
		//
	CAN_OUTL(minor, stcmd, cmd);
	}

	if (proc_dbgmask && (proc_dbgmask & DBG_DATA)) {
	    can_showstat(minor);
	    /* show max 1 ... n filter entries */
	    ifi_canfd_dump_filter(minor, 4 /* n */);
	}

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
	/* reset command */
	CAN_OUTL(minor, stcmd, 0xDEADCAFD);   /* Hard reset */
	/* Reset all interrupt masks */
	CAN_OUTL(minor, irqmask, 
		0
    		+ IFI_CANFD_INT_M_ERR_MSK
		+ IFI_CANFD_INT_M_OVER_MSK
		+ IFI_CANFD_INT_M_TFIFO_MSK
		+ IFI_CANFD_INT_M_RFIFO_MSK);
	/* clear all IRQs, don't set bit31 !!  */
	CAN_OUTL(minor, interrupt, 0x7fffffff);

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

 Must be done after CMD_START (CAN_StopChip)
 and before CMD_START (CAN_StartChip)

The IFI CAN IP controls this behaviour with the  "Status and Command Register"
"stcmd"

Bus monitoring mode, Silent Mode 3.3.1
set this Bit to 1 to enable that mode
*/
int can_set_listenonlymode(int minor,
	int arg)	/* true - set Listen Only, false - reset */
{
u32 tmp;
    (void)minor;
    (void)arg;

    DBGIN();
	tmp = CAN_INL(minor, stcmd);
    if (arg) {
	/* set listen only mode */
		tmp |= IFI_CANFD_STCMD_M331_MSK;
    } else {
	/* set active mode */
		tmp &= ~IFI_CANFD_STCMD_M331_MSK;
    }
	CAN_OUTL(minor, stcmd, tmp);

    DBGOUT();
    return 0;
}


/* set Acceptance Code and Mask Registers */
int can_set_mask (int minor, unsigned int code, unsigned int mask)
{
    (void)code;
    (void)mask;

	DBGIN();
	/* set register values */

	/* put values back in global variables for sysctl */
	proc_acccode[minor] = code;
	proc_accmask[minor] = mask;

	/* for now: receive all classic framse std id */
	ifi_canfd_wr_filter(minor, 0, 0xA0000000ul, 0x80000000ul);
	/* for now: receive all classic framse extd id */
	ifi_canfd_wr_filter(minor, 1, 
	    IFI_CANFD_FILTER_VALID_MSK + IFI_CANFD_FILTER_EXT_MSK,
	    IFI_CANFD_FILTER_VALID_MSK + IFI_CANFD_FILTER_EXT_MSK);
		
	ifi_canfd_wr_filter(minor, 2, 
	    IFI_CANFD_FILTER_VALID_MSK + IFI_CANFD_FILTER_EXT_MSK
	    + IFI_CANFD_FILTER_CANFD_MSK,
	    IFI_CANFD_FILTER_VALID_MSK + IFI_CANFD_FILTER_EXT_MSK
	    + IFI_CANFD_FILTER_CANFD_MSK);

	DBGOUT();
	return 0;
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
int i, j;
u32 fifostatus;
u32 dlc;
u32 id;

	DBGIN();

	/* wait for free space in the TX FIFO */
	fifostatus = CAN_INL(minor, txfifo);
	while( fifostatus & IFI_CANFD_TFIFO_FULL_MSK) 
		cond_resched();

	DBGPRINT(DBG_DATA,
		("CAN[%d]: tx.flags=%d tx.id=0x%lx tx.length=%d stat=0x%x",
		minor, tx->flags, (long unsigned int)tx->id,
		tx->length, fifostatus));

	DBGPRINT(DBG_DATA, (" Switching Bit Timing factor %d\n",
					proc_speedfactor[minor]));

	/* with canfd we get the length information
	 * which can be larger than 4 bit, take care
	*/
	if (tx->flags & MSG_CANFD) {
		dlc = len2dlc(tx->length);
		dlc = dlc & IFI_CANFD_DLC_DLC_MSK; /* limit length in DLC to 4 bit */
		dlc |= (IFI_CANFD_DLC_EDL_MSK);
		if (proc_speedfactor[minor] > 1) {
			dlc |= IFI_CANFD_DLC_BRS_MSK;
		}
	} else 
	    dlc = tx->length;

	if (tx->flags & MSG_RTR)
		dlc |= IFI_CANFD_DLC_RTR_MSK;

	/* fill in message id, message data, .... */
	CAN_OUTL(minor, tx_suspend, 0);
	CAN_OUTL(minor, tx_repeat, 0);		/* endless */


	CAN_OUTL(minor, tx_dlc, dlc);
	id = tx->id;
	if (tx->flags & MSG_EXT) {	/* read message format */
		/* IFI legacy needs bit shifting for extended frame format */
		u32 tmp;
		tmp =  ((id & 0x3FFFF) << IFI_CANFD_IDX_SHIFT_L)
		    + ((id & 0x1FFC0000) >> IFI_CANFD_IDX_SHIFT_H);
		id = tmp | IFI_CANFD_IDX_ON_MSK;
	}
	// ifi_canfd_parshow(minor);
	CAN_OUTL(minor, tx_id, id);

	DBGPRINT(DBG_DATA, (" --> CAN-%s dlc is %d\n",
		(tx->flags & MSG_CANFD)
		? "FD" : "classic",	dlc & IFI_CANFD_DLC_DLC_MSK));

	for (i = 0, j = 0; i < (((tx->length -1) / 4) + 1); i++, j+=4) {
		CAN_OUTL(minor, tx_data[i], 
			(tx->data[j] 
			+ (tx->data[j + 1] << 8)
			+ (tx->data[j + 2] << 16)
			+ (tx->data[j + 3] << 24))
			);
	}

	/* issue transmission request to the CAN controller */
	CAN_OUTL(minor, txfifo, IFI_CANFD_TFIFO_ADD_MSK);

	/* 
	* Save last message that was sent.
	* Multiple processes can access one CAN interface.
	* On a CAN interrupt this message is copied into 
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

u32 board_get_irq_status(int minor);


irqreturn_t can_interrupt(int irq, void *dev_id)
{
int minor;
int rx_fifo;
struct timeval  timestamp;
unsigned long flags;
int ext;			/* flag for extended message format */
int irqsrc;
msg_fifo_t   *RxFifo; 
msg_fifo_t   *TxFifo;
#if CAN_USE_FILTER
msg_filter_t *RxPass;
u32 id;
#endif 
#if 1
int first;
#endif 

	// (void)dummy;
	// (void)flags;

#if CONFIG_TIME_MEASURE
    set_measure_pin();
#endif

	first  = 0;
	irqsrc = 0;
	minor = *(int *)dev_id;
	RxFifo = &rx_buf[minor][0]; 
	TxFifo = &tx_buf[minor];
#if CAN_USE_FILTER
	RxPass = &Rx_Filter[minor];
#endif 



#if 0
    {
u32 rxstatus = CAN_INL(minor, rxfifo);
	if (rxstatus & IFI_CANFD_RFIFO_EMPTY_MSK) {
		pr_info("rxfifo empty\n");
	} else {
		u32 tmp;
		tmp = CAN_INL(minor, rx_dlc); 
		tmp = CAN_INL(minor, rx_id);
		CAN_OUTL(minor, rxfifo,  IFI_CANFD_RFIFO_DEC_MSK);
	}
    }
#endif

	if (board_get_irq_status(minor) == 0) {
	    return IRQ_RETVAL(IRQ_NONE);
	}
	/* read status if CAN has an interrupt pending */
	irqsrc = CAN_INL(minor, interrupt);

	if (irqsrc == 0) {
	     /* first call to ISR, it's not for me ! */
#if CONFIG_TIME_MEASURE
		reset_measure_pin();
#endif
		//board_clear_interrupts(minor);
		return IRQ_RETVAL(IRQ_NONE);
	}

#if 0
	pr_info("CAN[%d] - ISR\n", *(int *)dev_id);
	pr_info("Interrupt      0x%08x\n", irqsrc);
	pr_info("Interrupt mask 0x%08x\n", CAN_INL(minor, irqmask));
#endif
	CAN_OUTL(minor, interrupt, 0x7fffffff); // clear all IRQs, don't set bit31 !!


    /* Whatever interrupt we have, update the tx error counter
     * and rx error counter information in /proc/sys/dev/Can
     */
    {
    u32 tmp;
    tmp = CAN_INL(minor, error);
    proc_rxerrcounter[minor] = (tmp >> 16) & 0xFF;
    proc_txerrcounter[minor] = tmp & 0xFF;
    }

    do {
    /* loop as long as the CAN controller shows interrupts */
    /* can_dump(minor); */
#if defined(DEBUG)
    /* how often do we loop through the ISR ? */
    if(first) pr_info("n = %d\n", first);
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
	if( irqsrc & (IFI_CANFD_INT_RNEMPTY_MSK | IFI_CANFD_INT_RPNEMPTY_MSK)) {
		unsigned int length;
		int dlc, flags, edl;	/*dlc code, canmsg_t flags, extented data length flag */  
		u32 id;
		// pr_info("Got RX %d interrupt\n", minor);


		dlc = CAN_INL(minor, rx_dlc); 
		edl = 0;
		/* get message length as received in the frame */
		DBGPRINT(DBG_DATA, (" received dlc: 0x%08x, from mask %d\n",
			    dlc, (dlc & IFI_CANFD_DLC_OBJ_MSK) >> IFI_CANFD_DLC_OBJ_OFST));
		id  = CAN_INL(minor, rx_id);
		flags = 0;
		if (dlc & IFI_CANFD_DLC_RTR_MSK)
			flags |= MSG_RTR;
		if (dlc & IFI_CANFD_DLC_EDL_MSK) {
			flags |= MSG_CANFD;
			edl = 1;
			// pr_info("received CAN EDL frame\n");
		}
		if(edl) { /* only check in FD mode */
		    if (dlc & IFI_CANFD_DLC_BRS_MSK)
			    // pr_info("received CAN FD BRS frame\n");
			    flags |= MSG_RBRS;
		    if (dlc & IFI_CANFD_DLC_ESI_MSK)
			    flags |= MSG_RESI;
		}

		/* limit count to to the 4 bit dlc field*/
		dlc = (dlc & IFI_CANFD_DLC_DLC_MSK) >> IFI_CANFD_DLC_DLC_OFST;

		/* all flags contained in rx_id are evaluated, now reduce to id
		 * and for extended Id frames
		 * in IFI Legacy mode do the bit shifting */
		if (id & IFI_CANFD_IDX_ON_MSK) {
			u32 tmp;
			id &= IFI_CANFD_IDX_MSK;
			tmp = ((id & IFI_CANFD_IDX_ID_MSK) << IFI_CANFD_IDX_SHIFT_H)
			    + ((id & IFI_CANFD_IDX_IDX_MSK) >> IFI_CANFD_IDX_SHIFT_L);
			id = tmp & IFI_CANFD_IDX_MSK;
			flags |= MSG_EXT;
		} else 
			id &= IFI_CANFD_IDX_ID_MSK;


		if (edl)
			length = dlc2len(dlc);
		else {
			if ((dlc > 8) && !edl)
				length = 8;
			else
				length = dlc;
		}
		/* length is now length of data in byte */


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
			(RxFifo->data[RxFifo->head]).flags = flags;
			(RxFifo->data[RxFifo->head]).id = id;
			(RxFifo->data[RxFifo->head]).length = dlc;
#if 0
			if ((dlc > 8) && !edl)
				length = 8;
			else
				length = dlc;

			if (edl)
			    // length = adjustlength(dlc);   /* dlc to length */
			    length = dlc2len(dlc);   /* dlc to length */
#endif

			if (length > 0)  {
				int i, j;
				for (i = 0, j = 0; i < (((length -1) / 4) + 1); i++, j+=4) {
					u32 tmp = CAN_INL(minor, rx_data[i]);
					(RxFifo->data[RxFifo->head]).data[j]     = (tmp & 0xff);
					(RxFifo->data[RxFifo->head]).data[j + 1] = (tmp & 0xff00) >> 8;
					(RxFifo->data[RxFifo->head]).data[j + 2] = (tmp & 0xff0000) >> 16;
					(RxFifo->data[RxFifo->head]).data[j + 3] = (tmp & 0xff000000) >> 24;
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
		CAN_OUTL(minor, rxfifo,  IFI_CANFD_RFIFO_DEC_MSK);
		CAN_OUTL(minor, interrupt, (IFI_CANFD_INT_RNEMPTY_MSK | IFI_CANFD_INT_RPNEMPTY_MSK     ));
	}

	/*========== transmit interrupt */
	if( irqsrc & (IFI_CANFD_INT_TREMOVE_MSK | IFI_CANFD_INT_TEMPTY_MSK)) {
		/* CAN frame successfully sent */
		unsigned int id;
		unsigned len;
		unsigned dlc;
		int i, j;

		// pr_info("Got TX int\n");

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
						len2dlc(RxFifo->data[RxFifo->head].length);
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

		flags = (TxFifo->data[TxFifo->tail]).flags;
		ext = flags & MSG_EXT;
		id = (TxFifo->data[TxFifo->tail]).id;
		len = (TxFifo->data[TxFifo->tail]).length;

		/* with canfd we get the length information
		 * which can be larger than 4 bit, take care
		*/

		if (flags & MSG_CANFD) {
			dlc = len2dlc(len);
			dlc = dlc & IFI_CANFD_DLC_DLC_MSK; /* limit length in DLC to 4 bit */
			dlc |= (IFI_CANFD_DLC_EDL_MSK);
			if (proc_speedfactor[minor] > 1) {
				dlc |= IFI_CANFD_DLC_BRS_MSK;
			}
		} else 
			dlc = len & IFI_CANFD_DLC_DLC_MSK;;

		if (flags & MSG_RTR)
			dlc |= IFI_CANFD_DLC_RTR_MSK;

		/* fill in message id, message data, .... */
		CAN_OUTL(minor, tx_suspend, 0);
		CAN_OUTL(minor, tx_repeat, 0);		/* endless */

		CAN_OUTL(minor, tx_dlc, dlc);

		if (flags & MSG_EXT) {	/* read message format */
			/* IFI legacy needs bit shifting for extended frame format */
			u32 tmp;
			tmp =  ((id & 0x3FFFF) << IFI_CANFD_IDX_SHIFT_L)
			    + ((id & 0x1FFC0000) >> IFI_CANFD_IDX_SHIFT_H);
			id = tmp | IFI_CANFD_IDX_ON_MSK;
		}
		CAN_OUTL(minor, tx_id, id);

		/* pr_info(" --> %s dlc is %d\n",
			(flags & MSG_CANFD) ? "CAN FD" : "classic", dlc);
		*/
		if(dlc > 0) {
			for (i = 0, j = 0; i < (((dlc -1) / 4) + 1); i++, j+=4) {
				CAN_OUTL(minor, tx_data[i], 
				((TxFifo->data[TxFifo->tail]).data[j])
				+ (((TxFifo->data[TxFifo->tail]).data[j + 1]) << 8)
				+ (((TxFifo->data[TxFifo->tail]).data[j + 2]) << 16)
				+ (((TxFifo->data[TxFifo->tail]).data[j + 3]) << 24) 
				);
			}
		}

		/* issue transmission request to the CAN controller */
		CAN_OUTL(minor, txfifo, IFI_CANFD_TFIFO_ADD_MSK);

		TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
			++(TxFifo->tail);
			TxFifo->tail %= MAX_BUFSIZE;

		/* leave critical section */
		/* pr_info("CAN[%d][%d] leave\n", minor, rx_fifo); */
		local_irq_restore(flags);
		/* spin_unlock_irqrestore(&write_splock[minor], flags); */


    	    CAN_OUTL(minor, interrupt, IFI_CANFD_INT_TREMOVE_MSK | IFI_CANFD_INT_TEMPTY_MSK);

	    /* check for tx fifo emty */
	    goto tx_done;

	}
tx_done:

	/*========== arbitration lost */
	if( irqsrc &  0 /* CAN_ARBITR_LOST_INT */) {
	    proc_arbitrationlost[minor]++; 
	}

	/*========== error status */
	if( irqsrc & ( 0 /*
	      CAN_ERROR_WARN_INT 
	    | CAN_ERROR_PASSIVE_INT
	    | CAN_BUS_ERR_INT */
		)) {

	}
	/*========== CAN data overrun interrupt */
	if( irqsrc & 0 /* CAN_OVERRUN_INT */) {

	}


	/* load irq source again */
	irqsrc = CAN_INL(minor, interrupt);
    } while (irqsrc != 0);
	/* clear all IRQs, don't set bit31 !! */
	CAN_OUTL(minor, interrupt, 0x7fffffff);

	DBGPRINT(DBG_EXIT, (" => leave IRQ[%d]", minor));

	/*
	* this function is board, not CAN controller specific */
	board_clear_interrupts(minor);

#if CONFIG_TIME_MEASURE
	reset_measure_pin();
#endif

	return IRQ_RETVAL(IRQ_HANDLED);
}




