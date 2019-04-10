 
/* ###########################################################################
--
-- (c) IFI Franz Sprenger
--
-- file:    ifi_canfd_regs.h
--
-- function:include file : register and bit defines for ifi_can_fd IP-core  
--  
-- data:    1.0 25.01.2013 
--			1.1	15.04.2014 minor clean up
-- 
-- to do:    
--
-- Copyright 2013 IFI Franz Sprenger
-- ###########################################################################
 */
 /*
-- h read only bits, hardware updated automatically
-- w software writable
-- s software write 1 to Set, hardware clears, write 0 no operation
-- c software write 1 to Clear, hardware sets, write 0 no operation
*/

#ifndef __IFI_CANFD_REGS_H__
#define __IFI_CANFD_REGS_H__

//#include <io.h>

// Status and Command /////////////////////////////////////////////////////////
// bit 0      : w general enable  						
// bit 2      : h CAN IP-core in Error Active State
// bit 3      : h CAN IP-core in Error Passive State
// bit 4      : h CAN IP-core in Error Busoff State
// bit 16     : w bus monitoring Mode, Silent Mode 3.3.1
// bit 17     : w restricted operation mode 3.3.2
// bit 18     : w loop back
// bit 19     : w loop back external
// bit 24     : w 1 disable CAN-FD mode
// bit 30     :   can core in hard reset
// bit 31     : s set normal mode
#define STCMD_Hardreset					 (0xDEADCAFDu)
#define IORD_IFI_CANFD_STCMD(base)       IORD(base, 0) 
#define IOWR_IFI_CANFD_STCMD(base, data) IOWR(base, 0, data)

#define IFI_CANFD_STCMD_ENA_MSK          (0x00000001u)	//general enable
#define IFI_CANFD_STCMD_ENA_OFST         (0)
#define IFI_CANFD_STCMD_SEACTIV_MSK      (0x00000004u)	//error active (that is the normal working state)
#define IFI_CANFD_STCMD_SEACTIV_OFST     (2)
#define IFI_CANFD_STCMD_SEPASS_MSK       (0x00000008u)	//error passive
#define IFI_CANFD_STCMD_SEPASS_OFST      (3)
#define IFI_CANFD_STCMD_SEBOFF_MSK       (0x00000010u)	//busoff 
#define IFI_CANFD_STCMD_SEBOFF_OFST      (4)
#define IFI_CANFD_STCMD_M331_MSK         (0x00010000u)	//bus monitoring Mode, Silent Mode 3.3.1
#define IFI_CANFD_STCMD_M331_OFST        (16)
#define IFI_CANFD_STCMD_M332_MSK         (0x00020000u)	//restricted operation mode 3.3.2
#define IFI_CANFD_STCMD_M332_OFST        (17)
#define IFI_CANFD_STCMD_LOOP_MSK         (0x00040000u)	//loop back Mode
#define IFI_CANFD_STCMD_LOOP_OFST        (18)
#define IFI_CANFD_STCMD_LEXT_MSK         (0x00080000u)	//loop back Mode external
#define IFI_CANFD_STCMD_LEXT_OFST        (19)
#define IFI_CANFD_STCMD_EDGEFILT_ENA_MSK (0X00100000U)  // ISO
#define IFI_CANFD_STCMD_EDGEFILT_ENA_OFST (20)
#define IFI_CANFD_STCMD_EXCEPT_ENA_MSK	 (0X00200000U)  // ISO
#define IFI_CANFD_STCMD_EXCEPT_ENA_OFST	 (21)
#define IFI_CANFD_STCMD_64BitTS_ENA_MSK	 (0X00400000U)  // ISO
#define IFI_CANFD_STCMD_64BitTS_ENA_OFST (22)
#define IFI_CANFD_STCMD_noCANFD_MSK      (0x01000000u)	//disable CAN_FD
#define IFI_CANFD_STCMD_noCANFD_OFST     (24)
#define IFI_CANFD_STCMD_ISO_ENA_MSK	 (0x02000000u)  // ISO, 0: non-ISO
#define IFI_CANFD_STCMD_ISO_ENA_OFST	 (25)
#define IFI_CANFD_STCMD__7_9_8_8_BIT_MSK (0x04000000u)  // ISO Bittiming
// the old IFI has the format 4_12_6_6
#define IFI_CANFD_STCMD__7_9_8_8_BIT_OFST (26)
#define IFI_CANFD_STCMD_HARD_RESET_MSK   (0x40000000u)	//can core in hard reset
#define IFI_CANFD_STCMD_HARD_RESET_OFST  (30)
#define IFI_CANFD_STCMD_NM_MSK           (0x80000000u)	//normal mode
#define IFI_CANFD_STCMD_NM_OFST          (31)

// Status and Command Receive-FIFO ////////////////////////////////////////////
// bit 0      : s remove frame
// bit 7      : w cmd reset fifo, must clear this bit to enable transmit fifo
// bit 8      : h fifo empty
// bit 9      : h fifo more than eighth filled
// bit 10     : h fifo more than quarter filled
// bit 11     : h fifo more than half filled
// bit 12     : h fifo full
// bit 13     : h fifo overflow
// bit 31..16 : h fifo fill count
#define IORD_IFI_CANFD_RFIFO(base)       IORD(base, 1) 
#define IOWR_IFI_CANFD_RFIFO(base, data) IOWR(base, 1, data)

#define IFI_CANFD_RFIFO_DEC_MSK          (0x00000001u)   //remove frame
#define IFI_CANFD_RFIFO_DEC_OFST         (0)
#define IFI_CANFD_RFIFO_RES_MSK          (0x00000080u)   //cmd reset fifo
#define IFI_CANFD_RFIFO_RES_OFST         (7)
#define IFI_CANFD_RFIFO_EMPTY_MSK        (0x00000100u)   //fifo empty
#define IFI_CANFD_RFIFO_EMPTY_OFST       (8)
#define IFI_CANFD_RFIFO_EIGHTH_MSK       (0x00000200u)   //fifo less than eighth full
#define IFI_CANFD_RFIFO_EIGHTH_OFST      (9)
#define IFI_CANFD_RFIFO_QUARTER_MSK      (0x00000400u)   //fifo less than quarter full
#define IFI_CANFD_RFIFO_QUARTER_OFST     (10)
#define IFI_CANFD_RFIFO_HALF_MSK         (0x00000800u)   //fifo less than half full
#define IFI_CANFD_RFIFO_HALF_OFST        (11)
#define IFI_CANFD_RFIFO_FULL_MSK         (0x00001000u)   //fifo full
#define IFI_CANFD_RFIFO_FULL_OFST        (12)
#define IFI_CANFD_RFIFO_OVER_MSK         (0x00002000u)   //fifo overflow
#define IFI_CANFD_RFIFO_OVER_OFST        (13)
#define IFI_CANFD_RFIFO_CNT_MSK          (0xffff0000u)   //fifo fill count
#define IFI_CANFD_RFIFO_CNT_OFST         (16)

// Status and Command Transmit-FIFO ///////////////////////////////////////////
// bit 0      : s cmd add frame
// bit 1      : s cmd load high prio
// bit 6      : s remove pending message 
// bit 7      : w cmd reset fifo, must clear this bit to enable transmit fifo
// bit 8      : h fifo empty
// bit 9      : h fifo less than eighth filled
// bit 10     : h fifo less than quarter filled
// bit 11     : h fifo less than half filled
// bit 12     : h fifo full
// bit 13     : h fifo overflow
// bit 14     : h invalid access 
// bit 15     : h high prio pending
// bit 31..16 : h fifo fill count
#define IORD_IFI_CANFD_TFIFO(base)       IORD(base, 2) 
#define IOWR_IFI_CANFD_TFIFO(base, data) IOWR(base, 2, data)

#define IFI_CANFD_TFIFO_ADD_MSK          (0x00000001u)   //cmd add frame
#define IFI_CANFD_TFIFO_ADD_OFST         (0)
#define IFI_CANFD_TFIFO_HIGH_MSK         (0x00000002u)   //cmd load high prio
#define IFI_CANFD_TFIFO_HIGH_OFST        (1)
#define IFI_CANFD_TFIFO_REMOVE_MSK       (0x00000040u)   //remove pending message
#define IFI_CANFD_TFIFO_REMOVE_OFST      (6)
#define IFI_CANFD_TFIFO_RES_MSK          (0x00000080u)   //cmd reset fifo
#define IFI_CANFD_TFIFO_RES_OFST         (7)
#define IFI_CANFD_TFIFO_EMPTY_MSK        (0x00000100u)   //fifo empty
#define IFI_CANFD_TFIFO_EMPTY_OFST       (8)
#define IFI_CANFD_TFIFO_EIGHTH_MSK       (0x00000200u)   //fifo less than eighth full
#define IFI_CANFD_TFIFO_EIGHTH_OFST      (9)
#define IFI_CANFD_TFIFO_QUARTER_MSK      (0x00000400u)   //fifo less than quarter full
#define IFI_CANFD_TFIFO_QUARTER_OFST     (10)
#define IFI_CANFD_TFIFO_HALF_MSK         (0x00000800u)   //fifo less than half full
#define IFI_CANFD_TFIFO_HALF_OFST        (11)
#define IFI_CANFD_TFIFO_FULL_MSK         (0x00001000u)   //fifo full
#define IFI_CANFD_TFIFO_FULL_OFST        (12)
#define IFI_CANFD_TFIFO_OVER_MSK         (0x00002000u)   //fifo overflow
#define IFI_CANFD_TFIFO_OVER_OFST        (13)
#define IFI_CANFD_TFIFO_INVALID_MSK      (0x00004000u)   //invalid access
#define IFI_CANFD_TFIFO_INVALID_OFST     (14)
#define IFI_CANFD_TFIFO_HPEND_MSK        (0x00008000u)   //high prio pending
#define IFI_CANFD_TFIFO_HPEND_OFST       (15)
#define IFI_CANFD_TFIFO_CNT_MSK          (0xffff0000u)   //fifo fill count
#define IFI_CANFD_TFIFO_CNT_OFST         (16)

// Interrupt Mask /////////////////////////////////////////////////////////////
// bit 7  : write enable for Error Interrupt mask
// bit 15 : write enable for Counter Overrun interrupt mask
// bit 23 : write enable for transmit fifo interrupt mask
// bit 31 : write enable for receive fifo interrupt mask

// bit 0      : c Interrupt Status/Mask CAN-Status busoff
// bit 1      : c Interrupt Status/Mask CAN-Status error warning

// bit 8      : c Interrupt Status/Mask TimeStamp Counter overrun into Bit32
// bit 9      : c Interrupt Status/Mask Error Counter overrun into bit 7

// bit 16     : c Interrupt Status/Mask Transmit Fifo empty
// bit 17     : c Interrupt Status/Mask Transmit Fifo less than eighth filled
// bit 18     : c Interrupt Status/Mask Transmit Fifo less than quarter filled
// bit 19     : c Interrupt Status/Mask Transmit Fifo less than half filled
// bit 20     : c Interrupt Status/Mask Transmit Fifo full
// bit 21     : c Interrupt Status/Mask Transmit Fifo Overflow
// bit 22     : c Interrupt Status/Mask Transmit one message removed  (transmit OK)

// bit 24     : c Interrupt Status/Mask Receive Fifo not empty
// bit 25     : c Interrupt Status/Mask Receive Fifo permanent not empty
// bit 26     : c Interrupt Status/Mask Receive Fifo more than half filled
// bit 27     : c Interrupt Status/Mask Receive Fifo more than three quarter filled
// bit 28     : c Interrupt Status/Mask Receive Fifo more than seven eighth filled
// bit 29     : c Interrupt Status/Mask Receive Fifo full
// bit 30     : c Interrupt Status/Mask Receive Fifo overrun

// bit 31     : w enable direct writing to interrupt pending register for software test
#define IORD_IFI_CANFD_IRQMASK(base)           IORD(base, 4) 
#define IOWR_IFI_CANFD_IRQMASK(base, data)     IOWR(base, 4, data)


#define IFI_CANFD_INT_MEBOFF_MSK        (0x00000001u)   //Int CAN-Status busoff
#define IFI_CANFD_INT_MEBOFF_OFST       (0)
#define IFI_CANFD_INT_MEWARN_MSK        (0x00000002u)   //Int CAN-Status error warning
#define IFI_CANFD_INT_MEWARN_OFST       (1)

#define IFI_CANFD_INT_TCNT_MSK          (0x00000100u)   //TimeStamp Counter overrun into Bit32
#define IFI_CANFD_INT_TCNT_OFST         (8)
#define IFI_CANFD_INT_ECNT_MSK          (0x00000200u)   //Error Counter overrun into bit 7
#define IFI_CANFD_INT_ECNT_OFST         (9)

#define IFI_CANFD_INT_TEMPTY_MSK        (0x00010000u)   //Transmit Fifo empty 
#define IFI_CANFD_INT_TEMPTY_OFST       (16)
#define IFI_CANFD_INT_T8FULL_MSK        (0x00020000u)   //Transmit Fifo less than eighth filled
#define IFI_CANFD_INT_T8FULL_OFST       (17)
#define IFI_CANFD_INT_T4FULL_MSK        (0x00040000u)   //Transmit Fifo less than quarter filled 
#define IFI_CANFD_INT_T4FULL_OFST       (18)
#define IFI_CANFD_INT_T2FULL_MSK        (0x00080000u)   //Transmit Fifo less than half filled
#define IFI_CANFD_INT_T2FULL_OFST       (19)
#define IFI_CANFD_INT_TFULL_MSK         (0x00100000u)   //Transmit Fifo full
#define IFI_CANFD_INT_TFULL_OFST        (20)
#define IFI_CANFD_INT_TOVER_MSK         (0x00200000u)   //Transmit Fifo Overflow
#define IFI_CANFD_INT_TOVER_OFST        (21)
#define IFI_CANFD_INT_TREMOVE_MSK       (0x00400000u)   //Transmit one message removed  
#define IFI_CANFD_INT_TREMOVE_OFST      (22)

#define IFI_CANFD_INT_RNEMPTY_MSK       (0x01000000u)   //Receive Fifo not empty
#define IFI_CANFD_INT_RNEMPTY_OFST      (24)
#define IFI_CANFD_INT_RPNEMPTY_MSK      (0x02000000u)   //Receive Fifo permanent not empty
#define IFI_CANFD_INT_RPNEMPTY_OFST     (25)
#define IFI_CANFD_INT_R2FULL_MSK        (0x04000000u)   //Receive Fifo more than half filled
#define IFI_CANFD_INT_R2FULL_OFST       (26)
#define IFI_CANFD_INT_R34FULL_MSK       (0x08000000u)   //Receive Fifo more than three quarter filled
#define IFI_CANFD_INT_R34FULL_OFST      (27)
#define IFI_CANFD_INT_R78FULL_MSK       (0x10000000u)   //Receive Fifo more than seven eighth filled
#define IFI_CANFD_INT_R78FULL_OFST      (28)
#define IFI_CANFD_INT_RFULL_MSK         (0x20000000u)   //Receive Fifo full
#define IFI_CANFD_INT_RFULL_OFST        (29)
#define IFI_CANFD_INT_ROVER_MSK         (0x40000000u)   //Receive Fifo overrun
#define IFI_CANFD_INT_ROVER_OFST        (30)

// write enable for IRQ mask //////////////////////////////////////////////////
#define IFI_CANFD_INT_M_ERR_MSK         (0x00000080u)   //write enable error IRQ mask
#define IFI_CANFD_INT_M_ERR_OFST        (7)
#define IFI_CANFD_INT_M_OVER_MSK        (0x00008000u)   //write enable overrun IRQ mask
#define IFI_CANFD_INT_M_OVER_OFST       (15)
#define IFI_CANFD_INT_M_TFIFO_MSK       (0x00800000u)   //write enable transmit fifo irq
#define IFI_CANFD_INT_M_TFIFO_OFST      (23)
#define IFI_CANFD_INT_M_RFIFO_MSK       (0x80000000u)   //write enable receive fifo irq
#define IFI_CANFD_INT_M_RFIFO_OFST      (31)

// Interrupt pending, Interrupt Status, no function off interrupt mask ////////
// bit 0      : c Interrupt Status/Mask CAN-Status busoff
// bit 1      : c Interrupt Status/Mask CAN-Status error warning

// bit 8      : c Interrupt Status/Mask TimeStamp Counter overrun into Bit32
// bit 9      : c Interrupt Status/Mask Error Counter overrun into bit 7

// bit 16     : c Interrupt Status/Mask Transmit Fifo empty
// bit 17     : c Interrupt Status/Mask Transmit Fifo less than eighth filled
// bit 18     : c Interrupt Status/Mask Transmit Fifo less than quarter filled
// bit 19     : c Interrupt Status/Mask Transmit Fifo less than half filled
// bit 20     : c Interrupt Status/Mask Transmit Fifo full
// bit 21     : c Interrupt Status/Mask Transmit Fifo Overflow
// bit 22     : c Interrupt Status/Mask Transmit one message removed  (transmit OK)

// bit 24     : c Interrupt Status/Mask Receive Fifo not empty
// bit 25     : c Interrupt Status/Mask Receive Fifo permanent not empty
// bit 26     : c Interrupt Status/Mask Receive Fifo more than half filled
// bit 27     : c Interrupt Status/Mask Receive Fifo more than three quarter filled
// bit 28     : c Interrupt Status/Mask Receive Fifo more than seven eighth filled
// bit 29     : c Interrupt Status/Mask Receive Fifo full
// bit 30     : c Interrupt Status/Mask Receive Fifo overrun
#define IORD_IFI_CANFD_INT(base)        IORD(base, 3) 
#define IOWR_IFI_CANFD_INT(base, data)  IOWR(base, 3, data)
// use bits identical to Interrupt Mask ///////////////////////////////////////


// Bit Timing for arbitration and dataphase ///////////////////////////////////
// bit 5..0   : timeb
// bit 6      : write sjw
// bit 7      : write timeb
// bit 13..8  : timea
// bit 14     : write prescale
// bit 15     : write timea
// bit 27..16 : prescale
// bit 31..28 : sjw
#define IORD_IFI_CANFD_TIME(base)           IORD(base, 5) 
#define IOWR_IFI_CANFD_TIME(base, data)     IOWR(base, 5, data)
#define IORD_IFI_CANFD_FTIME(base)          IORD(base, 6) 
#define IOWR_IFI_CANFD_FTIME(base, data)    IOWR(base, 6, data)

#define IFI_CANFD_TIME_WTB_MSK          	(0x0000003Fu)	//timeb
#define IFI_CANFD_TIME_WTB_OFST         	(0)
#define IFI_CANFD_TIME_JON_MSK          	(0x00000040u)	//write enable sjw
#define IFI_CANFD_TIME_JON_OFST         	(6)
#define IFI_CANFD_TIME_BON_MSK          	(0x00000080u)	//write enable timeb
#define IFI_CANFD_TIME_BON_OFST         	(7)
#define IFI_CANFD_TIME_WTA_MSK          	(0x00003F00u)   //timea
#define IFI_CANFD_TIME_WTA_OFST         	(8)
#define IFI_CANFD_TIME_VON_MSK          	(0x00004000u)	//write enable prescale
#define IFI_CANFD_TIME_VON_OFST         	(14)
#define IFI_CANFD_TIME_AON_MSK          	(0x00008000u)	//write enable timea
#define IFI_CANFD_TIME_AON_OFST         	(15)
#define IFI_CANFD_TIME_VT_MSK           	(0x0fFF0000u)	//prescale
#define IFI_CANFD_TIME_VT_OFST          	(16)
#define IFI_CANFD_TIME_WSW_MSK          	(0xf0000000u)	//sjw
#define IFI_CANFD_TIME_WSW_OFST         	(28)

// transmitter delay used in fast dataphase ///////////////////////////////////
// bit 7..0   : additional transmitter delay in can_clock_ticks
// bit 8  	  : enable transmitter delay
// bit 31..16 : meassured transmitter loop delay in can_clock_ticks
#define IORD_IFI_CANFD_TDELAY(base)         IORD(base, 7) 
#define IOWR_IFI_CANFD_TDELAY(base, data)   IOWR(base, 7, data)

#define IFI_CANFD_TDELAY_DLY_MSK            (0x00000FFFu)   // in clock ticks
#define IFI_CANFD_TDELAY_DLY_OFST           (0)
#define IFI_CANFD_TDELAY_ADLY_MSK           (0x00004000u)   //
#define IFI_CANFD_TDELAY_ADLY_OFST          (14)
#define IFI_CANFD_TDELAY_ENA_MSK            (0x00008000u)   //
#define IFI_CANFD_TDELAY_ENA_OFST           (15)
#define IFI_CANFD_TDELAY_MDLY_MSK           (0xFFFF0000u)   // measured tx delay
#define IFI_CANFD_TDELAY_MDLY_OFST          (16)	

// CAN error counter read only ////////////////////////////////////////////////
// bit 0..8   : Transmit error counter 
// bit 16..23 : Receive error counter
#define IORD_IFI_CANFD_ERROR(base)          IORD(base, 8) 

#define IFI_CANFD_ERROR_TRA_MSK             (0x000001FFu)   //transmit error counter
#define IFI_CANFD_ERROR_TRA_OFST            (0)
#define IFI_CANFD_ERROR_REC_MSK             (0x00FF0000u)   //receive error counter
#define IFI_CANFD_ERROR_REC_OFST            (16)

// additional error counters arbitration phase data phase read only ///////////
// each error increments by 1, cleared when setting normal mode bit
// bit 7..0   : transmit error in arbitration phase
// bit 15..8  : transmit error in data phase
// bit 23..16 : receive error in arbitration phase
// bit 31..24 : receive error in data phase
#define IORD_IFI_CANFD_ERRCNT(base)         IORD(base, 9) 

#define IFI_CANFD_ERRCNT_TRAARBIT_MSK       (0x000000FFu)   //transmit error during arbitration
#define IFI_CANFD_ERRCNT_TRAARBIT_OFST      (0)
#define IFI_CANFD_ERRCNT_TRADATA_MSK        (0x0000FF00u)   //transmit error during data
#define IFI_CANFD_ERRCNT_TRADATA_OFST       (8)
#define IFI_CANFD_ERRCNT_RECARBIT_MSK       (0x00FF0000u)   //receive error during arbitration
#define IFI_CANFD_ERRCNT_RECARBIT_OFST      (16)
#define IFI_CANFD_ERRCNT_RECDATA_MSK        (0xFF000000u)   //receive error during data
#define IFI_CANFD_ERRCNT_RECDATA_OFST       (24)

// bit 0..23  : default transmission suspend in us ////////////////////////////
#define IORD_IFI_CANFD_SUSPEND(base)         IORD(base, 10)  
#define IOWR_IFI_CANFD_SUSPEND(base, data)   IOWR(base, 10, data)

#define IFI_CANFD_SUSPEND_MSK                (0x00FFFFFFu)   
#define IFI_CANFD_SUSPEND_OFST          	 (0)

// bit 15..0  : default transmission repeat count /////////////////////////////
// used when not set individually in transmit buffer
// bit 15..0  : 0 => repeat transmission endless (until successfully)
// bit 15..0  : 1 => just one try, no repeat (single shot mode)
// bit 15..0  : 2 => repeat ones
// bit 15..0  : 8 => repeat 7 times, so make 8 attempts to transmit 
#define IORD_IFI_CANFD_REPEAT(base)          IORD(base, 11)  
#define IOWR_IFI_CANFD_REPEAT(base, data)    IOWR(base, 11, data)

#define IFI_CANFD_REPEAT_MSK                 (0x0000FFFFu)   
#define IFI_CANFD_REPEAT_OFST          		 (0)

// bit 31..0 bus traffic rate /////////////////////////////////////////////////
// bit 15..0   : traffic rate 0 to 100% (0 to 65535)
// bit 29..0   : traffic frames 
// bit 30 	   : w 1 count frames, else in % 
// bit 31 	   : s 1 restart bus traffic rate
#define IORD_IFI_CANFD_TRAFFIC(base)         IORD(base, 12) 
#define IOWR_IFI_CANFD_TRAFFIC(base,data)    IOWR(base, 12, data) 

#define IFI_CANFD_TRAFFIC_RATE_MSK           (0x0000FFFFu)   //rate in 0 to 100%
#define IFI_CANFD_TRAFFIC_RATE_OFST          (0)
#define IFI_CANFD_TRAFFIC_FRAMES_MSK         (0x3FFFFFFFu)   //frames 
#define IFI_CANFD_TRAFFIC_FRAMES_OFST        (0)
#define IFI_CANFD_TRAFFIC_CNTFRAME_MSK       (0x40000000u)   //count frames
#define IFI_CANFD_TRAFFIC_CNTFRAME_OFST      (30)
#define IFI_CANFD_TRAFFIC_RESTART_MSK        (0x80000000u)   //restart bus traffic rate
#define IFI_CANFD_TRAFFIC_RESTART_OFST       (31)

// bit 31..0  : Time Stamp counter control ///////////////////////////////////
// bit 0 	  : s write 1 resets Time Stamp Counter (auto clear)
// bit 1 	  : w 1 use external TimeStampReset
// bit 2 	  : w 1 use external TimeStampClock, else internal 1 us
// bit 3 	  : w 1 sel SOF as Trigger for TimeStamp, else use ACK
// bit 31 	  : c 1 TimeStamp Counter overrun, write 1 to clear overrun bit
#define IORD_IFI_CANFD_TSCTRL(base)       IORD(base,13)   
#define IOWR_IFI_CANFD_TSCTRL(base,data)  IOWR(base,13, data)   

#define IFI_CANFD_TSCTRL_RESET_MSK        (0x00000001U)   
#define IFI_CANFD_TSCTRL_RESET_OFST       (0)
#define IFI_CANFD_TSCTRL_extRESET_MSK     (0x00000002u)   
#define IFI_CANFD_TSCTRL_extRESET_OFST    (1)
#define IFI_CANFD_TSCTRL_extCLOCK_MSK     (0x00000004u)   
#define IFI_CANFD_TSCTRL_extCLOCK_OFST    (2)
#define IFI_CANFD_TSCTRL_useSOF_MSK       (0x00000008u)   
#define IFI_CANFD_TSCTRL_useSOF_OFST      (3)
#define IFI_CANFD_TSCTRL_OVER_MSK         (0x80000000u)   
#define IFI_CANFD_TSCTRL_OVER_OFST        (31)
	
// bit 31..0  : actual running timestamp counter read only ////////////////////
#define IORD_IFI_CANFD_TSC(base)       IORD(base,14)  
	
// bit 31..0  : last stored timestamp read only ///////////////////////////////
#define IORD_IFI_CANFD_TST(base)       IORD(base,15)   

// bit 31..0  : reserved 1 do not write ///////////////////////////////////////
#define IORD_IFI_CANFD_RES1(base)      IORD(base,16)  

// bit 31..0  : reserved 2 do not write ///////////////////////////////////////
#define IORD_IFI_CANFD_RES2(base)      IORD(base,17)  
 
// bit 31..0  : Compiler Parameter read only //////////////////////////////////
// bit 7..0   : transmit fifo size in kbyte
// bit 15..8  : receive fifo size in kbyte
// bit 16 	  : 1 use Single Clock, 	else use dual Clock for system and CAN
// bit 18 	  : 1 no TimeStamp, 		else use Timestamp hardware (with / without external IOs)
// bit 19 	  : 1 no Filter,     		else use Filter hardware
// bit 20 	  : 1 no Bus Statistic, 	else use Bus Statistic hardware
#define IORD_IFI_CANFD_PAR(base)       IORD(base,18)   

#define IFI_CANFD_PAR_TXSZ_MSK         (0x000000FFu)   
#define IFI_CANFD_PAR_TXSZ_OFST        (0)
#define IFI_CANFD_PAR_RXSZ_MSK         (0x0000FF00u)   
#define IFI_CANFD_PAR_RXSZ_OFST        (8)
#define IFI_CANFD_PAR_SCLK_MSK         (0x00010000u)   
#define IFI_CANFD_PAR_SCLK_OFST        (16)
#define IFI_CANFD_PAR_NO_TSTAMP_MSK    (0x00020000u)   
#define IFI_CANFD_PAR_NO_TSTAMP_OFST   (17)
#define IFI_CANFD_PAR_NO_FILTER_MSK    (0x00040000u)   
#define IFI_CANFD_PAR_NO_FILTER_OFST   (18)
#define IFI_CANFD_PAR_NO_BUSSTATS_MSK  (0x00080000u)   
#define IFI_CANFD_PAR_NO_BUSSTATS_OFST (19)
#define IFI_CANFD_PAR_CANID_MSK	       (0x00300000u)
#define IFI_CANFD_PAR_CANID_OFST       (20)

// bit 31..0  : clock parameter in Hz read only ///////////////////////////////
#define IORD_IFI_CANFD_CANCLOCK(base)     IORD(base,19)
#define IORD_IFI_CANFD_SYSCLOCK(base)     IORD(base,20)
 
// bit 31..0  : Revision of IP-core read only /////////////////////////////////
// bit 7..0   : core revision
// bit 15..8  : minimum Quartus version
// bit 23..16 : Year
// bit 31..24 : Month
#define IORD_IFI_CANFD_VER(base)          IORD(base,21)   

// bit 31..0  : IP core ID 0xD073CAFD read only ///////////////////////////////
// 0xD073 for IFI
// 0xCAFD for CANFD
#define IORD_IFI_CANFD_IP(base)           IORD(base,22)   

// bit 31..0  : test register /////////////////////////////////////////////////
// resetvalue 0xAFFEDEAD
// write bit-invers
#define IORD_IFI_CANFD_TEST(base)         IORD(base,23)   
#define IOWR_IFI_CANFD_TEST(base, data)   IOWR(base,23,data)   


///////////////////////////////////////////////////////////////////////////////
// Part 2 receive FIFO ////////////////////////////////////////////////////////
// read only !! 
// offset to base is 24 words (0x60 or 96 bytes)

// individual transmit repeat count ///////////////////////////////////////////
// bit 15..0 : used transmit repeat count when own receive enabled
#define IORD_IFI_CANFD_RPTCOUNT(base)    IORD(base, 24+0) 

#define IFI_CANFD_RPTCOUNT_MSK           (0x0000FFFFu)   // 
#define IFI_CANFD_RPTCOUNT_OFST          (0)

// bit 31..0 : time stamp
#define IORD_IFI_CANFD_TIMESTAMP(base)   IORD(base, 24+1) 

// receive DLC Data Length Code  //////////////////////////////////////////////
// bit 3..0  : DLC Data Length Code
// bit 4     : RTR Remote Transmission Request
// bit 5     : EDL Extended Data Length (CAN-FD) 
// bit 6     : BRS Bit Rate Switch
// bit 7     : ESI Error State Indicator 
// bit 16..8 : filter match object number 
// bit 31..24: frame number own receive when > 0
#define IORD_IFI_CANFD_DLC(base)        IORD(base, 24+2) 

#define IFI_CANFD_DLC_DLC_MSK           (0x0000000Fu)   //data length code
#define IFI_CANFD_DLC_DLC_OFST          (0)
#define IFI_CANFD_DLC_RTR_MSK           (0x00000010u)   //remote bit
#define IFI_CANFD_DLC_RTR_OFST          (4)
#define IFI_CANFD_DLC_EDL_MSK           (0x00000020u)    //can FD
#define IFI_CANFD_DLC_EDL_OFST          (5)
#define IFI_CANFD_DLC_BRS_MSK           (0x00000040u)   //BaudRateSwitch
#define IFI_CANFD_DLC_BRS_OFST          (6)
#define IFI_CANFD_DLC_ESI_MSK           (0x00000080u)   //ESI bit
#define IFI_CANFD_DLC_ESI_OFST          (7)
#define IFI_CANFD_DLC_OBJ_MSK           (0x0001FF00u)   //object id for 256 Objects
#define IFI_CANFD_DLC_OBJ_OFST          (8)
#define IFI_CANFD_DLC_FRN_MSK           (0xFF000000u)   //frame_number
#define IFI_CANFD_DLC_FRN_OFST          (24)

// receive ID identifier ////////////////////////////////////////////////////
// IFI_legacy
// bit 0..10  : identifier
// bit 11..28 : additional bits for extended identifier
// bit 29     : use extended identifier
#define IORD_IFI_CANFD_IDX(base)        IORD(base, 24+3) 

#define IFI_CANFD_IDX_ID_MSK            (0x000007FFu)   //standard id
#define IFI_CANFD_IDX_ID_OFST           (0)
#define IFI_CANFD_IDX_IDX_MSK           (0x1FFFF800u)   //additional bits for extended id
#define IFI_CANFD_IDX_IDX_OFST          (0)
#define IFI_CANFD_IDX_ON_MSK            (0x20000000u)   //use extended
#define IFI_CANFD_IDX_ON_OFST           (29)
#define IFI_CANFD_IDX_MSK		(0x1FFFFFFFu)   // complete Id mask 


/* In IFI legacy mode 

Convert extended ID from CANalyzer to IFI_legacy
− transmitID = ((value & 0x3FFFF)<<11) + ((value &
  0x1FFC0000)>>18);
− transmitID |= 0x20000000; // set IDE for extended ID !!!

Convert extended IFI_legacy to CANalyzer
− value = ((receiveID & 0x7FF)<<18) + ((receiveID &
  0x1FFFF800)>>11);
− value &= ~0x20000000; // mask the IDE
*/
#define IFI_CANFD_IDX_18_28  0x000007FF
#define IFI_CANFD_IDX_SHIFT_L	(11)
#define IFI_CANFD_IDX_00_17  0x1FFFF800
#define IFI_CANFD_IDX_SHIFT_H	(18)

// receive ID identifier ////////////////////////////////////////////////////
// CANalyzer
// bit 0..28  : identifier linear
// bit 29     : use extended identifier
#define IORD_IFI_CANFD_IDX(base)        IORD(base, 24+3) 

#define IFI_CANFD_IDX1ID_MSK            (0x1FFFFFFFu)   //id
#define IFI_CANFD_IDX1ID_OFST           (0)
#define IFI_CANFD_IDX_ON_MSK            (0x20000000u)   //use extended
#define IFI_CANFD_IDX_ON_OFST           (29)

// treceive ID identifier ////////////////////////////////////////////////////
// ID_other (M_CAN...)
// bit 0..17  : additional bits for extended ID
// bit 18..28 : standard identifier
// bit 29     : use extended identifier
#define IORD_IFI_CANFD_IDX(base)        IORD(base, 24+3) 

#define IFI_CANFD_IDX2ID_MSK            (0x1FFC0000u)   //standard id
#define IFI_CANFD_IDX2ID_OFST           (0)
#define IFI_CANFD_IDX2IDX_MSK           (0x1FFFFFFFu)   //extended id
#define IFI_CANFD_IDX2IDX_OFST          (0)
#define IFI_CANFD_IDX_ON_MSK            (0x20000000u)   //use extended
#define IFI_CANFD_IDX_ON_OFST           (29)

// receive data //////////////////////////////////////////////////////////////
// maximum 16 dwords or 64 bytes
// bit 0..7   : data byte 1
// bit 8..15  : data byte 2
// bit 16..23 : data byte 3
// bit 24..31 : data byte 4
#define IORD_IFI_CANFD_DATA(base,ix)    IORD(base, 24+4+ix) 


///////////////////////////////////////////////////////////////////////////////
// Part 3 transmit FIFO ///////////////////////////////////////////////////////
// write only !! 
// offset to base is 44 words (0xb0 or 176 bytes)

// individual suspend and repeat count /////////////////////////////////////////
// bit 23..0 : transmission suspend in us (micro-seconds)
#define IOWR_IFI_CANFD_TX_SUSPEND(base, data)   IOWR(base, 44+0, data)

#define IFI_CANFD_TX_SUSPEND_MSK           (0x00FFFFFFu)   // suspend in us
#define IFI_CANFD_TX_SUSPEND_OFST          (0)

// bit 15..0 : maximum transmission repeat
// 0         : endless
// 1         : one try => single shot mode
// 2         : repeat ones
// example 8 : make maximum 8 attempts to transmit message

#define IOWR_IFI_CANFD_TX_REPEAT(base, data)     IOWR(base, 44+1, data)

#define IFI_CANFD_TX_REPEAT_MSK           (0x0000FFFFu)   // suspend in us
#define IFI_CANFD_TX_REPEAT_OFST          (0)

// transmit DLC Data Length Code  /////////////////////////////////////////////
// bit 3..0  : DLC Data Length Code
// bit 4     : RTR Remote Transmission Request
// bit 5     : EDL Extended Data Length (CAN-FD) 
// bit 6     : BRS Bit Rate Switch
// bit 31..24: frame number, if > 0 enables own receive
#define IOWR_IFI_CANFD_DLC(base, data)     IOWR(base, 44+2, data)

#define IFI_CANFD_DLC_DLC_MSK           (0x0000000Fu)   //data length code
#define IFI_CANFD_DLC_DLC_OFST          (0)
#define IFI_CANFD_DLC_RTR_MSK           (0x00000010u)   //remote bit
#define IFI_CANFD_DLC_RTR_OFST          (4)
#define IFI_CANFD_DLC_EDL_MSK           (0x00000020u)    //can FD
#define IFI_CANFD_DLC_EDL_OFST          (5)
#define IFI_CANFD_DLC_BRS_MSK           (0x00000040u)   //BaudRateSwitch
#define IFI_CANFD_DLC_BRS_OFST          (6)
#define IFI_CANFD_DLC_FRN_MSK           (0xFF000000u)   //frame_number
#define IFI_CANFD_DLC_FRN_OFST          (24)

// transmit ID identifier ////////////////////////////////////////////////////
// IFI_legacy
// bit 0..10  : identifier
// bit 11..28 : extended identifier
// bit 29     : use extended identifier
#define IOWR_IFI_CANFD_IDX(base, data)  IOWR(base, 44+3, data)

#define IFI_CANFD_IDX_ID_MSK            (0x000007FFu)   //standard id
#define IFI_CANFD_IDX_ID_OFST           (0)
#define IFI_CANFD_IDX_IDX_MSK           (0x1FFFF800u)   //extended id
#define IFI_CANFD_IDX_IDX_OFST          (0)
#define IFI_CANFD_IDX_ON_MSK            (0x20000000u)   //use extended
#define IFI_CANFD_IDX_ON_OFST           (29)

// transmit ID identifier ////////////////////////////////////////////////////
// CANalyzer
// bit 0..28  : identifier linear
// bit 29     : use extended identifier
#define IOWR_IFI_CANFD_IDX(base, data)  IOWR(base, 44+3, data)

#define IFI_CANFD_IDX1ID_MSK            (0x1FFFFFFFu)   //id
#define IFI_CANFD_IDX1ID_OFST           (0)
#define IFI_CANFD_IDX_ON_MSK            (0x20000000u)   //use extended
#define IFI_CANFD_IDX_ON_OFST           (29)

// transmit ID identifier ////////////////////////////////////////////////////
// ID_other (M_CAN...)
// bit 0..17  : additional bits for exteneded ID
// bit 18..28 : standard identifier
// bit 29     : use extended identifier
#define IOWR_IFI_CANFD_IDX(base, data)  IOWR(base, 44+3, data)

#define IFI_CANFD_IDX2ID_MSK            (0x1FFC0000u)   //standard id
#define IFI_CANFD_IDX2ID_OFST           (0)
#define IFI_CANFD_IDX2IDX_MSK           (0x1FFFFFFFu)   //extended id
#define IFI_CANFD_IDX2IDX_OFST          (0)
#define IFI_CANFD_IDX_ON_MSK            (0x20000000u)   //use extended
#define IFI_CANFD_IDX_ON_OFST           (29)


// transmit data //////////////////////////////////////////////////////////////
// maximum 16 dwords or 64 bytes, no byte-enables !
// bit 0..7   : data byte 1
// bit 8..15  : data byte 2
// bit 16..23 : data byte 3
// bit 24..31 : data byte 4
#define IOWR_IFI_CANFD_DATA(base,ix, data) IOWR(base, 44+4+ix, data)


///////////////////////////////////////////////////////////////////////////////
// Part 4 Filter-Mask when enabled  ///////////////////////////////////////////
// offset to base is 512 words (0x800 or 2048 bytes)
// IFI_legacy, CANalyzer, ID_other
// for standard / extended ID order see receive / transmit FIFO
// 1. word bit 0..28  : filter identifier mask
// 1. word bit 29     : filter extended messages mask
// 1. word bit 30     : filter can fd messages mask
// 1. word bit 31     : filter mask valid
// 2. word bit 0..28  : filter identifier 
// 2. word bit 29     : filter extended/standard
// 2. word bit 30     : filter can fd/normal can
// 2. word bit 31     : filter identifier valid
// ......
// hint: enable all IDs with writing:  
// IOWR_IFI_CANFD_FILTER(base,0,CANFD_FILTER_enaALL);
// IOWR_IFI_CANFD_FILTER(base,1,CANFD_FILTER_enaALL);
#define CANFD_FILTER_enaALL				  (0x80000000u)

#define IORD_IFI_CANFD_FILTER(base, index)           IORD(base, (512+index)) 
#define IOWR_IFI_CANFD_FILTER(base, index, data)     IOWR(base, (512+index), data)

#define IFI_CANFD_FILTER_MASK_MSK         (0x1FFFFFFFu)    //filter mask or identifier
#define IFI_CANFD_FILTER_MASK_OFST        (0)
#define IFI_CANFD_FILTER_EXT_MSK          (0x20000000u)    //filter mask or identifier extended/standard
#define IFI_CANFD_FILTER_EXT_OFST         (29)
#define IFI_CANFD_FILTER_CANFD_MSK        (0x40000000u)    //filter mask or identifier canfd/normal can
#define IFI_CANFD_FILTER_CANFD_OFST       (30)
#define IFI_CANFD_FILTER_VALID_MSK        (0x80000000u)    //filter mask or identifier valid
#define IFI_CANFD_FILTER_VALID_OFST       (31)


#endif	// __IFI_CANFD_REGS_H__

/* end of file */
