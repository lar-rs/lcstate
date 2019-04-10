/* dcan.h 
 * For each CAN controller hardware depending register definitions
 */

#ifndef __CAN_DCAN_
#define __CAN_DCAN_

/* How much  is occupied by the D_CAN module?
256 Byte ID mask storage and 1056 byte of message buffer storage
according Table 24-2 "FlexCAN Module Memory MAP - 0x97F
This is also valid for TX25, for each of the CAN channels
*/
#define CAN_RANGE 0x2000   /* or only 0x980 ?? */

/*=== Register Layout of the FlexCAN module ==============================*/
typedef struct {
    volatile u32     ctl;		/* CAN control register		*/
    volatile u32     es;		/* error und status register	*/
    volatile u32     errc;		/* error counter register	*/
    volatile u32     btr;		/* bit timing register		*/
    volatile u32     intr;		/* interrupt register		*/
    volatile u32     test;		/* test register		*/
    volatile u32     perr;		/* parity error register	*/
    volatile u32     abotr;		/* auto bus on time out register*/
    volatile u32     txrq_x;		/* transmission request reg x	*/
    volatile u32     txrq12;		/* transmission request reg 12	*/
    volatile u32     txrq34;		/* transmission request reg 34	*/
    volatile u32     txrq56;		/* transmission request reg 56	*/
    volatile u32     txrq78;		/* transmission request reg 78	*/
    volatile u32     nwdat_x;		/* New data register x		*/
    volatile u32     nwdat12;		/* New data register 12		*/
    volatile u32     nwdat34;		/* New data register 34		*/
    volatile u32     nwdat56;		/* New data register 56 	*/
    volatile u32     nwdat78;		/* New data register 78		*/
    volatile u32     msgval_x;		/* message valid register x	*/
    volatile u32     msgval12;		/* message valid register 12	*/
    volatile u32     msgval34;		/* message valid register 34	*/
    volatile u32     msgval56;		/* message valid register 56	*/
    volatile u32     msgval78;		/* message valid register 78	*/
    volatile u32     intmux12;		/* interrupt Multiplexer reg 12	*/
    volatile u32     intmux34;		/* interrupt Multiplexer reg 34	*/
    volatile u32     intmux56;		/* interrupt Multiplexer reg 56	*/
    volatile u32     intmux78;		/* interrupt Multiplexer reg 78	*/

    volatile u32     if1cmd;		/* IF1 command register		*/
    volatile u32     if1msk;		/* IF1 mask register		*/
    volatile u32     if1arb;		/* IF1 arbitration register	*/
    volatile u32     if1mctl;		/* IF1 message control register	*/
    volatile u32     if1data;		/* IF1 data A register		*/
    volatile u32     if1datb;		/* IF1 data B register		*/

    volatile u32     if2cmd;		/* IF2 command register		*/
    volatile u32     if2msk;		/* IF2 mask register		*/
    volatile u32     if2arb;		/* IF2 arbitration register	*/
    volatile u32     if2mctl;		/* IF2 message control register	*/
    volatile u32     if2data;		/* IF2 data A register		*/
    volatile u32     if2datb;		/* IF2 data B register		*/

    volatile u32     if3obs;		/* IF3 observation register	*/
    volatile u32     if3msk;		/* IF3 mask register		*/
    volatile u32     if3arb;		/* IF3 arbitration register	*/
    volatile u32     if3mctl;		/* IF3 message control register	*/
    volatile u32     if3data;		/* IF3 data A register		*/
    volatile u32     if3datb;		/* IF3 data B register		*/
    volatile u32     if3upd12;		/* IF3 update enable register 12*/
    volatile u32     if3upd34;		/* IF3 update enable register 34*/
    volatile u32     if3upd56;		/* IF3 update enable register 56*/
    volatile u32     if3upd78;		/* IF3 update enable register 78*/

    volatile u32     tioc;		/* CAN TX IO control register	*/
    volatile u32     rioc;		/* CAN RX IO control register 	*/

    /* end at 0x1e7 */
} canregs_t;


/* Register definitions 
 *
 *
 */
/*--- CTL register -----------------------------------------------------*/
#define CAN_CTL_INIT		0x00000001
#define CAN_CTL_IE0		0x00000002	/* int line 0 enable	*/
#define CAN_CTL_SIE		0x00000004	/* status int enable	*/
#define CAN_CTL_EIE		0x00000008	/* error int enable	*/
#define CAN_CTL_DAR		0x00000020	/* disable auto retrans	*/
#define CAN_CTL_CCE		0x00000040	/* config change enable	*/
#define CAN_CTL_ABO		0x00000200	/* auto bus on enable	*/
#define CAN_CTL_IE1		0x00020000	/* int line 0 enable	*/


/*--- ES  register -----------------------------------------------------*/
#define CAN_ES_LEC		0x00000007	/* last error code	*/
#define CAN_ES_TXOK		0x00000008	/* transmit OK		*/
#define CAN_ES_RXOK		0x00000010	/* received message OK	*/
#define CAN_ES_EPASS		0x00000020	/* error passive state	*/
#define CAN_ES_EWARN		0x00000040	/* warning state	*/
#define CAN_ES_BOFF		0x00000080	/* bus-off state	*/
#define CAN_ES_PERR		0x00000100	/* parity error detected*/


/*--- ERRC  register ---------------------------------------------------*/
#define CAN_ERRC_TEC		0x000000FF	/* tx error counter	*/
#define CAN_ERRC_REC		0x00007F00	/* rx error counter	*/
#define CAN_ERRC_RP		0x00008000	/* receive err passive	*/
/* RP 	0x0 = The receive error counter is below the error passive level.
	0x1 = The receive error counter has reached the error passive level
	as defined in the CAN specification. */


/*--- ERRC  register ---------------------------------------------------*/

#define CAN_BTR_BRP		0x0000003F	/* bit rate pre-scaler	*/
#define CAN_BTR_SJW		0x000000C0	/* sync jump witdh	*/
#define CAN_BTR_TSEG1		0x00000F00	/* Time seg 1		*/
#define CAN_BTR_TSEG2		0x00007000	/* Time seg 2		*/
#define CAN_BTR_BRPE		0x000F0000	/* pre-scaler extension	*/



/*--- Interrupt Register -----------------------------------*/


/* generated bit rate table by
 * http://www.port.de/engl/canprod/sv_req_form.html
 */
#if CAN_SYSCLK == 8
/* these timings are valid for XXXXX, using 8.0 Mhz*/
#  define CAN_TIM0_10K		0x18
#  define CAN_TIM1_10K		0x2f
#  define CAN_TIM0_20K		0x18	
#  define CAN_TIM1_20K		0x07
#  define CAN_TIM0_40K		   0	/* not supported */
#  define CAN_TIM1_40K		   0
#  define CAN_TIM0_50K		0x09
#  define CAN_TIM1_50K		0x07
#  define CAN_TIM0_100K         0x04 
#  define CAN_TIM1_100K         0x07
#  define CAN_TIM0_125K		0x04
#  define CAN_TIM1_125K		0x05
#  define CAN_TIM0_250K		   0
#  define CAN_TIM1_250K		0x2f
#  define CAN_TIM0_500K		   0
#  define CAN_TIM1_500K		0x07
#  define CAN_TIM0_800K		   0	/* not supported */
#  define CAN_TIM1_800K		0x00
#  define CAN_TIM0_1000K	   0	/* not supported */
#  define CAN_TIM1_1000K	0x00

#define CAN_SYSCLK_IS_OK            1

#endif

#if CAN_SYSCLK == 24
/* these timings are valid for XXXXX, using 48.0 Mhz*/



#define CAN_SYSCLK_IS_OK            1

#endif




/* for more CAN_SYSCLK values */


#ifndef CAN_SYSCLK_IS_OK
#  error Please specify a valid CAN_SYSCLK value (i.e. 8, 10) or define new parameters
#endif



#endif 	/* __CAN_DCAN__ */
