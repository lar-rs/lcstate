/************************************************************************/
/* hardware access functions or macros					*/
/* for the Contemporary Controls board CANPCI-CO/DN using SJA1000	*/
/************************************************************************/


extern void board_clear_interrupts(int minor);

#ifdef  CAN_PORT_IO

/* using port I/O with inb()/outb() for Intel architectures */

#ifdef IODEBUG
#  define CAN_OUT(bd, adr, v)      \
	(printk("Cout: (%x)=%x\n", (int)&((canregs_t *)proc_base[bd])->adr, v), \
		outb(v, (int)&((canregs_t *)proc_base[bd])->adr))
#else
#  define CAN_OUT(bd, adr, v)      \
	(outb(v, (long)&((canregs_t *)proc_base[bd])->adr))
#endif  /* IODEBUG */


#define CAN_IN(bd, adr)           \
	(inb((long)&((canregs_t *)proc_base[bd])->adr))
#define CAN_SET(bd, adr, m)        \
	outb((inb((long) &((canregs_t *)proc_base[bd])->adr)) \
		| m, (long) &((canregs_t *)proc_base[bd])->adr)
#define CAN_RESET(bd, adr, m)      \
	outb((inb((long)&((canregs_t *)proc_base[bd])->adr)) \
		& ~m, (long)&((canregs_t *)proc_base[bd])->adr)
#define CAN_TEST(bd, adr, m)       \
	(inb((long) &((canregs_t *)proc_base[bd])->adr  ) & m)

#endif 	/* CAN_PORT_IO */
