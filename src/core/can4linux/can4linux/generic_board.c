/* generic board handling
 *
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include <linux/pci.h>




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
	/* pr_info(" Free IRQ %d  minor %d\n", irq, minor);  */
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

	/* PCI scan for CPC-PCI (or others ) has already remapped the address */
	/* pr_info(" assign address direct\n"); */
	can_iobase[minor] = (void __iomem *)proc_base[minor];

	/* The Interrupt Line is already requested by th PC CARD Services
	* (in case of CPC-Card: cpc-card_cs.c)
	*/

	/* pr_info("MAX_IRQNUMBER %d/IRQ %d\n", MAX_IRQNUMBER, IRQ[minor]); */
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


void board_clear_interrupts(int minor)
{
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

	DBGIN();
	ret = 0;
	if (!already_called && virtual == 0) {
		/* make some sysctl entries read only
		 * IRQ number
		 * Base address
		 * and access mode
		 * are fixed and provided by the PCI BIOS
		 */
		can_sysctl_table[CAN_SYSCTL_IRQ - 1].mode = 0444;
		can_sysctl_table[CAN_SYSCTL_BASE - 1].mode = 0444;
		already_called = 1;
	}
	DBGOUT();
	return ret;
}

void exit_board_hw(void)
{
}
