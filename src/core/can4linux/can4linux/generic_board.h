/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/

/* using memory acces with readb(), writeb() */
/* #error  memory I/O access */



/* This is only an example for defining the access macros.
 * The generic driver does not use hardware access
 * but can be used as template for a real driver
 */



#ifdef IODEBUG
#  define CAN_OUT(bd, adr, v)	\
	(pr_info("Cout: (%x)=%x\n", (u32)&((canregs_t *)can_iobase[bd])->adr, v), \
		writeb(v, (u32) &((canregs_t *)can_iobase[bd])->adr))

#define CAN_SET(bd, adr, m)     do	{\
	unsigned char v;	\
	v = (readb((void __iomem *) &((canregs_t *)can_iobase[bd])->adr)); \
	pr_info("CAN_SET %x |= %x\n", (v), (m)); \
	writeb(v | (m), (u32) &((canregs_t *)can_iobase[bd])->adr); \
	} while (0)

#define CAN_RESET(bd, adr, m)	do {\
	unsigned char v; \
	v = (readb((u32) &((canregs_t *)can_iobase[bd])->adr)); \
	pr_info("CAN_RESET %x &= ~%x\n", (v), (m)); \
	writeb(v & ~(m), (u32) &((canregs_t *)can_iobase[bd])->adr); \
	} while (0)

#define CAN_OUTL(bd, adr, v)	\
	(pr_info("Cout: (%x)=%lx\n", (u32)&((canregs_t *)can_iobase[bd])->adr, v),\
		(writel(v, (u32) &((canregs_t *)can_iobase[bd])->adr)))
#else
   /* Memory Byte access */
#define CAN_OUT(bd, adr, v)	\
		(writeb(v, (void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr))
#define CAN_SET(bd, adr, m)	\
	writeb((readb((void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)) \
		| (m), (void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)
#define CAN_RESET(bd, adr, m)	\
	writeb((readb((void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)) \
		& ~(m), (void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)
#endif  /* IODEBUG */

#define CAN_IN(bd, adr)		\
		(readb((void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr))
#define CAN_TEST(bd, adr, m)	\
	(readb((void __iomem *) &((canregs_t *)can_iobase[bd])->adr) & (m))

   /* Memory word access */
#define CAN_OUTW(bd, adr, v)	\
		(writew((v), (u32) &((canregs_t *)can_iobase[bd])->adr))


#define CAN_OUTWD(bd, adr, v)	\
	(pr_info("Cout: (%x)=%x\n", (u32)&((canregs_t *)can_iobase[bd])->adr, v), \
		writew((v), (u32) &((canregs_t *)can_iobase[bd])->adr))


#define CANsetw(bd, adr, m)	\
	writew((readw((u32) &((canregs_t *)can_iobase[bd])->adr)) \
		| (m) , (u32) &((canregs_t *)can_iobase[bd])->adr)
#define CAN_RESETW(bd, adr, m)	\
	writew((readw((u32) &((canregs_t *)can_iobase[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_iobase[bd])->adr)
#define CAN_INW(bd, adr)		\
		(readw((u32) &((canregs_t *)can_iobase[bd])->adr))
#define CAN_INWD(bd, adr)		\
	(pr_info("Cin: (%x)\n", (u32)&((canregs_t *)can_iobase[bd])->adr), \
		readw((u32) &((canregs_t *)can_iobase[bd])->adr))
#define CAN_TESTW(bd, adr, m)	\
	(readw((u32) &((canregs_t *)can_iobase[bd])->adr) & (m))


   /* Memory long word access */
#define CAN_OUTL(bd, adr, v)	\
		(writel(v, (u32) &((canregs_t *)can_iobase[bd])->adr))
#define CAN_SETL(bd, adr, m)	\
	writel((readl((u32) &((canregs_t *)can_iobase[bd])->adr)) \
		| (m), (u32) &((canregs_t *)can_iobase[bd])->adr)
#define CAN_RESETL(bd, adr, m)	\
	writel((readl((u32) &((canregs_t *)can_iobase[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_iobase[bd])->adr)
#define CAN_INL(bd, adr)		\
		(readl((u32) &((canregs_t *)can_iobase[bd])->adr))
#define CAN_TESTL(bd, adr, m)	\
	(readl((u32) &((canregs_t *)can_iobase[bd])->adr) & (m))


