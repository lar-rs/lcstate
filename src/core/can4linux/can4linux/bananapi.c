/* generic board handling
 *
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include <linux/pci.h>

     #include <linux/delay.h>

#include <linux/clk.h>			/* clk_get */
#include <mach/clock.h>
#include <mach/gpio.h>

/* already defined in arch/arm/plat-sunxi/include/plat/platform.h */
// #define PIO_BASE 0x01c20800    /* SW_PA_PORTC_IO_BASE */

#define PI_CFG0_OFFSET 0x120
#define PI_CFG1_OFFSET 0x124
#define PI_CFG2_OFFSET 0x128
#define PI_CFG3_OFFSET 0x12c
#define PI_DATA_OFFSET 0x130


#define PH_CFG2_OFFSET 0x104

int can_requestirq(int minor, int irq, irqreturn_t (*handler)(int, void *))
{
int err;

	DBGIN();
	/*

	int request_irq(unsigned int irq,			// interrupt number
	void (*handler)(int, void *, struct pt_regs *), // pointer to ISR
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

	*/

	err = request_irq(irq, handler, IRQF_SHARED, "can4linux", &can_minors[minor]);


	if (!err) {
		DBGPRINT(DBG_BRANCH, ("Requested IRQ: %d @ 0x%lx",
			irq, (unsigned long)handler));
		irq_requested[minor] = 1;
	}
	DBGOUT();
	return err;
}


int can_freeirq(int minor, int irq)
{
	DBGIN();
	irq_requested[minor] = 0;
	// pr_info(" Free IRQ %d  minor %d\n", irq, minor);
	free_irq(irq, &can_minors[minor]);
	DBGOUT();
	return 0;
}

/*
 * Perform Vendor-Init, that means sometimes CAN controller
 * or only board manufacturer specific initialization.
 *
 * Mainly it gets needed IO and IRQ resources and initializes
 * special hardware functions.
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
		    proc_base[minor], can_range[minor] , "CAN")) {
		DBGPRINT(DBG_DATA,
			("Request_mem_region CAN-IO failed at %lx\n",
		proc_base[minor]));
		DBGOUT();
		return -EBUSY;
	}

	// pr_info("Second call to ioremap() for phy addr %lx\n", proc_base[minor]); 
	can_iobase[minor] = ioremap(proc_base[minor], can_range[minor]);
	// pr_info(" 0x%08lx remapped to 0x%08lx\n",
	//    proc_base[minor], (long unsigned int)can_iobase[minor]);


	/* pr_info("MAX_IRQNUMBER %d, use IRQ %d\n", MAX_IRQNUMBER, IRQ[minor]); */

	if (IRQ[minor] > 0 && IRQ[minor] < MAX_IRQNUMBER) {
		if (can_requestirq(minor, IRQ[minor], can_interrupt)) {
			pr_err("Can[%d]: Can't request IRQ %d\n",
					minor, IRQ[minor]);
			DBGOUT();
			return -EBUSY;
		}
	} else {
		/* Invalid IRQ number in /proc/.../IRQ */
		DBGOUT();
		return -EINVAL;
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
static int already_called;
int ret;
int minor = -1;
void __iomem *gpio_base = ioremap(PIO_BASE, 0x400);
void __iomem * port_h_config = gpio_base + PH_CFG2_OFFSET;

uint32_t tmp;

	DBGIN();
	ret = 0;
	
	/* only one CAN Channel, no need for index n */
	proc_base[0] = CAN0_BASE_ADDR;
	IRQ[0] = CAN0_INT0;


	


	/* ioremap gpio config adresses
http://forum.lemaker.org/viewthread.php?tid=230&pid=12212&page=1&extra=#pid12212
	 * */

/* 

volatile unsigned long GPIO_BASE;
volatile unsigned long *PI_CFG;
volatile unsigned long *PI_DATA;



GPIO_BASE=(unsigned long)ioremap(PIO_BASE,0x400);
PI_CFG = (unsigned long *)(GPIO_BASE + PI_CFG2_OFFSET);
PI_DATA= (unsigned long *)(GPIO_BASE + PI_DATA_OFFSET);
writel(0x00000000,PI_CFG);//set port i as input

*/


/* PH20 select			Connector3 Pin
 *  bit 18:16  100 CAN_TX	16
 *  bit 19 -
 * PH21 select
 *  bit 22:20  100 CAN_RX	18
 *  */

	msleep(100);
	// pr_info("gpio base %p, %p\n", gpio_base, port_h_config); 
	msleep(100);
	tmp = readl(port_h_config);
	// pr_info(" PIOH 0x%08x\n", tmp); /* PIOH 0x10110000 */
	msleep(100);
	tmp &= 0xFF00FFFF;
	writel(tmp | (4 << 16) | (4 << 20), port_h_config);
	msleep(100);
	// pr_info(" PIOH 0x%08x\n", readl(port_h_config));
	msleep(100);

	/* enable clock
	 * Manual 1.5.4.21  APB1 Module Clock Gating offset 0x6c
	 * Bit 4 CAN_ABP_GATING 
	 *
	 * from 24 Mhz Quartz?
	 * two dividers CLK_RAT_N    /1, /2, /4, /8
	 *              CLK_RAT_M    1/1...32
	 */

	/* FIXME ioremap adress */
        writel(readl(0xF1C20000 + 0x6C) | (1 << 4), 0xF1C20000 + 0x6C);

	if (!already_called && virtual == 0) {
		/* make some sysctl entries read only
		 * IRQ number
		 * Base address
		 * and access mode
		 * are fixed and provided by the PCI BIOS
		 */
		can_sysctl_table[CAN_SYSCTL_IRQ - 1].mode = 0444;
		can_sysctl_table[CAN_SYSCTL_BASE - 1].mode = 0444;
/*
arch/arm/mach-sun7i/include/mach/clock.h
139:#define CLK_MOD_CAN             "can"
*/
		proc_clock = clk_get_rate(clk_get(NULL, CLK_MOD_CAN));
		already_called = 1;
	}
	DBGOUT();
	return ret;
}

void exit_board_hw(void)
{
}

/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.

*/

void board_clear_interrupts(int minor)
{
	DBGIN();
	;
	DBGOUT();
}
