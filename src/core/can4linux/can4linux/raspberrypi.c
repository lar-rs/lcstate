/* RaspberryPi board handling
*
* mainly
*  - setting IRQ handling
*  - connecting/disconnecting the MCP2515 to the SPI controller
*
*/

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"

#include <linux/delay.h>

#include <linux/gpio.h>

#define		IRQREGDEVNAME	"can4linux SPI"
#define		GPIO_CAN_INT	25	/* used GPIO */

int can_requestirq(int minor, int irq, irqreturn_t (*handler)(int, void *))
{
int err = 0;

	DBGIN();
	/*
	int request_irq(unsigned int irq,		// interrupt number
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

	From include/linux/irq.h
	IRQ line status.
	IRQ types
	IRQ_TYPE_NONE		Default, unspecified type
	IRQ_TYPE_EDGE_RISING	Edge rising type
	IRQ_TYPE_EDGE_FALLING	Edge falling type
	IRQ_TYPE_EDGE_BOTH (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
	IRQ_TYPE_LEVEL_HIGH	Level high type
	IRQ_TYPE_LEVEL_LOW	Level low type
	IRQ_TYPE_SENSE_MASK	Mask of the above
	IRQ_TYPE_PROBE		Probing in progress
	From include/linux/irq.h
	IRQF_DISABLED - keep irqs disabled when calling the action handler
	*/


	disable_irq(irq);
	/* Interrupt Line */

	err = request_irq(irq, handler,
		IRQF_ONESHOT
		| IRQF_TRIGGER_FALLING,
		IRQREGDEVNAME, &can_minors[minor]);

	if (!err) {
		DBGPRINT(DBG_BRANCH, ("Requested IRQ: %d @ 0x%lx",
			irq, (unsigned long)handler));
			irq_requested[minor] = 1;
	}
	DBGOUT(); return err;
}

int can_freeirq(int minor, int irq)
{
	DBGIN();
	irq_requested[minor] = 0;
	/* pr_info(" Free IRQ %d  minor %d\n", irq, minor); */
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


	/* SPI specific Tasks:
	 * register the ISR
	 * enable the PIO pins where the CAN is connected to
	 *
	 * CAN interrupts are later enabled in
	 * can_start_chip(minor)
	 * according to can4linux rules
	 */
	if (IRQ[minor] > 0 && IRQ[minor] < MAX_IRQNUMBER) {
		if (can_requestirq(minor, IRQ[minor], can_interrupt)) {
			pr_err("Can[%d]: Can't request IRQ %d\n",
				minor, IRQ[minor]);
			DBGOUT(); return -EBUSY;
		}
	} else {
		/* Invalid IRQ number in /proc/.../IRQ */
		DBGOUT(); return -EBUSY;
	}

	listenmode = 0; /* set NORMAL mode when opening the driver */

	DBGOUT(); return 0;
}


/* Called from __init,  once when driver is loaded
   set up physical addresses, irq number
   and initialize clock source for the CAN module

   Take care it will be called only once
   because it is called for every CAN channel out of MAX_CHANNELS
*/
int init_board_hw(int n)
{
int err;
int minor = -1;
static int already_called;

	DBGIN();
	err = 0;

	/* base address not used, SPI, but has to be != 0 */
	proc_base[n] = 0xFFFFFFFF;
	IRQ[n] = gpio_to_irq(GPIO_CAN_INT);
	pr_info("Using GPIO %d for IRQ[%d] %d\n",
		GPIO_CAN_INT, n, IRQ[n]);

	/* SPI driver is calling the probe() function */
	pr_info(" %s() register CAN SPI driver\n", __func__);
	err = spi_register_driver(&mcp251x_can_driver);
	if (err)
		pr_info("      returned %d\n", err);

	if (!already_called && virtual == 0) {
		/* make some sysctl entries read only
		 * IRQ number
		 * Base address
		 * and access mode
		 * are fixed and provided by the PCI BIOS
		 */
		already_called = 1;
	}
	DBGOUT();
	return err;
}

void exit_board_hw(void)
{
struct mcp251x_priv *priv = &realone;
int minor = -1;

	DBGIN();
	/* extern struct mcp251x_priv realone; */
	pr_info("%s(): calling unregister spi\n", __func__);
	spi_unregister_driver(&mcp251x_can_driver);
	destroy_workqueue(priv->wq);
	DBGOUT();
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
