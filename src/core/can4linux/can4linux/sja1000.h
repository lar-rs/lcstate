
#ifndef __CAN_SJA1000__
#define __CAN_SJA1000__

extern u8 CanTiming[10][2];

/*----------*/


/* #ifdef CAN4LINUX_PCI */
#ifdef CPC_PCI
  /* this isn't really a define for a PCI board
   * but some special define for accessing the memory mapped 
   * registers with 4 byte offset 
   */
/* Definition with offset 4, EMS CPC-PCI */
union frame {
	struct {
	    u8	 canid1;
	    u8	 			dummy01;
	    u8	 			dummy02;
	    u8	 			dummy03;
	    u8	 canid2;
	    u8	 			dummy04;
	    u8	 			dummy05;
	    u8	 			dummy06;
	    u8	 canid3;
	    u8	 			dummy07;
	    u8	 			dummy08;
	    u8	 			dummy09;
	    u8	 canid4;
	    u8	 			dummy10;
	    u8	 			dummy11;
	    u8	 			dummy12;
	    /* !! use offset 4 !! */
	    u8    canxdata[8*4];

	} extframe;
	struct {
	    u8	 canid1;
	    u8	 			dummy01;
	    u8	 			dummy02;
	    u8	 			dummy03;
	    u8	 canid2;
	    u8	 			dummy04;
	    u8	 			dummy05;
	    u8	 			dummy06;
	    /* !! use offset 4 !! */
	    u8    candata[8*4];
	} stdframe;
};

typedef struct canregs {
	u8    canmode;		/* 0 */
	u8	 			dummy01;
	u8	 			dummy02;
	u8	 			dummy03;
	u8    cancmd;
	u8	 			dummy04;
	u8	 			dummy05;
	u8	 			dummy06;
	u8    canstat;
	u8	 			dummy07;
	u8	 			dummy08;
	u8	 			dummy09;
	u8	 canirq;
	u8	 			dummy10;
	u8	 			dummy11;
	u8	 			dummy12;
	u8	 canirq_enable;
	u8	 			dummy13;
	u8	 			dummy14;
	u8	 			dummy15;
	u8	 reserved1;		/* 5 */
	u8	 			dummy16;
	u8	 			dummy17;
	u8	 			dummy18;
	u8	 cantim0;
	u8	 			dummy19;
	u8	 			dummy20;
	u8	 			dummy21;
	u8	 cantim1;
	u8	 			dummy22;
	u8	 			dummy23;
	u8	 			dummy24;
	u8	 canoutc;
	u8	 			dummy25;
	u8	 			dummy26;
	u8	 			dummy27;
	u8	 cantest;
	u8	 			dummy28;
	u8	 			dummy29;
	u8	 			dummy30;
	u8	 reserved2;		/* 10 */
	u8	 			dummy31;
	u8	 			dummy32;
	u8	 			dummy33;
	u8	 arbitrationlost;	/* read only */
	u8	 			dummy34;
	u8	 			dummy35;
	u8	 			dummy36;
	u8	 errorcode;		/* read only */
	u8	 			dummy37;
	u8	 			dummy38;
	u8	 			dummy39;
	u8	 errorwarninglimit;
	u8	 			dummy40;
	u8	 			dummy41;
	u8	 			dummy42;
	u8	 rxerror;
	u8	 			dummy43;
	u8	 			dummy44;
	u8	 			dummy45;
	u8	 txerror;		/* 15 */
	u8	 			dummy46;
	u8	 			dummy47;
	u8	 			dummy48;
	u8	 frameinfo;
	u8	 			dummy49;
	u8	 			dummy50;
	u8	 			dummy51;
	union    frame frame;
	u8	 reserved3;
	u8	 			dummy52;
	u8	 			dummy53;
	u8	 			dummy54;
	u8	 canrxbufferadr		/* 30 */;
	u8	 			dummy55;
	u8	 			dummy56;
	u8	 			dummy57;
	u8	 canclk; 	 
	u8	 			dummy58;
	u8	 			dummy59;
	u8	 			dummy60;
} __attribute__((packed)) canregs_t;

#else

/* Standard definition with offset 1
 * also used for CAN_PORT_IO
 * via inb() and outb()
 */
union frame {
	struct {
	    u8	 canid1;
	    u8	 canid2;
	    u8	 canid3;
	    u8	 canid4;
	    u8   canxdata[8];
	} extframe;
	struct {
	    u8	 canid1;
	    u8	 canid2;
	    u8   candata[8];
	} stdframe;
};

typedef struct canregs {
	u8	canmode;		/* 0 */
	u8	cancmd;
	u8	canstat;
	u8	canirq;
	u8	canirq_enable;
	u8	reserved1;		/* 5 */
	u8	cantim0;
	u8	cantim1;
	u8	canoutc;
	u8	cantest;
	u8	reserved2;		/* 10 */
	u8	arbitrationlost;	/* read only */
	u8	errorcode;		/* read only */
	u8	errorwarninglimit;
	u8	rxerror;
	u8	txerror;		/* 15 */
	u8	frameinfo;
	union   frame frame;
	u8	reserved3;
	u8	canrxbufferadr		/* 30 */;
	u8	canclk; 	 
} __attribute__((packed)) canregs_t;
#endif

#if defined(MMC_SJA1000)
#define CAN_RANGE 2		/* CAN_INDEXED_ , only two memory addresses */
#elif defined(CPC_PCI)
#define CAN_RANGE (0x20*4)      /* register offset 4 */
#else
#define CAN_RANGE 0x20		/* default: 32 registers */
#endif

/*--- Mode Register -------- PeliCAN -------------------*/

#  define CAN_SLEEP_MODE		0x10    /* Sleep Mode */
#  define CAN_ACC_FILT_MASK		0x08    /* Acceptance Filter Mask */
#  define CAN_SELF_TEST_MODE		0x04    /* Self test mode */
#  define CAN_LISTEN_ONLY_MODE		0x02    /* Listen only mode */
#  define CAN_RESET_REQUEST		0x01	/* reset mode */
#  define CAN_MODE_DEF CAN_ACC_FILT_MASK	 /* Default ModeRegister Value*/

   /* bit numbers of mode register */
#  define CAN_SLEEP_MODE_BIT		4	/* Sleep Mode */
#  define CAN_ACC_FILT_MASK_BIT		3	/* Acceptance Filter Mask */
#  define CAN_SELF_TEST_MODE_BIT	2	/* Self test mode */
#  define CAN_LISTEN_ONLY_MODE_BIT	1	/* Listen only mode */
#  define CAN_RESET_REQUEST_BIT		0	/* reset mode */


/*--- Interrupt enable Reg -----------------------------*/
#define CAN_BUS_ERR_INT_ENABLE			(1<<7)
#define CAN_ARBITR_LOST_INT_ENABLE		(1<<6)
#define CAN_ERROR_PASSIVE_INT_ENABLE		(1<<5)
#define CAN_WAKEUP_INT_ENABLE			(1<<4)
#define CAN_OVERRUN_INT_ENABLE			(1<<3)
#define CAN_ERROR_WARN_INT_ENABLE		(1<<2)
#define CAN_TRANSMIT_INT_ENABLE			(1<<1)
#define CAN_RECEIVE_INT_ENABLE			(1<<0)

/*--- Frame information register -----------------------*/
#define CAN_EFF				0x80	/* extended frame */
#define CAN_SFF				0x00	/* standard fame format */


/*--- Command Register ------------------------------------*/
 
#define CAN_GOTO_SLEEP				(1<<4)
#define CAN_CLEAR_OVERRUN_STATUS		(1<<3)
#define CAN_RELEASE_RECEIVE_BUFFER		(1<<2)
#define CAN_ABORT_TRANSMISSION			(1<<1)
#define CAN_TRANSMISSION_REQUEST		(1<<0)

/*--- Status Register --------------------------------*/
 
#define CAN_BUS_STATUS 				(1<<7)
#define CAN_ERROR_STATUS			(1<<6)
#define CAN_TRANSMIT_STATUS			(1<<5)
#define CAN_RECEIVE_STATUS			(1<<4)
#define CAN_TRANSMISSION_COMPLETE_STATUS	(1<<3)
#define CAN_TRANSMIT_BUFFER_ACCESS		(1<<2)
#define CAN_DATA_OVERRUN			(1<<1)
#define CAN_RECEIVE_BUFFER_STATUS		(1<<0)

/*--- Status Register --------------------------------*/
 
#define CAN_BUS_STATUS_BIT 			(1<<7)
#define CAN_ERROR_STATUS_BIT			(1<<6)
#define CAN_TRANSMIT_STATUS_BIT			(1<<5)
#define CAN_RECEIVE_STATUS_BIT			(1<<4)
#define CAN_TRANSMISSION_COMPLETE_STATUS_BIT	(1<<3)
#define CAN_TRANSMIT_BUFFER_ACCESS_BIT		(1<<2)
#define CAN_DATA_OVERRUN_BIT			(1<<1)
#define CAN_RECEIVE_BUFFER_STATUS_BIT		(1<<0)

/*--- Interrupt Register -----------------------------------*/
 
#define CAN_BUS_ERR_INT				(1<<7)
#define CAN_ARBITR_LOST_INT			(1<<6)
#define CAN_ERROR_PASSIVE_INT			(1<<5)
#define CAN_WAKEUP_INT				(1<<4)
#define CAN_OVERRUN_INT				(1<<3)
#define CAN_ERROR_WARN_INT			(1<<2)
#define CAN_TRANSMIT_INT			(1<<1)
#define CAN_RECEIVE_INT 			(1<<0)

/*--- Error Code Capture Register ---------------------------*/
#define CAN_ECC_DIRECTION_MASK			(1<<5)
# define CAN_ECC_DIRECTION_TX			0
# define CAN_ECC_DIRECTION_RX			(1<<5)
#define CAN_ECC_SEGMENT_MASK			(0x1F)
# define CAN_ECC_ACT_ERROR_FLAG			(0x11)
# define CAN_ECC_PASS_ERROR_FLAG		(0x16)




/*--- Output Control Register -----------------------------------------*/
/*
 *	7	6	5	4	3	2	1	0
 * 	OCTP1	OCTN1	OCPOL1	OCTP0	OCTN0	OCPOL0	OCMODE1	OCMODE0
 *	----------------------  ----------------------  ---------------
 *	    TX1 Output		    TX0 Output		  programmable
 *	  Driver Control	  Driver Control	  output functions
 *
 *	MODE
 *	OCMODE1	OCMODE0
 *	  0	  1	Normal Mode; TX0, TX1 bit sequence TXData
 *	  1	  1	Normal Mode; TX0 bit sequence, TX1 busclock TXCLK
 *	  0	  0	Biphase Mode
 *	  1	  0	Test Mode; TX0 bit sequence, TX1 COMPOUT
 *
 *	In normal Mode Voltage Output Levels depend on 
 *	Driver Characteristic: OCTPx, OCTNx
 *	and programmed Output Polarity: OCPOLx
 *
 *	Driver Characteristic
 *	OCTPx	OCTNx
 *	  0	 0	always Floating Outputs,
 *	  0	 1	Pull Down
 *	  1	 0	Pull Up
 *	  1	 1	Push Pull
 */
 
/*--- Output control register --------------------------------*/

#define CAN_OCTP1			(1<<7)
#define CAN_OCTN1			(1<<6)
#define CAN_OCPOL1			(1<<5)
#define CAN_OCTP0			(1<<4)
#define CAN_OCTN0			(1<<3)
#define CAN_OCPOL0			(1<<2)
#define CAN_OCMODE1			(1<<1)
#define CAN_OCMODE0			(1<<0)

/*--- Clock Divider register ---------------------------------*/

#define CAN_MODE_BASICCAN		(0x00)
#define CAN_MODE_PELICAN		(0xC0)
#define CAN_MODE_CLK			(0x07)		/* CLK-out = Fclk   */
#define CAN_MODE_CLK2			(0x00)		/* CLK-out = Fclk/2 */


/*--- Remote Request ---------------------------------*/
/*    Notes:
 *    Basic CAN: RTR is Bit 4 in TXDES1.
 *    Peli  CAN: RTR is Bit 6 in frameinfo.
 */
# define CAN_RTR				(1<<6)


/* the base address register array */
/* extern unsigned int Base[]; */

/*---------- Timing values */
/* generated bit rate table by
 * http://www.bittiming.can-wiki.info
 */
#if CAN_SYSCLK == 5
/* these timings are valid for Weidmüller MMC, using 10,0 Mhz*/
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

/* !!! ACHTUNG  the table is exactly for 6.25 Mhz derived from an 12.5
   MHz input clock */
#if CAN_SYSCLK == 6
/* these timings are valid for Weidmüller MMC, using 12.5 Mhz*/
#  define CAN_TIM0_10K		0x18
#  define CAN_TIM1_10K		0x7f
#  define CAN_TIM0_20K		   0	
#  define CAN_TIM1_20K		   0
#  define CAN_TIM0_40K		   0
#  define CAN_TIM1_40K		   0
#  define CAN_TIM0_50K		   4
#  define CAN_TIM1_50K		0x7f
#  define CAN_TIM0_100K            0
#  define CAN_TIM1_100K            0
#  define CAN_TIM0_125K		   4
#  define CAN_TIM1_125K		0x07
#  define CAN_TIM0_250K		   0
#  define CAN_TIM1_250K		0x7f
#  define CAN_TIM0_500K		   0
#  define CAN_TIM1_500K		0x00
#  define CAN_TIM0_800K		   0
#  define CAN_TIM1_800K		0x00
#  define CAN_TIM0_1000K	   0
#  define CAN_TIM1_1000K	0x00

#define CAN_SYSCLK_IS_OK            1
#endif

#if CAN_SYSCLK == 8
/* these timings are valid for clock 8Mhz */
#  define CAN_TIM0_10K		  49
#  define CAN_TIM1_10K		0x1c
#  define CAN_TIM0_20K		  24	
#  define CAN_TIM1_20K		0x1c
#  define CAN_TIM0_40K		0x89	/* Old Bit Timing Standard of port */
#  define CAN_TIM1_40K		0xEB	/* Old Bit Timing Standard of port */
#  define CAN_TIM0_50K		   9
#  define CAN_TIM1_50K		0x1c
#  define CAN_TIM0_100K              4    /* sp 87%, 16 abtastungen, sjw 1 */
#  define CAN_TIM1_100K           0x1c
#  define CAN_TIM0_125K		   3
#  define CAN_TIM1_125K		0x1c
#  define CAN_TIM0_250K		   1
#  define CAN_TIM1_250K		0x1c
#  define CAN_TIM0_500K		   0
#  define CAN_TIM1_500K		0x1c
#  define CAN_TIM0_800K		   0
#  define CAN_TIM1_800K		0x16
#  define CAN_TIM0_1000K	   0
#  define CAN_TIM1_1000K	0x14

#define CAN_SYSCLK_IS_OK            1
#endif


#if CAN_SYSCLK == 10
/* these timings are valid for clock 10Mhz */
/* 20 Mhz cristal */
#  define CAN_TIM0_10K		0x31
#  define CAN_TIM1_10K		0x2f
#  define CAN_TIM0_20K		0x18
#  define CAN_TIM1_20K		0x2f
#  define CAN_TIM0_50K		0x18
#  define CAN_TIM1_50K		0x05
#  define CAN_TIM0_100K		0x04
#  define CAN_TIM1_100K		0x2f
#  define CAN_TIM0_125K		0x04
#  define CAN_TIM1_125K		0x1c
#  define CAN_TIM0_250K		0x04
#  define CAN_TIM1_250K		0x05
#  define CAN_TIM0_500K		0x00
#  define CAN_TIM1_500K		0x2f
#  define CAN_TIM0_800K		0x00
#  define CAN_TIM1_800K		0x00
#  define CAN_TIM0_1000K  	0x00
#  define CAN_TIM1_1000K  	0x07

#define CAN_SYSCLK_IS_OK            1
#endif

#if CAN_SYSCLK == 12
/* these timings are valid for clock 12Mhz */
/* 24 Mhz cristal */
#  define CAN_TIM0_10K		0x31
#  define CAN_TIM1_10K		0x6f
#  define CAN_TIM0_20K		0x18
#  define CAN_TIM1_20K		0x6f
#  define CAN_TIM0_50K		0x0e
#  define CAN_TIM1_50K		0x1c
#  define CAN_TIM0_100K		0x05
#  define CAN_TIM1_100K		0x2f
#  define CAN_TIM0_125K		0x05
#  define CAN_TIM1_125K		0x1c
#  define CAN_TIM0_250K		0x02
#  define CAN_TIM1_250K		0x1c
#  define CAN_TIM0_500K		0x00
#  define CAN_TIM1_500K		0x6f
#  define CAN_TIM0_800K		0x00
#  define CAN_TIM1_800K		0x1b
#  define CAN_TIM0_1000K  	0x00
#  define CAN_TIM1_1000K  	0x09

#define CAN_SYSCLK_IS_OK            1
#endif

#ifndef CAN_SYSCLK_IS_OK
#  error Please specify a valid CAN_SYSCLK value (i.e. 8, 10) or define new parameters
#endif


#endif 	/* __CAN_SJA1000__ */
