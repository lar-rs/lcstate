/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/


#ifdef IODEBUG
#define CAN_OUTL(bd, adr, v)	\
	(pr_info("Cout: (%x)=%x\n", (u32)&((canregs_t *)can_iobase[bd])->adr, v),\
		(writel(v, (u32) &((canregs_t *)can_iobase[bd])->adr)))
#else

   /* Memory long word access */
#define CAN_OUTL(bd, adr, v)	\
		(writel(v, (u32) &((canregs_t *)can_iobase[bd])->adr))
#endif /* IODEBUG */

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


