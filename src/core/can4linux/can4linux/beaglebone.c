/* generic board handling
 *
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include <linux/of.h>

#include <linux/platform_device.h>


# define DRV_NAME               "D_CAN"

static int /* __devinit */ flexcan_probe(struct platform_device *pdev)
{
int minor = -1;			/* only for DBGIN(); DBGOUT(); */
	DBGIN();
	DBGOUT();
	return 0;
}

static int /* __devexit */ flexcan_remove(struct platform_device *pdev)
{
int devid = pdev->id;
int minor = -1;			/* only for DBGIN(); DBGOUT(); */

	(void)devid;

	DBGIN();
	DBGOUT();
	return 0;
}


static struct of_device_id flexcan_of_match[] = {
	{
		.compatible = "fsl,p1010-flexcan",
	},
	{},
};


static struct platform_driver flexcan_driver = {
	.driver = {
		.name  = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = flexcan_of_match,
	},
	.probe = flexcan_probe,
	.remove = /* __devexit_p ( */ flexcan_remove /* ) */,
};

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

/* Called from __init,  once when driver is loaded
   set up physical adresses, irq number
   and initalize clock source for the CAN module

   take care it will be called only once
*/
void init_board_hw(int n)
{
static int already_called;
int ret;
int minor = -1;

	DBGIN();
	ret = 0;

	if (!already_called) {
/* ------------------------------------- */
	    print_device_tree_node(of_find_node_by_path("/"), 0);
	    {
	    // char *path = "/amba@0/ps7-can@e0008000";
	    char *path = "/pinmux";
	    /* char *path = "/amba@0/ps7_can_0"; */
	    struct device_node *dt_node;
	    const u32 *property;
	    int len;

	    dt_node = of_find_node_by_path(path);
	    if (!dt_node) {
		    pr_err("(E) Failed to find device-tree node: %s\n", path);
		    // return /* -ENODEV */;
	    } else {

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
	    }
/* ------------------------------------- */

		pr_info("register \"%s\" can4linux driver\n", DRV_NAME);
		ret = platform_driver_register(&flexcan_driver);
		already_called = 1;
	}

#if CONFIG_TIME_MEASURE
	init_measure();
#endif
	DBGOUT();
}

void exit_board_hw(void)
{
	platform_driver_unregister(&flexcan_driver);
}









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
 * Mainly it gets needed IO and IRQ ressources and initilaizes
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

	/* The Interrupt Line is alrady requestes by th PC CARD Services
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
