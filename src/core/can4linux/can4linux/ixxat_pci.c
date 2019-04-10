/* Ixxat PCIe IB500 board handling
 *
 * CAN-IB500/PCIe - passive CAN FD board with one CAN channel
 *
 * It will not work with
 * CAN-IB600/PCIe - active CAN FD board with 1/2 CAN channels
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "defs.h"
#include <linux/pci.h>
#include "ixxat_pci.h"
#include "ifi_canfd.h"

MODULE_SUPPORTED_DEVICE("IXXAT Automation GmbH CAN-IB5X0 interface");

/* used for storing the global pci register address */
/* one element more than needed for marking the end */
//struct	pci_dev *can_pcidev[MAX_CHANNELS + 1] = { 0 };

void __iomem *irq_ptr;				/* ptr to PITA control */

/* IXXAT specific PCIe board registers */
#define PCIE_ALTERA_LCR_INTCSR              0x0040  /* Interrupt clear and status register */
#define PCIE_ALTERALCR_A2P_INTENA           0x0050  /* Interrupt enable register */

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

	err = request_irq(irq, handler, IRQF_SHARED | IRQF_NO_THREAD, "can4linux", &can_minors[minor]);


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
	//
//	can_iobase[minor] = (void __iomem *)proc_base[minor];

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
		pr_info("Invalid IRQ %d number in /proc/.../IRQ\n", IRQ[minor]);
		DBGOUT();
		return -EINVAL;
	}



	board_clear_interrupts(minor);

#if 0
pr_info("Are they different?? can_pcidev[%d] = %p, irq_ptr = %p\n",
		minor, can_pcidev[minor], irq_ptr);
#endif

//	pci_write_config_dword(can_pcidev[minor], PCIE_ALTERALCR_A2P_INTENA, 0x00000080);
	writel(0x80, irq_ptr + PCIE_ALTERALCR_A2P_INTENA);


	DBGOUT();
	return 0;
}


/* ixxat has three interrupt helper functions 
   unsigned int IntGetStat(struct device_data_t * pDeviceData);
   bool         IntEnaReq(struct device_data_t * pDeviceData, bool fEnable);
   void         IntClrReq(struct device_data_t * pDeviceData);
*/


/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.

void IntClrReq_PCIE_ALTERA(struct device_data_t * pDeviceData)
*/
void board_clear_interrupts(int minor)
{
u32 irq_status;

	DBGIN();
	irq_status = readl(irq_ptr + PCIE_ALTERA_LCR_INTCSR);
	writel(irq_status, irq_ptr + PCIE_ALTERA_LCR_INTCSR);
	DBGOUT();
}


u32 board_get_irq_status(int minor)
{
u32 irq_status;
    irq_status = readl(irq_ptr + PCIE_ALTERA_LCR_INTCSR);
    irq_status &= 0xFF0080;
    /* if (irq_status != 0) pr_info("PCI IRQ status 0x%08x\n", irq_status); */
    return (irq_status);
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
		if (pcimod_scan()) {
			pr_err("no valid PCI CAN found");
			ret = -EIO;
		} else
			pr_info("  pci scan success");

		can_sysctl_table[CAN_SYSCTL_BASE - 1].mode = 0444;
		already_called = 1;
#if 1
		{
		canmsg_t msg;
		pr_info(
		"can4linux is using a frame structure size of %d bytes\n",
			(int)sizeof(canmsg_t));
		pr_info(" - size of struct timeval is %d bytes\n", (int)sizeof(struct timeval));
		pr_info(" - size of length is %d bytes\n", (int)sizeof(msg.length));
		pr_info(" - CAN FD is in %sISO mode\n", noniso ? "non-" : ""); 
		}
#endif
	}
	DBGOUT();
	return ret;
}

void exit_board_hw(void)
{
    /* for .... */
	CAN_OUTL(0, stcmd, 0xDEADCAFD);   /* Hard reset */
	CAN_OUTL(1, stcmd, 0xDEADCAFD);   /* Hard reset */

	/* FIXME: and disable PCI interrupts */
}


#define IXXAT_VENDOR_ID			0x1BEE 
#define CAN_IB100_DEVICE_ID		0x0002
#define CAN_IB500_DEVICE_ID		0x000E
int ifi_canfd_wr_filter(int minor, u32 filternr, u32 mask, u32 filter);
void ifi_canfd_dump_filter(int n);


/* Show IP core parameters
 * Reads out the two compiler parameter registers and uses pr_info to display
 * it on syslog
 */
void ifi_canfd_parshow(int minor)
{
u32 tmp;
u32 version;

    // pr_info("Read %d: 0x58 %x\n", minor, CAN_INL(minor, ip));
    tmp = CAN_INL(minor, par);
    pr_info("CAN%d: Compiler parameter %x\n", minor + 1, tmp);
    pr_info("CAN%d:  RX Fifo size %d KiB, Tx Fifo size %d KiB\n",
	    minor + 1, ((tmp & 0xff00 >> 8)), (tmp & 0xff));
    pr_info("CAN%d:  ID format %d\n", minor + 1, (tmp & IFI_CANFD_PAR_CANID_MSK) >> IFI_CANFD_PAR_CANID_OFST);
    pr_info("CAN%d:  use %s clock\n", minor + 1, (tmp & IFI_CANFD_PAR_SCLK_MSK) ? "single" : "dual");
    pr_info("CAN%d: CAN clock %d\n", minor + 1, CAN_INL(minor, canclock));
    pr_info("CAN%d: SYS clock %d\n", minor + 1, CAN_INL(minor, sysclock));
    pr_info("CAN%d: IP  %08x\n", minor + 1, CAN_INL(minor, ip));
    version = CAN_INL(minor, ver);
    pr_info("Date %d/%d, min. Quartus Version %d, Core Revision %d\n",
		    ((version >> 24) & 0x0f),
		    ((version >> 16) & 0xff), 
		    ((version >>  8) & 0xff), 
		    ((version)       & 0xff));
}

int pcimod_scan(void)
{
struct	pci_dev *pdev;
int	candev;				/* number of devices found */
void __iomem /* *ptr, */ *ptr2;		
int ret;
u32 tmp;

	ret = -1;
	candev = 0;
	pdev = NULL;
	while ((pdev =
		pci_get_device(IXXAT_VENDOR_ID, CAN_IB500_DEVICE_ID, pdev))) {

		pr_info("  found IXXAT-PCI: %s\n", pci_pretty_name(pdev));
		pr_info("               : %s\n", pci_name(pdev));
		if (pci_enable_device(pdev))
			continue;
		pr_info("        using IRQ %d\n", pdev->irq);

#define PCI_BASE_ADDRESS0(dev) (dev->resource[0].start)
#define PCI_BASE_ADDRESS1(dev) (dev->resource[1].start)
#define PCI_BASE_ADDRESS2(dev) (dev->resource[2].start)
#define PCI_BASE_ADDRESS3(dev) (dev->resource[3].start)


#if 1
	pr_info("Address PCI_BASE_ADDRESS0(pdev) = %p\n", (void *)PCI_BASE_ADDRESS0(pdev));
	pr_info("Address PCI_BASE_ADDRESS1(pdev) = %p\n", (void *)PCI_BASE_ADDRESS1(pdev));
	pr_info("Address PCI_BASE_ADDRESS2(pdev) = %p\n", (void *)PCI_BASE_ADDRESS2(pdev));
	pr_info("Address PCI_BASE_ADDRESS3(pdev) = %p\n", (void *)PCI_BASE_ADDRESS3(pdev));
#endif 

irq_ptr =  ioremap(PCI_BASE_ADDRESS0(pdev), 1024*16);
//ptr2 = ioremap(PCI_BASE_ADDRESS2(pdev), 1024*16);


/* pci_iomap checks if the area is Cachable or not. */
ptr2 = pci_iomap(pdev, 2, 1024*16);



/* Read out board information, located at board base address */
#define CARD_NAME_ADDRESS		0
#define CARD_HW_VERSION_ADDRESS		0x10
#define CARD_FPGA_VERSION_ADDRESS	0x20

pr_info("Cardname: %s\n", (char __iomem *)(ptr2 + CARD_NAME_ADDRESS)); 
pr_info("Hardware version: %d\n", readl((void __iomem *)(ptr2 + CARD_HW_VERSION_ADDRESS))); 
pr_info("FPGA version:     %d\n", readl((void __iomem *)(ptr2 + CARD_FPGA_VERSION_ADDRESS))); 


#define NCAN 2
#define CAN_START  0x2000
#define CAN_OFFSET 0x1000

proc_base[0] = PCI_BASE_ADDRESS2(pdev) + CAN_START;
proc_base[1] = proc_base[0] + CAN_OFFSET;

can_iobase[0] = ptr2 + CAN_START;
can_iobase[1] = ptr2 + CAN_START + CAN_OFFSET;
IRQ[0] = pdev->irq;
IRQ[1] = pdev->irq;
//can_pcidev[0] = pdev;
////can_pcidev[1] = pdev;

	{
	int minor;
		for (minor = 0; minor < NCAN; minor++) {
			ifi_canfd_parshow(minor);
			can_showstat(minor); 	    	
		}
	}


#if 0
	/* for debugging purposes dump register content of memory area */

	{
	int i;
	    pr_info("Address %p\n", ptr2 + CAN_START);
	    for (i = 0; i < 256; i += 4) { 
		if ((i % 16) == 0) printk("\n%04x: ", i);  
		printk(" 0x%08x", readl((void __iomem *)(ptr2 + i +0x2000)));
	    }
	}
	{
	int i;
	    pr_info("Address %p\n", ptr2 + CAN_START + CAN_OFFSET);
	    for (i = 0; i < 256; i += 4) { 
		if ((i % 16) == 0) printk("\n%04x: ", i);  
		printk(" 0x%08x", readl((void __iomem *)(ptr2 + i +0x3000)));
	    }
	}
#endif


	{
	int i;
	for (i = 0; i < 256; i++)
		ifi_canfd_wr_filter(0, i, CANFD_FILTER_enaALL, CANFD_FILTER_enaALL);
	}

	/* set CAN clock value in /proc */
	tmp = CAN_INL(0, par);
	proc_clock = (tmp & IFI_CANFD_PAR_SCLK_MSK)
		? CAN_INL(0, sysclock) : CAN_INL(0, canclock);

 	ret = 0; /* flag that at least one CAN was found */		
	}   /* while( get device ) */
    /* ----------------------------------------------------------------*/

	return ret;
}
/*
IP-core ID 0x0058 r 0xD073CAFD for IFI CAN_FD IP-core

https://github.com/billfarrow/pcimem


03:00.0 Unassigned class [ff00]: Device 1bee:0002 (rev 01)



*/
