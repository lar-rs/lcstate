/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/

/* using memory acces with readb(), writeb() */


		/* printk("at %p, 0x%x\n", &((canregs_t *)can_iobase[bd])->adr, v); \  */

/* Memory byte access */
#define CAN_OUT(bd, adr, v)	\
		(writeb(v, (void __iomem *)&((canregs_t *)can_iobase[bd])->adr))
/* Memory long word access */
#define CAN_OUTL(bd, adr, v)	\
		(writel(v, (void __iomem *)&((canregs_t *)can_iobase[bd])->adr))
#define CAN_SETL(bd, adr, m)	\
	writel((readl((void __iomem *)&((canregs_t *)can_iobase[bd])->adr)) \
		| (m), (void __iomem *)&((canregs_t *)can_iobase[bd])->adr)
#define CAN_RESETL(bd, adr, m)	\
	writel((readl((void __iomem *)&((canregs_t *)can_iobase[bd])->adr)) \
		& ~(m), (void __iomem *)&((canregs_t *)can_iobase[bd])->adr)
#define CAN_INL(bd, adr)		\
		(readl((void __iomem *)&((canregs_t *)can_iobase[bd])->adr))
#define CAN_TESTL(bd, adr, m)	\
	(readl((void __iomem *)&((canregs_t *)can_iobase[bd])->adr) & (m))

