/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/

/* using memory acces with readb(), writeb() */

/* Memory Byte access */
#define CAN_OUT(bd, adr, v)	\
		(writeb(v, (void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr))
#define CAN_SET(bd, adr, m)	\
	writeb((readb((void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)) \
		| (m), (void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)
#define CAN_RESET(bd, adr, m)	\
	writeb((readb((void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)) \
		& ~(m), (void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr)

#define CAN_IN(bd, adr)		\
		(readb((void __iomem *) &((canregs_t __iomem *)can_iobase[bd])->adr))
#define CAN_TEST(bd, adr, m)	\
	(readb((void __iomem *) &((canregs_t *)can_iobase[bd])->adr) & (m))


