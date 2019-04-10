/* can_sysctl
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
 * (c) 2001-2015 Heinz-Jürgen Oertel <hj.oertel@t-online.de>
 *------------------------------------------------------------------
 */
/*
 * This file implements the SYSCTL basics, and handler/strategy routines.
 *
 */
#include "defs.h"
#include <linux/mm.h>
#include <linux/sysctl.h>
#include <linux/ctype.h>

//#define OLD_SYCTL
static struct ctl_table_header *can_systable;

/* ----- Prototypes */

/* ----- global variables accessible through /proc/sys/dev/Can */
/* or, if defined at /proc/sys/dev/Can$(CAN_MODULE_POSTFIX) */

char proc_version[] = VERSION;
char proc_iomodel[MAX_CHANNELS] = { 0 };

/* allow a number with max two digits "\tyy(xx)" */
char proc_opencount[MAX_CHANNELS * 7];


char proc_chipset[PROC_CHIPSET_LENGTH] =
#if defined(CPC_PCI) || defined(CPC_PCI2) || defined(CC_CANPCI)
	"SJA1000"
#elif defined(KVASER_PCICAN)
	"SJA1000"
#elif defined(ATCANMINI_PELICAN)
	"SJA1000"
#elif defined(IME_SLIMLINE)
	"SJA1000"
#elif defined(PCM3680)
	"SJA1000"
#elif defined(IXXAT_PCI03)
	"SJA1000"
#elif defined(CCPC104)
	"SJA1000"
#elif defined(MCF5282) || defined(IMX35) || defined(IMX25) || defined(IMX28)
	"FlexCAN"
#elif defined(GENERIC_I82527) || defined(SBS_PC7)
	"i82527"
#elif defined(AD_BLACKFIN)
	"BlackFIN"
#elif defined(ATMEL_SAM9)
	"AT91SAM9263"
#elif defined(ECAN1000)
	"SJA1000"
#elif defined(ZEDBOARD)
	"Xilinx xcanps"
#elif defined(RASPI)
	"MCP2515"
#else
	""
#endif
;

int IRQ[MAX_CHANNELS];		/*              = { 0x0 }; */
/* don't assume a standard address, always configure,
 * address                         = = 0 means no board available
 * upointer_t can be so different as
 * 4 byte pointer on 32 bit systems
 * 8 byte pointer on X86_64
 * u32 for /IO address at ISA bus
 */
upointer_t proc_base[MAX_CHANNELS] = { 0x0 };
int proc_baud[MAX_CHANNELS] = { 0x0 };

#if defined CANFD
/* CAN FD speed factor */
int proc_speedfactor[MAX_CHANNELS] = {[0 ... (MAX_CHANNELS - 1)] = 1 };
int proc_tdelay[MAX_CHANNELS] = {[0 ... (MAX_CHANNELS - 1)] = 100 };
#endif /* CANFD */


#if defined(IMX35)   || defined(IMX25) || defined(IMX28)
/* eight filters per CAN controller in FIFO mode */
unsigned int proc_acccode[FLEXCAN_MAX_FILTER][MAX_CHANNELS] = { {0x0} };
unsigned int proc_accmask[FLEXCAN_MAX_FILTER][MAX_CHANNELS] = { {0x0} };
#else
/* only one filter can be used */
unsigned int proc_acccode[MAX_CHANNELS] = { 0x0 };
unsigned int proc_accmask[MAX_CHANNELS] = { 0x0 };
#endif

unsigned int proc_clock = 0x0;	/* Clock frequency driving the CAN */
/* maximum length of data in a CAN frame */
unsigned int proc_framelength = CAN_MSG_LENGTH;
/* transmit timeout in ms */
int proc_timeout[MAX_CHANNELS] = {[0 ... (MAX_CHANNELS - 1)] = 100 };

/* predefined value of the output control register,
* depends of TARGET set by Makefile */
int proc_outc[MAX_CHANNELS] = { 0x0 };
int proc_txerr[MAX_CHANNELS] = { 0x0 };
int proc_rxerr[MAX_CHANNELS] = { 0x0 };
int proc_txerrcounter[MAX_CHANNELS] = { 0x0 };/* CAN controllers err counter */
int proc_rxerrcounter[MAX_CHANNELS] = { 0x0 };/* CAN controllers err counter */
int proc_overrun[MAX_CHANNELS] = { 0x0 };     /* number of HW CAN RX overruns */
int proc_arbitrationlost[MAX_CHANNELS] = { 0x0 };

#ifdef DEBUG_COUNTER
int Cnt1[MAX_CHANNELS] = { 0x0 };
int Cnt2[MAX_CHANNELS] = { 0x0 };
#endif /* DEBUG_COUNTER */

/* Hardware dependant sysctl entries */

#if defined JANZ_PCIL
 /* the hex switch on the board */
int proc_board_id[MAX_CHANNELS] = { 0 };
#endif

/* /end Hardware dependant sysctl entries */

/* ----- the sysctl table */

/* The ctl_table format has changed in 2.6.33:
Author: Marc Dionne <marc.c.dionne@gmail.com>
Date:   Wed Dec 9 19:06:18 2009 -0500

    Linux: deal with ctl_name removal

    The binary sysctl interface will be removed in kernel 2.6.33 and
    ctl_name will be dropped from the ctl_table structure.
    Make the code that uses ctl_name conditional on a configure test.

*/


ctl_table can_sysctl_table[] = {
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "OpenCount",
	 .data = &proc_opencount,
	 .maxlen = MAX_CHANNELS * 10,
	 .mode = 0444,
	 .proc_handler = &proc_dostring,
	 },
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "version",
	 .data = &proc_version,
	 .maxlen = PROC_VER_LENGTH,
	 .mode = 0444,
	 .proc_handler = &proc_dostring,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "framelength",
	 .data = &proc_framelength,
	 .maxlen = sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Chipset",
	 .data = &proc_chipset,
	 .maxlen = PROC_CHIPSET_LENGTH,
	 .mode = 0444,
	 .proc_handler = &proc_dostring,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "IOModel",
	 .data = &proc_iomodel,
	 .maxlen = MAX_CHANNELS + 1,	/* +1 for '\0' */
	 .mode = 0444,
	 .proc_handler = &proc_dostring,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "IRQ",
	 .data = IRQ,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Base",
	 .data = proc_base,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 /* .maxlen = MAX_CHANNELS * sizeof(void __iomem *), */
	 .mode = 0644,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Baud",
	 .data = proc_baud,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
#if defined CANFD
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Speedfactor",
	 .data = proc_speedfactor,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Transmitterdelay",
	 .data = proc_tdelay,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
#endif
/* ---------------------------------------------------- */
#if defined(IMX35) || defined(IMX25) || defined(IMX28)
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode0",
	 .data = &proc_acccode[0],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask0",
	 .data = &proc_accmask[0],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode1",
	 .data = &proc_acccode[1],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask1",
	 .data = &proc_accmask[1],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode2",
	 .data = &proc_acccode[2],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask2",
	 .data = &proc_accmask[2],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode3",
	 .data = &proc_acccode[3],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask3",
	 .data = &proc_accmask[3],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode4",
	 .data = &proc_acccode[4],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask4",
	 .data = &proc_accmask[4],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode5",
	 .data = &proc_acccode[5],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask5",
	 .data = &proc_accmask[5],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode6",
	 .data = &proc_acccode[6],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask6",
	 .data = &proc_accmask[6],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode7",
	 .data = &proc_acccode[7],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask7",
	 .data = &proc_accmask[7],
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
#else
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccCode",
	 .data = proc_acccode,
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "AccMask",
	 .data = proc_accmask,
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0666,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "ArbitrationLost",
	 .data = proc_arbitrationlost,
	 .maxlen = MAX_CHANNELS * sizeof(unsigned int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
#endif
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Timeout",
	 .data = proc_timeout,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Outc",
	 .data = proc_outc,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "TxErr",
	 .data = proc_txerr,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "RxErr",
	 .data = proc_rxerr,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "TxErrCounter",
	 .data = proc_txerrcounter,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "RxErrCounter",
	 .data = proc_rxerrcounter,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Overrun",
	 .data = proc_overrun,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "dbgMask",
	 .data = &proc_dbgmask,
	 .maxlen = 1 * sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "CAN clock",
	 .data = &proc_clock,
	 .maxlen = 1 * sizeof(unsigned int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
#if defined JANZ_PCIL
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
         .procname = "BoardId",
         .data = &proc_board_id,
         .maxlen = MAX_CHANNELS * sizeof(int),
         .mode = 0444,
         .proc_handler = &proc_dointvec,
         },
#endif	/* JANZ_PCIL */
/* ---------------------------------------------------- */
#ifdef DEBUG_COUNTER
/* ---------------------------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "cnt1",
	 .data = Cnt1,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------- */
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "cnt2",
	 .data = Cnt2,
	 .maxlen = MAX_CHANNELS * sizeof(int),
	 .mode = 0444,
	 .proc_handler = &proc_dointvec,
	 },
/* ---------------------------------------------------------------------- */
#endif /* DEBUG_COUNTER */

	{.procname = NULL}
};

static ctl_table can_root[] = {
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_UNNUMBERED,
#endif
	 .procname = "Can" CAN_MODULE_POSTFIX,
	 .maxlen = 0,
	 .mode = 0555,
	 .child = can_sysctl_table,
	 },
#if defined OLD_SYCTL
	{.ctl_name = 0 /* NULL */ }
#else
	{.procname = NULL}
#endif
};

static ctl_table dev_root[] = {
	{
#if defined OLD_SYCTL
	 .ctl_name = CTL_DEV,
#endif
	 .procname = "dev",
	 .maxlen = 0,
	 .mode = 0555,
	 .child = can_root,
	 },
#if defined OLD_SYCTL
	{.ctl_name = 0 /* NULL */}
#else
	{.procname = NULL}
#endif
};

/* ----- register and unregister entrys */

void register_systables(void)
{
	can_systable = register_sysctl_table(dev_root);
}

void unregister_systables(void)
{
	unregister_sysctl_table(can_systable);
}
