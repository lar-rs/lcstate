/*
 * can_core - can4linux CAN driver module
 *
 * can4linux -- LINUX CAN device driver source
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 * Copyright (c) 2001-2011 port GmbH Halle/Saale
 * (c) 2001-2013 Heinz-Jürgen Oertel (oe@port.de)
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013-2016 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 *------------------------------------------------------------------
 */

/****************************************************************************/
/**
* \mainpage  can4linux - CAN network device driver
*
The LINUX CAN driver
can be used to control the CAN bus (http://www.can-cia.org)
connected to a PC running LINUX or embedded LINUX systems using uClinux.
Different interface boards and target micro controllers are supported
(see TARGET=VARIABLE in Makefile).
The most popular interfaces are the old fashioned ISA board
AT-CAN-MINI (port GmbH)
and
CPC-PCI  http://www.ems-wuensche.de
or PCI-express

This project was born in cooperation with the  LINUX LLP Project
to control laboratory or automation devices via CAN.
It started already in 1995 and is now considered as mature.

\attention
The former and older can4linux version 1.x
did support many different interface boards.
It was possible to use different kinds of boards at the same time.
Up to four boards could be placed in one computer.
With this feature it was possible to use /dev/can0 and
/dev/can2 for two boards AT-CAN-MINI with SJA1000
and /dev/can1 and /dev/can3 with two CPC-XT equipped with Intel 82527.
\attention
\b Attention: This can4linux version isn't supported anymore \b !

Instead the \b new version has to be compiled for the target hardware.
It was unlikely in the past that a PC or embedded device
was equipped with different CAN controllers.

\par Virtual CAN
In a special mode which is selected by setting the kernel parameter \e virtual
to true,
no hardware at all is needed.
The driver implements something like a virtual CAN network,
where producer and consumer exchange CAN frames only virtually.
\code
/sbin/insmod can4linux.ko virtual=1
\endcode

In all these configurations
the programmer sees the same driver interface with
open(), close(), read(), write() and ioctl() calls
( can_open(), can_close(), can_read(), can_write(), can_ioctl() ).

The driver itself is highly configurable
using the /proc interface of the LINUX kernel.

The following listing shows a typical configuration with three boards:

\code
$ grep . /proc/sys/dev/Can/\*
/proc/sys/dev/Can/AccCode:  -1       -1      -1      -1
/proc/sys/dev/Can/AccMask:  -1       -1      -1      -1
/proc/sys/dev/Can/ArbitrationLost   0	    0	    0	    0
/proc/sys/dev/Can/Base:     800      672     832     896
/proc/sys/dev/Can/Baud:     125      125     125     250
/proc/sys/dev/Can/CAN clock:8000000
/proc/sys/dev/Can/Chipset:  SJA1000
/proc/sys/dev/Can/dbgmask:  0
/proc/sys/dev/Can/framelength:8
/proc/sys/dev/Can/IOModel:  pppp
/proc/sys/dev/Can/IRQ:      5     7       3       5
/proc/sys/dev/Can/OpenCount:    0(4)    0(4)    0(4)    0(4
/proc/sys/dev/Can/Outc:     250   250     250     0
/proc/sys/dev/Can/Overrun:  0     0       0       0
/proc/sys/dev/Can/RxErr:    0     0       0       0
/proc/sys/dev/Can/RxErrCounter:0  0       0       0
/proc/sys/dev/Can/Speedfactor:1   1       1       1
/proc/sys/dev/Can/Timeout:  100   100     100     100
/proc/sys/dev/Can/Transmitterdelay:100  100     100     100
/proc/sys/dev/Can/TxErr:    0     0       0       0
/proc/sys/dev/Can/TxErrCounter:    0     0       0       0
/proc/sys/dev/Can/version:  4.0_ATCANMINI_PELICAN SVN Revision: 239
\endcode

This above mentioned full flexibility
is not needed in embedded applications.
For this applications, a stripped-down version exists.
It uses the same programming interface
but does the most configurations at compile time.
That means especially that only one CAN controller support with
a special register access method is compiled into the driver.
Actually the only CAN controller supported by this version
is the Philips SJA 1000 in both the compatibility mode
\b BasicCAN and the Philips \b PeliCAN mode (compile time selectable).

The version of can4linux currently available at SourceForge
http://sourceforge.net/projects/can4linux
is also supporting the Motorola FlexCAN module as ist is implemented
on Motorolas ColdFire 5282 CPU,
the FlexCAN controllers found on the Freescale ARM i.MX series controllers,
the Analog Devices BlackFin DSP with CAN,
Atmels ARM AT91SAM9263 with integrated CAN and also Microchips
stand alone CAN CAN controller MCP2515 connected via SPI.
One version of the SPI controlled MCP2515 is using direct register access
to the SPI controller found on Atmels AT91 CPUs.
This design was chosen to improve the performance of this special
can4linux version.
Care has to be taken not to use the Linux SPI driver at the same time.
Another possible make target, TARGET)MCP2515SPI,
is using the kernels SPI master driver to control the CAN MCP2515.

Since version 3.4.6 can4linux
assumes that your distribution uses \b udev to have the device
`/dev/can[0-9]' automatically created.
It is usually necessary to change the device access rights set by \b udev .
With the Fedora Core >= 4 or SuSE/novell you can do:

\code
echo 'KERNEL=="[Cc]an*", NAME="%k", MODE="0666"' \
     > /etc/udev/rules.d/91-Can.rules
\endcode

Alternatively create the device inodes in
/lib/udev/devices .
At system start-up,
the contents of that directory is copied to the /dev directory
with the same ownership and permissions as the files in /lib/udev/devices.

The driver creates class Can,
with information in /sys/class/Can/

See also udev (7)

\par The following sections are describing the \e sysctl entries.

\par AccCode/AccMask
contents of the message acceptance mask and acceptance code registers
of 82x200/SJA1000 compatible CAN controllers (see can_ioctl()).

\par Base
CAN controllers base address for each board.
Depending of the \e IOModel entry that can be a memory or I/O address.
(read-only for PCI boards)
\par Baud
used bit rate for this board in Kbit/s
\par Chipset
name of the supported CAN chip used with this boards
Read only for this version.
\par IOModel
one letter for each port. Readonly.
Read the CAN register access model.
The following models are currently supported:
\li b - special mode for the B&R CAN card,
     two special I/O addresses for register addressing and access
\li f - fast register access, special mode for the 82527
     uses memory locations for register addresses
     (ELIMA)
\li i - indexed access, one address serves as register selector,
     the next one as register access (MMC_SJA1000)
\li m - memory access, the registers are directly mapped into memory
\li p - port I/O,  80x86 specific I/O address range
	(AT-CAN-MINI,CTI_CANPRO, KVASER_PCICAN)
\li s - access via SPI (the only CAN controller supported so far
	is the Microchip MCP2515)

Since version 2.4 the \e IOModel is set at compile time.
\par IRQ
used IRQ numbers, one value for each board.
(read-only for PCI boards)
\par Outc
value of the output control register of the CAN controller
Since version 2.4 set at compile time.
A board specific value is used when the module the first time is loaded.
This board specific value can be reloaded by writing the value 0
to \e Outc .
\par
With the most boards using a Philips SJA1000,
by changing the value of the \e Outc it is possible
to inhibit generating the CAN Acknowledge.
Using this feature, it is possible to implement a
\b listen \b only
mode.
Please refer the CAN controller documentation for more details.
\par
Another way is implementing access to the \b mode register with an
\e ioctl () call in later \e can4linux versions.

\par Overrun
counter for overrun conditions in the CAN controller
\par RxErr
counter for CAN controller RX error conditions
- CAN controller RX buffer hardware overflow
\par RxErrCounter
CAN controllers RX error counter
\par Timeout
time out value for waiting for a successful transmission

\par Transmitterdelay
Only in CAN FD mode.
Specifies the CAN Transceivers transmitter delay in ns.
Transceiver Delay Compensation (TDC).

\par TxErr
counter for CAN controller TX error conditions
\par TxErrCounter
CAN controllers TX error counter

\par dbgMask
if compiled with debugging support, writing a value greater then 0
enables debugging to \b syslogd .
The value is bit coded.
\code
Bit 0 print all debug messages
Bit 1 print function entry message
Bit 2 print function exit message
Bit 3 print if a function branches in two different branches
Bit 4 print debug data statements
\endcode

\par version
read only entry containing the drivers version number and hardware acronym

\par OpenCount
displays for each CAN channel the number of processes
using currently the device
and in parenthesis the maximum number of processes which can use the device.

\par framelength
Number of bytes which are used for the data section of the CAN frame.
This is typical 8 for classic CAN an 64 for CAN FD frames.

\par Speedfactor
Only in CAN FD mode.
Factor to be used for the bit timing in the optional High Bit Rate section
(data phase)
of an CAN FD frame.
Data type is integer, the default is 1.

Example. If the standard arbitration bit rate is 500Kbit/s (/proc/sys/dev/Can/Baud)
and the Speedfactor is 4 (/proc/sys/dev/Can/Speedfactor),
the used data bit rate is 2 Mbit/s

\par CAN errors
In case the driver detects internal or CAN controller related errors
it reports this on two ways.
The \e flags field of a received message is used
by signaling common CAN errors like ERROR PASSIVE or Buffer overflows.
This can be combined with a real received message.
Or the driver uses a special error signaling message with an invalid
message  \e id 0f 0xFFFF.FFFF together with the \e flags.

If the driver is using the NXP SJA1000 it is possible to detect
CAN error frames caused by bit errors, crc errors, stuff errors, and so on.
In this case the driver reports an error signaling message to the read() caller
with \id =xFFFF.FFFF and two data bytes containing the content of the
Error Code Register.
		data[0] = ecc;
		data[1] = ecc & 0x1f;

This special error diagnosis feature
must be enabled at driver load time
by setting the module parameter \e errint .
\code
/sbin/insmod can4linux.ko errint=1
\endcode

\par miscellaneous

Since 2012 a faster version of CAN exists called CAN FD, for flexible data rate.
This mode uses a higher bitrate when sending data but is using the
\e old bitrate for the arbitration, that is sending the CAN Id.
In 2015 this was standardised by the ISO, but with a little difference.
Therefore old controllers may exist and new ones following the ISO standard.
can4linux supports the ISO mode.
But if a controller is able to still switch to the old behaviour,
the non-ISO mode, can4linux can switch to non-ISO too.
To tell the driver to use the non-ISO mode, a kernel parameter is used:
\code
/sbin/insmod can4linux.ko virtual=1
\endcode

Since 2010 the driver is hosted at SourceForge.
The used svn version number can be obtained by asking /sbin/modinfo.

Please see also at can_ioctl() for some additional descriptions.

For initially writing these sysctl entries after loading the driver
(or at any time) a shell script utility does exist.
It uses a board configuration file that is written over \e /proc/sys/dev/Can .
\code
utils/cansetup port.conf
\endcode
or, like used in the Makefile:
\code
CONFIG := $(shell uname -n)

# load host specific CAN configuration
load:
	@echo "Loading etc/$(CONFIG).conf CAN configuration"
	utils/cansetup etc/$(CONFIG).conf
	echo 0 >/proc/sys/dev/Can/dbgMask
\endcode
Example *.conf files are located in the \e etc/ directory.

\note
This documentation was created using the wonderful tool
\b Doxygen http://www.doxygen.org/index.html .
Die Dokumentation wurde unter Verwendung von
\b Doxygen http://www.doxygen.org/index.html
erstellt

*/

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/fs.h>		/* register_chrdev() */
#include <linux/pci.h>

#include "defs.h"
#include <linux/device.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

MODULE_VERSION("4.3");
#if 0
/* would be nice to have, but compiler says:
error: invalid application of 'sizeof' to incomplete type 'int[]'
*/
module_param_array(IRQ, int, NULL, S_IRUGO);

static int argc_proc_baud = MAX_CHANNELS;
module_param_array(proc_baud, int, &argc_proc_baud, S_IRUGO);
#endif

/* This name is used to register the char device */
#define CANREGDEVNAME "can4linux" CAN_MODULE_POSTFIX

int irq_requested[MAX_CHANNELS] = { 0 };
int can_minors[MAX_CHANNELS] = { 0 };	/* used as IRQ dev_id */

int virtual;
int noniso;
module_param(virtual, int, S_IRUGO);
MODULE_PARM_DESC(virtual, "Switch the driver into virtual mode, not using any CAN hadware");
module_param(errint, int, S_IRUGO);
MODULE_PARM_DESC(errint, "Enable all possible error interrupts of an SJA1000");
module_param(noniso, int, S_IRUGO);
MODULE_PARM_DESC(noniso, "If supported, switch the CAN FD controller into the non-ISO mode");

static int can_major = CAN_MAJOR;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*
There's a C99 way of assigning to elements of a structure,
and this is definitely preferred over using the GNU extension.
gcc 2.95 (and later versions) supports the new C99 syntax.
The meaning is clear, and you should be aware
that any member of the structure which you don't explicitly assign
will be initialized to NULL by gcc.
*/

static const struct file_operations can_fops = {
	.owner = THIS_MODULE,
	.open = can_open,
	.release = can_close,
	.read = can_read,
	.write = can_write,
	.poll = can_select,
	.unlocked_ioctl = can_ioctl,
	.fasync = can_fasync,
};

static struct class *can_class;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

static void can_device_destroy(void)
{
	if (!IS_ERR(can_class)) {
		int i;
		for (i = 0; i < MAX_CHANNELS; i++) {
			device_destroy(can_class, MKDEV(can_major, i));
			;
		}
		class_destroy(can_class);
	}
}

static int __init can_init(void)
{
	int i, j, err = 0;
	int minor = -1;

	/* do you want do see debug message already while loading the driver ?
	 * Then enable this line and set the mask != 0
	 */
#if 1
	proc_dbgmask = 7;
#endif
	DBGIN();


#if defined(GENERIC)
	pr_info("The generic driver supports the virtual CAN network feature\n");
	virtual = 1;
#endif
	/* try udev support */
	i = register_chrdev(can_major, CANREGDEVNAME, &can_fops);
	if (i) {
		pr_err("-> faild to register chardev, can't get Major %d = %d\n", can_major, i);
		return -EIO;
	}

	/* udev support */
	can_class = class_create(THIS_MODULE, CANREGDEVNAME);
	if (IS_ERR(can_class)) {
		pr_err("No udev support.\n");
		err = PTR_ERR(can_class);
		goto out_devfs;
	}

	/*

	   The kernel API for device_create()
	   starting in 2.6.27 it changed to:

	   struct device *device_create(
	   struct class *cls, struct device *parent,
	   dev_t devt, void *drvdata,
	   const char *fmt, ...)

	 */
	for (i = 0; i < MAX_CHANNELS; i++) {
		device_create(can_class, NULL, MKDEV(can_major, i),
			      NULL, "can%d", i);
	}

	pr_info(__CAN_TYPE__ "CAN Driver " VERSION " (c) "
		__DATE__ " " __TIME__ "\n");
	if (virtual == 0) {
#if defined(MCF5282)
		pr_info(" FlexCAN port by H.J. Oertel (hj.oertel@t-online.de)\n");
#elif defined(AD_BLACKFIN)
		pr_info(" BlackFin port by H.J. Oertel (hj.oertel@t-online.de)\n");
#elif defined(ATMEL_SAM9)
		pr_info
		    (" Atmel AT91SAM9263 port by H.J. Oertel (hj.oertel@t-online.de)\n");
#elif defined(SSV_MCP2515) || defined(AuR_MCP2515)
		pr_info
		    (" Atmel AT91 and MCP2515 by H.J. Oertel (hj.oertel@t-online.de)");
		pr_info(" - QF by mha@ist1.de\n");
#elif defined(MCP2515SPI)
		pr_info(" MCP2515 SPI port by H.J. Oertel (hj.oertel@t-online.de)\n");
#elif defined(IMX35) || defined(IMX25) || defined(IMX28)
		pr_info
		    (" Freescale FlexCAN port by H.J. Oertel (hj.oertel@t-online.de)\n");
#else
		pr_info(" H.J. Oertel (hj.oertel@t-online.de)\n");
#endif
	} else {
		pr_info(" virtual CAN network, (c) H.J. Oertel (hj.oertel@t-online.de)\n");
	}

	pr_info(" MAX_CHANNELS %d\n", MAX_CHANNELS);
	pr_info(" CAN_MAX_OPEN %d\n", CAN_MAX_OPEN);

	/* refresh open count in /proc/ */
	format_proc_device_open_count();

	/*
	   initialize the variables laid down in /proc/sys/dev/Can
	   ========================================================
	 */
	for (i = 0; i < MAX_CHANNELS; i++) {
		atomic_set(&can_isopen[i], 0);
		for (j = 0; j < CAN_MAX_OPEN; j++)
			selfreception[i][j] = 1;
		use_timestamp[i] = 1;
		proc_iomodel[i] = IO_MODEL;
		proc_baud[i] = 125;

#if !defined(IMX35) && !defined(IMX25) && !defined(IMX28)
		proc_acccode[i] = proc_accmask[i] = STD_MASK;
#endif
		proc_outc[i] = CAN_OUTC_VAL;
		irq_requested[i] = 0;
		can_minors[i] = i;	/* used as IRQ dev_id */

		spin_lock_init(&write_splock[i]);

#if defined(MCF5282)
		/* we have a really fixed address here */
		proc_base[i] = (MCF_MBAR + 0x1c0000);
		/* Because the MCF FlexCAN is using
		 * more then 1 Interrupt vector,
		 * what should be specified here ?
		 * For information purpose
		 * let's only specify  the first used here
		 */
		IRQ[i] = 136;
#endif
#if defined(IMX35) || defined(IMX25) || defined(IMX28)
		/* we have a really fixed address here */
		/* better would be using platform device pdev.resource.start */

#if defined(IMX25)
		proc_base[i] = CAN1_BASE_ADDR + (0x4000 * i);
#endif
#if defined(IMX28)
#define CAN1_BASE_ADDR MX28_CAN0_BASE_ADDR
#define MXC_INT_CAN1 MX28_INT_CAN0
		proc_base[i] = CAN1_BASE_ADDR + (0x2000 * i);
#endif
#if defined(IMX35)
#define CAN1_BASE_ADDR	0x53FE4000 
#define MXC_INT_CAN1	43
		/* CAN1 0x53FE_4000 */
		/* CAN2 0x53FE_8000 */
		proc_base[i] = CAN1_BASE_ADDR + (0x4000 * i);
#endif
		/* printk("can%d: 0x%08lx\n", i, proc_base[i]); */
		IRQ[i] = MXC_INT_CAN1 + (i * 1);
		/* printk("can%d: %d\n", i, IRQ[i]); */
		init_imx35_hw(i);
		{
			int masks;
			for (masks = 0; masks < 8; masks++) {
				proc_acccode[masks][i]
					= proc_accmask[masks][i] = STD_MASK;
			}
		}

		erroractive[i] = 1;	/* set error active true */
#endif

#if defined(AD_BLACKFIN)
		/* we have a really fixed address here */
		/* starting with Mailbox config reg 1  */
		proc_base[i] = BFCAN_BASE;
		/* Because the AD BlackFin CAN
		 * is using more then 1 Interrupt vector,
		 * what should be specified here ?
		 * For information purpose
		 * let's only specify  the first used here.
		 * Next one is +1
		 */
		IRQ[i] = IRQ_CAN_RX;
#endif

#if defined(ATMEL_SAM9)
		/* we have a really fixed address here */
		proc_base[i] = (AT91SAM9263_BASE_CAN);
		/*
		 */
		IRQ[i] = AT91SAM9263_ID_CAN;
#endif

#if defined(VCMA9)
		proc_base[i] = 0x28000000;
		can_sysctl_table[CAN_SYSCTL_IRQ - 1].mode = 0444;
		can_sysctl_table[CAN_SYSCTL_BASE - 1].mode = 0444;
		IRQ[i] = 37 + 16;

#endif

#if defined(CCPC104)
		pc104_irqsetup();
		/* The only possible vector on CTRLink's 5282 CPU */
		IRQ[i] = 67;
		proc_base[i] = 0x40000280;
#endif

/*
static struct spi_board_info sbc_spi_devices[].irq
*/
#if defined(SSV_MCP2515) || defined(AuR_MCP2515)
		proc_base[i] = 0xFFFFFFFF;	/* not used, SPI ? */
		/* IRQ[i]           = gpio_to_irq(CAN_SPI_INTERRUPT_PIN);  */
		/* set the used IRQ via /proc/sys/dev/Can/IRQ */
		IRQ[i] = 0;
#endif

#if defined(MMC_SJA1000)

		proc_base[i] = 0x30000000 + (2 * i);
		/* IRQ[i]           = gpio_to_irq(CAN_IRQ_PIN); */
		IRQ[i] = AT91SAM9260_ID_IRQ0;	/* 29 bei Heyfra */
#endif


#if defined(BEAGLEBONE) || defined(CAN4LINUX_PCI) \
	|| defined(ZEDBOARD) \
	|| defined(MMC_SJA1000) || defined(MULTILOG_SJA1000) \
	|| defined(GENERIC) || defined(BANANAPI) \
	|| defined(RASPI) \
	|| defined(PCM3680) \
	|| defined(GENERIC_I82527) || defined(MOBA_I82527)

		/* This should be the name for all other boards as well
		 * : rename	init_zynq_hw(i), init_imx_hw() ... */
		err = init_board_hw(i);
		if (err != 0)
			goto out_class;
#endif


	} /* end of for loop initializing all CAN channels */

	if (virtual == 0) {
		/*
		   ========== Begin HW initialization ==================
		 */


#if defined(VCMA9)
		/* only one SJA1000 available
		 * we can check if it is available when loading the module
		 */
		if (!controller_available(0x28000000, 1)) {
			err = -EIO;
			goto out_class;
		}
#endif

#if defined(CCPC104)
		/* The only possible interrupt
		 * could be IRQ4 on the PC104 Board */
		can_sysctl_table[CAN_SYSCTL_IRQ - 1].mode = 0444;
#endif

#if defined(MCF5282) || defined(IMX35) || defined(IMX25) || defined(IMX28)\
	|| defined(SSV_MCP2515) || defined(AuR_MCP2515) || defined(MCP2515SPI)
		can_sysctl_table[CAN_SYSCTL_BASE - 1].mode = 0444;
		can_sysctl_table[CAN_SYSCTL_IRQ - 1].mode = 0444;
#endif

#if defined(SSV_MCP2515) || defined(AuR_MCP2515)
		/* call the probe function direct */
		err = mcp251x_can_probe();
		if (err)
			goto out_class;
#endif

		/* after initializing channel based parameters
		 * finish some entries
		 * and do drivers specific initialization
		 */
		proc_iomodel[i] = '\0';

		/* end of hardware specific part */
	} else {
		/* do nothing hardware related but overwrite
		   /proc/sys/.../Chipset */
		strncpy(proc_chipset, "virtual CAN", PROC_CHIPSET_LENGTH);
	}

	/*
	   ========== end HW initialization ==================
	 */

#if defined(LDDK_USE_PROCINFO)
	register_procinfo();
#endif
#if defined(LDDK_USE_SYSCTL)
	register_systables();
#endif

#if 0
	{
#include <linux/fs.h>
	    struct file *f;
	    f = filp_open("/tmp/can4linux.1", O_RDWR | O_CREAT, 0);
	    
	    filp_close(f,NULL);
	}
#endif
	DBGOUT();
	return 0;

out_class:
	pr_err(" error in board init, class_destroy:\n");
	can_device_destroy();
out_devfs:
	pr_err(" error in board init, unregister char device\n");
	unregister_chrdev(can_major, CANREGDEVNAME);
	unregister_systables();
	DBGOUT();
	return err;
}

static void __exit can_exit(void)
{
int minor = -1;

	DBGIN();

	/* each board package should contain this function */
	exit_board_hw();

	unregister_chrdev(can_major, CANREGDEVNAME);
	pr_info(" char device \"" CANREGDEVNAME "\" removed\n");

#if defined(IMX35) || defined(IMX25) || defined(IMX28)
	exit_imx35_hw();
#endif

#if defined(ZEDBOARD)
	exit_zynq_hw();
#endif

	can_device_destroy();

#if defined(LDDK_USE_PROCINFO)
	unregister_procinfo();
#endif

#if defined(SSV_MCP2515) || defined(AuR_MCP2515)
	{
		mcp251x_can_remove();
		/* only with direct using SPI */
		release_mem_region(0xfffa4000ul, 0x4000);
	}
#endif

#if LDDK_USE_SYSCTL
	unregister_systables();
#endif
	DBGOUT();
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

module_init(can_init);
module_exit(can_exit);

MODULE_AUTHOR("H.-J.Oertel <hj.oertel@t-online.de");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CAN fieldbus driver " __CAN_TYPE__);
