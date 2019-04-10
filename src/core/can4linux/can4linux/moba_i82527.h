/************************************************************************/
/* hardware access functions or macros */
/* for I82527  passive ISA board using Intel i82527 CAN */
/************************************************************************/
#if defined(CAN_PORT_IO)
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

#elif defined(CAN_INDEXED_PORT_IO)	/* CAN_PORT_IO */
// #warning "Using  Indexed Intel port I/O access"
/* using port I/O with indexed inb()/outb() for Intel architectures like 
   SSV TRM/816 DIL-NET-PC */
   
static inline unsigned Indexed_Inb(u32 base, u32 adr) {
    unsigned val;
    outb(adr, base);
    val = inb(base + 1);
#ifdef IODEBUG
  printk("CAN_IN: base: %x adr: %x, got: %x\n",
  	(u32)base, (u8)adr, (u8)val);
#endif
    return val;
}

#ifdef IODEBUG
#define CAN_OUT(bd,adr,v) {\
        printk("CAN_OUT bd:%x base:%x reg:%x val:%x\n", \
                bd, (u32) proc_base[bd], \
		(u32) &regbase->adr,v); \
        outb((u32) &regbase->adr,(u32) proc_base[bd]);\
        outb(v,((u32) proc_base[bd])+1);\
  }
#else
#if 0
/* Calculate the register address */
        (u32) &regbase->adr
	(u32) &((canregs_t *)((u32)proc_base[bd]))->adr

#endif
#define CAN_OUT(bd,adr,v) {\
        outb((long)&((canregs_t *)((long)proc_base[bd]))->adr, (u32)proc_base[bd]);\
        outb(v,((u32) proc_base[bd])+1);\
}
#endif
#define CAN_IN(bd,adr) \
        Indexed_Inb((u32) proc_base[bd], (long)&((canregs_t *)((long)proc_base[bd]))->adr)
        // Indexed_Inb((u32) proc_base[bd],(u32) &regbase->adr)

#if 0 /*not used macros, not yet ported to beready to use */
#define CAN_SET(bd,adr,m) {\
        unsigned val; \
        val=Indexed_Inb((u32) proc_base[bd],(u32) &regbase->adr);\
        outb((u32) &regbase->adr,(u32) proc_base[bd]);\
        outb(val | m,((u32) proc_base[bd])+1);\
}
#define CAN_RESET(bd,adr,m) {\
        unsigned val; \
        val=Indexed_Inb((u32) proc_base[bd],(u32) &regbase->adr);\
        outb((u32) &regbase->adr,(u32) proc_base[bd]);\
        outb(val & ~m,((u32) proc_base[bd])+1);\
}
#define CAN_TEST(bd,adr,m) \
        (Indexed_Inb((u32) proc_base[bd],(u32) &regbase->adr) & m)
#endif
#endif 	/* CAN_PORT_IO */



