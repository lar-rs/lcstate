
#ifndef __IFI_CANFD_H__
#define __IFI_CANFD_H__


#define CIA_PLUGFEST


#define IORD(a, b)
#define IOWR(a, b, c)

#define CAN_RANGE	0x1000	/* how many bytes are occupied ? */

typedef struct canregs {
	u32	stcmd;		/* 0  Status and Command*/
	u32	rxfifo;		/* Status and Command Receive-FIFO */
	u32	txfifo;		/* Status and Command Transmit-FIFO */
	u32	interrupt;	/* Interrupt pending, Interrupt Status */
	u32	irqmask;	/* Interrupt Mask */
	u32	time;		/* Bit Timing for arbitration phase */
	u32	ftime;		/* Bit Timing for data phase */
	u32	tdelay;		/* transmitter delay used in fast data phase */
	u32	error;		/* CAN error counter read only */
	u32	errcnt;		/* additional error counters arbitration phase data phase read only */
	u32	suspend;	/* default transmission suspend in us */
	u32	repeat;		/* default transmission repeat count */
	u32	traffic;	/* bus traffic rate */
	u32	tscontrol;	/* Time Stamp counter control */
	u32	tsc;		/* actual running time stamp counter read only */
	u32	tst;		/* last stored time stamp read only */
	u32	res1;		/* reserved 1 do not write */
	u32	res2;		/* reserved 2 do not writ */
	u32	par;		/* Compiler Parameter read only */
	u32	canclock;	/* clock parameter in Hz read only */
	u32	sysclock;	/* clock parameter in Hz read only */
	u32	ver;		/* Revision of IP-core read only  */
	u32	ip;		/* IP core ID 0xD073CAFD read only */
	u32	test;		/* test register */
	/* RX FIFO registers */
	u32	rptcount;	/* individual transmit repeat count */
	u32	timestamp;	/* time stamp */
	u32	rx_dlc;		/* receive DLC Data Length Code */
	u32	rx_id;		/* receive ID identifier */
	u32	rx_data[16];	/* receive data, 16 dwords, 64 byte */
	/* TX FIFO registers */
	u32	tx_suspend;	/* individual suspend count */
	u32	tx_repeat;	/* individual repeat count */
	u32	tx_dlc;		/* transmit DLC Data Length Code */
	u32	tx_id;		/* transmit ID identifier */
	u32	tx_data[16];	/* receive data, 16 dwords, 64 byte */
	/* Part 4 Filter-Mask when enabled */

} __attribute__((packed)) canregs_t;



#define CAN_BUS_STATUS 				(1<<7)
#define CAN_ERROR_STATUS			(1<<6)
#define CAN_TRANSMIT_STATUS			(1<<5)
#define CAN_RECEIVE_STATUS			(1<<4)


#include <ifi_canfd_regs.h>


/* generated bit rate table by
 * http://www.bittiming.can-wiki.info/
 */
#if CAN_SYSCLK == 80000000
/* these timings are valid for 80 Mhz CAN clock  20 tq, SP 85 %*/


#if defined(CIA_PLUGFEST)
#  define CAN_TIME_10K		0x7062fece	/* 80 tq, 80 %, sjw 8 */
#  define CAN_TIME_20K		0x7030fece
#  define CAN_TIME_40K		0x0
#  define CAN_TIME_50K		0x7012fece	/* "" */
#  define CAN_TIME_100K		0x7008fece
#  define CAN_TIME_125K		0x7006fece
#  define CAN_TIME_250K		0x7002fece
#  define CAN_TIME_500K		0x7000fece	/* 80 tq, SJW 8, SP 80 % */
#  define CAN_TIME_800K		0x7000e6c8 	/* 50 tq */  
#  define CAN_TIME_1000K	0x7000dec6	/* 40 tq, SJW 8, SP 80 % */
#else
#  define CAN_TIME_10K		0x018ecfc1
#  define CAN_TIME_20K		0x00c6cfc1	
#  define CAN_TIME_40K		0x00000000
#  define CAN_TIME_50K		0x004ecfc1
#  define CAN_TIME_100K		0x0026cfc1 
#  define CAN_TIME_125K		0x001ecfc1
#  define CAN_TIME_250K		0x000ecfc1
#  define CAN_TIME_500K		0x0006cfc1
#  define CAN_TIME_800K		0x0003cfc1   
#  define CAN_TIME_1000K	0x0002cfc1
#endif

/* The new IFI CAN Timing used in ISO mode
IFI Called it 7_9_8_8_bit
*/
#  define CAN_TIMEI_10K		0x02c61e06	/* 40 tq, 80 % */
#  define CAN_TIMEI_20K		0x02621e06	/* 40 tq, 80 % */
#  define CAN_TIMEI_40K		0x00000000	/* 40 tq, 80 % */
#  define CAN_TIMEI_50K		0x02261e06	/* 40 tq, 80 % */
#  define CAN_TIMEI_100K	0x02121e06	/* 40 tq, 80 % */
#  define CAN_TIMEI_125K	0x020e1e06	/* 40 tq, 85 % */
#  define CAN_TIMEI_250K	0x00062004	/* 40 tq, 85 % */
#  define CAN_TIMEI_500K	0x02021e06	/* 40 tq, 85 % */
#  define CAN_TIMEI_800K	0x02002608	/* 50 tq, 80 % */
#  define CAN_TIMEI_1000K	0x02001e06	/* 40 tq, 80 % */

/* The CiA Plugfest settings require 
 * arbitration 500 k 80 tq SP 80 %
 * data 1M 80%, 2 M 80 %, 4 M 80 %, 5 M 75 %, 8 M 80 % * 
 * for the fast data bit rate sjw = 2 */
#  define CAN_FTIME_50K		0	/* 20 tq */
#  define CAN_FTIME_100K	0x1026cec2	/* 20 tq */
#  define CAN_FTIME_125K	0x101ecec2	/* 20 tq */
#  define CAN_FTIME_200K	0x1012cec2	/* 20 tq */
#  define CAN_FTIME_400K	0x1008cec2	/* 20 tq */
#  define CAN_FTIME_250K	0x100ecec2	/* 20 tq */
#  define CAN_FTIME_500K	0x1006cec2	/* 20 tq */
#  define CAN_FTIME_800K	0x1003cec2	/* 20 tq */
#  define CAN_FTIME_1000K	0x1002cec2	/* 20 tq */
#  define CAN_FTIME_2000K	0x1000cec2	/* 20 tq */
#  define CAN_FTIME_4000K	0x1000c6c0	/* 10 tq */
#  define CAN_FTIME_5000K	0x1000c4c0	/* 8 tq, 75 % */
#  define CAN_FTIME_8000K	0x0


/* The new IFI CAN Timing used in ISO mode */
#  define CAN_FTIMEI_50K		0	/* */
#  define CAN_FTIMEI_100K	0x02260e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_125K	0x021e0e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_200K	0x02120e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_400K	0x02080e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_250K	0x020e0e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_500K	0x02060e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_800K	0x02030e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_1000K	0x02020e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_2000K	0x02000e02	/* 20 tq, 80 % */
#  define CAN_FTIMEI_4000K	0x02000600	/* 10 tq, 80 % */
#  define CAN_FTIMEI_5000K	0x02000400	/*  8 tq, 75 %*/
#  define CAN_FTIMEI_8000K	0x0



#define CAN_SYSCLK_IS_OK            1
#endif

#if CAN_SYSCLK == 40000000
/* these timings are valid for XXXXX, using 40.0 Mhz*/

#error "40 Mhz timings not defined"


#define CAN_SYSCLK_IS_OK            1
#endif

/* for more CAN_SYSCLK values */


#ifndef CAN_SYSCLK_IS_OK
#  error Please specify a valid CAN_SYSCLK value (i.e. 8, 10) or define new parameters
#endif



#endif	// __IFI_CANFD_H__

/* end of file */
