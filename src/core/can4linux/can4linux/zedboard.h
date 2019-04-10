/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/

#if defined(CAN_PORT_IO) || defined(CAN_INDEXED_PORT_IO)
# error "Only memory access allowed, "
#endif

/* Memory long word access */
#define CAN_OUTL(bd, adr, v)	\
	    (writel(v, (void __iomem *) &((canregs_t *)can_iobase[bd])->adr))
#define CAN_SETL(bd, adr, m)	\
	writel((readl((u32) &((canregs_t *)can_iobase[bd])->adr)) \
	    | (m), (u32) &((canregs_t *)can_iobase[bd])->adr)
#define CAN_RESETL(bd, adr, m)	\
	writel((readl((u32) &((canregs_t *)can_iobase[bd])->adr)) \
	    & ~(m), (u32) &((canregs_t *)can_iobase[bd])->adr)
#define CAN_INL(bd, adr)		\
	    (readl((void __iomem *) &((canregs_t *)can_iobase[bd])->adr))
#define CAN_TESTL(bd, adr, m)	\
	(readl((u32) &((canregs_t *)can_iobase[bd])->adr) & (m))
