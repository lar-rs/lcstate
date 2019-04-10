/* allwinner.h 
 * For each CAN controller hardware depending register definitions
 *  Allwinner A20 CAN
 */

#ifndef __CAN_ALLWINNER_
#define __CAN_ALLWINNER_



#define CAN0_BASE_ADDR	0x01C2BC00 	/* SW_VA_CAN0_IO_BASE */
#define CAN0_INT0	58		/* SW_INT_IRQNO_CAN */

#define CAN_RANGE	1024	/* how many bytes are occupied ? */

/* Register definitions like these: 
 */


/* That is hoe the CAN controller looks like for us programmers */
typedef struct canregs {
/* 00 */ u32	msel;	/* Mode Select Register			    */
/* 04 */ u32	cmd;	/* Command Register			    */
/* 08 */ u32	sta;	/* Status Register			    */
/* 0c */ u32	interrupt;	/* Interrupt Register		    */
/* 10 */ u32	inten;	/* interrupt Enable Register		    */
/* 14 */ u32	bustime;/* Bus Timing Register			    */
/* 18 */ u32	tewl;	/* TX Error Warning Limit Register	    */
/* 1c */ u32	errc;	/* Error Counter Register		    */
/* 20 */ u32	rmcnt;	/* Receive Message Counter Register	    */
/* 24 */ u32	rbuf_saddr;	/* Receive Buffer Start Address Register			    */
	 u32	dummy[6];
/* 40 */ u32	trbuf0;	/* TX/RX message buffer 0 Register	    */
	 u32	trbuf1;	/* TX/RX message buffer Register	    */
/* 48 */ u32	trbuf2;	/* TX/RX message buffer Register	    */
	 u32	trbuf3;	/* TX/RX message buffer Register	    */
/* 50 */ u32	trbuf4;	/* TX/RX message buffer Register	    */
	 u32	trbuf5;	/* TX/RX message buffer Register	    */
/* 58 */ u32	trbuf6;	/* TX/RX message buffer Register	    */
	 u32	trbuf7;	/* TX/RX message buffer Register	    */
/* 60 */ u32	trbuf8;	/* TX/RX message buffer Register	    */
	 u32	trbuf9;	/* TX/RX message buffer Register	    */
/* 68 */ u32	trbuf10;	/* TX/RX message buffer Register    */
	 u32	trbuf11;	/* TX/RX message buffer Register    */
/* 70 */ u32	trbuf12;	/* TX/RX message buffer Register    */
} __packed canregs_t;


#define frameinfo trbuf0

/*--- Mode Select Register ---------------------------------*/

#define CAN_MSEL_SLEEP_SEL			(1<<4)
#define CAN_MSEL_ACP_FLT_MOD_SEL		(1<<3)
#define CAN_MSEL_LB_MOD_SEL			(1<<2)	/* Loop Back Mode		*/
#define CAN_MSEL_LST_ONLY_SEL			(1<<1)	/* Listen Only Mode		*/
#define CAN_MSEL_RST_SEL			(1<<0)	/* Reset Mode			*/

/*--- Command Register -------------------------------------*/

#define CAN_CMD_BUS_OFF				(1<<5)	/* Software Bus-OFF Request	*/
#define CAN_CMD_SELF_REQ			(1<<4)	/* Self Reception Request	*/
#define CAN_CMD_CLR_OR_FLAG			(1<<3)	/* Clear Data Overrun Flag	*/
#define CAN_CMD_REL_RX_BUF			(1<<2)	/* Release RX Buffer		*/
#define CAN_CMD_ABT_REQ				(1<<1)	/* Abort Request		*/
#define CAN_CMD_TRANS_REQ			(1<<0)	/* Transmission Request		*/

/*--- Status Register --------------------------------------*/

#define CAN_STA_ERR_CODE_POS			(22)	/* Error Capture Error Code	*/
#define CAN_STA_ERR_CODE_MASK		(0x000003UL <<	CAN_STA_ERR_CODE_POS)
#define CAN_STA_ERR_DIR				(1<<21)	/* Error Capture Direction	*/
							/* 1 - reception, 0 - transmit	*/
#define CAN_STA_ERR_SEG_CODE_POS		(16)	/* Error Capture Segment Code	*/
#define CAN_STA_ERR_SEG_CODE_MASK	(0x00001FUL <<	CAN_STA_ERR_SEG_CODE_POS)
#define CAN_STA_ARB_LOST_POS			(8)	/* Arbitration Lost		*/
#define CAN_STA_ARB_LOST_MASK		(0x00001FUL <<	CAN_STA_ARB_LOST_POS)
#define CAN_STA_BUS_STA				(1<<7)	/* Bus Off Status		*/
#define CAN_STA_ERR_STA				(1<<6)	/* Error Status Warning Limit	*/
#define CAN_STA_TX_STA				(1<<5)	/* Transmit Status - transmitting */
#define CAN_STA_RX_STA				(1<<4)	/* Receive Status - receiving	*/
#define CAN_STA_TX_OVER				(1<<3)	/* Transmission Complete	*/
#define CAN_STA_TX_RDY				(1<<2)	/* TX Buffer Ready		*/
#define CAN_STA_DATA_OR				(1<<1)	/* Data Overrun			*/
#define CAN_STA_RX_RDY				(1<<0)	/* RX Buffer Ready		*/

/*--- Interrupt Register -----------------------------------*/
 
#define CAN_BUS_ERR_INT				(1<<7)
#define CAN_ARBITR_LOST_INT			(1<<6)
#define CAN_ERROR_PASSIVE_INT			(1<<5)
#define CAN_WAKEUP_INT				(1<<4)
#define CAN_OVERRUN_INT				(1<<3)
#define CAN_ERROR_WARN_INT			(1<<2)
#define CAN_TRANSMIT_INT			(1<<1)
#define CAN_RECEIVE_INT 			(1<<0)

/*--- Interrupt enable Reg ---------------------------------*/

#define CAN_BUS_ERR_INT_ENABLE			(1<<7)
#define CAN_ARBITR_LOST_INT_ENABLE		(1<<6)
#define CAN_ERROR_PASSIVE_INT_ENABLE		(1<<5)
#define CAN_WAKEUP_INT_ENABLE			(1<<4)
#define CAN_OVERRUN_INT_ENABLE			(1<<3)
#define CAN_ERROR_WARN_INT_ENABLE		(1<<2)
#define CAN_TRANSMIT_INT_ENABLE			(1<<1)
#define CAN_RECEIVE_INT_ENABLE			(1<<0)

/*--- Error Counter Register ---------------------------------*/

#define CAN_ERRC_RX_CNT_POS			(16)	/* RX Error Counter		*/
#define CAN_ERRC_RX_CNT_MASK		(0x000000FFUL << CAN_ERRC_RX_CNT_POS)
#define CAN_ERRC_TX_CNT_POS			(0)	/* RX Error Counter		*/
#define CAN_ERRC_TX_CNT_MASK		(0x000000FFUL << CAN_ERRC_TX_CNT_POS)

/*--- TX Warning Limit Register  ------------------------------------*/
/* writeable only in Reset Mode */
#define CAN_TEWL_MASK			(0x0ff)	


/*--- Receive Message Counter Register ------------------------------*/

#define CAN_RMCNT_CNT_MASK		(0x000000FFUL)	/* Receive Message Counter	*/




/* All buffer registers are using only bits 7:0 */
/*--- Buffer 0 Register ------------------------------*/
/* Contains the CAN frame information depending on Read or Write access
 * In reset mode contains Acceptance Code 31:0 */
#define acccode trbuf0 

#define CAN_BUF0_DLC_MASK	0x04
#define CAN_BUF0_RTR_POS	(6)
#define CAN_BUF0_RTR_MASK	(1 << CAN_BUF0_RTR_POS)
#define CAN_BUF0_EFF_POS	(7)	/* extended frame format */
#define CAN_BUF0_EFF_MASK	(1 << CAN_BUF0_EFF_POS)

/*--- Buffer 1 Register ------------------------------*/
/* CAN frame Id 28:21
 * In Resetmode contains Acceptance Mask 31:0 */
#define accmask trbuf1



/* generated bit rate table by
 * http://www.bittiming.can-wiki.info
 */
/* TIM0 contains the value of the brp 
 * TIM1 contains TSEG1 and TSEG2
 * TIM0 == BRP is 10 bit,  9:0 
 * TIM1                   23:16 
 */
#if CAN_SYSCLK == 8000000
/* these timings are valid using 8.0 Mhz */
#  define CAN_TIM0_10K		0x31
#  define CAN_TIM1_10K		0x1C

#  define CAN_TIM0_20K		0x18	
#  define CAN_TIM1_20K		0x1c

#  define CAN_TIM0_40K		   0	/* not supported */
#  define CAN_TIM1_40K		   0

#  define CAN_TIM0_50K		0x09
#  define CAN_TIM1_50K		0x1c

#  define CAN_TIM0_100K         0x04 
#  define CAN_TIM1_100K         0x1c

#  define CAN_TIM0_125K		0x03
#  define CAN_TIM1_125K		0x1c

#  define CAN_TIM0_250K		0x01
#  define CAN_TIM1_250K		0x1c

#  define CAN_TIM0_500K		0x00
#  define CAN_TIM1_500K		0x1c
#  define CAN_TIM0_800K		0x00	
#  define CAN_TIM1_800K		0x07
#  define CAN_TIM0_1000K	0x00   
#  define CAN_TIM1_1000K	0x05

#define CAN_SYSCLK_IS_OK            1

#endif

#if CAN_SYSCLK == 24000000
/* these timings are valid using 24.0 Mhz*/
#  define CAN_TIM0_10K		0x95
#  define CAN_TIM1_10K		0x1c	/* 87.5 % */

#  define CAN_TIM0_20K		0x3b	
#  define CAN_TIM1_20K		0x2f

#  define CAN_TIM0_40K		   0	/* not supported */
#  define CAN_TIM1_40K		   0

#  define CAN_TIM0_50K		0x1d
#  define CAN_TIM1_50K		0x1c

#  define CAN_TIM0_100K         0x0e 
#  define CAN_TIM1_100K         0x1c

#  define CAN_TIM0_125K		0x0b
#  define CAN_TIM1_125K		0x1c

#  define CAN_TIM0_250K		0x05
#  define CAN_TIM1_250K		0x1c

#  define CAN_TIM0_500K		0x02
#  define CAN_TIM1_500K		0x1c

#  define CAN_TIM0_800K		0x01	
#  define CAN_TIM1_800K		0x1b	/* 86.7 % */

#  define CAN_TIM0_1000K	0x01   
#  define CAN_TIM1_1000K	0x18	/* 83.3 % */



#define CAN_SYSCLK_IS_OK            1

#endif




/* for more CAN_SYSCLK values */


#ifndef CAN_SYSCLK_IS_OK
#  error Please specify a valid CAN_SYSCLK value (i.e. 8, 10) or define new parameters
#endif



#endif 	/* __CAN_ALLWINNER__ */
