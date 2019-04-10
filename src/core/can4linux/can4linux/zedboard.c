/* zedboard.c
 *
 * can4linux -- LINUX CAN device driver source
 *
 * ZedBoard board specific functions
 * featuring the xpscan CAN module
 *
 * Currently most of the parameters for the CAN controller driver are fixed
 * in the code or constants.
 * E.g. peripheral addresses are fixed at least for the Zynq platform.
 * If the XCAN will be used on other platforms
 * it might make sense to use the Linux device tree for such kind of data.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * (c) 2013 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 *------------------------------------------------------------------
 */

/* ZedBoard board specific functions
 * featuring the xpscan CAN module
 *
 * Currently most of the parameters for the CAN controller driver are fixed
 * in the code or constants.
 * E.g. peripheral addresses are fixed at least for the Zynq platform.
 * If the XCAN will be used on other platforms
 * it might make sense to use the Linux device tree for such kind of data.
 *
 */


/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include "defs.h"
#include <linux/device.h>
#include <linux/module.h>
/* #include <linux/platform_device.h> */
#include <linux/of_device.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/of.h>


/* three ressouces needed, allocate it at load time
   free it at remove time */
static struct clk *clk[MAX_CHANNELS];
/* static struct regulator *regulator[MAX_CHANNELS]; */
static struct xcan_platform_data *plat_data[MAX_CHANNELS];


#define DRV_NAME "ps7-can"

/*
* The probe() function checks for all defined CAN controllers if they are
* available, switches the core regulator and clock source on
*
*/
/* static */ int xcan_probe(struct platform_device *pdev)
{
int minor = -1;			/* only for DBGIN();DBGOUT(); */
int err;
static int devid;

struct clk *devclk;  /* CAN[01]_ref_clock is the CAN clock input */
struct clk *aperclk; /* AMBA peripheral clock control */

	DBGIN();
	err = 0;

#if defined(CAN_DEBUG)
	pr_info("Probe: Modul %d\n", devid);
	dev_info(&pdev->dev, "Probe: Modul %d\n", devid);

	pr_info("platform.name   : %s\n", pdev->name);
	pr_info("platform.id     : %d\n", pdev->id);
	pr_info("platform.id_auto: %d\n", pdev->id_auto);
	pr_info("platform.num_resources: %d\n", pdev->num_resources);

#endif /* defined(CAN_DEBUG) */

	if (devid > (MAX_CHANNELS - 1)) {
		pr_err("\tXCAN module > %d not supported yet\n", devid - 1);
		return -ENODEV;
	}

	plat_data[devid] = (pdev->dev).platform_data;


	/* Getting the CAN devclk info */
	/* clk[devid] = clk_get(&pdev->dev, "ref_clk"); */
	/* CAN[01]_ref_clock is the CAN clock input */
	devclk = clk_get(&pdev->dev, "ref_clk");
	if (IS_ERR(devclk)) {
		dev_err(&pdev->dev, "Device clock not found.\n");
		pr_err("Device clock not found.\n");
		err = PTR_ERR(devclk);
		goto err_free;
	}

	aperclk = clk_get(&pdev->dev, "aper_clk");
	if (IS_ERR(aperclk)) {
		dev_err(&pdev->dev, "aper clock not found\n");
		err = PTR_ERR(aperclk);
		goto err_devclk;
	}


	/* dev_info(&pdev->dev, "\tenable CAN device clock\n"); */
	err = clk_prepare_enable(devclk);
	if (err) {
		dev_err(&pdev->dev, "unable to enable CAN device clock\n");
		goto err_aperclk;
	}


	/* dev_info(&pdev->dev, "\tenable  AMBA peripheral clock\n"); */
	err = clk_prepare_enable(aperclk);
	if (err) {
		dev_err(&pdev->dev, "unable to enable  AMBA peripheral clock\n");
		goto err_unprepar_disabledev;
	}


	if (plat_data[devid]) {
		pr_info("\tCAN%d Resource .start  %x\n",
			devid, pdev->resource->start);
		/* plat_data[devid]->active(pdev); */
	}

	proc_clock = clk_get_rate(devclk);
	pr_info("\tXCAN %d is using  the bus clock \"ref_clk\" = %d\n",
		devid, proc_clock);


	/* Check if we have a correct bit rate table for this */
	/* There might be some uncertaince in the reported clock frequency
	 * check for it */
	if (abs(proc_clock - CAN_SYSCLK) > 2) {
		pr_err("\tPlease check the Bit Rate table in xcanps.h\n");
		pr_err("\tCurrently set to %d\n", CAN_SYSCLK);
		err = -1;

		goto err_unprepar_disabledev;
	} else {
		/* set the clock value in /proc/sys/dev/can/clock
		 * only for informal uses */
		proc_clock = CAN_SYSCLK;
		goto err_free;
	}

err_unprepar_disabledev:
	clk_disable_unprepare(devclk);
err_aperclk:
	clk_put(aperclk);
err_devclk:
	clk_put(devclk);
err_free:
	devid++;
	DBGOUT(); return err;

}

/* static */ int xcan_remove(struct platform_device *pdev)
{
int devid = pdev->id;

	/* pr_info("called xcan_remove(id=%d)\n", devid); */
	if (devid > (MAX_CHANNELS - 1))
		return 0;
	clk_put(clk[devid]);
	/* regulator_put(regulator[devid]); */
	/* free used io pins */
	/* plat_data[devid]->inactive(pdev);     */
	return 0;
}


/* Match table for OF platform binding
   The connection between the kernel driver
   and the 'compatible' entries it should be attached to,
   is made by a code segment as follows.
   Each match will cause the current driver to load.
   The xcan_probe() function is called.
 */
static struct of_device_id xcan_of_match[] = {
	{ .compatible = "xlnx,ps7-can-1.00.a", },
	{ .compatible = "xlnx,can-1.00.a", },
	{ }, /* end of list */
};

MODULE_DEVICE_TABLE(of, xcan_of_match);


static struct platform_driver xcan_driver = {
	.probe = xcan_probe,
	.remove = xcan_remove,
	.driver = {
	   .owner = THIS_MODULE,
	   .name = DRV_NAME,
	   .of_match_table	= xcan_of_match,
	},
};


void exit_zynq_hw(void)
{
	/* pr_info("calling platform_driver_unregister()\n"); */
	platform_driver_unregister(&xcan_driver);
}

#if defined(CAN_HELPER)
static void print_device_tree_node(struct device_node *node, int depth)
{
int i;
struct device_node *child;
struct property    *properties;
char                indent[255] = "";

	for (i = 0; i < depth * 3; i++)
		indent[i] = ' ';
	indent[i] = '\0';
	++depth;

	for_each_child_of_node(node, child) {
		pr_info("%s{ name = %s\n", indent, child->name);
		pr_info("%s  type = %s\n", indent, child->type);
		for (properties = child->properties;
				properties != NULL;
				properties = properties->next) {
			pr_info("%s  %s (%d)\n",
				indent, properties->name, properties->length);
		}
		print_device_tree_node(child, depth);
		pr_info("%s}\n", indent);
	}
}
#endif /* defined(CAN_HELPER) */

/* Called from __init,  once when driver is loaded
   set up physical adresses, irq number
   and initalize clock source for the CAN module

   take care it will be called only once

    Example: Configure Rx/Tx Signals to MIO Pins
    1. Configure MIO pin 46 for the Rx signal.
    Write 0x0000_1221 to the slcr.MIO_PIN_46 register:
       a. Route CAN0 Rx signal to pin 46.
       b. Output disabled (set TRI_ENABLE = 1).
       c. LVCMOS18 (refer to the register definition for other voltage options).
       d. Slow CMOS edge (benign setting).
       e. Enable internal pull-up resistor.
       f. Diable HSTL receiver.
    2. Configure MIO pin 47 for the Tx signal.
    Write 0x0000_1220 to the slcr.MIO_PIN_47 register:
       a. Route CAN0 Tx signal to pin 47.
       b. 3-state controlled by CAN (TRI_ENABLE = 0).
       c. LVCMOS18 (refer to the register definition for other voltage options).
       d. Slow CMOS drive edge.
       e. Enable internal pull-up resistor.
       f. Disable HSTL receiver.

    From forum
    "A question about the mapping relationships between the IO signal
    an its values"
    If you take a look in the latest version of the Technical Reference
    Manual V1.2, on page 1515 you will find the start of the register
    definition for the MIO_PIN_46 register. This register is at address
    0xF80007B8 and carries a reset value of 0x0000_1601.

    By setting the value of this register to 0x0000_12E1, the L3_SEL bits
    are changed from the default GPIO mode (000) to UART 0 RxD mode (111).

    Another way to understand the configuration of the MIO would be to
    experiment with the PS configuration within XPS. Once the PS has been
    configured with the IO behavior you are targeting, you can export
    the configuration to SDK where you get a ps7_init.h and ps7_init.c
    file which contains the register configuration for the MIO.

    Take a look through these register definitions, they are useful in
    determining the IO signal mapping relationships.

    etwa page 1515
    What can be used for CAN ???
    MIO_PIN_35          CAN 0 Tx, Output
    MIO_PIN_36	    CAN 1 Tx, Output
    MIO_PIN_37	    CAN 1 Rx, Input
    MIO_PIN_38	    CAN 0 Rx, Input
    MIO_PIN_39	    CAN 0 Tx, Output
    MIO_PIN_40	    CAN 1 Tx, Output
    MIO_PIN_41	    CAN 1 Rx, Input
    MIO_PIN_42	    CAN 0 Rx, Input
    MIO_PIN_43	    CAN 0 Tx, Output
    MIO_PIN_44	    CAN 1 Tx, Output
    MIO_PIN_45	    CAN 1 Rx, Input
    MIO_PIN_46	    CAN 0 Rx, Input

		0x0000_1221  = 0001 0020 0020 0001
		  PIN          5432 1098 7654 3210
		   13     0    Operates the same as MIO_PIN_00[DisableRcvr]
		   12     1    [PULLUP]
				Enables Pullup on IO Buffer pin
				0: disable
				1: enable
		   11:9   001  [IO_Type]
				000: LVTTL
				001: LVCMOS18
				010: LVCMOS25
				011, 101, 110, 111: LVCMOS33
				100: HSTL

		      8	  0    speed as MIO 00
		    7:5   001  CAN 0 Rx, Input
		    4:3   00   Level3 Mux
		    2	  0    Level2 Mux
		    1     0    Level1 Mux
		    0     1    Operates the same as MIO_PIN_00[TRI_ENABLE]


	MIO_PIN_47	    CAN 0 Tx, Output
	MIO_PIN_48	    CAN 1 Tx, Output
	MIO_PIN_49	    CAN 1 Rx, Input
	MIO_PIN_50	    CAN 0 Rx, Input
	MIO_PIN_51	    CAN 0 Tx, Output
	 ....

	MIO_LOOPBACK	CAN0_LOOP_CAN1   on bit 2 == 1


	CAN1_CPU1X_RST	Absolute Address 0xF8000220
	CAN0_CPU1X_RST		    "
	    CAN x AMBA software reset. On assertion of this
	    reset, the AMBA clock portion of the CAN x
	    subsystem will be reset.
	    0: No reset
	    1: AMBA clock portion of CAN 1 subsytem held
	    in reset
*/
int init_board_hw(int n)
{
int ret;
static int already_called;
int minor = -1;

	DBGIN();
	ret = 0;

/*********************************/
	/* platform data
	 * have to come from the device tree in recent kernels */
	/* can0: 0xE0008000     irq 60
	 * can1: 0xE0009000     irq 83 */
	proc_base[n] = 0xE0008000 + (0x1000 * n);
	IRQ[n] = 60 + (23 * n);

#if defined(CAN_DEBUG)

	/* print_device_tree_node(of_find_node_by_path("/"), 0); */
	{
	char *path = "/amba@0/ps7-can@e0008000";
	/* char *path = "/amba@0/ps7_can_0"; */
	struct device_node *dt_node;
	const u32 *property;
	int len;

	dt_node = of_find_node_by_path(path);
	if (!dt_node) {
		pr_err("(E) Failed to find device-tree node: %s\n", path);
		return -ENODEV;
	}

	pr_info("(I) Found device-tree node: %s.  Now retrieving property.\n",
		path);
	property = of_get_property(dt_node, "reg", &len);
	pr_info("(I) len=%d\n", len); /* expect len==8, 2 values */
	pr_info("(I) reg[0]=0x%08lX\n",
		(unsigned long) be32_to_cpu(property[0]));
	pr_info("(I) reg[1]=0x%08lX\n",
		(unsigned long) be32_to_cpu(property[1]));

	property = of_get_property(dt_node, "interrupts", &len);
	pr_info("(I) len=%d\n", len); /* expect len==12, 3 values */
	pr_info("(I) reg[0]=%08ld\n",
		(unsigned long) be32_to_cpu(property[0]));
	pr_info("(I) reg[1]=%08ld\n",
		(unsigned long) be32_to_cpu(property[1]));
	pr_info("(I) reg[2]=%08ld\n",
		(unsigned long) be32_to_cpu(property[2]));
	}

#endif /* defined(CAN_DEBUG) */

/*********************************/

	if (!already_called) {
		/* pr_info("%d: register \"%s\" can4linux driver\n",
			n, DRV_NAME); */ 

		ret = platform_driver_register(&xcan_driver);

		/* prepare all what is needed
		 * before requesting the memory region.
		 * This will be done in Vendor_Init()
		 * But may be we have to enable the CAN module as such
		 * or switch on the clock source to have access to CAN
		 *
		 * Enabling the address area and enabling the clock
		 * is not necessary. It all is done In the XPS Tool
		 * (Xilinx Platform Studio).
		 *
		 */

		/* Enter configuration mode
		 * Writing a 1 to the SRST bit in the SRR register.
		 * The controller enters Configuration mode
		 * immediately following the software reset.
		 */

#if defined(CONFIG_TIME_MEASURE)
	    init_measure();
#endif
	    already_called = 1;
	}

	DBGOUT();
	return ret;
}

void exit_board_hw(void)
{
}

void board_clear_interrupts(int minor)
{
}


/*
 * Perform Vendor, that means sometimes CAN controller
 * or only board manufacturer specific initialization.
 *
 * Mainly it gets needed IO and IRQ resources and initializes
 * special hardware functions.
 *
 *
 *  From include/linux/irq.h
 * IRQ line status.
 * IRQ types
 * IRQ_TYPE_NONE		Default, unspecified type
 * IRQ_TYPE_EDGE_RISING	Edge rising type
 * IRQ_TYPE_EDGE_FALLING	Edge falling type
 * IRQ_TYPE_EDGE_BOTH (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
 * IRQ_TYPE_LEVEL_HIGH	Level high type
 * IRQ_TYPE_LEVEL_LOW	Level low type
 * IRQ_TYPE_SENSE_MASK	Mask of the above
 * IRQ_TYPE_PROBE		Probing in progress
 *
 * IRQF_DISABLED - keep irqs disabled when calling the action handler.
 *                 DEPRECATED. This flag is a NOOP and scheduled to be removed
 * IRQF_SHARED - allow sharing the irq among several devices
 * IRQF_PROBE_SHARED - set by callers
 *		  when they expect sharing mismatches to occur
 * IRQF_TIMER - Flag to mark this interrupt as timer interrupt
 * IRQF_PERCPU - Interrupt is per cpu
 * IRQF_NOBALANCING - Flag to exclude this interrupt from irq balancing
 * IRQF_IRQPOLL - Interrupt is used for polling (only the interrupt that is
 *                registered first in an shared interrupt is considered for
 *                performance reasons)
 * IRQF_ONESHOT - Interrupt is not reenabled after the hardirq handler finished.
 *                Used by threaded interrupts which need to keep the
 *                irq line disabled until the threaded handler has been run.
 * IRQF_NO_SUSPEND - Do not disable this IRQ during suspend
 * IRQF_FORCE_RESUME - Force enable it on resume even if IRQF_NO_SUSPEND is set
 * IRQF_NO_THREAD - Interrupt cannot be threaded
 * IRQF_EARLY_RESUME - Resume IRQ early during syscore instead of at device
 *                resume time.
 *
*/

int can_vendor_init(int minor)
{
	DBGIN();

	can_range[minor] = CAN_RANGE;
	DBGPRINT(DBG_DATA, ("Assume Address of CAN%d at %lx, range 0x%x\n",
		minor,  proc_base[minor], CAN_RANGE));

	/* Request the controllers address space */
	if (NULL == request_mem_region(
		    proc_base[minor], can_range[minor] , "xcanps")) {
		DBGPRINT(DBG_DATA,
			("Request_mem_region CAN-IO failed at %lx\n",
		proc_base[minor]));
		DBGOUT();
		return -EBUSY;
	}

	can_iobase[minor] = ioremap(proc_base[minor], can_range[minor]);
	/* pr_info(" 0x%08lx remapped to 0x%08lx\n", */
	    /* proc_base[minor], (long unsigned int)can_iobase[minor]); */

	/* memory can be used now */
	/* can_register_dump(minor, 4); */

	/* Enter configuration mode
	 * Writing a 1 to the SRST bit in the SRR register.
	 * The controller enters Configuration mode
	 * immediately following the software reset.
	 */
	CAN_OUTL(minor, srr, XCANPS_SRR_SRST_MASK);

	/*
	int request_irq(unsigned int irq,	// interrupt number
		  void (*handler)(int, void *, struct pt_regs *),
	     // pointer to ISR
			irq, dev_id, registers on stack
			unsigned long irqflags, const char *devname,
			void *dev_id);

	   dev_id - The device ID of this handler (see below).
	   This parameter is usually set to NULL,
	   but should be non-null if you wish to do  IRQ  sharing.
	   This  doesn't  matter when hooking the
	   interrupt, but is required so  that,  when  free_irq()  is
	   called,  the  correct driver is unhooked.  Since this is a
	   void *, it can point to anything (such  as  a  device-spe-
	   cific  structure,  or even empty space), but make sure you
	   pass the same pointer to free_irq().

	   If the kernel receives an interrupt he knows
	   how often it was registered,
	   and calls the ISR for each registered interrupt.
	   In this case, can4linux, for example
	   every time with a different pointer to can_minors[]
	   containing the device minor which has registered.
	   The ISR should now check this CAN controller related to the minor
	   number, if it was the cause of the interrupt.

	*/


	/* Each CAN controller on the Zynq has it's own Interrupt
	   no need to share Interrupts between modules. */

	/* test for valid IRQ number in /proc/.../IRQ */
	if (IRQ[minor] > 0 && IRQ[minor] < MAX_IRQNUMBER) {
		int err;
		err = request_irq(IRQ[minor], can_interrupt,
		    /* IRQF_SAMPLE_RANDOM */ IRQF_PERCPU,
		    "xcanps", &can_minors[minor]);

		if (!err) {
			DBGPRINT(DBG_BRANCH, ("Requested IRQ: %d @ %p",
				    IRQ[minor], can_interrupt));

		    irq_requested[minor] = 1;
		    /* we are here if all is OK */
		    /* ======================== */
		    DBGOUT(); return 0;
		    /* ==== OK return ========= */
		}
	pr_err("Can[%d]: Can't request IRQ %d\n", minor, IRQ[minor]);
	} else {
	    ;
	DBGPRINT(DBG_DATA, ("Can[%d]: invalid IRQ number %d\n",
						minor, IRQ[minor]));
	}

	/* if something fails, we are here */
	/* release I/O memory mapping -> release virtual memory */
	/* pr_info("iounmap %p\n", can_iobase[minor]); */
	iounmap(can_iobase[minor]);

	/* Release the memory region */
	/* pr_info("release mem %x\n", proc_base[minor]); */

	release_mem_region(proc_base[minor], can_range[minor]);
	DBGOUT();
	return -EBUSY;
}


int can_freeirq(int minor, int irq)
{
	DBGIN();
	irq_requested[minor] = 0;
	free_irq(irq, &can_minors[minor]);
	DBGOUT();
	return 0;
}



#if defined(CONFIG_TIME_MEASURE)
/* Functions to Switch the stat of output pins
   used for time measurement within the CAN ISR
 */
#include <mach/gpio.h>

/*
  gpio_* functions are defined in ./drivers/gpio/gpiolib.c

  after setting the output direction,
  void gpio_set_value(unsigned gpio, int value)


  The GPIO pins which are controlled
  are located on the MLB connector (P3) pin 2,4,6

  Before toggling the GPIO the following steps have to be performed.
  - Connect the MLB connector pin 20 to MLB connector pin 15

*/

#if 0
 /* TODO: this is currenntly empty code for reference only.
  * It must be replaced by ZedBoard platform code.
  */
void init_measure(void)
{
	/* set port to output and initially to low */

	/* set mux */

	/* set gpio  to default value */

}

void set_measure_pin(void)
{
	/* set port to output and initially to high */
}

void reset_measure_pin(void)
{
	/* set port to output and initially to high */
}
#endif
#endif
