/*
 * can_i82527funcs.c - i82527 related code
 *
 * Original version Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * This software is released under the GPL-License.
 *
 * Modified and extended to support the esd elctronic system
 * design gmbh PC/104-CAN Card (www.esd-electronics.com)
 * by Jean-Jacques Tchouto (tchouto@fokus.fraunhofer.de), 2003
 *
 * Major Refactoring and Integration into can4linux version 3.1 by
 * Henrik W Maier of FOCUS Software Engineering Pty Ltd <www.focus-sw.com>
 *
 * Modified and extended to support the SBS PC7compact DINrail mounted
 * industry PC by FOCUS Software Engineering Pty Ltd <www.focus-sw.com>
 *
 * Updated for 2.6 kernel by Henrik W Maier of FOCUS Software
 * Engineering Pty Ltd <www.focus-sw.com>.
 * Bugfix in CAN_SendMessage, added wake_up_interruptible for irq_write_handler
 * Disabled redundant status change interrupt for RX/TX to decrease int
 * load on CPU.
 * Fixed issue of a second iteration in the ISR when sending because the
 * send interrupt didn't sometimes get acknowledged.
 *
 *
* (c) 2016 Heinz-Juergen Oertel (hj.oertel@t-online.de)
 *
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include "linux/delay.h"
#include "i82527.h"
#include <linux/sched.h>
#include <linux/ioport.h>


#if defined (PC104_200)
inline void CANactivateIRQline(int bd)
{
    unsigned long canIoPort;
    canIoPort = (unsigned long)proc_base[bd];
    outb_p(0x86, canIoPort + 3);
}
#endif

int can_freeirq(int minor, int irq)
{
    DBGIN();
    irq_requested[minor] = 0;
    free_irq(irq, &can_minors[minor]);
    DBGOUT();
    return 0;
}

void exit_board_hw(void)
{
}

/*
 * Timing values
 */
uint8_t iCanTiming[10][2]={
	{CAN_TIM0_10K,  CAN_TIM1_10K},
	{CAN_TIM0_20K,  CAN_TIM1_20K},
	{CAN_TIM0_40K,  CAN_TIM1_40K},
	{CAN_TIM0_50K,  CAN_TIM1_50K},
	{CAN_TIM0_100K, CAN_TIM1_100K},
	{CAN_TIM0_125K, CAN_TIM1_125K},
	{CAN_TIM0_250K, CAN_TIM1_250K},
	{CAN_TIM0_500K, CAN_TIM1_500K},
	{CAN_TIM0_800K, CAN_TIM1_800K},
	{CAN_TIM0_1000K,CAN_TIM1_1000K}};


/*
 * Clear and invalidate message objects
 */
int can_clear_objects(int minor)
{
    int i;
    int id;
    int data;

    DBGIN();

    for (i = 1; i <= 15; i++) {
        CAN_OUT(minor, msgArr[i].messageReg.msgCtrl0Reg,
            INTPD_RES | RXIE_RES | TXIE_RES | MVAL_RES);
        CAN_OUT(minor, msgArr[i].messageReg.msgCtrl1Reg,
            NEWD_RES | MLST_RES | TXRQ_RES | RMPD_RES);
        for (data = 0; data < 8; data++)
            CAN_OUT(minor, msgArr[i].messageReg.dataReg[data], 0);
        for (id = 0; id < 4; id++)
            CAN_OUT(minor, msgArr[i].messageReg.idReg[id], 0);
        CAN_OUT(minor, msgArr[i].messageReg.messageConfigReg, 0);
    }

    DBGOUT();
    return 0;
}


/*
 * Board and chip reset
 *
 * Resets and fully configures the chip. Sets baud rate and masks.
 *
 * Note: The chip remains bus-off. All interrupts remain disabled.
 */
int can_chip_reset(int minor)
{
    DBGIN();

    // Configure cpu interface
    CAN_OUT(minor, cpuInterfaceReg, (iCPU_DMC | iCPU_DSC | iCPU_CEN));

    // Enable configuration and puts chip in bus-off, disable interrupts
    CAN_OUT(minor, controlReg, (iCTL_CCE | iCTL_INI));

    // Set clock out slew rates
    CAN_OUT(minor, clkOutReg, (iCLK_SL1 | iCLK_CD1));

    // Bus configuration
    CAN_OUT(minor, busConfigReg, (iBUS_CBY));

    // Clear interrupts
    CAN_IN(minor, interruptReg);

    // Clear status register
    CAN_OUT(minor, statusReg, 0);

    // Write test pattern
    CAN_OUT(minor, message1Reg.dataReg[1], 0x25);
    CAN_OUT(minor, message2Reg.dataReg[3], 0x52);
    CAN_OUT(minor, message10Reg.dataReg[6], 0xc3);

    // Read back test pattern
    if ((CAN_IN(minor, message1Reg.dataReg[1]) != 0x25 ) ||
        (CAN_IN(minor, message2Reg.dataReg[3]) != 0x52 ) ||
        (CAN_IN(minor, message10Reg.dataReg[6]) != 0xc3 )) {
        DBGPRINT(DBG_DATA,("Could not read back from the hardware."));
        DBGPRINT(DBG_DATA,("This probably means that your hardware is not correctly configured!"));
        return -1;
    }
    else {
        DBGPRINT(DBG_DATA,("Could read back, hardware is probably configured correctly"));
    }

    can_clear_objects(minor);
    can_set_timing(minor, proc_baud[minor]);

    CAN_OUT(minor, globalMaskStandardReg[0], 0);	
    CAN_OUT(minor, globalMaskStandardReg[1], 0);
    CAN_OUT(minor, globalMaskExtendedReg[0], 0);
    CAN_OUT(minor, globalMaskExtendedReg[1], 0);
    CAN_OUT(minor, globalMaskExtendedReg[2], 0);
    CAN_OUT(minor, globalMaskExtendedReg[3], 0);
    // Set message 15 mask, we are not using it, only standard mask is used
    // We set all bits to one because this mask is anded with the global mask.
    CAN_OUT(minor, message15MaskReg[0], 0xFF);
    CAN_OUT(minor, message15MaskReg[1], 0xFF);
    CAN_OUT(minor, message15MaskReg[2], 0xFF);
    CAN_OUT(minor, message15MaskReg[3], 0xFF);

    DBGPRINT(DBG_DATA, ("[%d] CAN_CON 0x%x\n", minor, CAN_IN(minor, controlReg)));
    DBGPRINT(DBG_DATA, ("[%d] CAN_CPU 0x%x\n", minor, CAN_IN(minor, cpuInterfaceReg)));

    // Note: At this stage the CAN chip is still in bus-off condition
    // and must be started using can_start_chip()

    DBGOUT();
    return 0;
}




/*
 * Configures bit timing registers directly. Chip must be in configuration mode.
 */
int can_set_btr(int minor, int btr0, int btr1)
{
    DBGIN();
    DBGPRINT(DBG_DATA, ("[%d] btr0=%d, btr1=%d", minor, btr0, btr1));

    CAN_OUT(minor, bitTiming0Reg, (uint8_t) (btr0 & 0xff ));
    CAN_OUT(minor, bitTiming1Reg, (uint8_t) (btr1 & 0xff ));
    DBGPRINT(DBG_DATA,("[%d] CAN_BTIME0 0x%x CAN_BTIME1 0x%x", minor,
                       CAN_IN(minor, bitTiming0Reg),
                       CAN_IN(minor, bitTiming1Reg)));

    DBGOUT();
    return 0;
}



/*
 * Configures bit timing. Chip must be in configuration mode.
 */
int can_set_timing(int minor, int baud)
{
    int i = 5;
    int custom = 0;

    DBGIN();
    DBGPRINT(DBG_DATA, ("baud[%d]=%d", minor, baud));

    switch(baud)
    {
        case   10: i = 0; break;
        case   20: i = 1; break;
        case   40: i = 2; break;
        case   50: i = 3; break;
        case  100: i = 4; break;
        case  125: i = 5; break;
        case  250: i = 6; break;
        case  500: i = 7; break;
        case  800: i = 8; break;
        case 1000: i = 9; break;
        default:
            custom = 1;
    }
    if( custom ) {
        CAN_OUT(minor, bitTiming0Reg, (uint8_t) (baud >> 8) & 0xff);
        CAN_OUT(minor, bitTiming1Reg, (uint8_t) (baud & 0xff ));
    } else {
        CAN_OUT(minor, bitTiming0Reg, (uint8_t) iCanTiming[i][0]);
        CAN_OUT(minor, bitTiming1Reg, (uint8_t) iCanTiming[i][1]);
    }
    DBGPRINT(DBG_DATA,("[%d] CAN_BTIME0 0x%x CAN_BTIME1 0x%x", baud,
                       CAN_IN(minor, bitTiming0Reg),
                       CAN_IN(minor, bitTiming1Reg)));

    DBGOUT();
    return 0;
}


/*
 * Enables interrupts and put chip out off bus-off mode
 * Configures message objects for reception.
 */
int can_start_chip(int minor)
{
    DBGIN();

    proc_rxerr[minor] = 0;
    proc_txerr[minor] = 0;

    // Clear interrupts
    CAN_IN(minor, interruptReg);

    // Clear status register
    CAN_OUT(minor, statusReg, 0);

    // Configure message object for reception
    CAN_OUT(minor, message15Reg.msgCtrl1Reg,
           NEWD_RES | MLST_RES | TXRQ_RES | RMPD_RES);
    CAN_OUT(minor, message15Reg.msgCtrl0Reg,
           MVAL_SET | TXIE_RES | RXIE_SET | INTPD_RES);

    can_set_mask(minor, proc_acccode[minor], proc_accmask[minor]);

    // Clear message object for send
    CAN_OUT(minor, message1Reg.msgCtrl1Reg,
           RMPD_RES | TXRQ_RES | CPUU_RES | NEWD_RES);
    CAN_OUT(minor, message1Reg.msgCtrl0Reg,
           MVAL_RES | TXIE_RES | RXIE_RES | INTPD_RES);

    // Clear bus-off, Interrupts only for errors, not for status change
    CAN_OUT(minor, controlReg, iCTL_IE | iCTL_EIE);

    DBGPRINT(DBG_DATA, ("[%d] CAN_CON 0x%x\n", minor, CAN_IN(minor, controlReg)));

    DBGOUT();
    return 0;
}


/*
 * Puts chip in bus-off mode
 *
 * Enable configuration and puts chip in bus-off, disable interrupts.
 * Invalidates message objects.
 */
int can_stopchip(int minor)
{
    DBGIN();

    // Enable configuration and puts chip in bus-off, disable interrupts
    CAN_OUT(minor, controlReg, iCTL_CCE | iCTL_INI);

    // Clear interrupts
    CAN_IN(minor, interruptReg);

    // Clear status register
    CAN_OUT(minor, statusReg, 0);

    // Clear message object for receiption
    CAN_OUT(minor, message15Reg.msgCtrl1Reg,
           NEWD_RES | MLST_RES | TXRQ_RES | RMPD_RES);
    CAN_OUT(minor, message15Reg.msgCtrl0Reg,
           MVAL_RES | TXIE_RES | RXIE_RES | INTPD_RES);

    // Clear message object for send
    CAN_OUT(minor, message1Reg.msgCtrl1Reg,
           RMPD_RES | TXRQ_RES | CPUU_RES | NEWD_RES);
    CAN_OUT(minor, message1Reg.msgCtrl0Reg,
           MVAL_RES | TXIE_RES | RXIE_RES | INTPD_RES);

    DBGPRINT(DBG_DATA, ("[%d] CAN_CON 0x%x\n", minor, CAN_IN(minor, controlReg)));

    DBGOUT();
    return 0;
}


int can_send_message(int minor, canmsg_t *tx)
{
    int i = 0;
    int ext;
    uint8_t id0, id1, id2, id3;

    DBGIN();

    // Wait if there is a transmission in progress
    while ((CAN_IN(minor, message1Reg.msgCtrl1Reg) & TXRQ_UNC) == TXRQ_SET)
	    cond_resched();

    CAN_OUT(minor, message1Reg.msgCtrl1Reg,
           RMPD_RES | TXRQ_RES | CPUU_SET | NEWD_SET);
    CAN_OUT(minor,message1Reg.msgCtrl0Reg,
           MVAL_SET | TXIE_SET | RXIE_RES | INTPD_RES);

    tx->length %= 9; // Limit CAN message length to 8
    ext = (tx->flags & MSG_EXT); // Extended ID?
    if ( ext ) {
        CAN_OUT(minor, message1Reg.messageConfigReg,
               ( tx->length << 4 ) + ( MCFG_DIR | MCFG_XTD ));
        id0 = (uint8_t)( tx->id << 3 );
        id1 = (uint8_t)( tx->id >> 5 );
        id2 = (uint8_t)( tx->id >> 13 );
        id3 = (uint8_t)( tx->id >> 21 );
        CAN_OUT(minor, message1Reg.idReg[3], id0);
        CAN_OUT(minor, message1Reg.idReg[2], id1);
        CAN_OUT(minor, message1Reg.idReg[1], id2);
        CAN_OUT(minor, message1Reg.idReg[0], id3);
    }
    else {
        CAN_OUT(minor, message1Reg.messageConfigReg,
               ( tx->length << 4 ) + MCFG_DIR);
        id1 = (uint8_t)( tx->id << 5 );
        id0 = (uint8_t)( tx->id >> 3 );
        CAN_OUT(minor, message1Reg.idReg[1], id1);
        CAN_OUT(minor, message1Reg.idReg[0], id0);
    }

    for ( i=0; i < tx->length; i++ ) {
        CAN_OUT(minor, message1Reg.dataReg[i], tx->data[i]);
    }

    if ( (tx->flags & MSG_RTR) == MSG_RTR ) {
        CAN_OUT(minor,message1Reg.msgCtrl1Reg,
               RMPD_RES | TXRQ_RES | CPUU_RES | NEWD_UNC);
    }
    else {
        CAN_OUT(minor,message1Reg.msgCtrl1Reg,
               RMPD_RES | TXRQ_SET | CPUU_RES | NEWD_UNC);

    }

    DBGOUT();
    return i;
}


/*
 * ioctl poll
 */
int CAN_GetMessage(int minor, canmsg_t *rx)
{
    int i = 0;
    uint8_t msgctlreg;
    uint8_t ctl1reg;

    DBGIN();

    ctl1reg = CAN_IN(minor, message15Reg.msgCtrl1Reg);
    rx->flags = 0;
    rx->length = 0;

    if( ctl1reg & MLST_SET ) {
        proc_overrun[minor]++;
        DBGPRINT(DBG_DATA,("i82527: Previous message lost!\n"));
    }

    if(ctl1reg & NEWD_SET) {

        do_gettimeofday(&rx->timestamp);

        if (ctl1reg & RMPD_SET) {
            rx->flags |= MSG_RTR;
        }

        msgctlreg = CAN_IN(minor, message15Reg.messageConfigReg);
        if( msgctlreg & MCFG_XTD ) {
            int id0, id1, id2, id3;

            id0 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[3]));
            id1 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[2])) << 8;
            id2 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[1])) << 16;
            id3 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[0])) << 24;
            rx->flags |= MSG_EXT;
            rx->id =(id0 | id1 | id2 | id3) >> 3;

        } else {
            int id0, id1;

            id0 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[1]));
            id1 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[0])) << 8;
            rx->id =(id0 | id1 ) >> 5;
        }

        msgctlreg  &= 0xf0;/* strip length code */
        msgctlreg  = msgctlreg >>4;

        rx->length = msgctlreg;
        msgctlreg %= 9;	/* limit count to 8 bytes */

        for (i = 0; i < msgctlreg; i++) {
            rx->data[i] = CAN_IN(minor, message15Reg.dataReg[i]);
            DBGPRINT(DBG_DATA,("rx[%d]: 0x%x",i, rx->data[i]));
        }

        // Make the chip ready to receive the next message
        CAN_OUT(minor,message15Reg.msgCtrl0Reg,
               MVAL_SET | TXIE_RES | RXIE_SET | INTPD_RES);
        CAN_OUT(minor,message15Reg.msgCtrl1Reg,
               RMPD_RES | TXRQ_RES | MLST_RES |  NEWD_RES);
    }
    else {
        i = 0;
    }

    DBGOUT();
    return i;
}


/*
 * Subroutine of ISR for RX interrupts.
 *
 * Note: This code depends on using message object 15 for receiving.
 * Object 15 has a double buffer and using this routine would not work
 * reliably on other message objects!
 */
static void i82527_irq_read_msg15_handler(int minor, msg_fifo_t *RxFifo)
{
    int i;
    uint8_t msgctlreg;
    uint8_t ctl1reg;

    DBGIN();

    ctl1reg = CAN_IN(minor, message15Reg.msgCtrl1Reg);
    while (ctl1reg & NEWD_SET) {

        do_gettimeofday(&(RxFifo->data[RxFifo->head]).timestamp);

        if (ctl1reg & MLST_SET) {
            proc_overrun[minor]++;
            DBGPRINT(DBG_DATA,("i82527: Previous message lost!\n"));
        }

        if (ctl1reg & RMPD_SET) {
            (RxFifo->data[RxFifo->head]).flags |= MSG_RTR;
        }

        msgctlreg = CAN_IN(minor, message15Reg.messageConfigReg);
        if( msgctlreg & MCFG_XTD ) {
            int id0, id1, id2, id3;

            id0 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[3]));
            id1 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[2])) << 8;
            id2 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[1])) << 16;
            id3 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[0])) << 24;
            (RxFifo->data[RxFifo->head]).flags |= MSG_EXT;
            (RxFifo->data[RxFifo->head]).id =(id0 | id1 | id2 | id3) >> 3;

        } else {
            int id0, id1;

            id0 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[1]));
            id1 = (unsigned int)(CAN_IN(minor, message15Reg.idReg[0])) << 8;
            (RxFifo->data[RxFifo->head]).id =(id0 | id1 ) >> 5;
        }

        msgctlreg  &= 0xf0;/* strip length code */
        msgctlreg  = msgctlreg >>4;

        (RxFifo->data[RxFifo->head]).length = msgctlreg;
        msgctlreg %= 9;	/* limit count to 8 bytes */

        for (i = 0; i < msgctlreg; i++) {
            (RxFifo->data[RxFifo->head]).data[i] =
                CAN_IN(minor, message15Reg.dataReg[i]);
            DBGPRINT(DBG_DATA,("rx[%d]: 0x%x",i,
                               (RxFifo->data[RxFifo->head]).data[i]));
        }
        RxFifo->status = BUF_OK;
	++(RxFifo->head);
        RxFifo->head %= MAX_BUFSIZE;

        if(RxFifo->head == RxFifo->tail) {
            RxFifo->status = BUF_OVERRUN;
        }

        // Make the chip ready to receive the next message
        CAN_OUT(minor, message15Reg.msgCtrl0Reg,
               MVAL_SET | TXIE_RES | RXIE_SET | INTPD_RES);
        CAN_OUT(minor, message15Reg.msgCtrl1Reg,
               RMPD_RES | TXRQ_RES | MLST_RES | NEWD_RES);

        // Notify user app
        wake_up_interruptible(&can_wait[minor][0]);

        ctl1reg = CAN_IN(minor, message15Reg.msgCtrl1Reg);
    }

    DBGOUT();
}


/*
 * Subroutine of ISR for TX interrupts
 */
static void i82527_irq_write_handler(int minor, msg_fifo_t *TxFifo)
{
    unsigned long flags;
    uint8_t tx2reg;
    unsigned int id;
    int ext;
    int i;
    uint8_t id0, id1, id2, id3;

    DBGIN();

    // Enter critical section
    local_irq_save(flags);

    if( TxFifo->free[TxFifo->tail] == BUF_EMPTY ) {
        // Nothing more to send, switch off interrupts
        CAN_OUT(minor, message1Reg.msgCtrl0Reg,
               (MVAL_RES | TXIE_RES | RXIE_RES | INTPD_RES));
        TxFifo->status = BUF_EMPTY;
        TxFifo->active = 0;
        // We had some cases of repeated IRQ, so make sure the INT is acknowledged
        CAN_OUT(minor, message1Reg.msgCtrl0Reg,
               (MVAL_UNC | TXIE_UNC | RXIE_UNC | INTPD_RES));
        /* leave critical section */
        local_irq_restore(flags);
        // Notify user app
        wake_up_interruptible(&canout_wait[minor]);
        DBGOUT();
        return;
    }
    CAN_OUT(minor, message1Reg.msgCtrl1Reg,
                 RMPD_RES | TXRQ_RES | CPUU_SET | NEWD_RES);
    CAN_OUT(minor, message1Reg.msgCtrl0Reg,
                 MVAL_SET | TXIE_SET | RXIE_RES | INTPD_RES);

    tx2reg = (TxFifo->data[TxFifo->tail]).length;

    ext = (TxFifo->data[TxFifo->tail]).flags & MSG_EXT;
    id = (TxFifo->data[TxFifo->tail]).id;

    if ( ext ) {
        CAN_OUT(minor, message1Reg.messageConfigReg,
                     (tx2reg << 4 ) + ( MCFG_DIR | MCFG_XTD ));
        id0 = (uint8_t)( id << 3 );
        id1 = (uint8_t)( id >> 5 );
        id2 = (uint8_t)( id >> 13 );
        id3 = (uint8_t)( id >> 21 );
        CAN_OUT(minor, message1Reg.idReg[3], id0);
        CAN_OUT(minor, message1Reg.idReg[2], id1);
        CAN_OUT(minor, message1Reg.idReg[1], id2);
        CAN_OUT(minor, message1Reg.idReg[0], id3);
    }
    else {
        CAN_OUT(minor, message1Reg.messageConfigReg,
               ( tx2reg << 4 ) + MCFG_DIR);
        id1 = (uint8_t)( id << 5 );
        id0 = (uint8_t)( id >> 3 );
        CAN_OUT(minor, message1Reg.idReg[1], id1);
        CAN_OUT(minor, message1Reg.idReg[0], id0);
    }

    tx2reg &= 0x0f; //restore length only
    for ( i=0; i < tx2reg; i++ ) {
        CAN_OUT(minor, message1Reg.dataReg[i],
               (TxFifo->data[TxFifo->tail]).data[i]);
    }

    if ( ((TxFifo->data[TxFifo->tail]).flags  & MSG_RTR) == MSG_RTR ) {
        CAN_OUT(minor, message1Reg.msgCtrl1Reg,
               (RMPD_RES | TXRQ_RES | CPUU_RES | NEWD_UNC));
    }
    else {
        CAN_OUT(minor, message1Reg.msgCtrl1Reg,
               (RMPD_RES | TXRQ_SET | CPUU_RES | NEWD_UNC));

    }

    TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
    ++(TxFifo->tail);
    TxFifo->tail %= MAX_BUFSIZE;

    // HM: We had some cases of repeated IRQs, so make sure the INT is acknowledged
    // I know it's already further up, but doing again fixed the issue
    CAN_OUT(minor, message1Reg.msgCtrl0Reg,
           (MVAL_UNC | TXIE_UNC | RXIE_UNC | INTPD_RES));
    /* leave critical section */
    local_irq_restore(flags);

    DBGOUT();
}


/*
 * The plain i82527 interrupt
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
    int minor = *(int *) dev_id;
    msg_fifo_t *RxFifo = &rx_buf[minor][0];
    msg_fifo_t *TxFifo = &tx_buf[minor];
    uint8_t irqreg;
    uint8_t lastIrqreg;
     
    // Read the highest pending interrupt request
    irqreg = CAN_IN(minor, interruptReg);
    lastIrqreg = irqreg;
    
    while ( irqreg ) {
        switch (irqreg)
        {
            case 1: // Status register
            {
                uint8_t status;

                // Read the STATUS reg
                status = CAN_IN(minor, statusReg);
                CAN_OUT (minor, statusReg, 0);

                if ( status & iSTAT_RXOK ) {
                    // Intel datasheet: Software must clear this bit in ISR
                    CAN_OUT (minor, statusReg, status & ~iSTAT_RXOK);
                }
                if ( status & iSTAT_TXOK ) {
                    // Intel datasheet: Software must clear this bit in ISR
                    CAN_OUT (minor, statusReg, status & ~iSTAT_TXOK);
                }
                if ( status & iSTAT_WARN ) {
                    // Note: status bit is read-only, don't clear
                    // TODO must be implemented here for chip statistic
                    (RxFifo->data[RxFifo->head]).flags += MSG_PASSIVE;
                    DBGPRINT(DBG_DATA,("i82527_CAN_Interrupt: Bus warning\n" ));
                }
                if ( status & iSTAT_BOFF ) {
                    long flags;

                    // Note: status bit is read-only, don't clear
                    // TODO must be implemented here for chip statistic
                    (RxFifo->data[RxFifo->head]).flags += MSG_BUSOFF;

                    // Clear init flag and reenable interrupts
                    flags = CAN_IN(minor, controlReg) | ( iCTL_IE | iCTL_EIE );
                    flags &= ~iCTL_INI; // Reset init flag
                    CAN_OUT(minor, controlReg, flags);

                    (RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
                    RxFifo->status = BUF_OK;
                    ++(RxFifo->head);
                    RxFifo->head %= MAX_BUFSIZE;
                    if(RxFifo->head == RxFifo->tail) {
                        RxFifo->status = BUF_OVERRUN;
                    }
                    // Notify user app
                    wake_up_interruptible(&can_wait[minor][0]);
                    DBGPRINT(DBG_DATA,("i82527_CAN_Interrupt: Bus off\n" ));
                }
             }
             break;
             case 2: // Receiption, message object 15
                 i82527_irq_read_msg15_handler(minor, RxFifo);
             break;
             case 3: // Write, message object 1
                i82527_irq_write_handler(minor, TxFifo);
             break;
             case 4: // message object 2
                 //printk("*********** Unexpected i82527_CAN_Interrupt: irqreq2=0x%X\n", irqreg);
                 DBGPRINT(DBG_DATA,("Unexpected i82527_CAN_Interrupt: irqreq=0x%X\n", irqreg));
                 CAN_OUT(minor, message2Reg.msgCtrl0Reg, 
                        (MVAL_RES | TXIE_RES | RXIE_RES | INTPD_RES));
             break;
             default: // Unexpected
                 //printk("*********** Unexpected i82527_CAN_Interrupt: irqreq2=0x%X\n", irqreg);
                 DBGPRINT(DBG_DATA,("Unexpected i82527_CAN_Interrupt: irqreq=0x%X\n", irqreg));
             break;
        }
        // Get irq status again for next loop iteration
        irqreg = CAN_IN(minor, interruptReg);
        if (irqreg == lastIrqreg)
        {
           //printk("i82527_CAN_Interrupt: irqreq repeated!!!! 0x%X\n", irqreg);
           DBGPRINT(DBG_DATA,("i82527_CAN_Interrupt: irqreq repeated!!!! 0x%X\n", irqreg));
        }
        lastIrqreg =     irqreg;
    } /* end while (irqreq) */
    DBGOUT();
#if LINUX_VERSION_CODE >= 0x020500
    return IRQ_RETVAL(IRQ_HANDLED);
#endif
}


void can_showstat(int minor)
{
  // TODO: Implement
}


/*
 * This is currently an attempt to support acceptance code and mask
 * for i82527. However the interpretation of mask and code is currently
 * different to the SJA1000 function.
 *
 * This MUST change in the future to have a common exchangable API for
 * both chips.
 *
 * Currently if code and mask are both 0 OR both 0xFFFFFFFF the
 * acceptance filtering is disabled.
 *
 * If the highest bit of mask or code is set OR the value is bigger than
 * 0x7FF (11 bit), an extended mask is assumed.
 */
int can_set_mask(int minor, unsigned int code, unsigned int mask)
{
    unsigned char mask0, mask1, mask2, mask3;
    unsigned char code0, code1, code2, code3;

    DBGIN();

    // 0xfffff is a magic value and means no mask set.
    // We have to change this to 0 to make this work with i82527, here 0
    // means don't care.
    if (code == 0xffffffff)
        code = 0;
    if (mask == 0xffffffff)
        mask = 0;

    // Extended (29-bit) or basic (11-bit) mask?
    if (((code & 0x8000000) == 0x8000000) ||
        ((mask & 0x8000000) == 0x8000000) ||
        (code > 0x7FF) ||
        (mask > 0x7FF))
    {
        mask0 = (unsigned char) (mask >> 21);
        mask1 = (unsigned char) (mask >> 13);
        mask2 = (unsigned char) (mask >> 5);
        mask3 = (unsigned char) (mask << 3);
        code0 = (unsigned char) (code >> 21);
        code1 = (unsigned char) (code >> 13);
        code2 = (unsigned char) (code >> 5);
        code3 = (unsigned char) (code << 3);

        CAN_OUT(minor, globalMaskExtendedReg[0], mask0);
        CAN_OUT(minor, globalMaskExtendedReg[1], mask1);
        CAN_OUT(minor, globalMaskExtendedReg[2], mask2);
        CAN_OUT(minor, globalMaskExtendedReg[3], mask3);
        CAN_OUT(minor, message15Reg.idReg[0], code0);
        CAN_OUT(minor, message15Reg.idReg[1], code1);
        CAN_OUT(minor, message15Reg.idReg[2], code2);
        CAN_OUT(minor, message15Reg.idReg[3], code3);
        CAN_OUT(minor, message15Reg.messageConfigReg, MCFG_XTD);

        DBGPRINT(DBG_DATA,("[%d] CAN_EGMSK 0x%x ==> CAN_EGMSK0 0x%x "
                           "CAN_EGMSK1 0x%x CAN_EGMSK2 0x%x CAN_EGMSK3 0x%x" ,
                           minor, mask, mask0, mask1,mask2, mask3));
        DBGPRINT(DBG_DATA,("[%d] CAN_EGMSK 0x%x ==> CAN_MSGID0 0x%x "
                           "CAN_MSGID1 0x%x CAN_MSGID2 0x%x CAN_MSGID3 0x%x" ,
                           minor, code, code0, code1,code2, code3));
    }
    else
    {
        mask0 = (unsigned char) (mask >> 3);
        mask1 = (unsigned char) (mask << 5);
        code0 = (unsigned char) (code >> 3);
        code1 = (unsigned char) (code << 5);

        CAN_OUT(minor, globalMaskStandardReg[0], mask0);	
        CAN_OUT(minor, globalMaskStandardReg[1], mask1);
        CAN_OUT(minor, message15Reg.idReg[0], code0);
        CAN_OUT(minor, message15Reg.idReg[1], code1);
        CAN_OUT(minor, message15Reg.messageConfigReg, 0x00);

        DBGPRINT(DBG_DATA,("[%d] CAN_SGMSK0 0x%x CAN_SGMSK1 0x%x",
                           minor, mask0, mask1));
        DBGPRINT(DBG_DATA,("[%d] CAN_MSGID0 0x%x CAN_MSGID1 0x%x",
                           minor, code0, code1));
    }

    /* put values back in global variables for sysctl */
    proc_acccode[minor] = code;
    proc_accmask[minor] = mask;

    DBGOUT();
    return 0;
}


int can_set_mode(int minor, int arg)
{
  // Does not exist for i82527
  return -1;
}

int can_set_listenonlymode(int minor, int arg)
{
  // Does not exist for i82527
  return -1;
}

/*
 * Read back as many status information as possible
 *
 * Defined only to be API compatible with SJA1000 code.
 * Refer to SJA1000 code documentation.
 */
int can_getstat(
	struct inode *inode,
	struct file *file,
	can_statuspar_t *stat
	)
{
  // TODO: Implement
    return -1;
}


/*
 * Vendor/hardware specific initialisation
 *
 * Note: release region is done by can_close !
 */
int can_vendor_init(int minor)
{
struct resource *iores;

    DBGIN();

    if ((sizeof(canmessage_t) != 15) || (sizeof(canregs_t) != 256))
    {
        DBGPRINT(DBG_DATA,("Wrong sizes: %u %u ",
                           (unsigned int)sizeof(canmessage_t),
			   (unsigned int)sizeof(canregs_t)));
        return -EBUSY;
    }

/* Defaults valid for most vendors/configurations ------------------------- */

#ifdef CAN_INDEXED_PORT_IO
    can_range[minor] = 2; /* Note: This value is hard coded in can_close.c! */
#else
    can_range[minor] = 0x100; /* i82527 has 256 registers */
#endif

/* 1. Vendor specific part ------------------------------------------------ */

#if defined(PC104_200)
    can_range[minor] = 0x08; /* pc104/200 minor */
#elif defined(SBS_PC7)
    outb(5, 0x169);          /* Unlock special function register */
#else
#endif

/* End: 1. Vendor specific part ------------------------------------------- */

    /* Request the controllers address space */
#if defined(CAN_PORT_IO)
    /* It's port I/O */
    iores = request_region(proc_base[minor], can_range[minor], "CAN-IO");
    if(NULL == iores) {
	DBGPRINT(DBG_DATA,("Error request_region(Base %x, range %u) for std port IO\n",
                           (unsigned int)proc_base[minor],
			   (unsigned int)can_range[minor]));
	return -EBUSY;
    }
#else
#if defined(CAN_INDEXED_PORT_IO)
    /* It's indexed port I/O */
    iores = request_region(proc_base[minor], can_range[minor], "CAN-IO");
    if(NULL == iores) {
	DBGPRINT(DBG_DATA,("Error request_region(Base %x, range %u) for indexed port IO\n",
                           (unsigned int)proc_base[minor],
			   (unsigned int)can_range[minor]));
	return -EBUSY;
    }
#else
    /* It's Memory I/O */
    if(NULL == request_mem_region(proc_base[minor], can_range[minor], "CAN-IO")) {
	DBGPRINT(DBG_DATA,("Error request_mem_region(Base %xu, range %u) for mem IO\n",
                           (unsigned int)proc_base[minor],
			   (unsigned int)can_range[minor]));
	return -EBUSY;
    }
#endif
#endif

	can_iobase[minor] = ioremap(proc_base[minor], can_range[minor]);

    /* now the virtual address can be used for the register address macros */


/* 2. Vendor specific part ------------------------------------------------ */

#if defined (PC104_200)
    if( IRQ[minor] > 0 ) {
      CANactivate_irq(minor, IRQ[minor]);
      int i;
      for(i=0;i<500;i++) SLOW_DOWN_IO;
    }
#endif

/* End: 2. Vendor specific part ------------------------------------------- */

    /* test for valid IRQ number in /proc/sys/.../IRQ */
    if( IRQ[minor] > 0 && IRQ[minor] < MAX_IRQNUMBER ){
        int err;

	err = request_irq( IRQ[minor], can_interrupt, IRQF_SHARED, 
				"Can", &can_minors[minor]);
        if( !err ){
	    DBGPRINT(DBG_BRANCH,("Requested IRQ: %d @ 0x%lx",
				    IRQ[minor], (unsigned long)can_interrupt));
	    irq_requested[minor] = 1;
	} else {
	    pr_err("Can not get requested IRQ number %d\n", IRQ[minor]);
	    DBGOUT(); return -EBUSY;
	}
    } else {
	pr_err("Invalid IRQ number in /proc/.../IRQ %d\n", IRQ[minor]);
	DBGOUT(); return -EBUSY;
    }


    DBGOUT();
    return 0;
}

/* Called from __init,  once when driver is loaded
   set up physical addresses, irq number
   and initialize clock source for the CAN module

   Take care it will be called only once
   because it is called for every CAN channel out of MAX_CHANNELS
*/
int init_board_hw(int n)
{
int ret;
int minor = -1;

	DBGIN();
	ret = 0;
	if (virtual == 0) {
		/* make some sysctl entries read only
		;	 */
	}
	DBGOUT();
	return ret;
}
