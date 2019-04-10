/************************************************************************/
/* hardware access functions or macros */
/* for AT-CAN-MINI  passive ISA board using SJA1000 */
/************************************************************************/
#ifdef  CAN_PORT_IO
/* #warning "Using port-IO access macros" */
/* #error Intel port I/O access */
/* using port I/O with inb()/outb() for Intel architectures like 
   AT-CAN-MINI ISA board */
#ifdef IODEBUG
#  define CAN_OUT(bd,adr,v)      \
	(printk("Cout: (%lx)=%x\n", (long)&((canregs_t *)proc_base[bd])->adr, v), \
		outb(v, (long) &((canregs_t *)proc_base[bd])->adr ))
#else
#ifdef CONFIG_X86_64
#  define CAN_OUT(bd,adr,v)      \
	(outb(v, (long) &((canregs_t *)((u64)proc_base[bd]))->adr ))
#endif
#ifdef CONFIG_X86_32
#  define CAN_OUT(bd,adr,v)      \
	(outb(v, (u32) &((canregs_t *)((u32)proc_base[bd]))->adr ))
#endif
#endif  /* IODEBUG */


#ifdef CONFIG_X86_64
#define CAN_IN(bd,adr)           \
	(inb ((long) &((canregs_t *)((u64)proc_base[bd]))->adr  ))
	
#define CAN_SET(bd,adr,m)        \
	outb((inb((long) &((canregs_t *)((u64)proc_base[bd]))->adr)) \
		| m ,(long) &((canregs_t *)((u64)proc_base[bd]))->adr )

#define CAN_RESET(bd,adr,m)      \
	outb((inb((long) &((canregs_t *)((u64)proc_base[bd]))->adr)) \
		& ~m,(long) &((canregs_t *)((u64)proc_base[bd]))->adr )

#define CAN_TEST(bd,adr,m)       \
	(inb((long) &((canregs_t *)((u64)proc_base[bd]))->adr  ) & m )



#endif 	/* CONFIG_X86_64 */

#ifdef CONFIG_X86_32

#define CAN_IN(bd,adr)           \
	(inb ((u32) &((canregs_t *)((u32)proc_base[bd]))->adr  ))
	
#define CAN_SET(bd,adr,m)        \
	outb((inb((u32) &((canregs_t *)((u32)proc_base[bd]))->adr)) \
		| m ,(u32) &((canregs_t *)((u32)proc_base[bd]))->adr )

#define CAN_RESET(bd,adr,m)      \
	outb((inb((u32) &((canregs_t *)((u32)proc_base[bd]))->adr)) \
		& ~m,(u32) &((canregs_t *)((u32)proc_base[bd]))->adr )

#define CAN_TEST(bd,adr,m)       \
	(inb((u32) &((canregs_t *)((u32)proc_base[bd]))->adr  ) & m )


#endif 	/* CONFIG_X86_32 */

#endif 	/* CAN_PORT_IO */



