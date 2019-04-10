
/* Kvaser PCICan-4HS specific stuff
 * 
 * (c) 2006-2010 oe@port.de
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/pci.h>
#include "defs.h"

# ifndef CONFIG_PCI
#   error "trying to compile a PCI driver for a kernel without CONFIG_PCI"
# endif


/* used for storing the global pci register address */
/* one element more than needed for marking the end */
struct	pci_dev *can_pcidev[MAX_CHANNELS + 1] = { NULL };


/* PCI Bridge AMCC 5920 registers */
#define S5920_OMB    0x0C
#define S5920_IMB    0x1C
#define S5920_MBEF   0x34
#define S5920_INTCSR 0x38
#define S5920_RCR    0x3C
#define S5920_PTCR   0x60

#define INTCSR_ADDON_INTENABLE_M        0x2000
#define INTCSR_INTERRUPT_ASSERTED_M     0x800000


/* Called from __init,  once when driver is loaded
   set up physical addresses, irq number
   and initialize clock source for the CAN module

   take care it will be called only once
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
		/* printk(KERN_INFO "CAN pci test loaded\n"); */
		/* proc_bgmask = 0; */
		if (pcimod_scan()) {
			pr_err("  no valid PCI CAN found");
			ret = -EIO;
		} else
		    pr_info("  pci scan success");
		already_called = 1;
	}
	DBGOUT();
	return ret;
}

void exit_board_hw(void)
{
int i;
void *ptr;

	i = 0;
	ptr = NULL;
	/* The pointer to dev can be used up to four times,
	 * but we have to release the region only once */
	while (can_pcidev[i]) {
		if (ptr != can_pcidev[i]) {

			pr_info("Can[-1]: - : Kvaser: release PCI resources\n");
			/* disable PCI board interrupts */
			disable_pci_interrupt(pci_resource_start
					      (can_pcidev[i], 0));
			/* pr_devel("release Kvaser CAN region 2 (XILINX)\n");*/
			pci_release_region(can_pcidev[i], 2);/*release xilinx */
			/* pr_devel("release Kvaser CAN region 1 (CAN)\n"); */
			pci_release_region(can_pcidev[i], 1);	/*release i/o */
			/* pr_devel("release Kvaser CAN region 0 (PCI)\n"); */
			pci_release_region(can_pcidev[i], 0);	/*release pci */

		}
		ptr = can_pcidev[i];
		i++;
	}

}

inline void disable_pci_interrupt(unsigned int base)
{
unsigned long tmp;

    /* pr_info("disable pci int add 0x%x, 0x%x", base, base + S5920_INTCSR); */

    /* Disable PCI interrupts from card */
    tmp = inl(base + S5920_INTCSR);
    tmp &= ~INTCSR_ADDON_INTENABLE_M;
    outl(tmp, base + S5920_INTCSR);
}

inline void enable_pci_interrupt(unsigned int base)
{
unsigned long tmp;

    /* Enable PCI interrupts from card */
    tmp = inl(base + S5920_INTCSR);
    tmp |= INTCSR_ADDON_INTENABLE_M;
    outl(tmp, base + S5920_INTCSR);
}






/* reset all CAN controllers on the Kvaser-PCI Board */
void reset_kvaser_pci(unsigned long address)
{
int minor = -1;
    DBGIN();
}

/* check memory region if there is a CAN controller
*  assume the controller was resetted before testing 
*
*  The check for an avaliable controller is difficult !
*  After an Hardware Reset (or power on) the Conroller 
*  is in the so-called 'BasicCAN' mode.
*     we can check for: 
*         adress  name      value
*	    0x00  mode       0x21
*           0x02  status     0xc0
*           0x03  interrupt  0xe0
* Once loaded thr driver switches into 'PeliCAN' mode and things are getting
* difficult, because we now have only a 'soft reset' with not so  unique
* values. The have to be masked before comparing.
*         adress  name       mask   value
*	    0x00  mode               
*           0x01  command    0xff    0x00
*           0x02  status     0x37    0x34
*           0x03  interrupt  0xfb    0x00
*
*/
/* int controller_available(unsigned long address, int offset) */


int controller_available(upointer_t address, int offset)
{
int minor = -1;

    DBGIN();
    pr_info("controller_available 0x%lx, offset %d\n",
    				(unsigned long)address, offset);




    pr_info("0x%0x, ", inb(address ) );
    pr_info("0x%0x, ", inb(address + (2 * offset)) );
    pr_info("0x%0x\n", inb(address + (3 * offset)) );

    /* Try to reset the CAN Controller before reading it.
     * Not really correct, in case it's not a CAN card, anyway.
     */
 //   outb(CAN_RESET_REQUEST, address);

    if ( 0x21 == inb(address))  {
	/* compare rest values of status and interrupt register */
	if(   0x0c == inb(address + 2)
	   && 0xe0 == inb(address + 3) ) {
	    return 1;
	} else {
	    return 0;
	}
    } else {
	/* may be called after a 'soft reset' in 'PeliCAN' mode */
	/*   value     address                     mask    */
	if(   0x00 ==  inb((address + 1))
	   && 0x34 == (inb((address + 2))    & 0x37)
	   && 0x00 == (inb((address + 3))    & 0xfb)
	  ) {
	    return 1;
        } else {
	    return 0;
        }
    }
}


int pcimod_scan(void)
{
struct	pci_dev *pdev = NULL;
int	candev = 0;			/* number of devices found so far */
int	nextcandev;


    for_each_pci_dev(pdev) {
	if(pdev->vendor == PCI_VENDOR_CAN_KVASER
	&& pdev->device == PCI_DEVICE_CAN_KVASER) {
	    pr_info("  found new KVASER pci board %d", candev);
	    pr_info("  found KVASER-PCICAN: %s : %s\n",
	    		pci_pretty_name(pdev), pci_name(pdev));

	    if (pci_enable_device(pdev)) {
		continue;
	    }
	    pr_info("      using IRQ %d\n", pdev->irq);

	    /* this is the pci register range S5920 */
	    if ((pci_resource_flags(pdev, 0)) & IORESOURCE_IO) {
		    pr_info("  resource 0 IO %ld\n", 
			    (long)pci_resource_len(pdev, 0) );
		if(pci_request_region(pdev, 0, "kv_can_s5920") != 0)
		    return -ENODEV;

	    } else if((pci_resource_flags(pdev, 0)) & IORESOURCE_MEM) {
		    pr_info("  resource 0 MEM");
	    }

	    pr_info("  got PCI region\n");

	    /* this is the CAN  I/O register range */
	    if ((pci_resource_flags(pdev, 1)) & IORESOURCE_IO) {
		    pr_info("  resource 1 IO %ld\n", 
			    (long)pci_resource_len(pdev, 1) );
		if(pci_request_region(pdev, 1, "kv_can_sja1000") != 0)
		    goto error_io;

	    } else if((pci_resource_flags(pdev, 1)) & IORESOURCE_MEM) {
		    pr_info("  resource 1 MEM");
	    }

	    pr_info("  got CAN region\n");


	    /* this is the Xilinx register range */
	    if ((pci_resource_flags(pdev, 2)) & IORESOURCE_IO) {
		    pr_info("  resource 2 IO %ld\n", 
			    (long)pci_resource_len(pdev, 2) );
		if(pci_request_region(pdev, 2, "kv_can_xilinx") != 0)
		    goto error_xilinx;

	    } else if((pci_resource_flags(pdev, 2)) & IORESOURCE_MEM) {
		    pr_info("  resource 2 MEM");
	    }

	    pr_info("  got XILINX region\n");

	    /* Assert PTADR#
	     * - we're in passive mode so the other bits are not important */
	    outl(0x80808080L, pci_resource_start(pdev, 0) + S5920_PTCR);

	    pr_info("  PCI resource start 0x%x",
	    			(unsigned)pci_resource_start(pdev, 0));

	    /* Read version info from xilinx chip */
	    pr_info("  Xilinx chip version %d\n",
		    (inb(pci_resource_start(pdev, 2) + 7) >> 4));


	    /* Loop through the io area 1 to see how many CAN controllers */
	    /* are on board (1, 2 or 4)					  */
	    /* be prepared that this happens for each board		  */

	    nextcandev = candev + 4;   /* the PCICan has max four Controllers */
	    for(; candev < nextcandev; candev++) {
	    unsigned long io;
		can_pcidev[candev] = pdev;
		io = pci_resource_start(pdev, 1) + (candev * 0x20);
		if(controller_available(io, 1)) {
		    pr_info("  CAN at pos %d, io address %ld\n", candev + 1, io);
		    if(candev > MAX_CHANNELS) {
			pr_info("  CAN: only %d devices supported\n", MAX_CHANNELS);
			break; /* the devices scan loop */
		    }
		    proc_base[candev] = io;
		    IRQ[candev] = pdev->irq;
		    proc_iomodel[candev] = 'p';

		    /* can_dump(candev); */
		}
		/* its the same dev, the pointer is board global,
		and should be the same for all 4 devices */
		pr_info("  ==> candev %d : pointer %p", candev, can_pcidev[candev]);
	    }

	    /* disable_pci_interrupt(pci_resource_start(pdev, 0)); */

	} /* if KVASER */
    } /* for_each_pci_dev() */
    if (candev == 0)
    	return -ENODEV;
    else
	return 0;

error_xilinx:
	pci_release_region(pdev, 1);   /*release i/o */
error_io:
	pci_release_region(pdev, 0);   /*release pci */
    return -ENODEV;
}




int can_vendor_init(int minor)
{

    DBGIN();
    can_range[minor] = CAN_RANGE;
    
    /* Request the controllers address space
     * Nothing to do for the Kvaser PCICAN, we have io-addresses 
     * can_base in this case stores a (unsigned char *)
     *
     * CAN_PORT_IO only uses proc_base[]
     */

    /* test for valid IRQ number in /proc/sys/.../IRQ */
    if( IRQ[minor] > 0 && IRQ[minor] < MAX_IRQNUMBER ){
        int err;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18) 
	err = request_irq( IRQ[minor], can_interrupt, IRQF_SHARED, 
				"Can", &can_minors[minor]);
#else
	err = request_irq( IRQ[minor], can_interrupt, SA_SHIRQ, 
				"Can", &can_minors[minor]);
#endif

        if( !err ){
	    DBGPRINT(DBG_BRANCH,("Requested IRQ: %d @ 0x%lx",
				    IRQ[minor], (unsigned long)can_interrupt));
	    irq_requested[minor] = 1;
	} else {
	    DBGOUT(); return -EBUSY;
	}
    } else {
	/* Invalid IRQ number in /proc/.../IRQ */
	DBGOUT(); return -EBUSY;
    }

    enable_pci_interrupt(pci_resource_start(can_pcidev[minor], 0));

    DBGOUT(); return 0;
}




void board_clear_interrupts(int minor)
{}

int can_freeirq(int minor, int irq )
{
    DBGIN();
    irq_requested[minor] = 0;
    /* pr_info(" Free IRQ %d  minor %d", irq, minor); */

    /* Disable Interrupt on the PCI board only if all channels
     * are not in use */
    if(    irq_requested[0] == 0
        && irq_requested[1] == 0 
        && irq_requested[2] == 0 
        && irq_requested[3] == 0 )
    /* and what happens if we only have 2 channels on the board,
       or we have minor == 4, thats a second board ??) */
    {
	disable_pci_interrupt(pci_resource_start(can_pcidev[minor], 0));
    }
    free_irq(irq, &can_minors[minor]);
    DBGOUT();
    return 0;
}
